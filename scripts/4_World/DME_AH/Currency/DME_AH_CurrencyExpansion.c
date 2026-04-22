// DME Auction House - Expansion Market Currency Adapter
// Lives in 4_World because ExpansionMarketModule/ExpansionMarketATM_Data are
// declared in Expansion's 4_World layer and 3_Game cannot reference them.
//
// Must still compile cleanly when Expansion is NOT loaded. All references to
// Expansion types are wrapped in #ifdef EXPANSIONMODMARKET which is defined in
// 0_DayZExpansion_Market_Preload/Common/DayZExpansion_Market_Defines.c.
//
// Uses Man/EntityAI (also available in 4_World) for player iteration — we do
// not cast to PlayerBase since we do not need it here.

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
		int inventoryTotal = 0;
		if (player)
			inventoryTotal = CountMoneyInInventory(player);

		int bankTotal = 0;
		#ifdef EXPANSIONMODMARKET
		if (IsBankEnabled())
		{
			ExpansionMarketATM_Data atm = GetOrCreateATM(playerUID);
			if (atm)
				bankTotal = atm.GetMoney();
		}
		#endif

		DME_AH_Logger.Debug("CurrencyExpansion.GetBalance: uid=" + playerUID + " inv=" + inventoryTotal + " bank=" + bankTotal);
		return inventoryTotal + bankTotal;
	}

	override bool Deduct(string playerUID, int amount)
	{
		if (amount <= 0)
			return false;

		Man player = GetPlayerByUID(playerUID);

		int inventoryAvailable = 0;
		if (player)
			inventoryAvailable = CountMoneyInInventory(player);

		int deductFromInventory = amount;
		if (deductFromInventory > inventoryAvailable)
			deductFromInventory = inventoryAvailable;

		int remainder = amount - deductFromInventory;

		// Short-circuit: if we do not need the bank and inventory has enough, do the normal path.
		if (remainder <= 0)
		{
			if (!player)
				return false;
			return RemoveMoneyFromInventory(player, amount);
		}

		// We need the bank for the remainder.
		#ifdef EXPANSIONMODMARKET
		if (!IsBankEnabled())
		{
			DME_AH_Logger.Warning("CurrencyExpansion.Deduct: inventory short by " + remainder.ToString() + " and UseExpansionBank=false");
			return false;
		}

		ExpansionMarketATM_Data atm = GetOrCreateATM(playerUID);
		if (!atm)
		{
			DME_AH_Logger.Warning("CurrencyExpansion.Deduct: could not resolve ATM data for " + playerUID);
			return false;
		}

		int atmAvailable = atm.GetMoney();
		if (atmAvailable < remainder)
		{
			DME_AH_Logger.Info("CurrencyExpansion.Deduct: bank has " + atmAvailable.ToString() + " but needs " + remainder.ToString());
			return false;
		}

		// Drain inventory first (may be 0 if player offline).
		bool inventoryOk = true;
		if (player && deductFromInventory > 0)
			inventoryOk = RemoveMoneyFromInventory(player, deductFromInventory);

		if (!inventoryOk)
		{
			// Rollback: re-spawn what we think was taken. Best-effort.
			if (player && deductFromInventory > 0)
				SpawnMoneyToInventory(player, deductFromInventory);
			return false;
		}

		// Deduct remainder from ATM.
		atm.RemoveMoney(remainder);
		atm.Save();
		DME_AH_Logger.Info("CurrencyExpansion.Deduct: " + deductFromInventory.ToString() + " from inventory + " + remainder.ToString() + " from bank (uid=" + playerUID + ")");
		return true;
		#else
		DME_AH_Logger.Warning("CurrencyExpansion.Deduct: EXPANSIONMODMARKET not defined — Expansion Market not loaded, cannot use bank");
		return false;
		#endif
	}

	override bool Add(string playerUID, int amount)
	{
		if (amount <= 0)
			return false;

		Man player = GetPlayerByUID(playerUID);
		if (player)
		{
			if (SpawnMoneyToInventory(player, amount))
				return true;

			DME_AH_Logger.Warning("CurrencyExpansion: Inventory spawn failed, falling back to ATM");
		}
		else
		{
			DME_AH_Logger.Info("CurrencyExpansion: Player offline, depositing to ATM");
		}

		// Fallback: ATM deposit (works even for offline players).
		#ifdef EXPANSIONMODMARKET
		if (!IsBankEnabled())
		{
			DME_AH_Logger.Warning("CurrencyExpansion.Add: inventory spawn failed and UseExpansionBank=false");
			return false;
		}

		ExpansionMarketATM_Data atm = GetOrCreateATM(playerUID);
		if (!atm)
		{
			DME_AH_Logger.Warning("CurrencyExpansion.Add: could not resolve ATM data for " + playerUID);
			return false;
		}

		atm.AddMoney(amount);
		atm.Save();
		DME_AH_Logger.Info("CurrencyExpansion.Add: deposited " + amount.ToString() + " to bank (uid=" + playerUID + ")");
		return true;
		#else
		DME_AH_Logger.Warning("CurrencyExpansion.Add: EXPANSIONMODMARKET not defined");
		return false;
		#endif
	}

	#ifdef EXPANSIONMODMARKET
	// Gets the player's ATM data; auto-creates if missing (matches Expansion's
	// own OnInvokeConnect behavior, but safe if the player hadn't connected yet
	// with ATMSystemEnabled or if the file was purged).
	protected ExpansionMarketATM_Data GetOrCreateATM(string playerUID)
	{
		ExpansionMarketModule mod = ExpansionMarketModule.GetInstance();
		if (!mod)
		{
			DME_AH_Logger.Warning("CurrencyExpansion: ExpansionMarketModule.GetInstance() is null");
			return null;
		}

		ExpansionMarketATM_Data atm = mod.GetPlayerATMData(playerUID);
		if (atm)
			return atm;

		// Not found — resolve an online PlayerIdentity so Expansion can create the file.
		PlayerIdentity identity = null;
		array<Man> players = new array<Man>;
		if (g_Game)
			g_Game.GetPlayers(players);
		for (int i = 0; i < players.Count(); i++)
		{
			Man m = players[i];
			if (!m)
				continue;
			PlayerIdentity id = m.GetIdentity();
			if (id && id.GetPlainId() == playerUID)
			{
				identity = id;
				break;
			}
		}

		if (!identity)
		{
			DME_AH_Logger.Warning("CurrencyExpansion: player " + playerUID + " is offline and has no ATM data — cannot auto-create");
			return null;
		}

		DME_AH_Logger.Info("CurrencyExpansion: auto-creating ATM data for " + playerUID);
		mod.CreateATMData(identity);
		return mod.GetPlayerATMData(playerUID);
	}
	#endif

	protected bool IsBankEnabled()
	{
		DME_AH_Module mod = DME_AH_Module.GetInstance();
		if (!mod)
		{
			DME_AH_Logger.Debug("CurrencyExpansion.IsBankEnabled: DME_AH_Module instance null");
			return false;
		}
		DME_AH_Config cfg = mod.GetConfig();
		if (!cfg)
		{
			DME_AH_Logger.Debug("CurrencyExpansion.IsBankEnabled: config null");
			return false;
		}
		return cfg.UseExpansionBank;
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
				DME_AH_Logger.Warning("CurrencyExpansion: Failed to spawn money (inventory full?)");
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
