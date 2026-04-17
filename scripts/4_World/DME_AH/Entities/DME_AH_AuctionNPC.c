// DME Auction House - Auction NPC (Placeable Trader NPC)
// Invincible, non-lootable, non-targetable by AI

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

	override void EEHitBy(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
		super.EEHitBy(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);
		SetHealth(GetMaxHealth());
	}

	override bool CanBeTargetedByAI(EntityAI ai)
	{
		return false;
	}
}
