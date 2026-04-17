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

		// Apply loadout
		EntityAI entity = EntityAI.Cast(obj);
		if (entity && entry.Loadout)
		{
			for (int i = 0; i < entry.Loadout.Count(); i++)
			{
				string itemClass = entry.Loadout[i];
				EntityAI item = entity.GetInventory().CreateInInventory(itemClass);
				if (!item)
				{
					DME_AH_Logger.Warning("NPC ID " + entry.ID.ToString() + ": Failed to apply loadout item " + itemClass);
				}
			}
		}

		// Register for action detection
		s_SpawnedNPCs.Insert(obj);
		s_AuctionNPCSet.Insert(obj);

		string posStr = position[0].ToString() + " " + position[1].ToString() + " " + position[2].ToString();
		DME_AH_Logger.Info("NPC spawned: ID " + entry.ID.ToString() + " (" + entry.DisplayName + ") at " + posStr);
		return obj;
	}

	static bool IsAuctionNPC(Object obj)
	{
		if (!obj)
			return false;

		// Server-side: check registry (fast)
		if (s_AuctionNPCSet)
		{
			int idx = s_AuctionNPCSet.Find(obj);
			if (idx != -1)
				return true;
		}

		// Client+Server: check if this entity's classname matches any configured NPC
		DME_AH_Module module = DME_AH_Module.GetInstance();
		if (!module)
			return false;

		DME_AH_NPCConfig npcConfig = module.GetNPCConfig();
		if (!npcConfig || !npcConfig.NPCs)
			return false;

		string objType = obj.GetType();
		vector objPos = obj.GetPosition();

		for (int i = 0; i < npcConfig.NPCs.Count(); i++)
		{
			DME_AH_NPCEntry entry = npcConfig.NPCs[i];
			if (!entry || !entry.Active)
				continue;

			// Must be same classname
			if (entry.ClassName != objType)
				continue;

			// Check horizontal distance only (ignore Y differences)
			vector npcPos = entry.GetPositionVector();
			float dx = objPos[0] - npcPos[0];
			float dz = objPos[2] - npcPos[2];
			float distSq = (dx * dx) + (dz * dz);

			// Within 10m horizontal = match
			if (distSq < 100.0)
				return true;
		}

		return false;
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
