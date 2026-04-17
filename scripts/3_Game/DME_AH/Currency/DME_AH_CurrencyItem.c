// DME Auction House - In-Game Item Currency Adapter

class DME_AH_CurrencyItem : DME_AH_CurrencyAdapter
{
	protected string m_ItemClassName;

	void DME_AH_CurrencyItem()
	{
		m_ItemClassName = "MoneyRuble100";
	}

	void SetItemClassName(string className)
	{
		m_ItemClassName = className;
	}

	override string GetCurrencyName()
	{
		return m_ItemClassName;
	}

	override string GetCurrencySymbol()
	{
		return "x";
	}

	override int GetBalance(string playerUID)
	{
		PlayerBase player = GetPlayerByUID(playerUID);
		if (!player)
			return 0;

		return CountItemsInInventory(player, m_ItemClassName);
	}

	override bool Deduct(string playerUID, int amount)
	{
		if (amount <= 0)
			return false;

		PlayerBase player = GetPlayerByUID(playerUID);
		if (!player)
			return false;

		int available = CountItemsInInventory(player, m_ItemClassName);
		if (available < amount)
			return false;

		return RemoveItemsFromInventory(player, m_ItemClassName, amount);
	}

	override bool Add(string playerUID, int amount)
	{
		if (amount <= 0)
			return false;

		PlayerBase player = GetPlayerByUID(playerUID);
		if (!player)
			return false;

		return SpawnItemsToInventory(player, m_ItemClassName, amount);
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

	protected int CountItemsInInventory(PlayerBase player, string className)
	{
		if (!player)
			return 0;

		int count = 0;
		array<EntityAI> items = new array<EntityAI>;
		player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, items);

		for (int i = 0; i < items.Count(); i++)
		{
			EntityAI item = items[i];
			if (item && item.GetType() == className)
				count++;
		}
		return count;
	}

	protected bool RemoveItemsFromInventory(PlayerBase player, string className, int amount)
	{
		if (!player || amount <= 0)
			return false;

		int removed = 0;
		array<EntityAI> items = new array<EntityAI>;
		player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, items);

		for (int i = 0; i < items.Count(); i++)
		{
			if (removed >= amount)
				break;
			EntityAI item = items[i];
			if (item && item.GetType() == className)
			{
				g_Game.ObjectDelete(item);
				removed++;
			}
		}
		return removed >= amount;
	}

	protected bool SpawnItemsToInventory(PlayerBase player, string className, int amount)
	{
		if (!player || amount <= 0)
			return false;

		for (int i = 0; i < amount; i++)
		{
			EntityAI item = player.GetInventory().CreateInInventory(className);
			if (!item)
			{
				DME_AH_Logger.Warning("CurrencyItem: Could not spawn item " + className + " to player inventory");
				return false;
			}
		}
		return true;
	}
}
