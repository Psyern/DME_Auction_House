// DME Auction House - Auction NPC (Placeable Trader NPC)

class DME_AH_AuctionNPC : SurvivorBase
{
	override void SetActions()
	{
		super.SetActions();
		AddAction(DME_AH_ActionOpenAuction);
	}

	override bool IsInventoryVisible()
	{
		return false;
	}
}
