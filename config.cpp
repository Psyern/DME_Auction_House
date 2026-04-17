class CfgPatches
{
	class DME_AH_Scripts
	{
		units[] = {};
		weapons[] = {};
		requiredVersion = 0.1;
		requiredAddons[] = {"DZ_Data"};
	};
};

class CfgMods
{
	class DME_Auction_House
	{
		dir = "DME_Auction_House";
		name = "DME Auction House";
		version = "1.0.0";
		author = "Psyern";
		authorID = "";
		type = "mod";
		dependencies[] = {"Game", "World", "Mission"};

		class defs
		{
			class gameScriptModule
			{
				value = "";
				files[] = {
					"DME_Auction_House/scripts/Common",
					"DME_Auction_House/scripts/3_Game"
				};
			};
			class worldScriptModule
			{
				value = "";
				files[] = {
					"DME_Auction_House/scripts/Common",
					"DME_Auction_House/scripts/4_World"
				};
			};
			class missionScriptModule
			{
				value = "";
				files[] = {
					"DME_Auction_House/scripts/Common",
					"DME_Auction_House/scripts/5_Mission"
				};
			};
		};
	};
};
