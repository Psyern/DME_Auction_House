---
description: Check all .c files for EnforceScript compliance violations
---

Scan ALL `.c` files in this project for EnforceScript violations:

1. Use Grep to find:
   - Ternary operators: `? :`
   - Multiple variable declarations: `int a, b` patterns
   - `GetGame()` usage (should be `g_Game`)
   - `IsClient()` or `IsServer()` (should use `IsDedicatedServer()`)
   - `delete ` keyword usage
   - `ref` in function parameters or return types
   - Spaces-only indentation (should be tabs)
   - Multi-line function calls (opening paren followed by newline before closing)

2. Check script layer placement:
   - Verify no `modded class ItemBase/PlayerBase` in 3_Game
   - Verify no modules/managers in 4_World
   - Verify no GUI classes in 4_World

3. Check naming:
   - All classes prefixed with `DME_AH_`
   - Member variables in modded classes prefixed with `m_DME_AH_`
   - Enums prefixed with `EDME_AH_` or `DME_AH_`

Report all violations with file path, line number, and suggested fix.
