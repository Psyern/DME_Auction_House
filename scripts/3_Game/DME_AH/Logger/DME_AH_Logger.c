// DME Auction House - Logger

class DME_AH_Logger
{
	static const int LOG_NONE = 0;
	static const int LOG_ERROR = 1;
	static const int LOG_WARNING = 2;
	static const int LOG_INFO = 3;
	static const int LOG_DEBUG = 4;

	static int s_LogLevel = LOG_INFO;

	static void SetLogLevel(int level)
	{
		s_LogLevel = Math.Clamp(level, LOG_NONE, LOG_DEBUG);
	}

	static void Log(int level, string message)
	{
		if (level > s_LogLevel)
			return;

		string levelStr = "";
		if (level == LOG_ERROR)
			levelStr = "ERROR";
		else if (level == LOG_WARNING)
			levelStr = "WARNING";
		else if (level == LOG_INFO)
			levelStr = "INFO";
		else if (level == LOG_DEBUG)
			levelStr = "DEBUG";

		string timeStr = GetTimeString();
		PrintFormat("%1 [DME_AH] [%2] %3", timeStr, levelStr, message);
	}

	static void Error(string message)
	{
		Log(LOG_ERROR, message);
	}

	static void Warning(string message)
	{
		Log(LOG_WARNING, message);
	}

	static void Info(string message)
	{
		Log(LOG_INFO, message);
	}

	static void Debug(string message)
	{
		Log(LOG_DEBUG, message);
	}

	static string GetTimeString()
	{
		int year;
		int month;
		int day;
		int hour;
		int minute;
		int second;
		GetYearMonthDay(year, month, day);
		GetHourMinuteSecond(hour, minute, second);
		string timeStr = hour.ToStringLen(2) + ":" + minute.ToStringLen(2) + ":" + second.ToStringLen(2);
		return timeStr;
	}
}
