// DME Auction House - Expansion Market Currency Adapter
// NO compile-time dependency on Expansion or 4_World types
// Uses Man/EntityAI (available in 3_Game) instead of PlayerBase

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
		Man player = GetPlayerByUID(playerUID);
		if (!player)
			return 0;

		return CountMoneyInInventory(player);
	}

	override bool Deduct(string playerUID, int amount)
	{
		if (amount <= 0)
			return false;

		Man player = GetPlayerByUID(playerUID);
		if (!player)
			return false;

		int available = CountMoneyInInventory(player);
		if (available < amount)
			return false;

		return RemoveMoneyFromInventory(player, amount);
	}

	override bool Add(string playerUID, int amount)
	{
		if (amount <= 0)
			return false;

		Man player = GetPlayerByUID(playerUID);
		if (!player)
		{
			DME_AH_Logger.Warning("CurrencyExpansion: Player offline, cannot add money");
			return false;
		}

		return SpawnMoneyToInventory(player, amount);
	}

	protected int CountMoneyInInventory(Man player)
	{
		if (!player)
			return 0;

		int total = 0;
		array<EntityAI> items = new array<EntityAI>;
		player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, items);

		for (int i = 0; i < items.Count(); i++)
		{
			EntityAI item = items[i];
			if (!item)
				continue;

			int moneyValue = GetMoneyValue(item);
			if (moneyValue > 0)
			{
				int quantity = item.GetQuantity();
				if (quantity < 1)
					quantity = 1;
				total = total + (moneyValue * quantity);
			}
		}
		return total;
	}

	protected bool RemoveMoneyFromInventory(Man player, int amount)
	{
		if (!player || amount <= 0)
			return false;

		int remaining = amount;
		array<EntityAI> items = new array<EntityAI>;
		player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, items);

		for (int i = 0; i < items.Count(); i++)
		{
			if (remaining <= 0)
				break;

			EntityAI item = items[i];
			if (!item)
				continue;

			int moneyValue = GetMoneyValue(item);
			if (moneyValue <= 0)
				continue;

			int quantity = item.GetQuantity();
			if (quantity < 1)
				quantity = 1;

			int itemTotal = moneyValue * quantity;

			if (itemTotal <= remaining)
			{
				remaining = remaining - itemTotal;
				if (g_Game)
					g_Game.ObjectDelete(item);
			}
			else
			{
				int removeQty = remaining / moneyValue;
				if (removeQty > 0)
				{
					int newQty = quantity - removeQty;
					item.SetQuantity(newQty);
					remaining = remaining - (removeQty * moneyValue);
				}
				if (remaining > 0 && remaining < moneyValue)
					remaining = 0;
			}
		}

		return remaining <= 0;
	}

	protected bool SpawnMoneyToInventory(Man player, int amount)
	{
		if (!player || amount <= 0)
			return false;

		string moneyClass = "ExpansionBanknoteUSD";
		int remaining = amount;
		int maxIterations = 1000;
		int iteration = 0;

		while (remaining > 0 && iteration < maxIterations)
		{
			EntityAI spawned = player.GetInventory().CreateInInventory(moneyClass);
			if (!spawned)
			{
				DME_AH_Logger.Warning("CurrencyExpansion: Failed to spawn money");
				return false;
			}

			int value = GetMoneyValue(spawned);
			if (value <= 0)
				value = 1;

			if (remaining >= value)
				remaining = remaining - value;
			else
				remaining = 0;

			iteration = iteration + 1;
		}

		return true;
	}

	protected int GetMoneyValue(EntityAI item)
	{
		if (!item)
			return 0;

		string typeName = item.GetType();
		typeName.ToLower();

		if (typeName.Contains("expansionbanknote") || typeName.Contains("expansiongold") || typeName.Contains("expansionsilver"))
		{
			string cfgPath = "CfgVehicles " + item.GetType() + " ExpansionMoneyValue";
			if (g_Game && g_Game.ConfigIsExisting(cfgPath))
				return g_Game.ConfigGetInt(cfgPath);

			if (typeName.Contains("goldbar"))
				return 10000;
			if (typeName.Contains("goldnugget"))
				return 1000;
			if (typeName.Contains("silverbar"))
				return 100;
			if (typeName.Contains("silvernugget"))
				return 10;
			if (typeName.Contains("banknote"))
				return 1;
		}

		return 0;
	}

	protected Man GetPlayerByUID(string playerUID)
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
			PlayerIdentity identity = man.GetIdentity();
			if (!identity)
				continue;
			if (identity.GetPlainId() == playerUID)
				return man;
		}
		return null;
	}
}
