// DME Auction House - Category Data Class
// JSON-serializable: NO prefix on member variables

class DME_AH_Category
{
	int CategoryID;
	string DisplayName;
	string Icon;
	ref array<string> ClassFilters;

	void DME_AH_Category()
	{
		CategoryID = 0;
		DisplayName = "";
		Icon = "";
		ClassFilters = new array<string>;
	}

	bool MatchesItem(string className)
	{
		if (!ClassFilters || ClassFilters.Count() == 0)
			return true;

		for (int i = 0; i < ClassFilters.Count(); i++)
		{
			string filter = ClassFilters[i];
			if (filter == className)
				return true;
		}
		return false;
	}
}
