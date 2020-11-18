//#define TELEGRAM_DEBUG 1

#define HTTP_BUFFER_SIZE 10


#define MENU_STATE_COUNT 4

#define USER_CACHE_SIZE 30

#define RANDOM_PUNISHMENT_PROBABILITY 5
#define RANDOM_SHOCK_AUTO_OFF_SECONDS (3*3600)
#define RANDOM_SHOCKS_PER_HOUR_MAX 120

#define DEFAULT_SESSION_LENGTH 86400

#define UTC_OFFSET 1

#define TIME_MINIMUM 1800
#define TIME_MAXIMUM 7200

#define SLEEP_TIME_BEGIN 23
#define SLEEP_TIME_END_WORKDAYS 7
#define SLEEP_TIME_END_WEEKEND 10

#define SHOCKS_PER_SESSION 50
#define SHOCK_BACKFILL_PROBABILITY 100
#define SHOCK_NEEDINESS_THRESHOLD 60.0
#define SHOCK_COUNT_MAXMIMUM 30
#define SHOCK_DURATION_MINIMUM 1000
#define SHOCK_DURATION_DEFAULT 2000
#define SHOCK_DURATION_MAXIMUM 20000
#define SHOCK_BREAK_DURATION 2000

#define UNLOCK_DURATION 4000

#define VERIFICATIONS_MAX 5
#define VERIFICATION_STATUS_BEFORE 0
#define VERIFICATION_STATUS_WAITING 1
#define VERIFICATION_STATUS_RECEIVED 2
#define DURATION_EARLY_VERIFICATION 3*3600L
#define DURATION_LATE_VERIFICATION 4*3600L

#define REQUIREMENTS_ENABLED false
#define REQUIREMENTS_REQUEST_MAX 20
#define REQUIREMENTS_LOCK_PER_SESSION 1000
#define REQUIREMENTS_REMOVAL_PROBABILITY 100

#define PILLORY_PROBABILITY 6
#define PILLORY_MINIMUM 1800
#define PILLORY_MAXIMUM 7200

// number of messages in history/reading
#define MESSAGE_HISTORY_SIZE 40
// length of a message summary entry (per message)
#define MESSAGE_SUMMARY_SIZE 60

#define NO_REPLY false
#define IS_REPLY true

#define FORCE true
#define OPTIONAL false
#define ONLY_NAME true
#define FULL false

#define SYMBOL_LOCK_CLOSED "\xf0\x9f\x94\x92"
#define SYMBOL_LOCK_OPEN "\xf0\x9f\x94\x93"
#define SYMBOL_LOCK_KEY "\xf0\x9f\x94\x90"
#define SYMBOL_KEY "\xf0\x9f\x94\x91"
#define SYMBOL_MOON "\xf0\x9f\x8c\x99"
#define SYMBOL_SUN "\xe2\x98\x80"
#define SYMBOL_WATCHING_EYES "\xf0\x9f\x91\x80"
#define SYMBOL_POLICEMAN "\xf0\x9f\x91\xae"
#define SYMBOL_QUEEN "\xf0\x9f\x91\xb8"
#define SYMBOL_DEVIL_SMILE "\xf0\x9f\x98\x88"
#define SYMBOL_DEVIL_ANGRY "\xf0\x9f\x91\xbf"
#define SYMBOL_BELL "\xf0\x9f\x94\x94"
#define SYMBOL_TWO_CHAIN_LINKS "\xf0\x9f\x94\x97"
#define SYMBOL_CLOCK_3 "\xf0\x9f\x95\x92"
#define SYMBOL_SMILY_SMILE "\xf0\x9f\x98\x8a"
#define SYMBOL_SMILY_WINK "\xf0\x9f\x98\x89"
#define SYMBOL_SMILE_OHOH "\xf0\x9f\x98\xb3"
#define SYMBOL_FORBIDDEN "\xf0\x9f\x9a\xab"
#define SYMBOL_CUSTOMS "\xf0\x9f\x9b\x82"
#define SYMBOL_STOP "\xf0\x9f\x9b\x91"
#define SYMBOL_OK "\xf0\x9f\x86\x97"
#define SYMBOL_EYE "\xf0\x9f\x91\x81"
#define SYMBOL_SUN "\xf0\x9f\x8c\x9e"
#define SYMBOL_NO_ENTRY "\xe2\x9b\x94"
#define SYMBOL_CHAINS "\xe2\x9b\x93"


// ---------------------------------
// Persisted values
#define LAST_OPENING_TAG "LAOP"
//#define LAST_OPENING_TAG "LastOpening"
#define LAST_CLOSING_TAG "LACL"
#define LAST_SHOCK_TAG "LASH"
//#define LAST_SHOCK_TAG "LastShock"
#define RANDOM_MODE_START_TAG "RAMS"
//#define RANDOM_MODE_START_TAG "RandomModeStart"
#define TEASING_MODE_TAG "TEMO"
//#define TEASING_MODE_TAG "TeasingMode"
#define RANDOM_MODE_TAG "RAMO"
//#define RANDOM_MODE_TAG "RandomMode"
#define VERIFICATION_MODE_TAG "VEMO"
#define CREDITS_TAG "CRED"
//#define CREDITS_TAG "Credits"

// ---------------------------------

#define SSID1     "YOURSSID1"
#define PASSWORD1 "YOURPASSWORD1"
#define SSID2     "YOURSSID2"
#define PASSWORD2 "YOURPASSWORD2"
#define SSID3     "YOURSSID3"
#define PASSWORD3 "YOURPASSWORD3"

#define WEARER_NAME "EMLALOCK_WEARERUSERNAME"
#define WEARER_ID "EMLALOCK_WEARER_USER_ID"
#define WEARER_APIKEY "EMLALOCK_WEARER_APIKEY"

#define MISTRESS_NAME "EMLALOCK_SHOCKCELL_USERNAME"
#define MISTRESS_PASSWORD "EMLALOCK_SHOCKCELL_USERNAME"
#define MISTRESS_ID "EMLALOCK_SHOCKCELL_USER_ID"
#define MISTRESS_APIKEY "EMLALOCK_SHOCKCELL_APIKEY"

// ---------------------------------

#define SERVO_PIN 2
#define SHOCK_PIN 4
#define COVER_OPEN_PIN 15
#define COVER_OPEN 0
#define COVER_CLOSED 1

// ---------------------------------
// TELEGRAM

#define BOT_REQUEST_INTERVAL 1000

#define GROUP_CHAT_ID "TELEGRAM_CHAT_ID_FOR_GROUP"
#define USER_ID_BOT "TELEGRAM_ID_BOT"
// Initialize Telegram bot
// your bot token (from Botfather)
#define BOT_TOKEN "YOUR_BOT_TOKEN"
#define USER_ID_WEARER "TELEGRAM_USER_ID_WEARER"
#define USER_NAME_WEARER "TELEGRAM_USER_NAME_WEARER"

// ---------------------------------
// IFTTT
#define IFTTT_API_KEY "IFTTT_API_KEY"

// ---------------------------------
// CHASTIKEY

#define CHASTIKEY_USER_ID "CHASTIKEY_USER_ID"

//
