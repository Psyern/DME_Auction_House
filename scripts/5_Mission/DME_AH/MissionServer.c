// DME Auction House - Server-side Module Init

modded class MissionServer
{
	override void OnInit()
	{
		super.OnInit();

		DME_AH_Module module = DME_AH_Module.GetInstance();
		if (module)
			module.Init();

		DME_AH_Util.LogPrint("MissionServer initialized");
	}

	override void OnMissionFinish()
	{
		DME_AH_Module module = DME_AH_Module.GetInstance();
		if (module)
			module.OnMissionFinish();

		super.OnMissionFinish();
	}
}
