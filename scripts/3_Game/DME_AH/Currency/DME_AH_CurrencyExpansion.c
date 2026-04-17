// DME Auction House - Expansion Market Currency Adapter
// Uses ExpansionMarketModule API for money operations
// Supports both physical money (inventory) and ATM balance

class DME_AH_CurrencyExpansion : DME_AH_CurrencyAdapter
{
	override string GetCurrencyName()
	{
		return "Expansion";
	}

	override string GetCurrencySymbol()
	{
		return "$";
	}

	override int GetBalance(string playerUID)
	{
		PlayerBase player = GetPlayerByUID(playerUID);
		if (!player)
		{
			// Player offline - check ATM data only
			return GetATMBalance(playerUID);
		}

		ExpansionMarketModule market = GetMarketModule();
		if (!market)
			return 0;

		array<int> monies = new array<int>;
		int totalWorth = market.GetPlayerWorth(player, monies);
		return totalWorth;
	}

	override bool Deduct(string playerUID, int amount)
	{
		if (amount <= 0)
			return false;

		PlayerBase player = GetPlayerByUID(playerUID);
		if (!player)
		{
			// Player offline - deduct from ATM only
			return DeductFromATM(playerUID, amount);
		}

		ExpansionMarketModule market = GetMarketModule();
		if (!market)
			return false;

		// Step 1: Find and reserve money
		array<int> monies = new array<int>;
		bool found = market.FindMoneyAndCountTypes(player, amount, monies, true);
		if (!found)
		{
			market.UnlockMoney(player);
			return false;
		}

		// Step 2: Remove reserved money
		int totalRemoved = market.RemoveMoney(player);

		// Step 3: Spawn change if overpaid
		if (totalRemoved > amount)
		{
			int change = totalRemoved - amount;
			EntityAI parent = player;
			market.SpawnMoney(player, parent, change, true);
			market.CheckSpawn(player, parent);
		}

		DME_AH_Logger.Debug("CurrencyExpansion: Deducted " + amount.ToString() + " from " + playerUID);
		return true;
	}

	override bool Add(string playerUID, int amount)
	{
		if (amount <= 0)
			return false;

		PlayerBase player = GetPlayerByUID(playerUID);
		if (!player)
		{
			// Player offline - add to ATM
			return AddToATM(playerUID, amount);
		}

		ExpansionMarketModule market = GetMarketModule();
		if (!market)
			return false;

		EntityAI parent = player;
		array<ItemBase> spawned = market.SpawnMoney(player, parent, amount, true);
		market.CheckSpawn(player, parent);

		if (!spawned || spawned.Count() == 0)
		{
			// Inventory full - deposit to ATM instead
			DME_AH_Logger.Warning("CurrencyExpansion: Inventory full, depositing to ATM");
			return AddToATM(playerUID, amount);
		}

		DME_AH_Logger.Debug("CurrencyExpansion: Added " + amount.ToString() + " to " + playerUID);
		return true;
	}

	// --- ATM Operations (for offline players) ---
	protected int GetATMBalance(string playerUID)
	{
		ExpansionMarketModule market = GetMarketModule();
		if (!market)
			return 0;

		ExpansionMarketATM_Data atmData = market.GetPlayerATMData(playerUID);
		if (!atmData)
			return 0;

		return atmData.GetMoney();
	}

	protected bool DeductFromATM(string playerUID, int amount)
	{
		ExpansionMarketModule market = GetMarketModule();
		if (!market)
			return false;

		ExpansionMarketATM_Data atmData = market.GetPlayerATMData(playerUID);
		if (!atmData)
			return false;

		int atmBalance = atmData.GetMoney();
		if (atmBalance < amount)
			return false;

		atmData.RemoveMoney(amount);
		atmData.Save();
		DME_AH_Logger.Debug("CurrencyExpansion: Deducted " + amount.ToString() + " from ATM for " + playerUID);
		return true;
	}

	protected bool AddToATM(string playerUID, int amount)
	{
		ExpansionMarketModule market = GetMarketModule();
		if (!market)
			return false;

		ExpansionMarketATM_Data atmData = market.GetPlayerATMData(playerUID);
		if (!atmData)
			return false;

		atmData.AddMoney(amount);
		atmData.Save();
		DME_AH_Logger.Debug("CurrencyExpansion: Added " + amount.ToString() + " to ATM for " + playerUID);
		return true;
	}

	// --- Helpers ---
	protected ExpansionMarketModule GetMarketModule()
	{
		return ExpansionMarketModule.GetInstance();
	}

	protected PlayerBase GetPlayerByUID(string playerUID)
	{
		if (!g_Game)
			return null;

		array<Man> players = new array<Man>;
		g_Game.GetPlayers(players);

		for (int i = 0; i < players.Count(); i++)
		{
			Man man = players[i];
			if (!man)
				continue;
			PlayerBase player = PlayerBase.Cast(man);
			if (!player)
				continue;
			PlayerIdentity identity = player.GetIdentity();
			if (!identity)
				continue;
			if (identity.GetPlainId() == playerUID)
				return player;
		}
		return null;
	}
}
