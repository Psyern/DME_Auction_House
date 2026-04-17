// DME Auction House - Server-side Module Init + NPC Spawning

modded class MissionServer
{
	override void OnInit()
	{
		super.OnInit();

		DME_AH_Module module = DME_AH_Module.GetInstance();
		if (module)
		{
			module.Init();

			DME_AH_NPCConfig npcConfig = module.GetNPCConfig();
			if (npcConfig)
				DME_AH_NPCSpawner.SpawnAll(npcConfig);
		}

		DME_AH_Util.LogPrint("MissionServer initialized");
	}

	override void OnMissionFinish()
	{
		DME_AH_NPCSpawner.DespawnAll();

		DME_AH_Module module = DME_AH_Module.GetInstance();
		if (module)
			module.OnMissionFinish();

		super.OnMissionFinish();
	}
}
