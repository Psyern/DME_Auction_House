// DME Auction House - Currency Adapter Base Class

class DME_AH_CurrencyAdapter
{
	string GetCurrencyName()
	{
		return "Currency";
	}

	string GetCurrencySymbol()
	{
		return "$";
	}

	int GetBalance(string playerUID)
	{
		return 0;
	}

	bool Deduct(string playerUID, int amount)
	{
		return false;
	}

	bool Add(string playerUID, int amount)
	{
		return false;
	}

	bool HasEnough(string playerUID, int amount)
	{
		int balance = GetBalance(playerUID);
		return balance >= amount;
	}

	string FormatAmount(int amount)
	{
		return amount.ToString() + GetCurrencySymbol();
	}
}
