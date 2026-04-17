---
description: Verify all script files are in the correct DayZ script layer
---

Check every `.c` file in the project and verify it's in the correct script layer:

## Rules
- **3_Game:** Constants, enums, modules, managers, settings, RPC handlers, loggers, data classes
- **4_World:** modded class ItemBase/PlayerBase/EntityAI, new entities, actions, ActionConstructor, recipes
- **5_Mission:** modded class MissionGameplay/MissionServer/MissionBase, GUI/Menu classes (UIScriptedMenu), menu registration

## Process
1. Glob for all `scripts/**/*.c` files
2. Read each file to determine what classes it defines
3. Check if the containing folder matches the required layer
4. Report misplacements

Also verify that no lower-numbered layer references classes only available in higher layers.
