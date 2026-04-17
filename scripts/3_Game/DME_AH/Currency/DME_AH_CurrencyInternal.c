// DME Auction House - Internal Points Currency Adapter

class DME_AH_CurrencyInternal : DME_AH_CurrencyAdapter
{
	protected ref DME_AH_DataStore m_DataStore;

	void SetDataStore(DME_AH_DataStore dataStore)
	{
		m_DataStore = dataStore;
	}

	override string GetCurrencyName()
	{
		return "Points";
	}

	override string GetCurrencySymbol()
	{
		return "P";
	}

	override int GetBalance(string playerUID)
	{
		if (!m_DataStore)
			return 0;
		return m_DataStore.GetInternalBalance(playerUID);
	}

	override bool Deduct(string playerUID, int amount)
	{
		if (amount <= 0)
			return false;
		if (!m_DataStore)
			return false;

		int balance = m_DataStore.GetInternalBalance(playerUID);
		if (balance < amount)
			return false;

		int newBalance = balance - amount;
		m_DataStore.SetInternalBalance(playerUID, newBalance);
		return true;
	}

	override bool Add(string playerUID, int amount)
	{
		if (amount <= 0)
			return false;
		if (!m_DataStore)
			return false;

		int balance = m_DataStore.GetInternalBalance(playerUID);
		int newBalance = balance + amount;
		m_DataStore.SetInternalBalance(playerUID, newBalance);
		return true;
	}
}
