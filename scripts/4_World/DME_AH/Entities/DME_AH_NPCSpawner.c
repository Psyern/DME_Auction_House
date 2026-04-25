// DME Auction House - NPC Spawner
// Spawns NPCs from JSON config and tracks them for action detection

class DME_AH_NPCSpawner
{
	static ref array<Object> s_SpawnedNPCs;
	static ref set<Object> s_AuctionNPCSet;

	static void SpawnAll(DME_AH_NPCConfig config)
	{
		if (!config || !config.NPCs)
			return;

		if (!g_Game)
			return;

		if (!s_SpawnedNPCs)
			s_SpawnedNPCs = new array<Object>;
		if (!s_AuctionNPCSet)
			s_AuctionNPCSet = new set<Object>;

		int spawnCount = 0;
		for (int i = 0; i < config.NPCs.Count(); i++)
		{
			DME_AH_NPCEntry entry = config.NPCs[i];
			if (!entry)
				continue;
			if (!entry.Active)
				continue;

			Object npc = SpawnNPC(entry);
			if (npc)
				spawnCount++;
		}

		DME_AH_Logger.Info("Spawned " + spawnCount.ToString() + " Auction House NPCs");
	}

	static Object SpawnNPC(DME_AH_NPCEntry entry)
	{
		if (!entry || !g_Game)
			return null;

		vector position = entry.GetPositionVector();
		vector orientation = entry.GetOrientationVector();

		// Auto-snap to ground if Y is 0
		float posY = position[1];
		if (posY < 1.0)
		{
			float surfaceY = g_Game.SurfaceY(position[0], position[2]);
			position = Vector(position[0], surfaceY, position[2]);
		}

		Object obj = g_Game.CreateObject(entry.ClassName, position, false, false, true);
		if (!obj)
		{
			DME_AH_Logger.Error("Failed to spawn NPC ID " + entry.ID.ToString() + " (" + entry.ClassName + ")");
			return null;
		}

		obj.SetPosition(position);
		obj.SetOrientation(orientation);

		// Apply loadout. Each entry can be either:
		//   - a regular DayZ classname (e.g. "GorkaEJacket_Autumn"), spawned as one item, or
		//   - the name of an Expansion loadout JSON in $profile:ExpansionMod/Loadouts/
		//     (e.g. "LoadoutSurvivor"), in which case we hand off to Expansion.
		// If Expansion is not loaded, only the classname path is available.
		EntityAI entity = EntityAI.Cast(obj);
		if (entity && entry.Loadout)
		{
			for (int i = 0; i < entry.Loadout.Count(); i++)
			{
				ApplyLoadoutEntry(entity, entry.Loadout[i], entry.ID);
			}
		}

		// Register for action detection
		s_SpawnedNPCs.Insert(obj);
		s_AuctionNPCSet.Insert(obj);

		string posStr = position[0].ToString() + " " + position[1].ToString() + " " + position[2].ToString();
		DME_AH_Logger.Info("NPC spawned: ID " + entry.ID.ToString() + " (" + entry.DisplayName + ") at " + posStr);
		return obj;
	}

	// Tries Expansion loadout first (if Expansion is loaded AND a matching JSON
	// exists in $profile:ExpansionMod/Loadouts/<name>.json). Falls back to
	// spawning <name> as a vanilla DayZ classname.
	protected static void ApplyLoadoutEntry(EntityAI entity, string entryName, int npcID)
	{
		if (!entity || entryName == "")
			return;

		#ifdef EXPANSIONMODCORE
		string loadoutPath = EXPANSION_LOADOUT_FOLDER + entryName + ".json";
		if (FileExist(loadoutPath))
		{
			if (ExpansionHumanLoadout.Apply(entity, entryName))
			{
				DME_AH_Logger.Info("NPC ID " + npcID.ToString() + ": applied Expansion loadout '" + entryName + "'");
				return;
			}
			DME_AH_Logger.Warning("NPC ID " + npcID.ToString() + ": Expansion loadout '" + entryName + "' file exists but Apply failed, treating as classname");
		}
		#endif

		EntityAI item = entity.GetInventory().CreateInInventory(entryName);
		if (!item)
			DME_AH_Logger.Warning("NPC ID " + npcID.ToString() + ": Failed to apply loadout item " + entryName);
	}

	static bool IsAuctionNPC(Object obj)
	{
		if (!obj)
			return false;

		// Server-side: check registry (fast, always reliable)
		if (s_AuctionNPCSet)
		{
			int idx = s_AuctionNPCSet.Find(obj);
			if (idx != -1)
				return true;
		}

		// Client+Server fallback: a SurvivorBase without PlayerIdentity is a spawned NPC
		SurvivorBase survivor = SurvivorBase.Cast(obj);
		if (!survivor)
			return false;

		// Real players always have identity, spawned NPCs never do
		if (survivor.GetIdentity())
			return false;

		// Verify it's not a zombie or infected (they also have no identity)
		if (obj.IsInherited(ZombieBase))
			return false;

		return true;
	}

	static void DespawnAll()
	{
		if (!g_Game)
			return;

		if (s_SpawnedNPCs)
		{
			for (int i = 0; i < s_SpawnedNPCs.Count(); i++)
			{
				Object npc = s_SpawnedNPCs[i];
				if (npc)
					g_Game.ObjectDelete(npc);
			}
			s_SpawnedNPCs.Clear();
		}

		if (s_AuctionNPCSet)
			s_AuctionNPCSet.Clear();

		DME_AH_Logger.Info("All Auction House NPCs despawned");
	}
}
