---
name: layer-check
description: Verify that all code is placed in the correct DayZ script layer (3_Game, 4_World, 5_Mission)
---

# Script Layer Verification

Check that ALL files are in the correct script layer. Wrong placement causes compilation failures.

## Layer Assignment Rules

### 3_Game - Game Logic (loads BEFORE 4_World and 5_Mission)
MUST contain:
- Constants and enums (`DME_AH_Constants.c`, `DME_AH_Enums.c`)
- Modules (`DME_AH_Module.c`)
- Managers (`DME_AH_AuctionManager.c`)
- Settings/Config classes (`DME_AH_Config.c`)
- RPC handlers (`DME_AH_RPCHandler.c`)
- Logger (`DME_AH_Logger.c`)
- Data classes (auction listing, bid data structures)

MUST NOT contain:
- `modded class ItemBase/PlayerBase/EntityAI/VehicleBase` (goes to 4_World)
- `modded class MissionGameplay/MissionServer/MissionBase` (goes to 5_Mission)
- GUI/Menu classes (goes to 5_Mission)
- Action classes (goes to 4_World)
- Recipe classes (goes to 4_World)
- New entity classes inheriting from ItemBase etc. (goes to 4_World)

### 4_World - Entity and World Objects
MUST contain:
- `modded class ItemBase`
- `modded class PlayerBase`
- `modded class EntityAI`
- New entity classes (`class DME_AH_AuctionTerminal : HouseNoDestruct`)
- Action classes (`DME_AH_ActionOpenAuction`)
- ActionConstructor registration
- Recipe classes
- Entity-related utilities

MUST NOT contain:
- Modules, managers, settings (goes to 3_Game)
- `modded class MissionGameplay/MissionServer` (goes to 5_Mission)
- GUI classes (goes to 5_Mission)

### 5_Mission - Mission and GUI
MUST contain:
- `modded class MissionGameplay`
- `modded class MissionServer`
- `modded class MissionBase`
- All GUI/Menu classes (`UIScriptedMenu` subclasses)
- Menu registration (CreateScriptedMenu)
- Player input handlers
- Client-side UI controllers

MUST NOT contain:
- Entity modifications (goes to 4_World)
- Modules, managers (goes to 3_Game)
- Settings classes (goes to 3_Game)

## Verification Procedure
1. List all `.c` files in the project
2. For each file, determine what it contains (classes, modded classes, etc.)
3. Verify it's in the correct layer folder
4. Report any misplacements with the correct target layer
5. Check that no lower layer references higher layer classes