---
name: create-action
description: Create a new player action for DayZ (interact, continuous, or single-use)
---

# Create DayZ Action

Creates player actions following DayZ action system conventions.

## Required Input
- Action name (verb + object, e.g., "OpenAuction", "PlaceBid")
- Action type: Continuous, SingleUse, or Interact
- Conditions (what item/target is required)

## Steps

### 1. Create Action Class (4_World layer)
File: `scripts/4_World/DME_AH/Actions/DME_AH_Action[Name].c`

**Interact Action (instant on world object):**
```c
class DME_AH_ActionOpenAuction : ActionInteractBase
{
	void DME_AH_ActionOpenAuction()
	{
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_INTERACTONCE;
		m_Text = "#STR_DME_AH_ACTION_OPEN_AUCTION";
		m_StanceMask = DayZPlayerConstants.STANCEMASK_ERECT | DayZPlayerConstants.STANCEMASK_CROUCH;
	}

	override void CreateConditionComponents()
	{
		m_ConditionItem = new CCINone;
		m_ConditionTarget = new CCTCursor;
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		if (!target) return false;
		Object targetObject = target.GetObject();
		if (!targetObject) return false;
		return targetObject.IsInherited(DME_AH_AuctionTerminal);
	}

	override void OnExecuteClient(ActionData action_data)
	{
		if (!g_Game || g_Game.IsDedicatedServer()) return;
		UIManager uiManager = g_Game.GetUIManager();
		if (uiManager && !uiManager.IsMenuOpen(MENU_DME_AH_AUCTION))
		{
			uiManager.EnterScriptedMenu(MENU_DME_AH_AUCTION, null);
		}
	}
}
```

**Continuous Action (takes time with progress):**
```c
class DME_AH_ActionCustomCB : ActionContinuousBaseCB
{
	override void CreateActionComponent()
	{
		m_ActionData.m_ActionComponent = new CAContinuousTime(5.0);
	}
}

class DME_AH_ActionCustom : ActionContinuousBase
{
	void DME_AH_ActionCustom()
	{
		m_CallbackClass = DME_AH_ActionCustomCB;
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_CUSTOM;
		m_Text = "#STR_DME_AH_ACTION_CUSTOM";
		m_FullBody = false;
		m_StanceMask = DayZPlayerConstants.STANCEMASK_ERECT | DayZPlayerConstants.STANCEMASK_CROUCH;
	}

	override void CreateConditionComponents()
	{
		m_ConditionItem = new CCINonRuined;
		m_ConditionTarget = new CCTNone;
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		return (item && !item.IsRuined());
	}

	override void OnExecuteServer(ActionData action_data)
	{
		// Server-side execution
	}
}
```

### 2. Register in ActionConstructor (CRITICAL)
File: `scripts/4_World/DME_AH/ActionConstructor.c`
```c
modded class ActionConstructor
{
	override void RegisterActions(TTypenameArray actions)
	{
		super.RegisterActions(actions);
		actions.Insert(DME_AH_ActionOpenAuction);
		// Register ALL custom actions here
	}
}
```

### 3. Add Action to Items/Entities
File: `scripts/4_World/DME_AH/Entities/DME_AH_AuctionTerminal.c`
```c
class DME_AH_AuctionTerminal : HouseNoDestruct
{
	override void SetActions()
	{
		super.SetActions();
		AddAction(DME_AH_ActionOpenAuction);
	}
}
```

## Critical Rules
- Action classes go in 4_World layer ONLY
- Action naming: `DME_AH_Action[Verb][Object]`
- Callback naming: `DME_AH_Action[Name]CB`
- MUST register ALL custom actions in ActionConstructor
- Use `AddAction()` in entity's `SetActions()` method
- Check `g_Game` for null in `OnExecuteClient`
- Check `!g_Game.IsDedicatedServer()` for client-only code

## Checklist
- [ ] Action class in 4_World layer
- [ ] Registered in ActionConstructor
- [ ] Added to entity via SetActions/AddAction
- [ ] Proper condition components set
- [ ] g_Game null checks in client execution
- [ ] Prefixed with DME_AH_
- [ ] Callback class created (for continuous actions)
