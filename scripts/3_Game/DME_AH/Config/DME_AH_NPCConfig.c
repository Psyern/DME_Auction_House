// DME Auction House - NPC Configuration
// JSON-serializable: NO prefix on member variables

class DME_AH_NPCEntry
{
	int ID;
	string ClassName;
	ref array<float> Position;
	ref array<float> Orientation;
	string DisplayName;
	ref array<string> Loadout;
	bool Active;

	void DME_AH_NPCEntry()
	{
		ID = 0;
		ClassName = "SurvivorF_Eva";
		Position = new array<float>;
		Position.Insert(0);
		Position.Insert(0);
		Position.Insert(0);
		Orientation = new array<float>;
		Orientation.Insert(0);
		Orientation.Insert(0);
		Orientation.Insert(0);
		DisplayName = "Auction House NPC";
		Loadout = new array<string>;
		Active = false;
	}

	vector GetPositionVector()
	{
		if (!Position || Position.Count() < 3)
			return "0 0 0";
		return Vector(Position[0], Position[1], Position[2]);
	}

	vector GetOrientationVector()
	{
		if (!Orientation || Orientation.Count() < 3)
			return "0 0 0";
		return Vector(Orientation[0], Orientation[1], Orientation[2]);
	}
}

class DME_AH_NPCConfig
{
	ref array<ref DME_AH_NPCEntry> NPCs;

	void DME_AH_NPCConfig()
	{
		NPCs = new array<ref DME_AH_NPCEntry>;
	}

	void CreateDefaults()
	{
		NPCs = new array<ref DME_AH_NPCEntry>;

		DME_AH_NPCEntry example = new DME_AH_NPCEntry();
		example.ID = 1;
		example.ClassName = "SurvivorF_Eva";
		example.Position = new array<float>;
		example.Position.Insert(6575.0);
		example.Position.Insert(0.0);
		example.Position.Insert(2685.0);
		example.Orientation = new array<float>;
		example.Orientation.Insert(135.0);
		example.Orientation.Insert(0.0);
		example.Orientation.Insert(0.0);
		example.DisplayName = "Example Auction NPC - Set position for your map";
		example.Loadout = new array<string>;
		example.Loadout.Insert("GorkaEJacket_Autumn");
		example.Loadout.Insert("GorkaEPants_Autumn");
		example.Loadout.Insert("MilitaryBoots_Black");
		example.Active = false;

		NPCs.Insert(example);
	}

	void Save()
	{
		DME_AH_Config.MakeDirectoryIfNotExists();
		JsonFileLoader<DME_AH_NPCConfig>.JsonSaveFile(DME_AH_NPC_FILE, this);
	}

	static ref DME_AH_NPCConfig Load()
	{
		DME_AH_Config.MakeDirectoryIfNotExists();
		DME_AH_NPCConfig config = new DME_AH_NPCConfig();

		if (FileExist(DME_AH_NPC_FILE))
		{
			JsonFileLoader<DME_AH_NPCConfig>.JsonLoadFile(DME_AH_NPC_FILE, config);
			return config;
		}

		config.CreateDefaults();
		config.Save();
		return config;
	}
}
