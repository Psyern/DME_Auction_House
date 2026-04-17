---
name: create-menu
description: Create a new DayZ UI menu with layout file and menu class following all conventions
---

# Create DayZ UI Menu

Creates a complete UI menu following DayZ modding conventions.

## Required Input
- Menu name (e.g., "AuctionMenu", "ListingMenu")
- Menu purpose/description
- Required widgets (buttons, listboxes, edit boxes, etc.)

## Steps

### 1. Define Menu ID Constant
File: `scripts/3_Game/DME_AH/Constants/DME_AH_Constants.c`
```c
const int MENU_DME_AH_MENUNAME = 100XX;  // Start at 10000+, unique per menu
```

### 2. Create Layout File
File: `gui/DME_AH/layouts/menu_name.layout`

Rules:
- Use relative coordinates (0.0-1.0)
- All position flags set to 0 (relative mode)
- Widget naming: `btn` prefix for buttons, `lbl` for labels, `edit` for edit boxes, `lst` for listboxes, `chk` for checkboxes, `txt` for text
- Dark theme: background `color 0.05 0.05 0.05 0.97`, text `color 0.8 0.8 0.8 1`
- Font: `gui/fonts/metron16` standard, `gui/fonts/metron16bold` for titles
- Button style: `style Editor`
- All text via `#STR_` keys

### 3. Create Menu Class
File: `scripts/5_Mission/DME_AH/GUI/DME_AH_MenuName.c`

Required pattern:
```c
class DME_AH_MenuName : UIScriptedMenu
{
	// Member widgets with ref
	protected ButtonWidget m_BtnClose;
	// ... other widgets

	override Widget Init()
	{
		if (!g_Game) return null;
		layoutRoot = g_Game.GetWorkspace().CreateWidgets("DME_AH/layouts/menu_name.layout");
		// FindAnyWidget for each widget, cast to correct type, null check
		return layoutRoot;
	}

	override void OnShow()
	{
		super.OnShow();
		PPEffects.SetBlurMenu(0.5);
		g_Game.GetInput().ChangeGameFocus(1);
		g_Game.GetUIManager().ShowUICursor(true);
		g_Game.GetMission().GetHud().Show(false);
		TIntArray skip = { UAUIBack };
		ForceDisableInputs(true, skip);
	}

	override void OnHide()
	{
		super.OnHide();
		ForceDisableInputs(false);
		PPEffects.SetBlurMenu(0);
		g_Game.GetInput().ResetGameFocus();
		g_Game.GetUIManager().ShowUICursor(false);
		g_Game.GetMission().GetHud().Show(true);
		Close();
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		super.OnClick(w, x, y, button);
		if (w == m_BtnClose) { Close(); return true; }
		return false;
	}
}
```

### 4. Register Menu
File: `scripts/5_Mission/DME_AH/MissionGameplay.c`
```c
modded class MissionBase
{
	override UIScriptedMenu CreateScriptedMenu(int id)
	{
		UIScriptedMenu menu = super.CreateScriptedMenu(id);
		if (menu) return menu;
		switch (id)
		{
			case MENU_DME_AH_MENUNAME:
				menu = new DME_AH_MenuName;
				break;
		}
		return menu;
	}
}
```

### 5. Open Menu (from action or elsewhere)
```c
if (!g_Game || g_Game.IsDedicatedServer()) return;
UIManager uiManager = g_Game.GetUIManager();
if (uiManager && !uiManager.IsMenuOpen(MENU_DME_AH_MENUNAME))
{
	uiManager.EnterScriptedMenu(MENU_DME_AH_MENUNAME, null);
}
```

## Checklist
- [ ] Menu ID constant defined (10000+)
- [ ] Layout file created with relative coordinates
- [ ] Menu class in 5_Mission layer
- [ ] g_Game null checks everywhere
- [ ] Widget null checks after FindAnyWidget
- [ ] Menu registered in MissionBase.CreateScriptedMenu
- [ ] OnShow/OnHide handle focus, cursor, blur, hud
- [ ] All text uses #STR_ keys
- [ ] Tab indentation throughout
- [ ] No function calls broken across lines
