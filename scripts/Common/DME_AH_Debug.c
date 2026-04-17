// DME Auction House - Debug Utilities
// Shared across all script layers

void DME_AH_DebugPrint(string message)
{
	#ifdef DME_AH_DEBUG
	Print("[DME_AH][DEBUG] " + message);
	#endif
}

void DME_AH_Print(string message)
{
	Print("[DME_AH] " + message);
}

void DME_AH_ErrorPrint(string message)
{
	Print("[DME_AH][ERROR] " + message);
}
