// DME Auction House - Debug Utilities + Helpers
// Shared across all script layers via static class

class DME_AH_Util
{
	static void DebugPrint(string message)
	{
		#ifdef DME_AH_DEBUG
		Print("[DME_AH][DEBUG] " + message);
		#endif
	}

	static void LogPrint(string message)
	{
		Print("[DME_AH] " + message);
	}

	static void ErrorPrint(string message)
	{
		Print("[DME_AH][ERROR] " + message);
	}

	// Native timestamp helper (replaces CF_Date)
	// Returns seconds since 2020-01-01
	static int GetTimestamp()
	{
		int year;
		int month;
		int day;
		int hour;
		int minute;
		int second;
		GetYearMonthDay(year, month, day);
		GetHourMinuteSecond(hour, minute, second);

		int totalDays = 0;

		int y = 2020;
		while (y < year)
		{
			bool isLeap = false;
			int mod4 = y % 4;
			if (mod4 == 0)
				isLeap = true;
			int mod100 = y % 100;
			if (mod100 == 0)
				isLeap = false;
			int mod400 = y % 400;
			if (mod400 == 0)
				isLeap = true;

			if (isLeap)
				totalDays = totalDays + 366;
			else
				totalDays = totalDays + 365;
			y = y + 1;
		}

		int daysInMonth;
		int m = 1;
		while (m < month)
		{
			if (m == 1 || m == 3 || m == 5 || m == 7 || m == 8 || m == 10 || m == 12)
				daysInMonth = 31;
			else if (m == 4 || m == 6 || m == 9 || m == 11)
				daysInMonth = 30;
			else
			{
				daysInMonth = 28;
				int mod4c = year % 4;
				if (mod4c == 0)
					daysInMonth = 29;
				int mod100c = year % 100;
				if (mod100c == 0)
					daysInMonth = 28;
				int mod400c = year % 400;
				if (mod400c == 0)
					daysInMonth = 29;
			}
			totalDays = totalDays + daysInMonth;
			m = m + 1;
		}

		totalDays = totalDays + day - 1;

		int totalSeconds = (totalDays * 86400) + (hour * 3600) + (minute * 60) + second;
		return totalSeconds;
	}
}
