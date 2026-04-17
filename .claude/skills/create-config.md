---
name: create-config
description: Create a JSON-based configuration/settings system for DayZ mod with validation
---

# Create DayZ Config/Settings System

Creates server/client JSON configuration following DayZ conventions.

## Steps

### 1. Define Path Constants
File: `scripts/3_Game/DME_AH/Constants/DME_AH_Constants.c`
```c
const string DME_AH_ROOT_FOLDER = "$profile:DME_AH\\";
const string DME_AH_CONFIG_DIR = DME_AH_ROOT_FOLDER + "Config\\";
const string DME_AH_CONFIG_FILE = DME_AH_CONFIG_DIR + "Settings.json";
const string DME_AH_LOG_FOLDER = DME_AH_ROOT_FOLDER + "Logging\\";
```

### 2. Create Settings Class (NO prefix on JSON members)
File: `scripts/3_Game/DME_AH/Config/DME_AH_Config.c`
```c
class DME_AH_Config
{
	// NO prefix - these are JSON keys
	bool Enabled = true;
	int MaxListings = 50;
	float ListingFee = 0.05;
	int AuctionDurationMinutes = 1440;
	int MaxBidsPerPlayer = 10;
	string CurrencyType = "TraderPlus_Money";

	void Validate()
	{
		MaxListings = Math.Clamp(MaxListings, 1, 500);
		ListingFee = Math.Clamp(ListingFee, 0.0, 1.0);
		AuctionDurationMinutes = Math.Clamp(AuctionDurationMinutes, 1, 10080);
		MaxBidsPerPlayer = Math.Clamp(MaxBidsPerPlayer, 1, 100);
	}

	void MakeDirectoryIfNotExists()
	{
		if (!FileExist(DME_AH_ROOT_FOLDER))
			MakeDirectory(DME_AH_ROOT_FOLDER);
		if (!FileExist(DME_AH_CONFIG_DIR))
			MakeDirectory(DME_AH_CONFIG_DIR);
	}

	void Save()
	{
		MakeDirectoryIfNotExists();
		JsonFileLoader<DME_AH_Config>.JsonSaveFile(DME_AH_CONFIG_FILE, this);
	}

	static ref DME_AH_Config Load()
	{
		DME_AH_Config config = new DME_AH_Config();
		config.MakeDirectoryIfNotExists();

		if (FileExist(DME_AH_CONFIG_FILE))
		{
			JsonFileLoader<DME_AH_Config>.JsonLoadFile(DME_AH_CONFIG_FILE, config);
			config.Validate();
			return config;
		}

		config.Save();
		return config;
	}
}
```

## Critical Rules
- JSON config class members have NO prefix (keys must match JSON)
- ALWAYS validate after loading: `config.Validate()`
- ALWAYS create directories before saving
- Use `Math.Clamp` for numeric validation
- Use `$profile:DME_AH\\` as root path
- Provide sensible default values
- Use `JsonFileLoader<T>` for save/load

## Checklist
- [ ] Path constants defined
- [ ] Config class with default values
- [ ] Validate() method with Math.Clamp
- [ ] MakeDirectoryIfNotExists() creates all needed dirs
- [ ] Save() and Load() static methods
- [ ] NO prefix on member variables (JSON compatibility)
- [ ] Loaded in module OnInit, saved in OnUnloadModule
