# NPC Interaction: DME Auction House vs Expansion Market

## Vergleichstabelle

| Aspekt | Expansion Market | DME Auction House | Problem? |
|--------|-----------------|-------------------|----------|
| **NPC Basisklasse** | `ExpansionNPCBase` → `ExpansionNotPlayerBase` → `ManBase` | Vanilla `SurvivorF_Eva` → `SurvivorBase` → `PlayerBase` → `ManBase` | **JA** - SurvivorBase wird als Spieler-Entity behandelt, nicht als interagierbares Objekt |
| **NPC hat eigene Klasse?** | Ja: `ExpansionTraderNPCBase` (eigene Klasse mit Trader-Logik) | Nein: Spawnt vanilla Survivor ohne eigene Klasse | **JA** - Keine eigene Klasse = keine eigene SetActions() |
| **Action Klasse** | `ExpansionActionOpenTraderMenu : ActionInteractBase` | `DME_AH_ActionOpenAuction : ActionInteractBase` | Nein |
| **CCT (Condition Target)** | `CCTCursor` | `CCTCursor` | Nein |
| **CCI (Condition Item)** | `CCINone` | `CCINone` | Nein |
| **Action Registration (ActionConstructor)** | `actions.Insert(ExpansionActionOpenTraderMenu)` | `actions.Insert(DME_AH_ActionOpenAuction)` | Nein |
| **Action auf Player (SetActions)** | `AddAction(ExpansionActionOpenTraderMenu, InputActionMap)` in modded `PlayerBase` | `AddAction(DME_AH_ActionOpenAuction, InputActionMap)` in modded `PlayerBase` | Nein |
| **Action auf NPC Entity** | NPC hat eigenes `SetActions()` via `ExpansionNotPlayerBase` (leer, aber System existiert) | `DME_AH_AuctionNPC.SetActions()` existiert, aber gespawnter NPC ist vanilla `SurvivorF_Eva` - hat keine DME Action | **JA** |
| **NPC Erkennung (ActionCondition)** | `ExpansionMarketModule.GetTraderFromObject(target)` → Castet zu `ExpansionTraderNPCBase` | `IsInherited(DME_AH_AuctionNPC)` + `DME_AH_NPCSpawner.IsAuctionNPC()` | **JA** - Vanilla Survivor ist kein DME_AH_AuctionNPC |
| **NPC Erkennung Client-seitig** | Cast zu eigenem Typ funktioniert immer (Klasse ist bekannt) | Server-Registry existiert nicht auf Client, Positions-Check braucht Config die Client nicht hat | **JA** |
| **Wo oeffnet Menu?** | `OnStartServer()` → Server sendet RPC → Client oeffnet Menu | `OnExecuteClient()` → Client oeffnet Menu direkt | Nein (funktional ok) |
| **Menu Opening** | Server schickt `RPC_LoadTraderData` → Client ruft `OpenTraderMenu()` | Client ruft direkt `EnterScriptedMenu()` | Nein |
| **NPC Spawning** | Eigene Klassen (`ExpansionTraderNPCBase`) via Expansion Spawn-System | Vanilla `SurvivorF_Eva` via `g_Game.CreateObject()` | **JA** |
| **NPC Invincibility** | `Expansion_SetAllowDamage(false)` im eigenen Framework | `EEHitBy` Override heilt zurueck auf Max | Nein |
| **NPC Loadout** | `ExpansionHumanLoadout.Apply()` (eigenes System) | `CreateInInventory()` Loop | Nein |

## Kernproblem

**Expansion hat eigene NPC-Klassen**, die von `ManBase` erben (NICHT von `PlayerBase/SurvivorBase`). Diese Klassen:
- Haben ein eigenes Action-System (`ExpansionNotPlayerBase.GetActions()`)
- Koennen per `Class.CastTo()` eindeutig identifiziert werden
- Sind auf Client UND Server als eigener Typ bekannt

**DME_AH spawnt vanilla `SurvivorF_Eva`**, die:
- Erbt von `PlayerBase/SurvivorBase` (wird wie ein Spieler behandelt)
- Hat KEINE `DME_AH_ActionOpenAuction` in ihrem eigenen `SetActions()`
- Kann nicht per Typ-Check als Auction-NPC identifiziert werden
- Der Fallback (Identity-Check) koennte funktionieren, aber die Action-Registration auf der Entity fehlt

## Loesungsoptionen

### Option A: Eigene NPC-Klasse in CfgVehicles (Expansion-Ansatz)
- `DME_AH_AuctionNPC` in `config.cpp` unter `CfgVehicles` registrieren
- In NPCs.json `"ClassName": "DME_AH_AuctionNPC"` statt `"SurvivorF_Eva"`
- NPC hat eigene `SetActions()` mit `AddAction(DME_AH_ActionOpenAuction)`
- `IsInherited(DME_AH_AuctionNPC)` funktioniert auf Client+Server
- **Nachteil:** Nur ein Survivor-Modell, Vielfalt nur ueber Kleidung

### Option B: Action auf ALLEN SurvivorBase registrieren (Workaround)
- In `modded class SurvivorBase` ein `SetActions()` Override
- `AddAction(DME_AH_ActionOpenAuction)` auf JEDEM Survivor
- ActionCondition prueft dann ob es ein NPC ist (Identity-Check)
- **Nachteil:** Action wird fuer JEDEN Survivor geprueft (Performance)

### Option C: Hybrid - Eigene Klasse + CfgVehicles Varianten
- Mehrere Klassen: `DME_AH_NPC_Eva : DME_AH_AuctionNPC`, `DME_AH_NPC_Boris : DME_AH_AuctionNPC`, etc.
- Jede erbt Model von vanilla, aber hat DME_AH Action
- In config.cpp unter CfgVehicles registrieren
- **Nachteil:** Viele Klassen, aber maximale Flexibilitaet

### Empfehlung: Option A
- Einfachste Implementierung
- Garantiert funktionsfaehig (gleicher Ansatz wie Expansion)
- Vielfalt ueber Loadout/Kleidung statt verschiedene Modelle
- Nur 1 Eintrag in CfgVehicles noetig
