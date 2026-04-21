// DME Auction House - 4_World factory override for Expansion currency adapter.
// 3_Game cannot reference DME_AH_CurrencyExpansion (it lives in 4_World because
// ExpansionMarketModule is in 4_World). This modded class overrides the
// factory hook so the base module can still construct the adapter without
// knowing the concrete 4_World type.

modded class DME_AH_Module
{
	override protected DME_AH_CurrencyAdapter CreateExpansionCurrencyAdapter()
	{
		return new DME_AH_CurrencyExpansion();
	}
}
