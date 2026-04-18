// DME Auction House - Auction NPC
// Registered in CfgVehicles, invincible, non-lootable

class DME_AH_AuctionNPC : SurvivorF_Eva
{
	override void SetActions(out TInputActionMap InputActionMap)
	{
		super.SetActions(InputActionMap);
		AddAction(DME_AH_ActionOpenAuction, InputActionMap);
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
