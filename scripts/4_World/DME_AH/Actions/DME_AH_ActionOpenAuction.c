// DME Auction House - Action to Open Auction Menu

class DME_AH_ActionOpenAuction : ActionInteractBase
{
	void DME_AH_ActionOpenAuction()
	{
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_INTERACTONCE;
		m_Text = "Open Auction House";
		m_StanceMask = DayZPlayerConstants.STANCEMASK_ERECT | DayZPlayerConstants.STANCEMASK_CROUCH;
	}

	override void CreateConditionComponents()
	{
		m_ConditionItem = new CCINone;
		m_ConditionTarget = new CCTCursor;
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		if (!target)
			return false;

		Object targetObject = target.GetObject();
		if (!targetObject)
			return false;

		if (targetObject.IsInherited(DME_AH_AuctionTerminal))
			return true;
		if (targetObject.IsInherited(DME_AH_AuctionNPC))
			return true;

		return false;
	}

	override void OnExecuteClient(ActionData action_data)
	{
		if (!g_Game)
			return;
		if (g_Game.IsDedicatedServer())
			return;

		UIManager uiManager = g_Game.GetUIManager();
		if (!uiManager)
			return;

		bool isOpen = uiManager.IsMenuOpen(MENU_DME_AH_AUCTION);
		if (!isOpen)
		{
			uiManager.EnterScriptedMenu(MENU_DME_AH_AUCTION, null);
		}
	}
}
