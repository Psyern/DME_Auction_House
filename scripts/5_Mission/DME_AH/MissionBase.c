// DME Auction House - Menu Registration

modded class MissionBase
{
	override UIScriptedMenu CreateScriptedMenu(int id)
	{
		UIScriptedMenu menu = super.CreateScriptedMenu(id);
		if (menu)
			return menu;

		switch (id)
		{
			case MENU_DME_AH_AUCTION:
				menu = new DME_AH_AuctionMenu;
				break;
			case MENU_DME_AH_CREATE_LISTING:
				menu = new DME_AH_CreateListingDialog;
				break;
		}

		return menu;
	}
}
