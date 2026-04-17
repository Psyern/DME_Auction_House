// DME Auction House - Action Registration

modded class ActionConstructor
{
	override void RegisterActions(TTypenameArray actions)
	{
		super.RegisterActions(actions);
		actions.Insert(DME_AH_ActionOpenAuction);
	}
}
