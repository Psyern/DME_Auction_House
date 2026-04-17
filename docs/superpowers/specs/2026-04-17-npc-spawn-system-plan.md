# DME Auction House - NPC Spawn System Plan

## Context

The Auction House mod needs a way for server admins to place NPCs at specific map locations via JSON config, similar to how Expansion handles trader/quest NPCs. Currently `DME_AH_AuctionNPC` and `DME_AH_ActionOpenAuction` exist but NPCs can only be placed manually via admin tools. This plan adds automatic JSON-based NPC spawning at server start.

## JSON Format: `$profile:DME_AH/Config/NPCs.json`

```json
{
    "NPCs": [
        {
            "ID": 1,
            "ClassName": "SurvivorF_Eva",
            "Position": [6575.0, 0.0, 2685.0],
            "Orientation": [135.0, 0.0, 0.0],
            "DisplayName": "Auction House",
            "Loadout": [
                "GorkaEJacket_Autumn",
                "GorkaEPants_Autumn",
                "MilitaryBoots_Black",
                "BaseballCap_CMMG_Black"
            ],
            "Active": true
        }
    ]
}
```

- **ClassName**: Any SurvivorBase variant (SurvivorF_Eva, SurvivorM_Boris, etc.)
- **Position**: [x, y, z] - If y=0, auto-snap to ground height
- **Orientation**: [yaw, pitch, roll] - Yaw=0 is north, 90=east, etc.
- **Loadout**: Flat array of item classnames, auto-routed to correct body slots
- **Active**: false = skip spawning without deleting config entry

## Architecture: Approach F (Registry-Based)

Spawned NPCs are tracked in a static `set<Object>` registry. The action condition checks this registry in addition to the `DME_AH_AuctionNPC` type check. This means:
- Any SurvivorBase variant can be used as NPC (visual variety)
- No config.cpp CfgVehicles entries needed
- Map-placed `DME_AH_AuctionNPC` objects still work (backward compatible)

## Files to Create

### 1. `scripts/3_Game/DME_AH/Config/DME_AH_NPCConfig.c` (NEW)

Two classes following the same pattern as `DME_AH_CategoryConfig.c`:

**`DME_AH_NPCEntry`** (JSON data class, NO prefix on members):
- `int ID`
- `string ClassName`
- `ref array<float> Position` (3 floats)
- `ref array<float> Orientation` (3 floats)
- `string DisplayName`
- `ref array<string> Loadout`
- `bool Active`
- Helper: `vector GetPositionVector()`, `vector GetOrientationVector()`

**`DME_AH_NPCConfig`** (container + loader):
- `ref array<ref DME_AH_NPCEntry> NPCs`
- `void CreateDefaults()` - 1 example NPC (Active=false so nothing spawns unexpectedly)
- `void Save()` / `static ref DME_AH_NPCConfig Load()` - same pattern as CategoryConfig

### 2. `scripts/4_World/DME_AH/Entities/DME_AH_NPCSpawner.c` (NEW)

Static utility class:
- `static ref array<Object> s_SpawnedNPCs` - track spawned entities
- `static ref set<Object> s_AuctionNPCSet` - fast lookup for action condition
- `static void SpawnAll(DME_AH_NPCConfig config)` - loop active entries, call SpawnNPC each
- `static Object SpawnNPC(DME_AH_NPCEntry entry)`:
  - Get position, snap Y to ground if Y==0 via `g_Game.SurfaceY(x, z)`
  - `g_Game.CreateObjectEx(className, position, ECE_PLACE | ECE_CREATEPHYSICS)`
  - Set orientation
  - Apply loadout via `entity.GetInventory().CreateInInventory()` loop
  - Register in `s_AuctionNPCSet`
  - Log success
- `static bool IsAuctionNPC(Object obj)` - check registry
- `static void DespawnAll()` - delete all, clear sets

## Files to Modify

### 3. `scripts/3_Game/DME_AH/Constants/DME_AH_Constants.c`

Add:
```c
const string DME_AH_NPC_FILE = DME_AH_CONFIG_DIR + "NPCs.json";
```

### 4. `scripts/3_Game/DME_AH/Module/DME_AH_Module.c`

- Add member: `protected ref DME_AH_NPCConfig m_NPCConfig;`
- In `Init()`, after category config load: `m_NPCConfig = DME_AH_NPCConfig.Load();`
- Add accessor: `DME_AH_NPCConfig GetNPCConfig()`

### 5. `scripts/4_World/DME_AH/Entities/DME_AH_AuctionNPC.c`

Add invincibility + protection:
```c
override void EEHitBy(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
{
    super.EEHitBy(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);
    SetHealth(GetMaxHealth());
}
```

### 6. `scripts/4_World/DME_AH/Actions/DME_AH_ActionOpenAuction.c`

Add registry check in `ActionCondition()`:
```c
if (DME_AH_NPCSpawner.IsAuctionNPC(targetObject))
    return true;
```

### 7. `scripts/5_Mission/DME_AH/MissionServer.c`

In `OnInit()` after module init:
```c
DME_AH_NPCConfig npcConfig = module.GetNPCConfig();
if (npcConfig)
    DME_AH_NPCSpawner.SpawnAll(npcConfig);
```

In `OnMissionFinish()`:
```c
DME_AH_NPCSpawner.DespawnAll();
```

## Spawn Flow

```
Server Start
  → MissionServer.OnInit()
    → DME_AH_Module.Init()
      → Loads NPCs.json (or creates default)
    → DME_AH_NPCSpawner.SpawnAll(npcConfig)
      → For each active entry:
        → CreateObjectEx(ClassName, position)
        → SetOrientation()
        → Apply loadout items
        → Register in s_AuctionNPCSet
  → Player approaches NPC
    → DME_AH_ActionOpenAuction.ActionCondition()
      → DME_AH_NPCSpawner.IsAuctionNPC(target) → true
    → Player presses F → Menu opens
```

## Implementation Order

1. Add constant `DME_AH_NPC_FILE`
2. Create `DME_AH_NPCConfig.c` (data + loader)
3. Add config loading to `DME_AH_Module.c`
4. Create `DME_AH_NPCSpawner.c` (spawner + registry)
5. Modify `DME_AH_ActionOpenAuction.c` (registry check)
6. Enhance `DME_AH_AuctionNPC.c` (invincibility)
7. Modify `MissionServer.c` (trigger spawn/despawn)

## Verification

1. Server starts without compile errors
2. `$profile:DME_AH/Config/NPCs.json` generated on first start
3. Edit NPCs.json: set Active=true, valid position for your map
4. Restart server - NPC spawns at configured position with loadout
5. Approach NPC - "Open Auction House" action appears
6. Press F - Auction menu opens
7. Set Active=false, restart - NPC does not spawn
