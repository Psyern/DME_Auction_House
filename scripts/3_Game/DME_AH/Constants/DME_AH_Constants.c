// DME Auction House - Constants

// Version
const string DME_AH_VERSION = "1.0.0";

// File Paths
const string DME_AH_ROOT_FOLDER = "$profile:DME_AH\\";
const string DME_AH_CONFIG_DIR = DME_AH_ROOT_FOLDER + "Config\\";
const string DME_AH_DATA_DIR = DME_AH_ROOT_FOLDER + "Data\\";
const string DME_AH_LOG_DIR = DME_AH_ROOT_FOLDER + "Logging\\";
const string DME_AH_SETTINGS_FILE = DME_AH_CONFIG_DIR + "Settings.json";
const string DME_AH_CATEGORIES_FILE = DME_AH_CONFIG_DIR + "Categories.json";
const string DME_AH_ACTIVE_LISTINGS_FILE = DME_AH_DATA_DIR + "ActiveListings.json";
const string DME_AH_COMPLETED_LISTINGS_FILE = DME_AH_DATA_DIR + "CompletedListings.json";
const string DME_AH_PLAYER_DATA_FILE = DME_AH_DATA_DIR + "PlayerData.json";

// Menu IDs
const int MENU_DME_AH_AUCTION = 10100;

// Pagination
const int DME_AH_LISTINGS_PER_PAGE = 20;
const int DME_AH_HISTORY_PER_PAGE = 20;

// Expiry check interval (seconds)
const float DME_AH_EXPIRY_CHECK_INTERVAL = 60.0;

// Notification duration (seconds)
const float DME_AH_NOTIFICATION_DURATION = 5.0;
