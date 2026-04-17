---
name: enforce-script-review
description: Review EnforceScript code for DayZ modding compliance and common pitfalls
---

# EnforceScript Code Review

When reviewing `.c` files in this project, check for ALL of the following violations:

## Critical Syntax Violations (Will NOT compile)
1. **Ternary operators** (`? :`) - Replace with if-else
2. **Multiple variable declarations** in one line (`int a, b, c;`)
3. **Variable redeclaration** in nested scopes
4. **`ref` in parameters, return types, or local variables**
5. **Inheritance on modded classes** (`modded class X : Y`)
6. **Function calls broken across multiple lines**

## Runtime Crashes (Segfaults)
1. **Complex expressions in array assignments** - Use intermediate variables
2. **Empty `#ifdef`/`#endif` blocks** - Must contain statements
3. **Using `delete` keyword** - Set to `null` instead
4. **Missing null checks on `g_Game`**
5. **Using `GetGame()` instead of `g_Game`**

## Logic Errors
1. **Using `IsClient()` / `IsServer()`** - Use `IsDedicatedServer()` / `!IsDedicatedServer()`
2. **Missing `override` keyword** on overridden methods
3. **Missing `super` call** at start of overridden methods
4. **Missing `ref` on member variables** holding objects (premature GC)
5. **`1 < int.MIN`** returns TRUE (integer comparison quirk)

## Naming/Convention Violations
1. **Missing `DME_AH` prefix** on classes, enums, RPCs
2. **Missing `m_DME_AH_` prefix** on member variables in modded classes
3. **Wrong script layer** (entities in 3_Game, modules in 4_World, GUI in 4_World)
4. **Spaces instead of tabs** for indentation
5. **Opening braces on new line** instead of same line

## Config Violations
1. **`requiredAddons` contains anything besides `"DZ_Data"`**
2. **`requiredVersion` is not `0.1`** for this new mod
3. **Unvalidated JSON config data** after loading

## Review Procedure
1. Read the file completely
2. Check each rule above systematically
3. Report violations with line numbers
4. Suggest specific fixes for each violation
5. Verify correct script layer placement
