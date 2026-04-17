# DME Auction House - DayZ Mod

## Project Info
- **Mod Name:** DME_Auction_House
- **Prefix:** DME_AH (use for all classes, enums, member variables in modded classes, RPCs, settings paths)
- **Author:** Psyern
- **Community:** DeadmansEcho
- **Target:** DayZ 1.29 Experimental (EnforceScript)

## Reference Documentation
All DayZ modding documentation is located at:
`C:\Users\Administrator\Desktop\Mod Repositories\DAYZ_Enforce-Script-main\`

**CRITICAL:** Before implementing ANY feature, consult the relevant documentation:
- **Style Guide:** `Tips/EnScript-Style-Guide.md`
- **Safe Coding:** `Frameworks/Safe-AI-CodingPrompt.md`
- **Mod Structure:** `How-To/Basic-Mod-Structure.md`
- **Script Layers:** `How-To/Script-Layers-Guide.md`
- **RPCs:** `How-To/How-To-RPC.md`
- **UI Menus:** `How-To/How-To-UI-Menus.md`
- **Layouts:** `How-To/How-To-Layout.md`
- **Layout Controls:** `How-To/How-To-Layout-Controls.md`
- **Actions:** `How-To/How-To-Actions.md`
- **Module System:** `How-To/Module-System.md`
- **Profile Settings:** `How-To/How-To-Profile-Settings.md`
- **Config Validation:** `How-To/How-To-Validate-Config-Data.md`
- **Logger:** `How-To/How-To-Logger.md`
- **Enums:** `How-To/How-To-Enums.md`
- **Best Practices:** `Tips/Tips-Best-Practices.md`
- **Common Pitfalls:** `Tips/Tips-Common-Pitfalls.md`
- **DayZ 1.29 API:** `DayZGame/DayZ-1.29.161219.md`

## EnforceScript Rules (MANDATORY - ZERO EXCEPTIONS)

### Syntax Rules
- **NO ternary operators** (`? :`) - Use if-else instead
- **NO multiple variable declarations** in one line (`int a, b, c;` is FORBIDDEN)
- **NO variable redeclaration** in nested scopes
- **NO function calls broken across multiple lines** - Keep on single line or use intermediate variables
- **NO `ref` in parameters, return types, or local variables** - Only on member variables and typedefs
- **NO `delete` keyword** - Set to `null` instead
- **NO `GetGame()`** - ALWAYS use `g_Game` with null check
- **NO `IsClient()` / `IsServer()`** - Use `g_Game.IsDedicatedServer()` and `!g_Game.IsDedicatedServer()`
- **NO optional chaining** (`?.`), null coalescing (`??`), lambdas, generics, `var`, `auto`
- **NO inheritance syntax on modded classes** (`modded class X : Y` is FORBIDDEN)

### Formatting Rules
- **Tabs** for indentation (NEVER spaces)
- **Opening braces** on same line as statement
- **One blank line** between logical sections
- File extension: `.c` for all script files

### Naming Conventions
- **Classes:** `DME_AH_ClassName` (PascalCase with prefix)
- **Methods:** PascalCase (e.g., `GetAuctionList`, `SetPrice`)
- **Member variables:** `m_DME_AH_VarName` (in modded classes, ALWAYS prefix)
- **Member variables (own classes):** `m_VarName` (PascalCase with `m_` prefix)
- **Static variables:** `s_DME_AH_VarName` or `s_VarName`
- **Local variables:** camelCase (e.g., `playerName`, `itemCount`)
- **Constants:** `UPPER_SNAKE_CASE` (e.g., `DME_AH_MAX_LISTINGS`)
- **Enums:** `EDME_AH_EnumName` with values starting at 10000+ for RPCs
- **RPC names:** `DME_AH_RPC_ActionName`
- **Settings path:** `$profile:DME_AH\\`
- **JSON config classes:** NO prefix on members (JSON keys must match)

### Code Patterns
- **ALWAYS** check `g_Game` for null: `if (!g_Game) return;`
- **ALWAYS** use `override` keyword when overriding
- **ALWAYS** call `super` first in overrides
- **ALWAYS** prefix members in modded classes with `DME_AH`
- **ALWAYS** validate config data after loading JSON
- **ALWAYS** use `ref` on member variables holding objects
- **ALWAYS** register custom actions in `ActionConstructor`
- **ALWAYS** register recipes in `PluginRecipesManager`
- **NEVER** add `requiredAddons` except `"DZ_Data"`
- **NEW mod:** `requiredVersion = 0.1;` (NEVER change later)

### Script Layer Rules (CRITICAL - Wrong layer = compilation failure)
| Code Type | Layer | Example |
|-----------|-------|---------|
| Constants, Enums, Logging | 3_Game | `DME_AH_Constants.c` |
| Modules, Managers, Settings | 3_Game | `DME_AH_Module.c` |
| RPC Handlers | 3_Game | `DME_AH_RPCManager.c` |
| `modded class ItemBase/PlayerBase/EntityAI` | 4_World | Entity modifications |
| Custom items/entities | 4_World | `class DME_AH_AuctionItem : ItemBase` |
| Actions | 4_World | `DME_AH_ActionOpenAuction.c` |
| `modded class MissionGameplay/MissionServer` | 5_Mission | Mission modifications |
| GUI/Menu classes (`UIScriptedMenu`) | 5_Mission | `DME_AH_AuctionMenu.c` |

**Golden Rule:** Lower layers (1-3) CANNOT reference classes from higher layers (4-5).

### Memory Management
- `ref` ONLY on member variables and typedefs
- Set to `null` to release, never `delete`
- Use intermediate variables for complex array assignments (avoid segfaults)
- Avoid creating temporary objects in tight loops

### Common Pitfalls to Avoid
- Empty `#ifdef`/`#endif` blocks cause segfaults - always have content
- `1 < int.MIN` returns TRUE in EnScript
- Complex expressions in array assignments cause segfaults
- Compiler errors may point to wrong file - search for the class/variable name
- `GetObjectsAtPosition` is very expensive - use alternatives

