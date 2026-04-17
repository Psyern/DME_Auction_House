// DME Auction House - Category Configuration
// JSON-serializable: NO prefix on member variables

class DME_AH_CategoryConfig
{
	ref array<ref DME_AH_Category> Categories;

	void DME_AH_CategoryConfig()
	{
		Categories = new array<ref DME_AH_Category>;
	}

	void CreateDefaults()
	{
		Categories = new array<ref DME_AH_Category>;

		DME_AH_Category catWeapons = new DME_AH_Category();
		catWeapons.CategoryID = 1;
		catWeapons.DisplayName = "Weapons";
		catWeapons.Icon = "set:dayz_gui image:icon_weapon";
		Categories.Insert(catWeapons);

		DME_AH_Category catClothing = new DME_AH_Category();
		catClothing.CategoryID = 2;
		catClothing.DisplayName = "Clothing";
		catClothing.Icon = "set:dayz_gui image:icon_clothing";
		Categories.Insert(catClothing);

		DME_AH_Category catMedical = new DME_AH_Category();
		catMedical.CategoryID = 3;
		catMedical.DisplayName = "Medical";
		catMedical.Icon = "set:dayz_gui image:icon_medical";
		Categories.Insert(catMedical);

		DME_AH_Category catFood = new DME_AH_Category();
		catFood.CategoryID = 4;
		catFood.DisplayName = "Food";
		catFood.Icon = "set:dayz_gui image:icon_food";
		Categories.Insert(catFood);

		DME_AH_Category catVehicles = new DME_AH_Category();
		catVehicles.CategoryID = 5;
		catVehicles.DisplayName = "Vehicles";
		catVehicles.Icon = "set:dayz_gui image:icon_vehicle";
		Categories.Insert(catVehicles);

		DME_AH_Category catBuilding = new DME_AH_Category();
		catBuilding.CategoryID = 6;
		catBuilding.DisplayName = "Building";
		catBuilding.Icon = "set:dayz_gui image:icon_build";
		Categories.Insert(catBuilding);

		DME_AH_Category catOther = new DME_AH_Category();
		catOther.CategoryID = 7;
		catOther.DisplayName = "Other";
		catOther.Icon = "set:dayz_gui image:icon_gear";
		Categories.Insert(catOther);
	}

	DME_AH_Category GetCategoryByID(int categoryID)
	{
		if (!Categories)
			return null;

		for (int i = 0; i < Categories.Count(); i++)
		{
			DME_AH_Category cat = Categories[i];
			if (cat && cat.CategoryID == categoryID)
				return cat;
		}
		return null;
	}

	void Save()
	{
		DME_AH_Config.MakeDirectoryIfNotExists();
		JsonFileLoader<DME_AH_CategoryConfig>.JsonSaveFile(DME_AH_CATEGORIES_FILE, this);
	}

	static ref DME_AH_CategoryConfig Load()
	{
		DME_AH_Config.MakeDirectoryIfNotExists();
		DME_AH_CategoryConfig config = new DME_AH_CategoryConfig();

		if (FileExist(DME_AH_CATEGORIES_FILE))
		{
			JsonFileLoader<DME_AH_CategoryConfig>.JsonLoadFile(DME_AH_CATEGORIES_FILE, config);
			return config;
		}

		config.CreateDefaults();
		config.Save();
		return config;
	}
}
