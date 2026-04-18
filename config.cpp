class CfgPatches
{
	class DME_AH_Scripts
	{
		units[] = {"DME_AH_AuctionNPC"};
		weapons[] = {};
		requiredVersion = 0.1;
		requiredAddons[] = {"DZ_Data"};
	};
};

class CfgVehicles
{
	class SurvivorBase;
	class DME_AH_AuctionNPC : SurvivorBase {};
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
					"DME_Auction_House/scripts/3_Game"
				};
			};
			class worldScriptModule
			{
				value = "";
				files[] = {
					"DME_Auction_House/scripts/4_World"
				};
			};
			class missionScriptModule
			{
				value = "";
				files[] = {
					"DME_Auction_House/scripts/5_Mission"
				};
			};
		};
	};
};
