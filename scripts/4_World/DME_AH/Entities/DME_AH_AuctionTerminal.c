// DME Auction House - Auction Terminal (Placeable Object)

class DME_AH_AuctionTerminal : HouseNoDestruct
{
	override void SetActions()
	{
		super.SetActions();
		AddAction(DME_AH_ActionOpenAuction);
	}
}