## Mod Directory Structure
```
DME_Auction_House/
├── config.cpp
├── stringtable.csv                # EN/DE localization
├── scripts/
│   ├── Common/
│   │   └── DME_AH_Debug.c
│   ├── 3_Game/DME_AH/
│   │   ├── Constants/DME_AH_Constants.c
│   │   ├── Enums/DME_AH_Enums.c
│   │   ├── Config/DME_AH_Config.c
│   │   ├── Config/DME_AH_CategoryConfig.c
│   │   ├── Data/DME_AH_Listing.c
│   │   ├── Data/DME_AH_Bid.c
│   │   ├── Data/DME_AH_Transaction.c
│   │   ├── Data/DME_AH_Category.c
│   │   ├── Module/DME_AH_Module.c       # CF_ModuleWorld - main entry
│   │   ├── Manager/DME_AH_AuctionManager.c
│   │   ├── Manager/DME_AH_DataStore.c
│   │   ├── Currency/DME_AH_CurrencyAdapter.c     # Base class
│   │   ├── Currency/DME_AH_CurrencyExpansion.c   # Expansion Market
│   │   ├── Currency/DME_AH_CurrencyItem.c        # Physical items
│   │   ├── Currency/DME_AH_CurrencyInternal.c    # Points system
│   │   ├── Logger/DME_AH_Logger.c
│   │   └── RPC/DME_AH_RPCHandler.c      # CF RPCs (19 endpoints)
│   ├── 4_World/DME_AH/
│   │   ├── Entities/DME_AH_AuctionTerminal.c
│   │   ├── Entities/DME_AH_AuctionNPC.c
│   │   ├── Actions/DME_AH_ActionOpenAuction.c
│   │   └── Actions/ActionConstructor.c
│   └── 5_Mission/DME_AH/
│       ├── GUI/DME_AH_AuctionMenu.c     # Main menu (4 tabs)
│       ├── GUI/DME_AH_DetailPanel.c
│       ├── GUI/DME_AH_ListingRow.c
│       ├── GUI/DME_AH_CreateListingDialog.c
│       ├── GUI/DME_AH_ConfirmDialog.c
│       ├── GUI/DME_AH_NotificationHandler.c
│       ├── MissionBase.c
│       └── MissionGameplay.c
├── gui/DME_AH/layouts/
│   ├── auction_menu.layout               # Main UI
│   ├── auction_menu_listing_row.layout
│   ├── auction_menu_detail_panel.layout
│   ├── auction_menu_create_listing.layout
│   └── auction_menu_confirm_dialog.layout
└── docs/superpowers/specs/
    └── 2026-04-17-auction-house-gui-design.md
```

## Dependencies
- **Community Framework (CF)** - Required (RPC system, module system)
- **DayZ Expansion Market** - Optional (only if CurrencyType = "Expansion")

## Server Data Files ($profile:DME_AH/)
- `Config/Settings.json` - Server configuration
- `Config/Categories.json` - Category definitions
- `Data/ActiveListings.json` - Active auction listings
- `Data/CompletedListings.json` - Transaction history
- `Data/PlayerData.json` - Internal currency balances + pending pickups

## Layout File Conventions
- Use relative coordinates (0.0-1.0) by default
- Widget naming: `lbl` (label), `edit` (edit box), `btn` (button), `chk` (checkbox), `slider`, `txt` (text), `lst` (listbox)
- Always use `#STR_` keys for localized text
- Fonts: `gui/fonts/metron14`, `gui/fonts/metron16`, `gui/fonts/metron16bold`
- Dark theme colors: background `0.05 0.05 0.05 0.97`, text `0.8 0.8 0.8 1`
- Test at multiple resolutions (1080p, 1440p, ultrawide)
