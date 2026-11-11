/*
  ============================================================
  ESP8266 WIFI CLOCK
  DS3231 RTC
  32 x 8 WS2812 LED MATRIX
  OLED
  IR REMOTE

  DISPLAY:
    32 columns x 8 rows

  BRIGHTNESS:
    Night minimum = 1
    Day maximum   = 30

  DEFAULT TIMEZONE:
    Greece / EET-EEST

  WIFI / NTP:
    Connect and sync at power-up, then WiFi OFF.
    Reconnect only at the configured AUTOSYNC interval.
    Default interval = 72 hours.

  IR:
    1 = brightness down
    2 = brightness up
    7 = previous timezone
    8 = next timezone

  CENTER DISPLAY:
    Hour tens    X = 2
    Hour units   X = 8
    Minute tens  X = 19
    Minute units X = 25

  VERTICAL BREATHING SEPARATOR:
    Width = 2 pixels

    Rows from top:

       row 0     blank
       row 1     blank
       row 2     ██
       row 3     ██
       row 4     BLACK
       row 5     ██
       row 6     ██
       row 7     spare / BLACK

    Separator columns:
       X = 15 and X = 16

    This places the separator vertically in the center
    of the gap between the hour and minute digits.

  BRIGHTNESS PROTECTION:
    Night mode is NEVER allowed to increase brightness.

    The NeoPixel library's setBrightness() is destructive
    when used repeatedly. Therefore it is only changed
    before a complete redraw, never continuously during
    the breathing animation.
*/

// ============================================================
// LIBRARIES
// ============================================================

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <TZ.h>

// ============================================================
// TZ.H COMPATIBILITY FOR OLDER ESP8266 CORE RELEASES
// ============================================================
// Some ESP8266 core releases expose only the canonical IANA zone names,
// while newer TZ.h files also expose historical aliases / renamed zones.
// Keep the full timezone editor available without requiring a core update.
// These definitions are used only when the installed TZ.h does not already
// provide the corresponding name.

#ifndef TZ_Africa_Asmera
  #define TZ_Africa_Asmera TZ_Africa_Asmara
#endif
#ifndef TZ_Africa_Timbuktu
  #define TZ_Africa_Timbuktu TZ_Africa_Bamako
#endif

#ifndef TZ_America_Argentina_ComodRivadavia
  #define TZ_America_Argentina_ComodRivadavia TZ_America_Argentina_Catamarca
#endif
#ifndef TZ_America_Atka
  #define TZ_America_Atka TZ_America_Adak
#endif
#ifndef TZ_America_Buenos_Aires
  #define TZ_America_Buenos_Aires TZ_America_Argentina_Buenos_Aires
#endif
#ifndef TZ_America_Catamarca
  #define TZ_America_Catamarca TZ_America_Argentina_Catamarca
#endif
#ifndef TZ_America_Ciudad_Juarez
  #define TZ_America_Ciudad_Juarez PSTR("MST7MDT,M3.2.0,M11.1.0")
#endif
#ifndef TZ_America_Coral_Harbour
  #define TZ_America_Coral_Harbour TZ_America_Atikokan
#endif
#ifndef TZ_America_Cordoba
  #define TZ_America_Cordoba TZ_America_Argentina_Cordoba
#endif
#ifndef TZ_America_Ensenada
  #define TZ_America_Ensenada TZ_America_Tijuana
#endif
#ifndef TZ_America_Fort_Wayne
  #define TZ_America_Fort_Wayne TZ_America_Indiana_Indianapolis
#endif
#ifndef TZ_America_Indianapolis
  #define TZ_America_Indianapolis TZ_America_Indiana_Indianapolis
#endif
#ifndef TZ_America_Jujuy
  #define TZ_America_Jujuy TZ_America_Argentina_Jujuy
#endif
#ifndef TZ_America_Knox_IN
  #define TZ_America_Knox_IN TZ_America_Indiana_Knox
#endif
#ifndef TZ_America_Louisville
  #define TZ_America_Louisville TZ_America_Kentucky_Louisville
#endif
#ifndef TZ_America_Mendoza
  #define TZ_America_Mendoza TZ_America_Argentina_Mendoza
#endif
#ifndef TZ_America_Porto_Acre
  #define TZ_America_Porto_Acre TZ_America_Rio_Branco
#endif
#ifndef TZ_America_Rosario
  #define TZ_America_Rosario TZ_America_Argentina_Cordoba
#endif
#ifndef TZ_America_Santa_Isabel
  #define TZ_America_Santa_Isabel TZ_America_Tijuana
#endif
#ifndef TZ_America_Shiprock
  #define TZ_America_Shiprock TZ_America_Denver
#endif
#ifndef TZ_America_Virgin
  #define TZ_America_Virgin TZ_America_St_Thomas
#endif

#ifndef TZ_Antarctica_South_Pole
  #define TZ_Antarctica_South_Pole TZ_Pacific_Auckland
#endif

#ifndef TZ_Asia_Ashkhabad
  #define TZ_Asia_Ashkhabad TZ_Asia_Ashgabat
#endif
#ifndef TZ_Asia_Calcutta
  #define TZ_Asia_Calcutta TZ_Asia_Kolkata
#endif
#ifndef TZ_Asia_Chongqing
  #define TZ_Asia_Chongqing TZ_Asia_Shanghai
#endif
#ifndef TZ_Asia_Chungking
  #define TZ_Asia_Chungking TZ_Asia_Shanghai
#endif
#ifndef TZ_Asia_Dacca
  #define TZ_Asia_Dacca TZ_Asia_Dhaka
#endif
#ifndef TZ_Asia_Harbin
  #define TZ_Asia_Harbin TZ_Asia_Shanghai
#endif
#ifndef TZ_Asia_Istanbul
  #define TZ_Asia_Istanbul TZ_Europe_Istanbul
#endif
#ifndef TZ_Asia_Kashgar
  #define TZ_Asia_Kashgar TZ_Asia_Urumqi
#endif
#ifndef TZ_Asia_Katmandu
  #define TZ_Asia_Katmandu TZ_Asia_Kathmandu
#endif
#ifndef TZ_Asia_Macao
  #define TZ_Asia_Macao TZ_Asia_Macau
#endif
#ifndef TZ_Asia_Qostanay
  // Kazakhstan uses UTC+5 nationwide; this covers current Qostanay time.
  #define TZ_Asia_Qostanay PSTR("<+05>-5")
#endif
#ifndef TZ_Asia_Rangoon
  #define TZ_Asia_Rangoon TZ_Asia_Yangon
#endif
#ifndef TZ_Asia_Saigon
  #define TZ_Asia_Saigon TZ_Asia_Ho_Chi_Minh
#endif
#ifndef TZ_Asia_Tel_Aviv
  #define TZ_Asia_Tel_Aviv TZ_Asia_Jerusalem
#endif
#ifndef TZ_Asia_Thimbu
  #define TZ_Asia_Thimbu TZ_Asia_Thimphu
#endif
#ifndef TZ_Asia_Ujung_Pandang
  #define TZ_Asia_Ujung_Pandang TZ_Asia_Makassar
#endif
#ifndef TZ_Asia_Ulan_Bator
  #define TZ_Asia_Ulan_Bator TZ_Asia_Ulaanbaatar
#endif

#ifndef TZ_Atlantic_Faeroe
  #define TZ_Atlantic_Faeroe TZ_Atlantic_Faroe
#endif
#ifndef TZ_Atlantic_Jan_Mayen
  #define TZ_Atlantic_Jan_Mayen TZ_Europe_Oslo
#endif

#ifndef TZ_Australia_ACT
  #define TZ_Australia_ACT TZ_Australia_Sydney
#endif
#ifndef TZ_Australia_Canberra
  #define TZ_Australia_Canberra TZ_Australia_Sydney
#endif
#ifndef TZ_Australia_LHI
  #define TZ_Australia_LHI TZ_Australia_Lord_Howe
#endif
#ifndef TZ_Australia_NSW
  #define TZ_Australia_NSW TZ_Australia_Sydney
#endif
#ifndef TZ_Australia_North
  #define TZ_Australia_North TZ_Australia_Darwin
#endif
#ifndef TZ_Australia_Queensland
  #define TZ_Australia_Queensland TZ_Australia_Brisbane
#endif
#ifndef TZ_Australia_South
  #define TZ_Australia_South TZ_Australia_Adelaide
#endif
#ifndef TZ_Australia_Tasmania
  #define TZ_Australia_Tasmania TZ_Australia_Hobart
#endif
#ifndef TZ_Australia_Victoria
  #define TZ_Australia_Victoria TZ_Australia_Melbourne
#endif
#ifndef TZ_Australia_West
  #define TZ_Australia_West TZ_Australia_Perth
#endif
#ifndef TZ_Australia_Yancowinna
  #define TZ_Australia_Yancowinna TZ_Australia_Broken_Hill
#endif

#ifndef TZ_Europe_Belfast
  #define TZ_Europe_Belfast TZ_Europe_London
#endif
#ifndef TZ_Europe_Kyiv
  #define TZ_Europe_Kyiv TZ_Europe_Kiev
#endif
#ifndef TZ_Europe_Nicosia
  // Cyprus current rule, used when an older core exposes no Europe/Nicosia alias.
  #define TZ_Europe_Nicosia PSTR("EET-2EEST,M3.5.0/3,M10.5.0/4")
#endif
#ifndef TZ_Europe_Tiraspol
  #define TZ_Europe_Tiraspol TZ_Europe_Chisinau
#endif

#ifndef TZ_Pacific_Johnston
  #define TZ_Pacific_Johnston TZ_Pacific_Honolulu
#endif
#ifndef TZ_Pacific_Kanton
  #define TZ_Pacific_Kanton PSTR("<+13>-13")
#endif
#ifndef TZ_Pacific_Ponape
  #define TZ_Pacific_Ponape TZ_Pacific_Pohnpei
#endif
#ifndef TZ_Pacific_Samoa
  #define TZ_Pacific_Samoa TZ_Pacific_Pago_Pago
#endif
#ifndef TZ_Pacific_Truk
  #define TZ_Pacific_Truk TZ_Pacific_Chuuk
#endif
#ifndef TZ_Pacific_Yap
  #define TZ_Pacific_Yap TZ_Pacific_Chuuk
#endif

#ifndef TZ_UTC
  #define TZ_UTC PSTR("UTC0")
#endif

#include <WiFiManager.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Adafruit_NeoPixel.h>
#include <EEPROM.h>
#include <U8g2lib.h>
#include <RTClib.h>

// ============================================================
// PINS
// ============================================================

#define LED_PIN       13
#define IR_PIN        14
#define BUTTON_PIN    0

#define OLED_SDA      4
#define OLED_SCL      5

// ============================================================
// MATRIX
// ============================================================

#define MATRIX_WIDTH  32
#define MATRIX_HEIGHT 8
#define NUM_LEDS      256

// ============================================================
// BRIGHTNESS
// ============================================================

#define BRIGHTNESS_MIN 1
#define BRIGHTNESS_MAX 30
#define BRIGHTNESS_DEFAULT 6

int dayBrightness = BRIGHTNESS_DEFAULT;
int currentBrightness = BRIGHTNESS_DEFAULT;

bool isNightDimmingMode = false;

// ============================================================
// CLOCK DISPLAY POSITIONS
// ============================================================
//
// 32 columns:
//
//   00 01 02 03 04 05 06 07 08 09 ...
//
// Four 5-column digits:
//
//   HOUR TENS   = 2
//   HOUR UNITS  = 8
//   MIN TENS    = 19
//   MIN UNITS   = 25
//
// This gives:
//
//       2..6   8..12      19..23   25..29
//
// Overall: column 2 through 29
// Blank: 2 columns left and 2 columns right.
//
// ============================================================

#define HOUR_TENS_X    2
#define HOUR_UNITS_X   8
#define MIN_TENS_X     19
#define MIN_UNITS_X    25

#define DIGIT_Y        1

// ============================================================
// VERTICAL BREATHING SEPARATOR
// ============================================================
//
// The separator is now VERTICAL.
//
// It occupies two columns:
//
//       X15 X16
//
// From top to bottom:
//
//       row 0       blank
//       row 1       blank
//       row 2       ██
//       row 3       ██
//       row 4       BLACK
//       row 5       ██
//       row 6       ██
//       row 7       spare / BLACK
//
// The two-pixel-wide sections breathe together.
//
// ============================================================

#define SPARK_LEFT_X       15
#define SPARK_RIGHT_X      16

#define SPARK_TOP_Y        2
#define SPARK_TOP_Y2       3

#define SPARK_BLACK_Y      4

#define SPARK_BOTTOM_Y     5
#define SPARK_BOTTOM_Y2    6

#define SPARK_SPARE_Y      7

// ============================================================
// EEPROM
// ============================================================

#define EEPROM_SIZE             128

#define EEPROM_DIM_START_HOUR   0
#define EEPROM_DIM_START_MIN    1
#define EEPROM_DIM_END_HOUR     2
#define EEPROM_DIM_END_MIN      3

#define EEPROM_BRIGHTNESS       4
#define EEPROM_TIMEZONE         5

// Addresses 6..9 belonged to the original single Siesta daytime period.
// They are kept reserved so an upgrade does not reinterpret old data.
#define EEPROM_LEGACY_SIESTA_0   6
#define EEPROM_LEGACY_SIESTA_1   7
#define EEPROM_LEGACY_SIESTA_2   8
#define EEPROM_LEGACY_SIESTA_3   9

#define EEPROM_COLOR_INDEX      10
#define EEPROM_SYNC_HOURS       11

// EEPROM addresses 12..14 are intentionally left unused.
#define EEPROM_DAY_COLOR_MODE   15
#define EEPROM_COLOR_MODE_MAGIC 16
#define COLOR_MODE_MAGIC_VALUE  0xC7

// Seasonal Greek quiet-hours / Siesta configuration.
// 24 bytes of data + one magic byte.
#define EEPROM_SIESTA_SUM_DAY_SH     17
#define EEPROM_SIESTA_SUM_DAY_SM     18
#define EEPROM_SIESTA_SUM_DAY_EH     19
#define EEPROM_SIESTA_SUM_DAY_EM     20
#define EEPROM_SIESTA_SUM_NIGHT_SH   21
#define EEPROM_SIESTA_SUM_NIGHT_SM   22
#define EEPROM_SIESTA_SUM_NIGHT_EH   23
#define EEPROM_SIESTA_SUM_NIGHT_EM   24
#define EEPROM_SIESTA_SUM_START_D    25
#define EEPROM_SIESTA_SUM_START_MO   26
#define EEPROM_SIESTA_SUM_END_D      27
#define EEPROM_SIESTA_SUM_END_MO     28

#define EEPROM_SIESTA_WIN_DAY_SH     29
#define EEPROM_SIESTA_WIN_DAY_SM     30
#define EEPROM_SIESTA_WIN_DAY_EH     31
#define EEPROM_SIESTA_WIN_DAY_EM     32
#define EEPROM_SIESTA_WIN_NIGHT_SH   33
#define EEPROM_SIESTA_WIN_NIGHT_SM   34
#define EEPROM_SIESTA_WIN_NIGHT_EH   35
#define EEPROM_SIESTA_WIN_NIGHT_EM   36
#define EEPROM_SIESTA_WIN_START_D    37
#define EEPROM_SIESTA_WIN_START_MO   38
#define EEPROM_SIESTA_WIN_END_D      39
#define EEPROM_SIESTA_WIN_END_MO     40

#define EEPROM_SIESTA_SEASONS_MAGIC  41
#define SIESTA_SEASONS_MAGIC_VALUE   0xA6

// Display-format settings added after the finalized seasonal-Siesta build.
// A magic byte ensures an upgrade keeps the established behavior:
// 12-hour display + Style 1 unless the user explicitly changes it.
#define EEPROM_HOUR_FORMAT_24        42
#define EEPROM_CLOCK_FONT_STYLE      43
#define EEPROM_DISPLAY_SETTINGS_MAGIC 44
#define DISPLAY_SETTINGS_MAGIC_VALUE  0xD4

// OLED user-interface language.  English remains the safe default
// for upgrades from every previous firmware revision.
#define EEPROM_UI_LANGUAGE             45
#define EEPROM_UI_LANGUAGE_MAGIC       46
#define UI_LANGUAGE_MAGIC_VALUE        0xE7

// Country + public-holiday settings. Existing addresses 0..46 remain
// untouched so upgrades preserve all established clock settings.
#define EEPROM_COUNTRY_INDEX            47
#define EEPROM_PUBLIC_HOLIDAYS_ENABLED  48
#define EEPROM_PUBLIC_HOLIDAY_HOUR      49
#define EEPROM_PUBLIC_HOLIDAY_MIN       50
#define EEPROM_HOLIDAY_SETTINGS_MAGIC   51
#define HOLIDAY_SETTINGS_MAGIC_VALUE    0xB6

// Cached nationwide public-holiday dates for one calendar year.
// 366 days need 46 bytes.
#define EEPROM_HOLIDAY_CACHE_YEAR_LO    52
#define EEPROM_HOLIDAY_CACHE_YEAR_HI    53
#define EEPROM_HOLIDAY_CACHE_COUNTRY    54
#define EEPROM_HOLIDAY_CACHE_MAGIC      55
#define HOLIDAY_CACHE_MAGIC_VALUE       0xC4
#define EEPROM_HOLIDAY_BITMAP_START     56
#define HOLIDAY_BITMAP_BYTES            46

// Compact timezone-rule index. The legacy byte at address 5 is preserved
// for one-time migration from the established pre-expanded nine-zone firmware.
#define EEPROM_TIMEZONE_EXT_LO          102
#define EEPROM_TIMEZONE_EXT_HI          103
#define EEPROM_TIMEZONE_EXT_MAGIC       104
#define TIMEZONE_EXT_MAGIC_VALUE        0xDA

// Holiday override rule selection added after v7.
// Bit 0 = nationwide public holidays
// Bit 1 = Saturdays
// Bit 2 = Sundays
#define EEPROM_HOLIDAY_RULES_MASK       105
#define EEPROM_HOLIDAY_RULES_MAGIC      106
#define HOLIDAY_RULES_MAGIC_VALUE       0xE2

// ============================================================
// COLORS
// ============================================================

#define COLOR_BLUE     Adafruit_NeoPixel::Color(0, 0, 255)
#define COLOR_ORANGE   Adafruit_NeoPixel::Color(255, 80, 0)
#define COLOR_WHITE    Adafruit_NeoPixel::Color(255, 255, 255)
#define COLOR_GREEN    Adafruit_NeoPixel::Color(0, 255, 0)
#define COLOR_PURPLE   Adafruit_NeoPixel::Color(128, 0, 255)
#define COLOR_RED      Adafruit_NeoPixel::Color(255, 0, 0)
#define COLOR_OFF      Adafruit_NeoPixel::Color(0, 0, 0)

// MONO COLOR palette.
// The first five indexes are intentionally identical to the previous
// firmware so an already stored color remains unchanged after update.
// Orange and orange-like colors are excluded because orange is reserved
// exclusively for Siesta mode.

const char* clockColorNames[] = {
  "Blue",
  "White",
  "Green",
  "Red",
  "Purple",
  "Cyan",
  "Aqua",
  "Turquoise",
  "Mint",
  "Lime",
  "Sky Blue",
  "Azure",
  "Indigo",
  "Violet",
  "Magenta",
  "Pink"
};

const char* clockColorNamesGreek[] = {
  "Μπλε",
  "Λευκό",
  "Πράσινο",
  "Κόκκινο",
  "Μωβ",
  "Κυανό",
  "Άκουα",
  "Τιρκουάζ",
  "Μέντα",
  "Λάιμ",
  "Γαλάζιο",
  "Αζούρ",
  "Ίντιγκο",
  "Βιολετί",
  "Ματζέντα",
  "Ροζ"
};

uint32_t clockColors[] = {
  Adafruit_NeoPixel::Color(0,   0, 255),   // Blue
  Adafruit_NeoPixel::Color(255,255,255),   // White
  Adafruit_NeoPixel::Color(0, 255,   0),   // Green
  Adafruit_NeoPixel::Color(255,  0,   0),  // Red
  Adafruit_NeoPixel::Color(128,  0, 255),  // Purple
  Adafruit_NeoPixel::Color(0, 255, 255),   // Cyan
  Adafruit_NeoPixel::Color(0, 190, 255),   // Aqua
  Adafruit_NeoPixel::Color(0, 255, 170),   // Turquoise
  Adafruit_NeoPixel::Color(90,255, 180),   // Mint
  Adafruit_NeoPixel::Color(120,255,  0),   // Lime
  Adafruit_NeoPixel::Color(70,160, 255),   // Sky Blue
  Adafruit_NeoPixel::Color(0, 120, 255),   // Azure
  Adafruit_NeoPixel::Color(60,  0, 255),   // Indigo
  Adafruit_NeoPixel::Color(180, 0, 255),   // Violet
  Adafruit_NeoPixel::Color(255, 0, 255),   // Magenta
  Adafruit_NeoPixel::Color(255,70, 170)    // Pink
};

#define NUM_CLOCK_COLORS (sizeof(clockColors) / sizeof(clockColors[0]))
int clockColorIndex = 0;

enum DayColorMode {
  DAY_COLOR_MONO = 0,
  DAY_COLOR_SOLID = 1,
  DAY_COLOR_COLORFUL = 2,
  DAY_COLOR_FUNNY = 3,
  DAY_COLOR_DYNAMIC = 4,
  DAY_COLOR_ANIMATED = 5
};

uint8_t dayColorMode = DAY_COLOR_MONO;

// Generated once per calendar day.  Values are reconstructed from the
// date and ESP8266 chip ID, therefore a reboot during the same day does
// not unexpectedly choose a different daytime appearance.
uint32_t dailyColorDateKey = 0;
uint8_t dailySolidPaletteIndex = 0;
uint8_t dailyDigitPaletteIndex[4] = {0, 0, 0, 0};
uint8_t dailyAccentPaletteIndex = 0;
uint8_t dailyFunnyOffset = 0;
uint8_t dailyFunnyStep = 53;

// ============================================================
// IR CODES - DIRECT NEC DECODER
// ============================================================
// Full two-byte command words exactly as shown by the external
// tester: command byte followed by its NEC complement byte.
// Address is always 0x00FF for this remote.

#define IR_ADDRESS       0x00FF

#define IR_BUTTON_0      0x16E9
#define IR_BUTTON_1      0x0CF3
#define IR_BUTTON_2      0x18E7
#define IR_BUTTON_3      0x5EA1
#define IR_BUTTON_4      0x08F7
#define IR_BUTTON_5      0x1CE3
#define IR_BUTTON_6      0x5AA5
#define IR_BUTTON_7      0x42BD
#define IR_BUTTON_8      0x52AD
#define IR_BUTTON_9      0x4AB5

#define IR_MOVE_RIGHT    0x09F6
#define IR_MOVE_LEFT     0x07F8
#define IR_ENTER         0x15EA

#define IR_MENU          0x47B8
#define IR_TEST          0x45BA
#define IR_UP            0x40BF
#define IR_DOWN          0x19E6
#define IR_BACK          0x43BC
#define IR_CANCEL        0x0DF2

// Direct NEC timing windows in microseconds.
#define NEC_HEADER_MARK_MIN    7500
#define NEC_HEADER_MARK_MAX   10500
#define NEC_HEADER_SPACE_MIN   3500
#define NEC_HEADER_SPACE_MAX   5500
#define NEC_REPEAT_SPACE_MIN   1700
#define NEC_REPEAT_SPACE_MAX   3000
#define NEC_BIT_MARK_MIN        300
#define NEC_BIT_MARK_MAX        900
#define NEC_ZERO_SPACE_MIN      250
#define NEC_ZERO_SPACE_MAX      950
#define NEC_ONE_SPACE_MIN      1100
#define NEC_ONE_SPACE_MAX      2300

// ============================================================
// COUNTRY + WORLD TIMEZONES
// ============================================================
// Greece is deliberately entry 0 in the Country menu.
//
// The timezone menu is intentionally compact. It contains one entry for
// each practical present/future civil-time rule used by populated regions:
// fixed UTC offsets plus the distinct DST/seasonal-transition rules that
// cannot be represented by a fixed offset alone. Europe/Athens is entry 0
// so the established Greece default remains unchanged. Country selection
// never changes timezone automatically.
//
// The compact grouping follows the purpose of IANA tzdb zonenow.tab:
// geographic zones whose clocks agree from now on do not need separate
// user-facing entries merely because they have different city names.

#define NUM_COUNTRIES 249
#define NUM_TIMEZONES 63

uint8_t countryIndex = 0;
int timezoneIndex = 0;

// Master HOLIDAYS switch.  The existing EEPROM byte at address 48 is
// deliberately retained for upgrade compatibility with v7.
bool holidaysEnabled = false;

// The three independent date rules used when HOLIDAYS is On.
bool holidayPublicDatesEnabled = true;
bool holidaySaturdaysEnabled = false;
bool holidaySundaysEnabled = false;

// Fresh-install default requested for the common Night Mode end override.
uint8_t holidayOverrideHour = 8;
uint8_t holidayOverrideMinute = 0;

// Temporary editor values. They are committed only when the time row is
// accepted, so BACK/CANCEL leaves the saved configuration unchanged.
bool editHolidayPublicDatesEnabled = true;
bool editHolidaySaturdaysEnabled = false;
bool editHolidaySundaysEnabled = false;

uint16_t holidayCacheYear = 0;
uint8_t holidayCacheCountry = 0;
uint8_t holidayBitmap[HOLIDAY_BITMAP_BYTES] = {0};
bool holidayCacheValid = false;

PGM_P countryNamePtr(uint8_t index) {
  switch (index) {
    case 0: return PSTR("Greece");
    case 1: return PSTR("Afghanistan");
    case 2: return PSTR("Aland Islands");
    case 3: return PSTR("Albania");
    case 4: return PSTR("Algeria");
    case 5: return PSTR("American Samoa");
    case 6: return PSTR("Andorra");
    case 7: return PSTR("Angola");
    case 8: return PSTR("Anguilla");
    case 9: return PSTR("Antarctica");
    case 10: return PSTR("Antigua and Barbuda");
    case 11: return PSTR("Argentina");
    case 12: return PSTR("Armenia");
    case 13: return PSTR("Aruba");
    case 14: return PSTR("Australia");
    case 15: return PSTR("Austria");
    case 16: return PSTR("Azerbaijan");
    case 17: return PSTR("Bahamas");
    case 18: return PSTR("Bahrain");
    case 19: return PSTR("Bangladesh");
    case 20: return PSTR("Barbados");
    case 21: return PSTR("Belarus");
    case 22: return PSTR("Belgium");
    case 23: return PSTR("Belize");
    case 24: return PSTR("Benin");
    case 25: return PSTR("Bermuda");
    case 26: return PSTR("Bhutan");
    case 27: return PSTR("Bolivia");
    case 28: return PSTR("Bonaire, Sint Eustatius and Saba");
    case 29: return PSTR("Bosnia and Herzegovina");
    case 30: return PSTR("Botswana");
    case 31: return PSTR("Bouvet Island");
    case 32: return PSTR("Brazil");
    case 33: return PSTR("British Indian Ocean Territory");
    case 34: return PSTR("Brunei Darussalam");
    case 35: return PSTR("Bulgaria");
    case 36: return PSTR("Burkina Faso");
    case 37: return PSTR("Burundi");
    case 38: return PSTR("Cabo Verde");
    case 39: return PSTR("Cambodia");
    case 40: return PSTR("Cameroon");
    case 41: return PSTR("Canada");
    case 42: return PSTR("Cayman Islands");
    case 43: return PSTR("Central African Republic");
    case 44: return PSTR("Chad");
    case 45: return PSTR("Chile");
    case 46: return PSTR("China");
    case 47: return PSTR("Christmas Island");
    case 48: return PSTR("Cocos (Keeling) Islands");
    case 49: return PSTR("Colombia");
    case 50: return PSTR("Comoros");
    case 51: return PSTR("Congo");
    case 52: return PSTR("Congo, The Democratic Republic of the");
    case 53: return PSTR("Cook Islands");
    case 54: return PSTR("Costa Rica");
    case 55: return PSTR("Cote d'Ivoire");
    case 56: return PSTR("Croatia");
    case 57: return PSTR("Cuba");
    case 58: return PSTR("Curacao");
    case 59: return PSTR("Cyprus");
    case 60: return PSTR("Czechia");
    case 61: return PSTR("Denmark");
    case 62: return PSTR("Djibouti");
    case 63: return PSTR("Dominica");
    case 64: return PSTR("Dominican Republic");
    case 65: return PSTR("Ecuador");
    case 66: return PSTR("Egypt");
    case 67: return PSTR("El Salvador");
    case 68: return PSTR("Equatorial Guinea");
    case 69: return PSTR("Eritrea");
    case 70: return PSTR("Estonia");
    case 71: return PSTR("Eswatini");
    case 72: return PSTR("Ethiopia");
    case 73: return PSTR("Falkland Islands (Malvinas)");
    case 74: return PSTR("Faroe Islands");
    case 75: return PSTR("Fiji");
    case 76: return PSTR("Finland");
    case 77: return PSTR("France");
    case 78: return PSTR("French Guiana");
    case 79: return PSTR("French Polynesia");
    case 80: return PSTR("French Southern Territories");
    case 81: return PSTR("Gabon");
    case 82: return PSTR("Gambia");
    case 83: return PSTR("Georgia");
    case 84: return PSTR("Germany");
    case 85: return PSTR("Ghana");
    case 86: return PSTR("Gibraltar");
    case 87: return PSTR("Greenland");
    case 88: return PSTR("Grenada");
    case 89: return PSTR("Guadeloupe");
    case 90: return PSTR("Guam");
    case 91: return PSTR("Guatemala");
    case 92: return PSTR("Guernsey");
    case 93: return PSTR("Guinea");
    case 94: return PSTR("Guinea-Bissau");
    case 95: return PSTR("Guyana");
    case 96: return PSTR("Haiti");
    case 97: return PSTR("Heard Island and McDonald Islands");
    case 98: return PSTR("Holy See (Vatican City State)");
    case 99: return PSTR("Honduras");
    case 100: return PSTR("Hong Kong");
    case 101: return PSTR("Hungary");
    case 102: return PSTR("Iceland");
    case 103: return PSTR("India");
    case 104: return PSTR("Indonesia");
    case 105: return PSTR("Iran");
    case 106: return PSTR("Iraq");
    case 107: return PSTR("Ireland");
    case 108: return PSTR("Isle of Man");
    case 109: return PSTR("Israel");
    case 110: return PSTR("Italy");
    case 111: return PSTR("Jamaica");
    case 112: return PSTR("Japan");
    case 113: return PSTR("Jersey");
    case 114: return PSTR("Jordan");
    case 115: return PSTR("Kazakhstan");
    case 116: return PSTR("Kenya");
    case 117: return PSTR("Kiribati");
    case 118: return PSTR("Kuwait");
    case 119: return PSTR("Kyrgyzstan");
    case 120: return PSTR("Laos");
    case 121: return PSTR("Latvia");
    case 122: return PSTR("Lebanon");
    case 123: return PSTR("Lesotho");
    case 124: return PSTR("Liberia");
    case 125: return PSTR("Libya");
    case 126: return PSTR("Liechtenstein");
    case 127: return PSTR("Lithuania");
    case 128: return PSTR("Luxembourg");
    case 129: return PSTR("Macao");
    case 130: return PSTR("Madagascar");
    case 131: return PSTR("Malawi");
    case 132: return PSTR("Malaysia");
    case 133: return PSTR("Maldives");
    case 134: return PSTR("Mali");
    case 135: return PSTR("Malta");
    case 136: return PSTR("Marshall Islands");
    case 137: return PSTR("Martinique");
    case 138: return PSTR("Mauritania");
    case 139: return PSTR("Mauritius");
    case 140: return PSTR("Mayotte");
    case 141: return PSTR("Mexico");
    case 142: return PSTR("Micronesia, Federated States of");
    case 143: return PSTR("Moldova");
    case 144: return PSTR("Monaco");
    case 145: return PSTR("Mongolia");
    case 146: return PSTR("Montenegro");
    case 147: return PSTR("Montserrat");
    case 148: return PSTR("Morocco");
    case 149: return PSTR("Mozambique");
    case 150: return PSTR("Myanmar");
    case 151: return PSTR("Namibia");
    case 152: return PSTR("Nauru");
    case 153: return PSTR("Nepal");
    case 154: return PSTR("Netherlands");
    case 155: return PSTR("New Caledonia");
    case 156: return PSTR("New Zealand");
    case 157: return PSTR("Nicaragua");
    case 158: return PSTR("Niger");
    case 159: return PSTR("Nigeria");
    case 160: return PSTR("Niue");
    case 161: return PSTR("Norfolk Island");
    case 162: return PSTR("North Korea");
    case 163: return PSTR("North Macedonia");
    case 164: return PSTR("Northern Mariana Islands");
    case 165: return PSTR("Norway");
    case 166: return PSTR("Oman");
    case 167: return PSTR("Pakistan");
    case 168: return PSTR("Palau");
    case 169: return PSTR("Palestine, State of");
    case 170: return PSTR("Panama");
    case 171: return PSTR("Papua New Guinea");
    case 172: return PSTR("Paraguay");
    case 173: return PSTR("Peru");
    case 174: return PSTR("Philippines");
    case 175: return PSTR("Pitcairn");
    case 176: return PSTR("Poland");
    case 177: return PSTR("Portugal");
    case 178: return PSTR("Puerto Rico");
    case 179: return PSTR("Qatar");
    case 180: return PSTR("Reunion");
    case 181: return PSTR("Romania");
    case 182: return PSTR("Russian Federation");
    case 183: return PSTR("Rwanda");
    case 184: return PSTR("Saint Barthelemy");
    case 185: return PSTR("Saint Helena, Ascension and Tristan da Cunha");
    case 186: return PSTR("Saint Kitts and Nevis");
    case 187: return PSTR("Saint Lucia");
    case 188: return PSTR("Saint Martin (French part)");
    case 189: return PSTR("Saint Pierre and Miquelon");
    case 190: return PSTR("Saint Vincent and the Grenadines");
    case 191: return PSTR("Samoa");
    case 192: return PSTR("San Marino");
    case 193: return PSTR("Sao Tome and Principe");
    case 194: return PSTR("Saudi Arabia");
    case 195: return PSTR("Senegal");
    case 196: return PSTR("Serbia");
    case 197: return PSTR("Seychelles");
    case 198: return PSTR("Sierra Leone");
    case 199: return PSTR("Singapore");
    case 200: return PSTR("Sint Maarten (Dutch part)");
    case 201: return PSTR("Slovakia");
    case 202: return PSTR("Slovenia");
    case 203: return PSTR("Solomon Islands");
    case 204: return PSTR("Somalia");
    case 205: return PSTR("South Africa");
    case 206: return PSTR("South Georgia and the South Sandwich Islands");
    case 207: return PSTR("South Korea");
    case 208: return PSTR("South Sudan");
    case 209: return PSTR("Spain");
    case 210: return PSTR("Sri Lanka");
    case 211: return PSTR("Sudan");
    case 212: return PSTR("Suriname");
    case 213: return PSTR("Svalbard and Jan Mayen");
    case 214: return PSTR("Sweden");
    case 215: return PSTR("Switzerland");
    case 216: return PSTR("Syria");
    case 217: return PSTR("Taiwan");
    case 218: return PSTR("Tajikistan");
    case 219: return PSTR("Tanzania");
    case 220: return PSTR("Thailand");
    case 221: return PSTR("Timor-Leste");
    case 222: return PSTR("Togo");
    case 223: return PSTR("Tokelau");
    case 224: return PSTR("Tonga");
    case 225: return PSTR("Trinidad and Tobago");
    case 226: return PSTR("Tunisia");
    case 227: return PSTR("Turkiye");
    case 228: return PSTR("Turkmenistan");
    case 229: return PSTR("Turks and Caicos Islands");
    case 230: return PSTR("Tuvalu");
    case 231: return PSTR("Uganda");
    case 232: return PSTR("Ukraine");
    case 233: return PSTR("United Arab Emirates");
    case 234: return PSTR("United Kingdom");
    case 235: return PSTR("United States");
    case 236: return PSTR("United States Minor Outlying Islands");
    case 237: return PSTR("Uruguay");
    case 238: return PSTR("Uzbekistan");
    case 239: return PSTR("Vanuatu");
    case 240: return PSTR("Venezuela");
    case 241: return PSTR("Vietnam");
    case 242: return PSTR("Virgin Islands, British");
    case 243: return PSTR("Virgin Islands, U.S.");
    case 244: return PSTR("Wallis and Futuna");
    case 245: return PSTR("Western Sahara");
    case 246: return PSTR("Yemen");
    case 247: return PSTR("Zambia");
    case 248: return PSTR("Zimbabwe");
    default: return PSTR("Greece");
  }
}

PGM_P countryCodePtr(uint8_t index) {
  switch (index) {
    case 0: return PSTR("GR");
    case 1: return PSTR("AF");
    case 2: return PSTR("AX");
    case 3: return PSTR("AL");
    case 4: return PSTR("DZ");
    case 5: return PSTR("AS");
    case 6: return PSTR("AD");
    case 7: return PSTR("AO");
    case 8: return PSTR("AI");
    case 9: return PSTR("AQ");
    case 10: return PSTR("AG");
    case 11: return PSTR("AR");
    case 12: return PSTR("AM");
    case 13: return PSTR("AW");
    case 14: return PSTR("AU");
    case 15: return PSTR("AT");
    case 16: return PSTR("AZ");
    case 17: return PSTR("BS");
    case 18: return PSTR("BH");
    case 19: return PSTR("BD");
    case 20: return PSTR("BB");
    case 21: return PSTR("BY");
    case 22: return PSTR("BE");
    case 23: return PSTR("BZ");
    case 24: return PSTR("BJ");
    case 25: return PSTR("BM");
    case 26: return PSTR("BT");
    case 27: return PSTR("BO");
    case 28: return PSTR("BQ");
    case 29: return PSTR("BA");
    case 30: return PSTR("BW");
    case 31: return PSTR("BV");
    case 32: return PSTR("BR");
    case 33: return PSTR("IO");
    case 34: return PSTR("BN");
    case 35: return PSTR("BG");
    case 36: return PSTR("BF");
    case 37: return PSTR("BI");
    case 38: return PSTR("CV");
    case 39: return PSTR("KH");
    case 40: return PSTR("CM");
    case 41: return PSTR("CA");
    case 42: return PSTR("KY");
    case 43: return PSTR("CF");
    case 44: return PSTR("TD");
    case 45: return PSTR("CL");
    case 46: return PSTR("CN");
    case 47: return PSTR("CX");
    case 48: return PSTR("CC");
    case 49: return PSTR("CO");
    case 50: return PSTR("KM");
    case 51: return PSTR("CG");
    case 52: return PSTR("CD");
    case 53: return PSTR("CK");
    case 54: return PSTR("CR");
    case 55: return PSTR("CI");
    case 56: return PSTR("HR");
    case 57: return PSTR("CU");
    case 58: return PSTR("CW");
    case 59: return PSTR("CY");
    case 60: return PSTR("CZ");
    case 61: return PSTR("DK");
    case 62: return PSTR("DJ");
    case 63: return PSTR("DM");
    case 64: return PSTR("DO");
    case 65: return PSTR("EC");
    case 66: return PSTR("EG");
    case 67: return PSTR("SV");
    case 68: return PSTR("GQ");
    case 69: return PSTR("ER");
    case 70: return PSTR("EE");
    case 71: return PSTR("SZ");
    case 72: return PSTR("ET");
    case 73: return PSTR("FK");
    case 74: return PSTR("FO");
    case 75: return PSTR("FJ");
    case 76: return PSTR("FI");
    case 77: return PSTR("FR");
    case 78: return PSTR("GF");
    case 79: return PSTR("PF");
    case 80: return PSTR("TF");
    case 81: return PSTR("GA");
    case 82: return PSTR("GM");
    case 83: return PSTR("GE");
    case 84: return PSTR("DE");
    case 85: return PSTR("GH");
    case 86: return PSTR("GI");
    case 87: return PSTR("GL");
    case 88: return PSTR("GD");
    case 89: return PSTR("GP");
    case 90: return PSTR("GU");
    case 91: return PSTR("GT");
    case 92: return PSTR("GG");
    case 93: return PSTR("GN");
    case 94: return PSTR("GW");
    case 95: return PSTR("GY");
    case 96: return PSTR("HT");
    case 97: return PSTR("HM");
    case 98: return PSTR("VA");
    case 99: return PSTR("HN");
    case 100: return PSTR("HK");
    case 101: return PSTR("HU");
    case 102: return PSTR("IS");
    case 103: return PSTR("IN");
    case 104: return PSTR("ID");
    case 105: return PSTR("IR");
    case 106: return PSTR("IQ");
    case 107: return PSTR("IE");
    case 108: return PSTR("IM");
    case 109: return PSTR("IL");
    case 110: return PSTR("IT");
    case 111: return PSTR("JM");
    case 112: return PSTR("JP");
    case 113: return PSTR("JE");
    case 114: return PSTR("JO");
    case 115: return PSTR("KZ");
    case 116: return PSTR("KE");
    case 117: return PSTR("KI");
    case 118: return PSTR("KW");
    case 119: return PSTR("KG");
    case 120: return PSTR("LA");
    case 121: return PSTR("LV");
    case 122: return PSTR("LB");
    case 123: return PSTR("LS");
    case 124: return PSTR("LR");
    case 125: return PSTR("LY");
    case 126: return PSTR("LI");
    case 127: return PSTR("LT");
    case 128: return PSTR("LU");
    case 129: return PSTR("MO");
    case 130: return PSTR("MG");
    case 131: return PSTR("MW");
    case 132: return PSTR("MY");
    case 133: return PSTR("MV");
    case 134: return PSTR("ML");
    case 135: return PSTR("MT");
    case 136: return PSTR("MH");
    case 137: return PSTR("MQ");
    case 138: return PSTR("MR");
    case 139: return PSTR("MU");
    case 140: return PSTR("YT");
    case 141: return PSTR("MX");
    case 142: return PSTR("FM");
    case 143: return PSTR("MD");
    case 144: return PSTR("MC");
    case 145: return PSTR("MN");
    case 146: return PSTR("ME");
    case 147: return PSTR("MS");
    case 148: return PSTR("MA");
    case 149: return PSTR("MZ");
    case 150: return PSTR("MM");
    case 151: return PSTR("NA");
    case 152: return PSTR("NR");
    case 153: return PSTR("NP");
    case 154: return PSTR("NL");
    case 155: return PSTR("NC");
    case 156: return PSTR("NZ");
    case 157: return PSTR("NI");
    case 158: return PSTR("NE");
    case 159: return PSTR("NG");
    case 160: return PSTR("NU");
    case 161: return PSTR("NF");
    case 162: return PSTR("KP");
    case 163: return PSTR("MK");
    case 164: return PSTR("MP");
    case 165: return PSTR("NO");
    case 166: return PSTR("OM");
    case 167: return PSTR("PK");
    case 168: return PSTR("PW");
    case 169: return PSTR("PS");
    case 170: return PSTR("PA");
    case 171: return PSTR("PG");
    case 172: return PSTR("PY");
    case 173: return PSTR("PE");
    case 174: return PSTR("PH");
    case 175: return PSTR("PN");
    case 176: return PSTR("PL");
    case 177: return PSTR("PT");
    case 178: return PSTR("PR");
    case 179: return PSTR("QA");
    case 180: return PSTR("RE");
    case 181: return PSTR("RO");
    case 182: return PSTR("RU");
    case 183: return PSTR("RW");
    case 184: return PSTR("BL");
    case 185: return PSTR("SH");
    case 186: return PSTR("KN");
    case 187: return PSTR("LC");
    case 188: return PSTR("MF");
    case 189: return PSTR("PM");
    case 190: return PSTR("VC");
    case 191: return PSTR("WS");
    case 192: return PSTR("SM");
    case 193: return PSTR("ST");
    case 194: return PSTR("SA");
    case 195: return PSTR("SN");
    case 196: return PSTR("RS");
    case 197: return PSTR("SC");
    case 198: return PSTR("SL");
    case 199: return PSTR("SG");
    case 200: return PSTR("SX");
    case 201: return PSTR("SK");
    case 202: return PSTR("SI");
    case 203: return PSTR("SB");
    case 204: return PSTR("SO");
    case 205: return PSTR("ZA");
    case 206: return PSTR("GS");
    case 207: return PSTR("KR");
    case 208: return PSTR("SS");
    case 209: return PSTR("ES");
    case 210: return PSTR("LK");
    case 211: return PSTR("SD");
    case 212: return PSTR("SR");
    case 213: return PSTR("SJ");
    case 214: return PSTR("SE");
    case 215: return PSTR("CH");
    case 216: return PSTR("SY");
    case 217: return PSTR("TW");
    case 218: return PSTR("TJ");
    case 219: return PSTR("TZ");
    case 220: return PSTR("TH");
    case 221: return PSTR("TL");
    case 222: return PSTR("TG");
    case 223: return PSTR("TK");
    case 224: return PSTR("TO");
    case 225: return PSTR("TT");
    case 226: return PSTR("TN");
    case 227: return PSTR("TR");
    case 228: return PSTR("TM");
    case 229: return PSTR("TC");
    case 230: return PSTR("TV");
    case 231: return PSTR("UG");
    case 232: return PSTR("UA");
    case 233: return PSTR("AE");
    case 234: return PSTR("GB");
    case 235: return PSTR("US");
    case 236: return PSTR("UM");
    case 237: return PSTR("UY");
    case 238: return PSTR("UZ");
    case 239: return PSTR("VU");
    case 240: return PSTR("VE");
    case 241: return PSTR("VN");
    case 242: return PSTR("VG");
    case 243: return PSTR("VI");
    case 244: return PSTR("WF");
    case 245: return PSTR("EH");
    case 246: return PSTR("YE");
    case 247: return PSTR("ZM");
    case 248: return PSTR("ZW");
    default: return PSTR("GR");
  }
}

PGM_P timezoneNamePtr(uint16_t index) {
  switch (index) {
    case 0: return PSTR("UTC+02/+03 EU DST");
    case 1: return PSTR("UTC-11");
    case 2: return PSTR("UTC-10");
    case 3: return PSTR("UTC-10/-09 US DST");
    case 4: return PSTR("UTC-09:30");
    case 5: return PSTR("UTC-09");
    case 6: return PSTR("UTC-09/-08 US DST");
    case 7: return PSTR("UTC-08");
    case 8: return PSTR("UTC-08/-07 US DST");
    case 9: return PSTR("UTC-07");
    case 10: return PSTR("UTC-07/-06 US DST");
    case 11: return PSTR("UTC-06");
    case 12: return PSTR("UTC-06/-05 Chile");
    case 13: return PSTR("UTC-06/-05 US DST");
    case 14: return PSTR("UTC-05");
    case 15: return PSTR("UTC-05/-04 Cuba");
    case 16: return PSTR("UTC-05/-04 US DST");
    case 17: return PSTR("UTC-04");
    case 18: return PSTR("UTC-04/-03 Chile");
    case 19: return PSTR("UTC-04/-03 Atlantic");
    case 20: return PSTR("UTC-03:30/-02:30 DST");
    case 21: return PSTR("UTC-03");
    case 22: return PSTR("UTC-03/-02 DST");
    case 23: return PSTR("UTC-02");
    case 24: return PSTR("UTC-02/-01 EU DST");
    case 25: return PSTR("UTC-01");
    case 26: return PSTR("UTC-01/+00 EU DST");
    case 27: return PSTR("UTC+00");
    case 28: return PSTR("UTC+00/+01 EU DST");
    case 29: return PSTR("UTC+00/+02 Troll DST");
    case 30: return PSTR("UTC+01");
    case 31: return PSTR("UTC+01/+00 Morocco");
    case 32: return PSTR("UTC+01/+02 EU DST");
    case 33: return PSTR("UTC+02");
    case 34: return PSTR("UTC+02/+03 Egypt");
    case 35: return PSTR("UTC+02/+03 Lebanon");
    case 36: return PSTR("UTC+02/+03 Palestine");
    case 37: return PSTR("UTC+02/+03 Israel");
    case 38: return PSTR("UTC+03");
    case 39: return PSTR("UTC+03:30");
    case 40: return PSTR("UTC+04");
    case 41: return PSTR("UTC+04:30");
    case 42: return PSTR("UTC+05");
    case 43: return PSTR("UTC+05:30");
    case 44: return PSTR("UTC+05:45");
    case 45: return PSTR("UTC+06");
    case 46: return PSTR("UTC+06:30");
    case 47: return PSTR("UTC+07");
    case 48: return PSTR("UTC+08");
    case 49: return PSTR("UTC+08:45");
    case 50: return PSTR("UTC+09");
    case 51: return PSTR("UTC+09:30");
    case 52: return PSTR("UTC+09:30/+10:30 DST");
    case 53: return PSTR("UTC+10");
    case 54: return PSTR("UTC+10/+11 AUS DST");
    case 55: return PSTR("UTC+10:30/+11 DST");
    case 56: return PSTR("UTC+11");
    case 57: return PSTR("UTC+11/+12 DST");
    case 58: return PSTR("UTC+12");
    case 59: return PSTR("UTC+12/+13 NZ DST");
    case 60: return PSTR("UTC+12:45/+13:45 DST");
    case 61: return PSTR("UTC+13");
    case 62: return PSTR("UTC+14");
    default: return PSTR("UTC+02/+03 EU DST");
  }
}

PGM_P timezoneSpecPtr(uint16_t index) {
  switch (index) {
    case 0: return TZ_Europe_Athens;
    case 1: return TZ_Pacific_Pago_Pago;
    case 2: return TZ_Pacific_Tahiti;
    case 3: return TZ_America_Adak;
    case 4: return TZ_Pacific_Marquesas;
    case 5: return TZ_Pacific_Gambier;
    case 6: return TZ_America_Anchorage;
    case 7: return TZ_Pacific_Pitcairn;
    case 8: return TZ_America_Los_Angeles;
    case 9: return TZ_America_Phoenix;
    case 10: return TZ_America_Denver;
    case 11: return TZ_America_Mexico_City;
    case 12: return TZ_Pacific_Easter;
    case 13: return TZ_America_Chicago;
    case 14: return TZ_America_Lima;
    case 15: return TZ_America_Havana;
    case 16: return TZ_America_New_York;
    case 17: return TZ_America_Caracas;
    case 18: return TZ_America_Santiago;
    case 19: return TZ_America_Halifax;
    case 20: return TZ_America_St_Johns;
    case 21: return TZ_America_Sao_Paulo;
    case 22: return TZ_America_Miquelon;
    case 23: return TZ_America_Noronha;
    case 24: return TZ_America_Nuuk;
    case 25: return TZ_Atlantic_Cape_Verde;
    case 26: return TZ_Atlantic_Azores;
    case 27: return TZ_Africa_Abidjan;
    case 28: return TZ_Europe_London;
    case 29: return TZ_Antarctica_Troll;
    case 30: return TZ_Africa_Algiers;
    case 31: return TZ_Africa_Casablanca;
    case 32: return TZ_Europe_Paris;
    case 33: return TZ_Africa_Maputo;
    case 34: return TZ_Africa_Cairo;
    case 35: return TZ_Asia_Beirut;
    case 36: return TZ_Asia_Gaza;
    case 37: return TZ_Asia_Jerusalem;
    case 38: return TZ_Europe_Istanbul;
    case 39: return TZ_Asia_Tehran;
    case 40: return TZ_Asia_Dubai;
    case 41: return TZ_Asia_Kabul;
    case 42: return TZ_Asia_Tashkent;
    case 43: return TZ_Asia_Kolkata;
    case 44: return TZ_Asia_Kathmandu;
    case 45: return TZ_Asia_Dhaka;
    case 46: return TZ_Asia_Yangon;
    case 47: return TZ_Asia_Bangkok;
    case 48: return TZ_Asia_Singapore;
    case 49: return TZ_Australia_Eucla;
    case 50: return TZ_Asia_Tokyo;
    case 51: return TZ_Australia_Darwin;
    case 52: return TZ_Australia_Adelaide;
    case 53: return TZ_Asia_Vladivostok;
    case 54: return TZ_Australia_Sydney;
    case 55: return TZ_Australia_Lord_Howe;
    case 56: return TZ_Pacific_Bougainville;
    case 57: return TZ_Pacific_Norfolk;
    case 58: return TZ_Asia_Kamchatka;
    case 59: return TZ_Pacific_Auckland;
    case 60: return TZ_Pacific_Chatham;
    case 61: return TZ_Pacific_Tongatapu;
    case 62: return TZ_Pacific_Kiritimati;
    default: return TZ_Europe_Athens;
  }
}

void copyProgmemString(PGM_P source, char* dest, size_t size) {
  if (!dest || size == 0) return;
  strncpy_P(dest, source, size - 1);
  dest[size - 1] = '\0';
}

void getCountryName(uint8_t index, char* dest, size_t size) {
  copyProgmemString(countryNamePtr(index), dest, size);
}

void getCountryCode(uint8_t index, char* dest, size_t size) {
  copyProgmemString(countryCodePtr(index), dest, size);
}

void getTimezoneName(uint16_t index, char* dest, size_t size) {
  copyProgmemString(timezoneNamePtr(index), dest, size);
}

void getTimezoneSpec(uint16_t index, char* dest, size_t size) {
  copyProgmemString(timezoneSpecPtr(index), dest, size);
}

void applyTimezone(uint16_t index) {
  char spec[80];
  getTimezoneSpec(index, spec, sizeof(spec));
  setenv("TZ", spec, 1);
  tzset();
}

// ============================================================
// OBJECTS
// ============================================================

Adafruit_NeoPixel matrix =
  Adafruit_NeoPixel(
    NUM_LEDS,
    LED_PIN,
    NEO_GRB + NEO_KHZ800
  );

WiFiUDP ntpUDP;

NTPClient timeClient(
  ntpUDP,
  "pool.ntp.org",
  0,
  60000
);

RTC_DS3231 rtc;

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(
  U8G2_R0
);

// ============================================================
// OLED STATES
// ============================================================

enum OLEDState {

  OLED_OFF,
  OLED_WELCOME,
  OLED_WIFI_CONNECTING,
  OLED_WIFI_CONFIG,
  OLED_TIME_SYNC,
  OLED_IR_COMMAND,
  OLED_DIM_EDITING,
  OLED_ERROR,
  OLED_MANUAL_TIME
};

OLEDState currentOLEDState = OLED_OFF;

unsigned long oledOffTimer = 0;

const unsigned long OLED_DISPLAY_DURATION = 5000;

// ============================================================
// OLED USER-INTERFACE LANGUAGE
// ============================================================
//
// This setting affects ONLY OLED text.  Matrix time rendering,
// 12/24-hour logic, fonts, colors, Siesta, Night Mode, RTC, WiFi,
// NTP and IR behavior are unchanged.
//

enum UILanguage {
  UI_ENGLISH = 0,
  UI_GREEK = 1
};

uint8_t uiLanguage = UI_ENGLISH;

bool isGreekUI() {
  return uiLanguage == UI_GREEK;
}

// ============================================================
// DIM SCHEDULE
// ============================================================

int dimStartHour = 23;
int dimStartMin  = 0;

int dimEndHour = 7;
int dimEndMin  = 0;

bool isEditingDimTime = false;

int editStep = 0;

// ============================================================
// OTHER STATES
// ============================================================

bool isPurpleOverride = false;

unsigned long normalLightTimer = 0;

bool rtcInitialized = false;
bool timeSyncSuccess = false;

bool isEditingTimeManually = false;

int manualTimeEditStep = 0;

int manualEditHour = 0;
int manualEditMinute = 0;

unsigned long lastSyncAttempt = 0;

#define DEFAULT_SYNC_HOURS 24

uint8_t syncIntervalHours =
  DEFAULT_SYNC_HOURS;

String lastKnownIP   = "0.0.0.0";

unsigned long lastRedFlash = 0;

bool redFlashState = false;

unsigned long lastManualEditFlash = 0;

bool colonVisible = true;

unsigned long lastBreathingUpdate = 0;


// ============================================================
// EDITOR / DIRECT IR STATE
// ============================================================

struct SiestaSeasonSettings {
  uint8_t dayStartHour;
  uint8_t dayStartMin;
  uint8_t dayEndHour;
  uint8_t dayEndMin;

  uint8_t nightStartHour;
  uint8_t nightStartMin;
  uint8_t nightEndHour;
  uint8_t nightEndMin;

  uint8_t startDay;
  uint8_t startMonth;
  uint8_t endDay;
  uint8_t endMonth;
};

// Defaults follow the Greek Police quiet-hours schedule:
// Summer: 01/04..30/09, 15:00..17:30 and 23:00..07:00.
// Winter: 01/10..31/03, 15:30..17:30 and 22:00..07:30.
SiestaSeasonSettings summerSiesta = {
  15, 0, 17, 30,
  23, 0, 7, 0,
  1, 4, 30, 9
};

SiestaSeasonSettings winterSiesta = {
  15, 30, 17, 30,
  22, 0, 7, 30,
  1, 10, 31, 3
};

// Siesta settings are edited transactionally through eight screens.
// Nothing is written to EEPROM until all eight entries pass validation.
SiestaSeasonSettings editSummerSiesta;
SiestaSeasonSettings editWinterSiesta;
uint8_t siestaEditStep = 0;

enum EditorPage {
  EDITOR_MAIN,
  EDITOR_COUNTRY,
  EDITOR_TIMEZONE,
  EDITOR_TIME,
  EDITOR_HOUR_FORMAT,
  EDITOR_FONT,
  EDITOR_SIESTA,
  EDITOR_NIGHT,
  EDITOR_HOLIDAYS,
  EDITOR_HOLIDAY_OPTIONS,
  EDITOR_DIMMER,
  EDITOR_SYNC,
  EDITOR_MONO_COLOR,
  EDITOR_MULTI_COLOR,
  EDITOR_LANGUAGE
};

bool editorActive = false;
EditorPage editorPage = EDITOR_MAIN;
int editorMenuIndex = 0;
int editorSelection = 0;
int editorCursor = 0;
unsigned long editorLastActivity = 0;
unsigned long selectedUntil = 0;
bool selectedMessageActive = false;

int editDigits[8] = {0,0,0,0,0,0,0,0};

bool testInfoActive = false;
unsigned long testInfoStarted = 0;
bool pendingTimezoneSync = false;

struct NECResult {
  bool valid;
  bool repeat;
  uint16_t addressCode;
  uint16_t commandCode;
};

uint16_t lastIRAddress = 0;
uint16_t lastIRCommand = 0;
bool haveLastIRCommand = false;

// ============================================================
// 5x7 NUMBER FONT
// ============================================================

const uint8_t font5x7[10][5] = {

  {0x3E, 0x41, 0x41, 0x41, 0x3E}, // 0 - plain zero, no slash
  {0x00, 0x42, 0x7F, 0x40, 0x00}, // 1
  {0x42, 0x61, 0x51, 0x49, 0x46}, // 2
  {0x21, 0x41, 0x45, 0x4B, 0x31}, // 3
  {0x18, 0x14, 0x12, 0x7F, 0x10}, // 4
  {0x27, 0x45, 0x45, 0x45, 0x39}, // 5
  {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 6
  {0x01, 0x71, 0x09, 0x05, 0x03}, // 7
  {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
  {0x06, 0x49, 0x49, 0x29, 0x1E}  // 9
};

// ============================================================
// TIME DISPLAY FORMAT + SELECTABLE CLOCK FONTS
// ============================================================
//
// Style 1 - Original / established favorite 5x7 font.
// Style 2 - Bold 5x7.
// Style 3 - PacClock 7x7, using the digit set previously approved
//           for the PacClock project. The source rows are stored
//           as 8-bit values; bit 0 is blank, so the visible glyph
//           width is seven pixels.
// Style 4 - Digital / squared 5x7.
// Style 5 - Rounded 5x7, replacing the rejected Slim design.
// Style 6 - Matrix Screen 5x7, a crisp terminal / LED-board style.
//
// The OLED deliberately names them only "Style 1" ... "Style 6".
// These font selections affect ONLY the time digits. Other matrix
// messages keep using the original Style 1 font through drawNumber().
//

#define NUM_CLOCK_FONT_STYLES 6

bool use24HourFormat = false;
uint8_t clockFontStyle = 0;  // 0 = Style 1

// Style 2 - Bold
const uint8_t fontStyle2[10][7] = {
  {0b01110,0b11011,0b11011,0b11011,0b11011,0b11011,0b01110}, // 0
  {0b00110,0b01110,0b00110,0b00110,0b00110,0b00110,0b01111}, // 1
  {0b11110,0b00011,0b00011,0b11110,0b11000,0b11000,0b11111}, // 2
  {0b11110,0b00011,0b00011,0b01110,0b00011,0b00011,0b11110}, // 3
  {0b11000,0b11011,0b11011,0b11111,0b00011,0b00011,0b00011}, // 4
  {0b11111,0b11000,0b11000,0b11110,0b00011,0b00011,0b11110}, // 5
  {0b01111,0b11000,0b11000,0b11110,0b11011,0b11011,0b01110}, // 6
  {0b11111,0b00011,0b00110,0b00110,0b01100,0b01100,0b01100}, // 7
  {0b01110,0b11011,0b11011,0b01110,0b11011,0b11011,0b01110}, // 8
  {0b01110,0b11011,0b11011,0b01111,0b00011,0b00011,0b11110}  // 9
};

// Style 3 - PacClock approved digit set.
// Seven visible pixels are bits 7..1 of every stored row.
// The zero is plain; there is no slash.
const uint8_t fontStyle3PacClock[10][7] = {
  {0x38,0x4C,0xC6,0xC6,0xC6,0x64,0x38}, // 0
  {0x18,0x38,0x18,0x18,0x18,0x18,0x7E}, // 1
  {0x7C,0xC6,0x0E,0x3C,0x78,0xE0,0xFE}, // 2
  {0x7E,0x0C,0x18,0x3C,0x06,0xC6,0x7C}, // 3
  {0x1C,0x3C,0x6C,0xCC,0xFE,0x0C,0x0C}, // 4
  {0xFC,0xC0,0xFC,0x06,0x06,0xC6,0x7C}, // 5
  {0x3C,0x60,0xC0,0xFC,0xC6,0xC6,0x7C}, // 6
  {0xFE,0xC6,0x0C,0x18,0x30,0x30,0x60}, // 7
  {0x78,0xC4,0xE4,0x78,0x9E,0x86,0x7C}, // 8
  {0x7C,0xC6,0xC6,0x7E,0x06,0x0C,0x78}  // 9
};

// Style 4 - Digital / squared
const uint8_t fontStyle4[10][7] = {
  {0b11111,0b10001,0b10001,0b10001,0b10001,0b10001,0b11111}, // 0
  {0b00110,0b00110,0b00110,0b00110,0b00110,0b00110,0b00110}, // 1
  {0b11111,0b00001,0b00001,0b11111,0b10000,0b10000,0b11111}, // 2
  {0b11111,0b00001,0b00001,0b11111,0b00001,0b00001,0b11111}, // 3
  {0b10001,0b10001,0b10001,0b11111,0b00001,0b00001,0b00001}, // 4
  {0b11111,0b10000,0b10000,0b11111,0b00001,0b00001,0b11111}, // 5
  {0b11111,0b10000,0b10000,0b11111,0b10001,0b10001,0b11111}, // 6
  {0b11111,0b00001,0b00001,0b00010,0b00100,0b01000,0b01000}, // 7
  {0b11111,0b10001,0b10001,0b11111,0b10001,0b10001,0b11111}, // 8
  {0b11111,0b10001,0b10001,0b11111,0b00001,0b00001,0b11111}  // 9
};

// Style 5 - Rounded / soft dot-matrix
const uint8_t fontStyle5[10][7] = {
  {0b01110,0b11011,0b10001,0b10001,0b10001,0b11011,0b01110}, // 0
  {0b00100,0b01100,0b11100,0b00100,0b00100,0b00100,0b01110}, // 1
  {0b01110,0b10011,0b00011,0b00110,0b01100,0b11000,0b11111}, // 2
  {0b11110,0b00011,0b00110,0b00011,0b00001,0b10011,0b01110}, // 3
  {0b00110,0b01110,0b11010,0b10010,0b11111,0b00010,0b00010}, // 4
  {0b11111,0b11000,0b11110,0b00011,0b00001,0b10011,0b01110}, // 5
  {0b00110,0b01100,0b11000,0b11110,0b10011,0b10011,0b01110}, // 6
  {0b11111,0b00011,0b00110,0b01100,0b11000,0b11000,0b11000}, // 7
  {0b01110,0b10011,0b10011,0b01110,0b10011,0b10011,0b01110}, // 8
  {0b01110,0b10011,0b10011,0b01111,0b00011,0b00110,0b01100}  // 9
};

// Style 6 - Matrix Screen / terminal-inspired
const uint8_t fontStyle6[10][7] = {
  {0b11110,0b10001,0b10001,0b10001,0b10001,0b10001,0b01111}, // 0
  {0b00100,0b01100,0b10100,0b00100,0b00100,0b00100,0b11111}, // 1
  {0b11110,0b00001,0b00001,0b01110,0b10000,0b10000,0b11111}, // 2
  {0b11110,0b00001,0b00001,0b01110,0b00001,0b00001,0b11110}, // 3
  {0b10010,0b10010,0b10010,0b11111,0b00010,0b00010,0b00111}, // 4
  {0b11111,0b10000,0b10000,0b11110,0b00001,0b00001,0b11110}, // 5
  {0b01111,0b10000,0b10000,0b11110,0b10001,0b10001,0b01110}, // 6
  {0b11111,0b00001,0b00010,0b00100,0b01000,0b01000,0b01000}, // 7
  {0b01110,0b10001,0b10001,0b01110,0b10001,0b10001,0b01110}, // 8
  {0b01110,0b10001,0b10001,0b01111,0b00001,0b00010,0b11100}  // 9
};

// ============================================================
// FUNCTION PROTOTYPES
// ============================================================

int matrixIndex(
  int x,
  int y
);

void drawNumber(
  int number,
  int startX,
  int startY,
  uint32_t color
);

void drawText(
  const char* text,
  uint32_t color
);

void drawBreathingSpark(
  uint32_t baseColor
);

void drawManualColon(
  uint32_t color
);

void drawAMPMLED(
  int hour,
  uint32_t color
);

void clearDisplay();

void updateDisplay();

void checkBrightnessMode(
  int hour,
  int minute
);

bool isSiestaDaytime(const DateTime &now);
bool isSiestaNighttime(const DateTime &now);
bool isTimeInPeriod(
  int hour,
  int minute,
  int startHour,
  int startMinute,
  int endHour,
  int endMinute
);
bool isValidMonthDay(int day, int month);
bool isValidSeasonBoundary(int day, int month);
bool dateInRange(
  int day,
  int month,
  int startDay,
  int startMonth,
  int endDay,
  int endMonth
);
const SiestaSeasonSettings* activeSiestaSeason(
  const DateTime &now
);

bool siestaTimePeriodsOverlap(
  const SiestaSeasonSettings &season
);

bool validateSiestaConfiguration(
  const SiestaSeasonSettings &summer,
  const SiestaSeasonSettings &winter
);

bool initializeRTC();

bool syncRTCWithNTP();

bool syncESPTimeWithRTC();

void setManualTimeRTC(
  int hour,
  int minute
);

void handleIRCommands();

void saveDimSettings();

void loadDimSettings();

void saveBrightness();

void loadBrightness();

void saveTimezone();

void loadTimezone();
void saveHolidaySettings();
void loadHolidaySettings();
void saveHolidayRuleSettings();
void loadHolidayRuleSettings();
void invalidateHolidayCache();
void loadHolidayCache();
bool downloadPublicHolidays(uint16_t year);
bool refreshPublicHolidayCache(bool connectIfNeeded);
bool isPublicHolidayDate(const DateTime &now);
bool isHolidayOverrideDate(const DateTime &now);
int dayOfYearIndex(int year, int month, int day);
void showHolidayDownloadResult(bool ok);
void drawHolidayOptionsPage();

bool setTimezone(
  int index
);

void changeTimezone(
  int direction
);

void showOLEDMessage(
  uint8_t state,
  String customMessage = ""
);

void turnOLEDOff();

void setupWiFiManager();

void showStartupCheckmark();
void showStartupGreen();
void disconnectWiFiCompletely();
bool reconnectSavedWiFi(unsigned long timeoutMs);
void attemptScheduledSync();
void performSyncNow();
unsigned long getSyncIntervalMs();
void saveSyncInterval();
void loadSyncInterval();

unsigned long measureIRLevel(uint8_t level, unsigned long timeoutUs);
NECResult readNECFrame();
int digitFromCommand(uint16_t commandCode);
void enterEditor();
void exitEditor();
void showEditor();
void handleEditorCommand(uint16_t commandCode);
void showSelectedAndExit();
void showTestInfo();
void loadExtraSettings();
void saveSiestaSettings();
void loadSiestaSettings();
void setDefaultSiestaSettings();
void loadSiestaEditStep();
bool commitSiestaEditStep();
void drawSiestaEditorPage();
void drawDateDigitsLine(int y);
bool readEditedDate(int &day, int &month);
void saveColorSetting();
void saveDayColorMode();
void loadDayColorMode();
void saveDisplaySettings();
void loadDisplaySettings();
void saveLanguageSetting();
void loadLanguageSetting();
int clockFontWidth(
  uint8_t style
);
bool clockFontPixel(
  uint8_t style,
  int number,
  int col,
  int row
);
uint32_t palette256(uint8_t paletteIndex);
bool isOrangePaletteIndex(uint8_t paletteIndex);
uint32_t mixColorSeed(uint32_t value);
void generateDailyColors(const DateTime &now);
void ensureDailyColors(const DateTime &now);
uint32_t funnyColorForPixel(int x, int y);
uint32_t animatedColorForPixel(int x, int y);
uint8_t dynamicBasePaletteIndex(const DateTime &now);
uint32_t dynamicColorForDigitPixel(
  uint8_t basePaletteIndex,
  int digitIndex,
  int col,
  int row
);
uint32_t scaleColor(uint32_t color, float factor);
void drawClockDigit(
  int number,
  int startX,
  int startY,
  uint32_t color,
  bool funnyMode,
  bool animatedMode,
  bool dynamicMode,
  uint8_t dynamicBaseIndex,
  int digitIndex
);

// ============================================================
// MATRIX MAPPING
// ============================================================

int matrixIndex(
  int x,
  int y
) {

  if (
    x < 0 ||
    x >= MATRIX_WIDTH ||
    y < 0 ||
    y >= MATRIX_HEIGHT
  ) {

    return -1;
  }

  //
  // Correct orientation for the physical
  // 8 x 32 serpentine board.
  //

  int physicalRow =
    MATRIX_WIDTH - 1 - x;

  int physicalColumn =
    MATRIX_HEIGHT - 1 - y;

  int index =
    physicalRow * MATRIX_HEIGHT;

  if (physicalRow & 1) {

    index +=
      MATRIX_HEIGHT -
      1 -
      physicalColumn;

  } else {

    index +=
      physicalColumn;
  }

  return index;
}

// ============================================================
// SETUP
// ============================================================

void setup() {


  // ----------------------------------------------------------
  // OLED
  // ----------------------------------------------------------

  u8g2.begin();

  u8g2.setFont(
    u8g2_font_ncenB08_tr
  );

  u8g2.setPowerSave(1);

  // ----------------------------------------------------------
  // BUTTON
  // ----------------------------------------------------------

  pinMode(
    BUTTON_PIN,
    INPUT_PULLUP
  );

  // ----------------------------------------------------------
  // MATRIX
  // ----------------------------------------------------------
  //
  // Keep the WS2812 data line LOW as early as possible and
  // initialize the matrix at the lowest possible brightness.
  // Nothing bright is ever intentionally shown at startup.
  //

  pinMode(
    LED_PIN,
    OUTPUT
  );

  digitalWrite(
    LED_PIN,
    LOW
  );

  matrix.begin();

  matrix.setBrightness(
    BRIGHTNESS_MIN
  );

  clearDisplay();
  matrix.show();

  // ----------------------------------------------------------
  // IR
  // ----------------------------------------------------------

  pinMode(
    IR_PIN,
    INPUT
  );

  // ----------------------------------------------------------
  // EEPROM
  // ----------------------------------------------------------

  EEPROM.begin(
    EEPROM_SIZE
  );

  loadDimSettings();

  loadBrightness();

  loadTimezone();
  loadExtraSettings();
  loadSyncInterval();
  loadDayColorMode();
  loadDisplaySettings();
  loadLanguageSetting();
  loadHolidaySettings();
  loadHolidayRuleSettings();
  loadHolidayCache();

  // ----------------------------------------------------------
  // FORCE GREECE IF EEPROM INVALID
  // ----------------------------------------------------------

  if (
    timezoneIndex < 0 ||
    timezoneIndex >= NUM_TIMEZONES
  ) {

    timezoneIndex = 0;
  }

  // ----------------------------------------------------------
  // OLED
  // ----------------------------------------------------------

  showOLEDMessage(
    OLED_WELCOME
  );

  // ----------------------------------------------------------
  // RTC
  // ----------------------------------------------------------

  if (!initializeRTC()) {


    showOLEDMessage(
      OLED_ERROR,
      "RTC Error!"
    );
  }

  // ----------------------------------------------------------
  // WIFI - STARTUP CONNECTION ONLY
  // ----------------------------------------------------------
  //
  // IMPORTANT:
  // Do NOT call WiFiManager.autoConnect() during normal startup.
  // autoConnect() can enter its configuration portal and wait there,
  // which leaves the OLED permanently showing "Connecting to WiFi".
  //
  // Normal startup now makes ONE bounded attempt (20 seconds) using
  // the credentials already stored by WiFiManager in the ESP8266.
  // If the network is unavailable, startup continues with the DS3231.
  // WiFiManager is used only when BUTTON_PIN is held LOW at boot.
  // ----------------------------------------------------------

  if (
    digitalRead(BUTTON_PIN) == LOW
  ) {

    showOLEDMessage(
      OLED_WIFI_CONFIG
    );

    setupWiFiManager();

  } else {

    showOLEDMessage(
      OLED_WIFI_CONNECTING
    );

    reconnectSavedWiFi(
      20000UL
    );
  }

  // ----------------------------------------------------------
  // INITIAL TIME SYNC
  // ----------------------------------------------------------

  if (
    WiFi.status() ==
    WL_CONNECTED
  ) {

    showOLEDMessage(
      OLED_TIME_SYNC
    );


    if (
      syncRTCWithNTP()
    ) {

      timeSyncSuccess =
        true;

      if (holidaysEnabled && holidayPublicDatesEnabled) {
        downloadPublicHolidays(rtc.now().year());
      }

    } else {

      timeSyncSuccess =
        false;

    }

  } else {

    timeSyncSuccess =
      false;

  }

  // ----------------------------------------------------------
  // WIFI OFF AFTER STARTUP SYNC ATTEMPT
  // ----------------------------------------------------------

  disconnectWiFiCompletely();

  lastSyncAttempt =
    millis();

  syncESPTimeWithRTC();

  turnOLEDOff();

  // ----------------------------------------------------------
  // LOW-BRIGHTNESS STARTUP ANIMATION
  // ----------------------------------------------------------
  //
  // Both the white checkmark and the following full green
  // screen are ALWAYS shown at BRIGHTNESS_MIN.
  //

  showStartupCheckmark();
  showStartupGreen();

  // ----------------------------------------------------------
  // NOW APPLY THE NORMAL DAY/NIGHT BRIGHTNESS
  // ----------------------------------------------------------

  if (rtcInitialized) {

    DateTime startupNow =
      rtc.now();

    checkBrightnessMode(
      startupNow.hour(),
      startupNow.minute()
    );
  }

  matrix.setBrightness(
    currentBrightness
  );

  updateDisplay();


}

// ============================================================
// MAIN LOOP
// ============================================================

void loop() {

  static unsigned long lastSecondUpdate = 0;

  // ----------------------------------------------------------
  // IR
  // ----------------------------------------------------------

  handleIRCommands();

  // Editor inactivity timeout: 15 seconds.
  if (
    editorActive &&
    !selectedMessageActive &&
    millis() - editorLastActivity >= 15000UL
  ) {
    exitEditor();
  }

  // After a successful selection, show only SELECTED for 3 seconds.
  if (
    editorActive &&
    selectedMessageActive &&
    (long)(millis() - selectedUntil) >= 0
  ) {
    selectedMessageActive = false;
    exitEditor();
  }

  // TEST information stays on OLED for 10 seconds.
  if (
    testInfoActive &&
    millis() - testInfoStarted >= 10000UL
  ) {
    testInfoActive = false;
    turnOLEDOff();
  }

  // ----------------------------------------------------------
  // OLED TIMEOUT
  // ----------------------------------------------------------

  if (
    !editorActive &&
    !testInfoActive &&
    currentOLEDState != OLED_OFF &&
    millis() - oledOffTimer >
      OLED_DISPLAY_DURATION
  ) {

    turnOLEDOff();
  }

  // ----------------------------------------------------------
  // PURPLE OVERRIDE
  // ----------------------------------------------------------
  //
  // IMPORTANT:
  //
  // If an IR command is received during night mode, the old
  // code deliberately changed currentBrightness to
  // dayBrightness.
  //
  // That was the direct path that could make the display
  // suddenly become bright during the night.
  //
  // The purple override is retained, but during night mode
  // it is FORCED to BRIGHTNESS_MIN.
  //
  // Therefore:
  //
  // NIGHT = never brighter than BRIGHTNESS_MIN
  //
  // regardless of IR commands.
  //

  if (
    !editorActive &&
    isPurpleOverride &&
    millis() - normalLightTimer >
      10000UL
  ) {

    isPurpleOverride = false;

    DateTime now =
      rtc.now();

    checkBrightnessMode(
      now.hour(),
      now.minute()
    );

    updateDisplay();
  }

  // ----------------------------------------------------------
  // AUTOSYNC
  // ----------------------------------------------------------
  //
  // WiFi stays completely OFF between AUTOSYNC sessions.
  // A new session is attempted only after the configured
  // number of hours has elapsed since the previous AUTOSYNC attempt.
  //

  if (
    millis() - lastSyncAttempt >=
    getSyncIntervalMs()
  ) {

    lastSyncAttempt =
      millis();

    attemptScheduledSync();
  }

  // ----------------------------------------------------------
  // NORMAL CLOCK UPDATE
  // ----------------------------------------------------------
  //
  // A failed NTP sync never interrupts the clock display.
  // The DS3231 remains the time source until the next attempt.
  //

  if (
    millis() - lastSecondUpdate >=
    1000
  ) {

    lastSecondUpdate =
      millis();

    updateDisplay();
  }

  // ----------------------------------------------------------
  // ANIMATED COLOR UPDATE
  // ----------------------------------------------------------
  //
  // Animated mode is the Funny per-pixel look in motion.
  // Refresh only this mode at 25 fps so color transitions remain
  // smooth. No other clock mode or timing behavior is changed.
  //

  if (
    !editorActive &&
    dayColorMode == DAY_COLOR_ANIMATED
  ) {

    static unsigned long lastAnimatedUpdate = 0;

    if (
      millis() - lastAnimatedUpdate >=
      40UL
    ) {

      lastAnimatedUpdate =
        millis();

      updateDisplay();
    }
  }

  // ----------------------------------------------------------
  // BREATHING SPARK
  // ----------------------------------------------------------

  if (
    !isEditingTimeManually
  ) {

    if (
      millis() - lastBreathingUpdate >=
      25
    ) {

      lastBreathingUpdate =
        millis();

      DateTime now =
        rtc.now();

      uint32_t sparkColor;

      if (isPurpleOverride) {

        sparkColor =
          COLOR_PURPLE;

      } else if (
        isSiestaDaytime(now)
      ) {

        sparkColor =
          COLOR_ORANGE;

      } else {

        sparkColor =
          COLOR_BLUE;
      }

      drawBreathingSpark(
        sparkColor
      );
    }
  }

  // ----------------------------------------------------------
  // MANUAL TIME COLON
  // ----------------------------------------------------------

  if (
    isEditingTimeManually
  ) {

    if (
      millis() - lastManualEditFlash >=
      500
    ) {

      lastManualEditFlash =
        millis();

      colonVisible =
        !colonVisible;

      if (colonVisible) {

        drawManualColon(
          COLOR_RED
        );

      } else {

        drawManualColon(
          COLOR_OFF
        );
      }

      matrix.show();
    }
  }
}

// ============================================================
// 256-COLOR DAY PALETTE
// ============================================================

uint32_t palette256(
  uint8_t paletteIndex
) {

  uint16_t hue =
    (uint16_t)paletteIndex *
    257U;

  return Adafruit_NeoPixel::ColorHSV(
    hue,
    255,
    255
  );
}

bool isOrangePaletteIndex(
  uint8_t paletteIndex
) {

  // Approximately 11 to 56 degrees on the HSV wheel.
  // Solid Color rejects red-orange, orange and yellow-orange hues.
  return (
    paletteIndex >= 8 &&
    paletteIndex <= 40
  );
}

uint32_t mixColorSeed(
  uint32_t value
) {

  value ^= value >> 16;
  value *= 0x7FEB352DUL;
  value ^= value >> 15;
  value *= 0x846CA68BUL;
  value ^= value >> 16;

  return value;
}

void generateDailyColors(
  const DateTime &now
) {

  uint32_t dateKey =
    (uint32_t)now.year() * 10000UL +
    (uint32_t)now.month() * 100UL +
    (uint32_t)now.day();

  dailyColorDateKey =
    dateKey;

  uint32_t seed =
    mixColorSeed(
      dateKey ^
      ((uint32_t)ESP.getChipId() * 2654435761UL) ^
      0x53494553UL
    );

  // Solid Color: one daily color, with all orange hues excluded.
  uint8_t solidIndex =
    (uint8_t)(seed & 0xFF);

  while (
    isOrangePaletteIndex(
      solidIndex
    )
  ) {

    solidIndex =
      (uint8_t)(
        solidIndex + 37
      );
  }

  dailySolidPaletteIndex =
    solidIndex;

  // Colorful: four different digit colors and a fifth accent color.
  uint8_t usedColorful[5];
  uint32_t colorfulState =
    mixColorSeed(
      seed ^
      0x434F4C52UL
    );

  for (
    int i = 0;
    i < 5;
    i++
  ) {

    colorfulState =
      mixColorSeed(
        colorfulState +
        0x9E3779B9UL +
        (uint32_t)i * 0x85EBCA6BUL
      );

    uint8_t candidate =
      (uint8_t)(
        colorfulState &
        0xFF
      );

    bool duplicate =
      true;

    while (duplicate) {
      duplicate = false;

      for (
        int j = 0;
        j < i;
        j++
      ) {

        if (
          usedColorful[j] ==
          candidate
        ) {

          duplicate =
            true;

          candidate =
            (uint8_t)(
              candidate + 37
            );

          break;
        }
      }
    }

    usedColorful[i] =
      candidate;
  }

  for (
    int i = 0;
    i < 4;
    i++
  ) {

    dailyDigitPaletteIndex[i] =
      usedColorful[i];
  }

  dailyAccentPaletteIndex =
    usedColorful[4];

  // Funny: an odd step through 256 entries is a full permutation.
  // Thus every logical pixel position can have a different daily hue.
  dailyFunnyOffset =
    (uint8_t)(
      (seed >> 16) &
      0xFF
    );

  dailyFunnyStep =
    (uint8_t)(
      ((seed >> 24) & 0xFF) |
      0x01
    );

}

void ensureDailyColors(
  const DateTime &now
) {

  uint32_t dateKey =
    (uint32_t)now.year() * 10000UL +
    (uint32_t)now.month() * 100UL +
    (uint32_t)now.day();

  if (
    dailyColorDateKey !=
    dateKey
  ) {

    generateDailyColors(
      now
    );
  }
}

uint32_t funnyColorForPixel(
  int x,
  int y
) {

  uint16_t logicalPixel =
    (uint16_t)(
      y * MATRIX_WIDTH +
      x
    );

  uint8_t paletteIndex =
    (uint8_t)(
      dailyFunnyOffset +
      logicalPixel * dailyFunnyStep
    );

  return palette256(
    paletteIndex
  );
}


// ============================================================
// ANIMATED COLOR
// ============================================================
//
// Same per-pixel idea as Funny, but every lit LED continuously
// fades toward independently generated random-looking colors.
// Consecutive transition segments share their boundary color,
// so the motion is smooth and has no short repeating loop.
//
uint32_t animatedColorForPixel(
  int x,
  int y
) {

  const unsigned long TRANSITION_MS =
    1800UL;

  unsigned long nowMs =
    millis();

  uint32_t segment =
    nowMs /
    TRANSITION_MS;

  float phase =
    (float)(
      nowMs %
      TRANSITION_MS
    ) /
    (float)TRANSITION_MS;

  phase =
    phase *
    phase *
    (3.0f - 2.0f * phase);

  uint32_t logicalPixel =
    (uint32_t)(
      y * MATRIX_WIDTH +
      x
    );

  uint32_t baseSeed =
    logicalPixel * 0x9E3779B9UL ^
    ESP.getChipId();

  uint8_t fromIndex =
    (uint8_t)(
      mixColorSeed(
        baseSeed ^
        segment * 0x85EBCA6BUL
      ) &
      0xFF
    );

  uint8_t toIndex =
    (uint8_t)(
      mixColorSeed(
        baseSeed ^
        (segment + 1UL) *
        0x85EBCA6BUL
      ) &
      0xFF
    );

  uint32_t fromColor =
    palette256(
      fromIndex
    );

  uint32_t toColor =
    palette256(
      toIndex
    );

  uint8_t fromR =
    (fromColor >> 16) &
    0xFF;

  uint8_t fromG =
    (fromColor >> 8) &
    0xFF;

  uint8_t fromB =
    fromColor &
    0xFF;

  uint8_t toR =
    (toColor >> 16) &
    0xFF;

  uint8_t toG =
    (toColor >> 8) &
    0xFF;

  uint8_t toB =
    toColor &
    0xFF;

  uint8_t r =
    (uint8_t)(
      fromR +
      (
        (float)toR -
        (float)fromR
      ) * phase
    );

  uint8_t g =
    (uint8_t)(
      fromG +
      (
        (float)toG -
        (float)fromG
      ) * phase
    );

  uint8_t b =
    (uint8_t)(
      fromB +
      (
        (float)toB -
        (float)fromB
      ) * phase
    );

  return Adafruit_NeoPixel::Color(
    r,
    g,
    b
  );
}

uint8_t dynamicBasePaletteIndex(
  const DateTime &now
) {

  const int BLUE_INDEX = 170;   // 240 degrees: deep blue
  const int MID_INDEX = 85;     // 120 degrees: green, fixed at 12:00
  const int RED_INDEX = 0;      // 0 degrees: red
  const int NOON = 12 * 60;

  int current =
    now.hour() * 60 +
    now.minute();

  int dayStart =
    dimEndHour * 60 +
    dimEndMin;

  int dayEnd =
    dimStartHour * 60 +
    dimStartMin;

  // Normal night schedules bracket noon: Night Mode ends in the morning
  // and starts again in the evening.  This keeps 12:00 a fixed, stable
  // green reference regardless of the configured Night Mode boundaries.
  if (
    dayStart < NOON &&
    dayEnd > NOON
  ) {

    if (current <= NOON) {
      if (current <= dayStart) {
        return BLUE_INDEX;
      }

      long numerator =
        (long)(current - dayStart) *
        (MID_INDEX - BLUE_INDEX);

      long denominator =
        NOON - dayStart;

      return (uint8_t)(
        BLUE_INDEX +
        numerator / denominator
      );
    }

    if (current >= dayEnd) {
      return RED_INDEX;
    }

    long numerator =
      (long)(current - NOON) *
      (RED_INDEX - MID_INDEX);

    long denominator =
      dayEnd - NOON;

    return (uint8_t)(
      MID_INDEX +
      numerator / denominator
    );
  }

  // Fallback for an unusual Night Mode schedule that does not bracket
  // noon.  Noon still remains the fixed green reference point.
  if (current <= NOON) {
    return (uint8_t)(
      BLUE_INDEX +
      ((long)current *
       (MID_INDEX - BLUE_INDEX)) /
      NOON
    );
  }

  return (uint8_t)(
    MID_INDEX +
    ((long)(current - NOON) *
     (RED_INDEX - MID_INDEX)) /
    (1440 - NOON)
  );
}

uint32_t dynamicColorForDigitPixel(
  uint8_t basePaletteIndex,
  int digitIndex,
  int col,
  int row
) {

  // Dynamic keeps one time-derived reference hue for the whole clock,
  // but every digit occupies a clearly separated hue family.  Inside
  // each digit, a fixed pseudo-random field spreads the lit pixels across
  // that family.  The pattern is stable (no flicker); only the time-derived
  // base hue moves during the day.
  static const int8_t digitBias[4] = {
    -42, -14, 14, 42
  };

  int safeDigit =
    constrain(
      digitIndex,
      0,
      3
    );

  uint32_t key =
    0x44594E41UL ^
    ((uint32_t)(safeDigit + 1) * 0x9E3779B9UL) ^
    ((uint32_t)(col + 1) * 0x85EBCA6BUL) ^
    ((uint32_t)(row + 1) * 0xC2B2AE35UL);

  uint8_t randomValue =
    (uint8_t)(
      mixColorSeed(key) &
      0xFF
    );

  // +/-24 palette steps is intentionally strong enough to be obvious
  // to the eye while preserving the underlying time reference.
  int pixelSpread =
    map(
      randomValue,
      0,
      255,
      -24,
      24
    );

  int paletteIndex =
    (int)basePaletteIndex +
    digitBias[safeDigit] +
    pixelSpread;

  // Keep the dynamic path inside red -> yellow -> green -> cyan -> blue
  // -> violet without wrapping back to red at the upper edge.
  paletteIndex =
    constrain(
      paletteIndex,
      0,
      213
    );

  return palette256(
    (uint8_t)paletteIndex
  );
}

uint32_t scaleColor(
  uint32_t color,
  float factor
) {

  uint8_t r =
    (color >> 16) &
    0xFF;

  uint8_t g =
    (color >> 8) &
    0xFF;

  uint8_t b =
    color &
    0xFF;

  return matrix.Color(
    constrain(
      (int)(r * factor),
      0,
      255
    ),
    constrain(
      (int)(g * factor),
      0,
      255
    ),
    constrain(
      (int)(b * factor),
      0,
      255
    )
  );
}

int clockFontWidth(
  uint8_t style
) {

  // Style 3 uses the approved PacClock seven-pixel-wide glyphs.
  if (style == 2) {
    return 7;
  }

  return 5;
}

bool clockFontPixel(
  uint8_t style,
  int number,
  int col,
  int row
) {

  if (
    number < 0 ||
    number > 9 ||
    row < 0 ||
    row > 6
  ) {
    return false;
  }

  int width =
    clockFontWidth(
      style
    );

  if (
    col < 0 ||
    col >= width
  ) {
    return false;
  }

  // Style 1 retains the exact original column-based bitmap.
  if (style == 0) {
    return (
      font5x7[number][col] &
      (1 << row)
    );
  }

  // Style 3 uses the approved PacClock rows.
  // Visible columns are bits 7..1; bit 0 is intentionally blank.
  if (style == 2) {

    uint8_t rowBits =
      fontStyle3PacClock[number][row];

    return (
      rowBits &
      (1 << (7 - col))
    );
  }

  uint8_t rowBits = 0;

  switch (style) {
    case 1:
      rowBits = fontStyle2[number][row];
      break;

    case 3:
      rowBits = fontStyle4[number][row];
      break;

    case 4:
      rowBits = fontStyle5[number][row];
      break;

    case 5:
      rowBits = fontStyle6[number][row];
      break;

    default:
      return (
        font5x7[number][col] &
        (1 << row)
      );
  }

  // Five-pixel row-based fonts use bit 4 as the leftmost pixel.
  return (
    rowBits &
    (1 << (4 - col))
  );
}

void drawClockDigit(
  int number,
  int startX,
  int startY,
  uint32_t color,
  bool funnyMode,
  bool animatedMode,
  bool dynamicMode,
  uint8_t dynamicBaseIndex,
  int digitIndex
) {

  if (
    number < 0 ||
    number > 9
  ) {
    return;
  }

  int fontWidth =
    clockFontWidth(
      clockFontStyle
    );

  for (
    int col = 0;
    col < fontWidth;
    col++
  ) {

    for (
      int row = 0;
      row < 7;
      row++
    ) {

      if (
        !clockFontPixel(
          clockFontStyle,
          number,
          col,
          row
        )
      ) {
        continue;
      }

      int x =
        startX + col;

      int y =
        startY + row;

      int index =
        matrixIndex(
          x,
          y
        );

      if (
        index >= 0 &&
        index < NUM_LEDS
      ) {

        uint32_t pixelColor =
          color;

        if (funnyMode) {
          pixelColor =
            funnyColorForPixel(
              x,
              y
            );
        }
        else if (animatedMode) {
          pixelColor =
            animatedColorForPixel(
              x,
              y
            );
        }
        else if (dynamicMode) {
          pixelColor =
            dynamicColorForDigitPixel(
              dynamicBaseIndex,
              digitIndex,
              col,
              row
            );
        }

        matrix.setPixelColor(
          index,
          pixelColor
        );
      }
    }
  }
}

// ============================================================
// DRAW NUMBER
// ============================================================

void drawNumber(
  int number,
  int startX,
  int startY,
  uint32_t color
) {

  if (
    number < 0 ||
    number > 9
  ) {

    return;
  }

  for (
    int col = 0;
    col < 5;
    col++
  ) {

    uint8_t columnBits =
      font5x7[number][col];

    for (
      int row = 0;
      row < 7;
      row++
    ) {

      if (
        columnBits &
        (1 << row)
      ) {

        int x =
          startX + col;

        int y =
          startY + row;

        int index =
          matrixIndex(
            x,
            y
          );

        if (
          index >= 0 &&
          index < NUM_LEDS
        ) {

          matrix.setPixelColor(
            index,
            color
          );
        }
      }
    }
  }
}

// ============================================================
// DRAW TEXT
// ============================================================

void drawText(
  const char* text,
  uint32_t color
) {

  //
  // Apply the CURRENT brightness before writing new pixels.
  //
  // This is important because Adafruit_NeoPixel brightness
  // scaling is destructive to the existing RAM pixel data.
  //

  matrix.setBrightness(
    currentBrightness
  );

  clearDisplay();

  int len =
    strlen(text);

  int startX =
    (MATRIX_WIDTH -
     (len * 6)) / 2;

  int startY = 2;

  for (
    int i = 0;
    i < len &&
    i < 12;
    i++
  ) {

    char c =
      text[i];

    if (
      c >= '0' &&
      c <= '9'
    ) {

      drawNumber(
        c - '0',
        startX + i * 6,
        startY,
        color
      );

    } else {

      for (
        int y = 0;
        y < 5;
        y++
      ) {

        for (
          int x = 0;
          x < 4;
          x++
        ) {

          int px =
            startX +
            i * 6 +
            x;

          int py =
            startY +
            y;

          int index =
            matrixIndex(
              px,
              py
            );

          if (
            index >= 0 &&
            index < NUM_LEDS
          ) {

            matrix.setPixelColor(
              index,
              color
            );
          }
        }
      }
    }
  }

  matrix.show();
}

// ============================================================
// MANUAL SETTING COLON
// ============================================================

void drawManualColon(
  uint32_t color
) {

  //
  // Only used while manually setting time.
  // Normal clock operation uses the new
  // vertical breathing separator instead.
  //

  int x = 14;

  int index1 =
    matrixIndex(
      x,
      2
    );

  int index2 =
    matrixIndex(
      x,
      5
    );

  if (index1 >= 0) {

    matrix.setPixelColor(
      index1,
      color
    );
  }

  if (index2 >= 0) {

    matrix.setPixelColor(
      index2,
      color
    );
  }
}

// ============================================================
// BREATHING SPARK
// ============================================================
//
// VERTICAL SEPARATOR:
//
//       row 0       blank
//       row 1       blank
//       row 2       ██
//       row 3       ██
//       row 4       BLACK
//       row 5       ██
//       row 6       ██
//       row 7       spare / BLACK
//
//       X15 X16
//
// The width is exactly two pixels.
//
// The upper 2-pixel section and lower 2-pixel section
// breathe together during a one-second cycle.
//
// The single row between them is ALWAYS black.
//

void drawBreathingSpark(
  uint32_t baseColor
) {

  if (
    isEditingTimeManually
  ) {

    return;
  }

  DateTime now =
    rtc.now();

  ensureDailyColors(
    now
  );

  bool funnySpark =
    false;

  bool animatedSpark =
    false;

  if (isPurpleOverride) {

    baseColor =
      COLOR_PURPLE;

  } else if (
    isSiestaDaytime(now)
  ) {

    baseColor =
      COLOR_ORANGE;

  } else if (
    isNightDimmingMode
  ) {

    baseColor =
      clockColors[
        clockColorIndex
      ];

  } else {

    switch (
      dayColorMode
    ) {

      case DAY_COLOR_SOLID:
        baseColor =
          palette256(
            dailySolidPaletteIndex
          );
        break;

      case DAY_COLOR_COLORFUL:
        baseColor =
          palette256(
            dailyAccentPaletteIndex
          );
        break;

      case DAY_COLOR_FUNNY:
        funnySpark =
          true;
        break;

      case DAY_COLOR_ANIMATED:
        animatedSpark =
          true;
        break;

      case DAY_COLOR_DYNAMIC:
        baseColor =
          palette256(
            dynamicBasePaletteIndex(now)
          );
        break;

      case DAY_COLOR_MONO:
      default:
        baseColor =
          clockColors[
            clockColorIndex
          ];
        break;
    }
  }

  unsigned long phase =
    millis() % 1000UL;

  float angle =
    (2.0f * PI * phase) /
    1000.0f;

  float breath =
    (1.0f - cos(angle)) *
    0.5f;

  // Clear only the separator area.  Digit pixels are untouched.
  for (
    int y = 2;
    y <= 7;
    y++
  ) {

    for (
      int x = 14;
      x <= 17;
      x++
    ) {

      int index =
        matrixIndex(
          x,
          y
        );

      if (index >= 0) {

        matrix.setPixelColor(
          index,
          COLOR_OFF
        );
      }
    }
  }

  const int sparkX[8] = {
    SPARK_LEFT_X, SPARK_RIGHT_X,
    SPARK_LEFT_X, SPARK_RIGHT_X,
    SPARK_LEFT_X, SPARK_RIGHT_X,
    SPARK_LEFT_X, SPARK_RIGHT_X
  };

  const int sparkY[8] = {
    SPARK_TOP_Y, SPARK_TOP_Y,
    SPARK_TOP_Y2, SPARK_TOP_Y2,
    SPARK_BOTTOM_Y, SPARK_BOTTOM_Y,
    SPARK_BOTTOM_Y2, SPARK_BOTTOM_Y2
  };

  for (
    int i = 0;
    i < 8;
    i++
  ) {

    int x =
      sparkX[i];

    int y =
      sparkY[i];

    int index =
      matrixIndex(
        x,
        y
      );

    if (index >= 0) {

      uint32_t pixelColor =
        funnySpark ?
          funnyColorForPixel(
            x,
            y
          ) :
        animatedSpark ?
          animatedColorForPixel(
            x,
            y
          ) :
          baseColor;

      matrix.setPixelColor(
        index,
        scaleColor(
          pixelColor,
          breath
        )
      );
    }
  }

  matrix.show();
}

// ============================================================
// AM / PM INDICATOR
// ============================================================
//
// PM is now EXACTLY the same color as the
// current clock digits.
//
// No green.
//

void drawAMPMLED(
  int hour,
  uint32_t color
) {

  int index =
    matrixIndex(
      31,
      0
    );

  if (index < 0) {

    return;
  }

  // The PM indicator exists only in 12-hour display mode.
  if (
    !use24HourFormat &&
    hour >= 12
  ) {

    matrix.setPixelColor(
      index,
      color
    );

  } else {

    matrix.setPixelColor(
      index,
      COLOR_OFF
    );
  }
}

// ============================================================
// CLEAR MATRIX
// ============================================================

void clearDisplay() {

  for (
    int i = 0;
    i < NUM_LEDS;
    i++
  ) {

    matrix.setPixelColor(
      i,
      COLOR_OFF
    );
  }
}

// ============================================================
// UPDATE CLOCK DISPLAY
// ============================================================

void updateDisplay() {

  DateTime now =
    rtc.now();

  ensureDailyColors(
    now
  );

  int currentHour =
    now.hour();

  int currentMinute =
    now.minute();

  int displayHour;

  if (use24HourFormat) {

    // 24-hour mode displays 00..23 and always keeps the hour tens digit.
    displayHour =
      currentHour;

  } else {

    // Established 12-hour behavior: 1..12 with no leading zero.
    displayHour =
      currentHour % 12;

    if (displayHour == 0) {
      displayHour = 12;
    }
  }

  // Color modes never alter brightness.  Existing day/night brightness
  // logic remains the sole authority for matrix brightness.
  checkBrightnessMode(
    currentHour,
    currentMinute
  );

  matrix.setBrightness(
    currentBrightness
  );

  bool siestaDaytimeNow =
    isSiestaDaytime(now);

  bool siestaNighttimeNow =
    isSiestaNighttime(now);

  bool funnyMode =
    false;

  bool animatedMode =
    false;

  bool dynamicMode =
    false;

  uint8_t dynamicBaseIndex =
    0;

  uint32_t digitColor[4];
  uint32_t accentColor;

  // Priority: editor purple -> Siesta daytime orange -> Night Mode
  // Mono Color -> selected daytime Mono/Multi Color behavior.
  // Siesta nighttime does NOT alter Night Mode; it is indicated separately
  // by the upper-left orange LED.
  if (isPurpleOverride) {

    for (int i = 0; i < 4; i++) {
      digitColor[i] = COLOR_PURPLE;
    }
    accentColor = COLOR_PURPLE;

  } else if (siestaDaytimeNow) {

    for (int i = 0; i < 4; i++) {
      digitColor[i] = COLOR_ORANGE;
    }
    accentColor = COLOR_ORANGE;

  } else if (isNightDimmingMode) {

    uint32_t monoColor =
      clockColors[clockColorIndex];

    for (int i = 0; i < 4; i++) {
      digitColor[i] = monoColor;
    }
    accentColor = monoColor;

  } else {

    switch (dayColorMode) {

      case DAY_COLOR_SOLID: {
        uint32_t solidColor =
          palette256(
            dailySolidPaletteIndex
          );
        for (int i = 0; i < 4; i++) {
          digitColor[i] = solidColor;
        }
        accentColor = solidColor;
        break;
      }

      case DAY_COLOR_COLORFUL:
        for (int i = 0; i < 4; i++) {
          digitColor[i] =
            palette256(
              dailyDigitPaletteIndex[i]
            );
        }
        accentColor =
          palette256(
            dailyAccentPaletteIndex
          );
        break;

      case DAY_COLOR_FUNNY:
        funnyMode = true;
        for (int i = 0; i < 4; i++) {
          digitColor[i] =
            clockColors[clockColorIndex];
        }
        accentColor =
          funnyColorForPixel(31, 0);
        break;

      case DAY_COLOR_ANIMATED:
        animatedMode = true;
        for (int i = 0; i < 4; i++) {
          digitColor[i] =
            clockColors[clockColorIndex];
        }
        accentColor =
          animatedColorForPixel(31, 0);
        break;

      case DAY_COLOR_DYNAMIC:
        dynamicMode = true;
        dynamicBaseIndex =
          dynamicBasePaletteIndex(now);

        for (int i = 0; i < 4; i++) {
          digitColor[i] =
            palette256(
              dynamicBaseIndex
            );
        }

        accentColor =
          palette256(
            dynamicBaseIndex
          );
        break;

      case DAY_COLOR_MONO:
      default: {
        uint32_t monoColor =
          clockColors[clockColorIndex];
        for (int i = 0; i < 4; i++) {
          digitColor[i] = monoColor;
        }
        accentColor = monoColor;
        break;
      }
    }
  }

  clearDisplay();


  // Style 3 (PacClock) is seven pixels wide. It therefore uses its own
  // horizontal layout while preserving the established separator at X15/X16.
  //
  //   Hours:   X0..6 and X7..13
  //   Blank:   X14
  //   Spark:   X15..16
  //   Blank:   X17
  //   Minutes: X18..24 and X25..31
  //
  // All other styles keep the original 5x7 digit positions.
  int hourTensX =
    hourTensX;

  int hourUnitsX =
    hourUnitsX;

  int minuteTensX =
    minuteTensX;

  int minuteUnitsX =
    minuteUnitsX;

  if (clockFontStyle == 2) {
    hourTensX = 0;
    hourUnitsX = 7;
    minuteTensX = 18;
    minuteUnitsX = 25;
  }

  int hourTens =
    displayHour / 10;

  int hourUnits =
    displayHour % 10;

  uint32_t hourTensColor =
    digitColor[0];
  uint32_t hourUnitsColor =
    digitColor[1];
  uint32_t minuteTensColor =
    digitColor[2];
  uint32_t minuteUnitsColor =
    digitColor[3];

  bool drawFunnyDigits =
    funnyMode;

  if (
    use24HourFormat ||
    hourTens > 0
  ) {

    drawClockDigit(
      hourTens,
      hourTensX,
      DIGIT_Y,
      hourTensColor,
      drawFunnyDigits,
      animatedMode,
      dynamicMode,
      dynamicBaseIndex,
      0
    );

    drawClockDigit(
      hourUnits,
      hourUnitsX,
      DIGIT_Y,
      hourUnitsColor,
      drawFunnyDigits,
      animatedMode,
      dynamicMode,
      dynamicBaseIndex,
      1
    );

  } else {

    drawClockDigit(
      hourUnits,
      hourUnitsX,
      DIGIT_Y,
      hourUnitsColor,
      drawFunnyDigits,
      animatedMode,
      dynamicMode,
      dynamicBaseIndex,
      1
    );
  }

  int minuteTens =
    currentMinute / 10;

  int minuteUnits =
    currentMinute % 10;

  drawClockDigit(
    minuteTens,
    minuteTensX,
    DIGIT_Y,
    minuteTensColor,
    drawFunnyDigits,
    animatedMode,
    dynamicMode,
    dynamicBaseIndex,
    2
  );

  drawClockDigit(
    minuteUnits,
    minuteUnitsX,
    DIGIT_Y,
    minuteUnitsColor,
    drawFunnyDigits,
    animatedMode,
    dynamicMode,
    dynamicBaseIndex,
    3
  );

  uint32_t pmColor =
    funnyMode ?
      funnyColorForPixel(31, 0) :
    animatedMode ?
      animatedColorForPixel(31, 0) :
      accentColor;

  drawAMPMLED(
    currentHour,
    pmColor
  );

  // Nighttime Greek quiet-hours indicator.
  // This does not change Night Mode, color mode or brightness.
  // It simply lights the logical upper-left LED in orange while the
  // active seasonal nighttime quiet period is in force.
  if (
    !isPurpleOverride &&
    siestaNighttimeNow
  ) {

    int quietNightIndicator =
      matrixIndex(0, 0);

    if (quietNightIndicator >= 0) {
      matrix.setPixelColor(
        quietNightIndicator,
        COLOR_ORANGE
      );
    }
  }

  matrix.show();
}

// ============================================================
// SEASONAL GREEK QUIET HOURS / SIESTA
// ============================================================

bool isTimeInPeriod(
  int hour,
  int minute,
  int startHour,
  int startMinute,
  int endHour,
  int endMinute
) {

  int totalMinutes =
    hour * 60 + minute;

  int start =
    startHour * 60 + startMinute;

  int end =
    endHour * 60 + endMinute;

  if (start == end) {
    return false;
  }

  if (start < end) {
    return totalMinutes >= start && totalMinutes < end;
  }

  // Period crosses midnight.
  return totalMinutes >= start || totalMinutes < end;
}

bool isValidMonthDay(
  int day,
  int month
) {

  // Use a leap-capable calendar for runtime dates so 29 February
  // remains part of the active seasonal range in leap years.
  static const uint8_t daysInMonth[12] = {
    31, 29, 31, 30, 31, 30,
    31, 31, 30, 31, 30, 31
  };

  if (month < 1 || month > 12) {
    return false;
  }

  return day >= 1 &&
         day <= daysInMonth[month - 1];
}

bool isValidSeasonBoundary(
  int day,
  int month
) {

  // Seasonal boundaries recur every year.  29 February is therefore
  // deliberately not accepted as a configurable start/end boundary.
  static const uint8_t daysInMonth[12] = {
    31, 28, 31, 30, 31, 30,
    31, 31, 30, 31, 30, 31
  };

  if (month < 1 || month > 12) {
    return false;
  }

  return day >= 1 &&
         day <= daysInMonth[month - 1];
}

int siestaDayOfYear(
  int day,
  int month
) {

  // Leap-calendar offsets keep 29 February between 28 February
  // and 1 March when evaluating the recurring seasonal ranges.
  static const uint16_t monthOffset[12] = {
    0, 31, 60, 91, 121, 152,
    182, 213, 244, 274, 305, 335
  };

  return monthOffset[month - 1] + day;
}

bool dateInRange(
  int day,
  int month,
  int startDay,
  int startMonth,
  int endDay,
  int endMonth
) {

  if (
    !isValidMonthDay(day, month) ||
    !isValidMonthDay(startDay, startMonth) ||
    !isValidMonthDay(endDay, endMonth)
  ) {
    return false;
  }

  int current =
    siestaDayOfYear(day, month);

  int start =
    siestaDayOfYear(startDay, startMonth);

  int end =
    siestaDayOfYear(endDay, endMonth);

  if (start <= end) {
    return current >= start && current <= end;
  }

  // Seasonal range crosses New Year.
  return current >= start || current <= end;
}

const SiestaSeasonSettings* activeSiestaSeason(
  const DateTime &now
) {

  if (
    dateInRange(
      now.day(),
      now.month(),
      summerSiesta.startDay,
      summerSiesta.startMonth,
      summerSiesta.endDay,
      summerSiesta.endMonth
    )
  ) {
    return &summerSiesta;
  }

  if (
    dateInRange(
      now.day(),
      now.month(),
      winterSiesta.startDay,
      winterSiesta.startMonth,
      winterSiesta.endDay,
      winterSiesta.endMonth
    )
  ) {
    return &winterSiesta;
  }

  return nullptr;
}

bool isSiestaDaytime(
  const DateTime &now
) {

  const SiestaSeasonSettings* season =
    activeSiestaSeason(now);

  if (season == nullptr) {
    return false;
  }

  return isTimeInPeriod(
    now.hour(),
    now.minute(),
    season->dayStartHour,
    season->dayStartMin,
    season->dayEndHour,
    season->dayEndMin
  );
}

bool isSiestaNighttime(
  const DateTime &now
) {

  const SiestaSeasonSettings* season =
    activeSiestaSeason(now);

  if (season == nullptr) {
    return false;
  }

  return isTimeInPeriod(
    now.hour(),
    now.minute(),
    season->nightStartHour,
    season->nightStartMin,
    season->nightEndHour,
    season->nightEndMin
  );
}

bool siestaTimePeriodsOverlap(
  const SiestaSeasonSettings &season
) {

  for (int minute = 0; minute < 1440; minute++) {

    int hour = minute / 60;
    int min = minute % 60;

    bool dayActive =
      isTimeInPeriod(
        hour, min,
        season.dayStartHour,
        season.dayStartMin,
        season.dayEndHour,
        season.dayEndMin
      );

    bool nightActive =
      isTimeInPeriod(
        hour, min,
        season.nightStartHour,
        season.nightStartMin,
        season.nightEndHour,
        season.nightEndMin
      );

    if (dayActive && nightActive) {
      return true;
    }
  }

  return false;
}

bool validateSiestaConfiguration(
  const SiestaSeasonSettings &summer,
  const SiestaSeasonSettings &winter
) {

  // Every configured time period must have a non-zero duration.
  // The generic period logic supports both same-day and midnight-crossing
  // intervals; the user is therefore not locked to today's legal pattern.
  int summerDayStart =
    summer.dayStartHour * 60 + summer.dayStartMin;
  int summerDayEnd =
    summer.dayEndHour * 60 + summer.dayEndMin;
  int winterDayStart =
    winter.dayStartHour * 60 + winter.dayStartMin;
  int winterDayEnd =
    winter.dayEndHour * 60 + winter.dayEndMin;
  int summerNightStart =
    summer.nightStartHour * 60 + summer.nightStartMin;
  int summerNightEnd =
    summer.nightEndHour * 60 + summer.nightEndMin;
  int winterNightStart =
    winter.nightStartHour * 60 + winter.nightStartMin;
  int winterNightEnd =
    winter.nightEndHour * 60 + winter.nightEndMin;

  if (summerDayStart == summerDayEnd ||
      winterDayStart == winterDayEnd ||
      summerNightStart == summerNightEnd ||
      winterNightStart == winterNightEnd) {
    return false;
  }

  if (
    !isValidSeasonBoundary(summer.startDay, summer.startMonth) ||
    !isValidSeasonBoundary(summer.endDay, summer.endMonth) ||
    !isValidSeasonBoundary(winter.startDay, winter.startMonth) ||
    !isValidSeasonBoundary(winter.endDay, winter.endMonth)
  ) {
    return false;
  }

  if (siestaTimePeriodsOverlap(summer) ||
      siestaTimePeriodsOverlap(winter)) {
    return false;
  }

  // Summer and Winter must cover every calendar day exactly once.
  // This rejects both date gaps and date overlaps, including 29 February
  // in leap years.
  static const uint8_t daysInMonth[12] = {
    31, 29, 31, 30, 31, 30,
    31, 31, 30, 31, 30, 31
  };

  for (int month = 1; month <= 12; month++) {
    for (int day = 1; day <= daysInMonth[month - 1]; day++) {

      bool inSummer =
        dateInRange(
          day, month,
          summer.startDay, summer.startMonth,
          summer.endDay, summer.endMonth
        );

      bool inWinter =
        dateInRange(
          day, month,
          winter.startDay, winter.startMonth,
          winter.endDay, winter.endMonth
        );

      if (inSummer == inWinter) {
        // both true = overlap, both false = uncovered gap
        return false;
      }
    }
  }

  return true;
}

// ============================================================
// NIGHT / DAY BRIGHTNESS
// ============================================================
//
// CRITICAL BRIGHTNESS RULE:
//
// If the current time is inside the dimming schedule:
//
//       currentBrightness = BRIGHTNESS_MIN
//
// Nothing else is allowed to increase it.
//
// In particular, an IR command must NEVER change it to
// dayBrightness while night mode is active.
//
// ============================================================

void checkBrightnessMode(
  int hour,
  int minute
) {

  int totalMinutes =
    hour * 60 +
    minute;

  int start =
    dimStartHour * 60 +
    dimStartMin;

  int effectiveEndHour = dimEndHour;
  int effectiveEndMin = dimEndMin;

  // HOLIDAYS can extend only the Night Mode END time.  The master switch
  // and the three independently selected rules decide whether today's date
  // qualifies.  Night Mode START and every other clock behavior stay
  // unchanged.
  if (
    holidaysEnabled &&
    rtcInitialized
  ) {
    DateTime today = rtc.now();
    if (isHolidayOverrideDate(today)) {
      effectiveEndHour = holidayOverrideHour;
      effectiveEndMin = holidayOverrideMinute;
    }
  }

  int end =
    effectiveEndHour * 60 +
    effectiveEndMin;

  // ----------------------------------------------------------
  // PURPLE OVERRIDE
  // ----------------------------------------------------------
  //
  // Purple override is allowed to change the COLOR,
  // but it is NOT allowed to break night dimming.
  //

  if (isPurpleOverride) {

    if (isNightDimmingMode) {

      currentBrightness =
        BRIGHTNESS_MIN;
    }

    return;
  }

  // ----------------------------------------------------------
  // DETERMINE NIGHT TIME
  // ----------------------------------------------------------

  bool nightTime = false;

  if (start < end) {

    nightTime =
      (
        totalMinutes >= start &&
        totalMinutes < end
      );

  } else {

    nightTime =
      (
        totalMinutes >= start ||
        totalMinutes < end
      );
  }

  // ----------------------------------------------------------
  // NIGHT MODE
  // ----------------------------------------------------------

  if (nightTime) {

    //
    // CRITICAL:
    //
    // Always force minimum brightness.
    //
    // Never use dayBrightness here.
    //

    if (
      !isNightDimmingMode ||
      currentBrightness != BRIGHTNESS_MIN
    ) {

      isNightDimmingMode =
        true;

      currentBrightness =
        BRIGHTNESS_MIN;

    }

    return;
  }

  // ----------------------------------------------------------
  // DAY MODE
  // ----------------------------------------------------------

  if (
    isNightDimmingMode
  ) {

    isNightDimmingMode =
      false;

    currentBrightness =
      constrain(
        dayBrightness,
        BRIGHTNESS_MIN,
        BRIGHTNESS_MAX
      );


  }
}

// ============================================================
// RTC INITIALIZATION
// ============================================================

bool initializeRTC() {

  if (!rtc.begin()) {

    return false;
  }

  rtcInitialized =
    true;

  return true;
}

// ============================================================
// NTP -> RTC
// ============================================================
//
// Greece is the default timezone.
//
// EET = UTC+2
// EEST = UTC+3
//
// DST changes automatically according to
// the POSIX rule below.
//

bool syncRTCWithNTP() {

  if (
    WiFi.status() !=
    WL_CONNECTED
  ) {


    return false;
  }


  // ----------------------------------------------------------
  // APPLY TIMEZONE
  // ----------------------------------------------------------

  applyTimezone(
    timezoneIndex
  );

  // ----------------------------------------------------------
  // START SNTP
  // ----------------------------------------------------------

  char timezoneSpec[80];
  getTimezoneSpec(
    timezoneIndex,
    timezoneSpec,
    sizeof(timezoneSpec)
  );

  configTime(
    timezoneSpec,
    "pool.ntp.org",
    "time.nist.gov",
    "time.google.com"
  );

  time_t now =
    time(nullptr);

  int attempts = 0;

  while (
    now < 1700000000 &&
    attempts < 30
  ) {

    delay(500);

    now =
      time(nullptr);

    attempts++;
  }

  if (
    now < 1700000000
  ) {


    return false;
  }

  // ----------------------------------------------------------
  // CONVERT TO LOCAL TIME
  // ----------------------------------------------------------

  struct tm localTime;

  localtime_r(
    &now,
    &localTime
  );

  DateTime ntpLocalTime(

    localTime.tm_year + 1900,

    localTime.tm_mon + 1,

    localTime.tm_mday,

    localTime.tm_hour,

    localTime.tm_min,

    localTime.tm_sec
  );

  // ----------------------------------------------------------
  // WRITE LOCAL TIME INTO RTC
  // ----------------------------------------------------------

  rtc.adjust(
    ntpLocalTime
  );


  return true;
}

// ============================================================
// RTC -> ESP TIME
// ============================================================

bool syncESPTimeWithRTC() {

  return rtcInitialized;
}

// ============================================================
// MANUAL TIME SET
// ============================================================

void setManualTimeRTC(
  int hour,
  int minute
) {

  DateTime now =
    rtc.now();

  DateTime newTime(

    now.year(),
    now.month(),
    now.day(),

    hour,
    minute,
    0
  );

  rtc.adjust(
    newTime
  );

  syncESPTimeWithRTC();

  drawText(
    "TIME SET",
    COLOR_GREEN
  );

  delay(2000);

  isEditingTimeManually =
    false;

  timeSyncSuccess =
    true;

  updateDisplay();
}

// ============================================================
// DIRECT NEC IR DECODER
// ============================================================

unsigned long measureIRLevel(
  uint8_t level,
  unsigned long timeoutUs
) {
  unsigned long waitStart = micros();

  while (digitalRead(IR_PIN) != level) {
    if (micros() - waitStart > timeoutUs) return 0;
    yield();
  }

  unsigned long start = micros();

  while (digitalRead(IR_PIN) == level) {
    if (micros() - start > timeoutUs) return 0;
  }

  return micros() - start;
}

NECResult readNECFrame() {
  NECResult r;
  r.valid = false;
  r.repeat = false;
  r.addressCode = 0;
  r.commandCode = 0;

  unsigned long headerMark = measureIRLevel(LOW, 15000);
  if (headerMark < NEC_HEADER_MARK_MIN || headerMark > NEC_HEADER_MARK_MAX) {
    return r;
  }

  unsigned long headerSpace = measureIRLevel(HIGH, 8000);

  if (headerSpace >= NEC_REPEAT_SPACE_MIN && headerSpace <= NEC_REPEAT_SPACE_MAX) {
    unsigned long repeatMark = measureIRLevel(LOW, 1500);
    if (repeatMark >= NEC_BIT_MARK_MIN && repeatMark <= NEC_BIT_MARK_MAX) {
      r.repeat = true;
      if (haveLastIRCommand) {
        r.valid = true;
        r.addressCode = lastIRAddress;
        r.commandCode = lastIRCommand;
      }
    }
    return r;
  }

  if (headerSpace < NEC_HEADER_SPACE_MIN || headerSpace > NEC_HEADER_SPACE_MAX) {
    return r;
  }

  uint32_t rawData = 0;

  for (int bitNumber = 0; bitNumber < 32; bitNumber++) {
    unsigned long markLength = measureIRLevel(LOW, 1800);
    if (markLength < NEC_BIT_MARK_MIN || markLength > NEC_BIT_MARK_MAX) {
      return r;
    }

    unsigned long spaceLength = measureIRLevel(HIGH, 3000);

    if (spaceLength >= NEC_ZERO_SPACE_MIN && spaceLength <= NEC_ZERO_SPACE_MAX) {
      // zero
    } else if (spaceLength >= NEC_ONE_SPACE_MIN && spaceLength <= NEC_ONE_SPACE_MAX) {
      rawData |= ((uint32_t)1 << bitNumber);
    } else {
      return r;
    }
  }

  uint8_t b0 = (rawData >> 0)  & 0xFF;
  uint8_t b1 = (rawData >> 8)  & 0xFF;
  uint8_t b2 = (rawData >> 16) & 0xFF;
  uint8_t b3 = (rawData >> 24) & 0xFF;

  if (((b0 ^ b1) != 0xFF) || ((b2 ^ b3) != 0xFF)) {
    return r;
  }

  r.addressCode = ((uint16_t)b0 << 8) | b1;
  r.commandCode = ((uint16_t)b2 << 8) | b3;

  if (r.addressCode != IR_ADDRESS) {
    return r;
  }

  r.valid = true;
  lastIRAddress = r.addressCode;
  lastIRCommand = r.commandCode;
  haveLastIRCommand = true;

  return r;
}

int digitFromCommand(uint16_t commandCode) {
  switch (commandCode) {
    case IR_BUTTON_0: return 0;
    case IR_BUTTON_1: return 1;
    case IR_BUTTON_2: return 2;
    case IR_BUTTON_3: return 3;
    case IR_BUTTON_4: return 4;
    case IR_BUTTON_5: return 5;
    case IR_BUTTON_6: return 6;
    case IR_BUTTON_7: return 7;
    case IR_BUTTON_8: return 8;
    case IR_BUTTON_9: return 9;
    default: return -1;
  }
}

// ============================================================
// EDITOR OLED
// ============================================================

void drawEditorTitle(const char* title) {
  u8g2.setPowerSave(0);
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  u8g2.drawStr(0, 13, title);
}

void drawLocalizedEditorTitle(
  const char* englishTitle,
  const char* greekTitle
) {

  u8g2.setPowerSave(0);
  u8g2.clearBuffer();

  if (isGreekUI()) {
    u8g2.setFont(u8g2_font_cu12_t_greek);
    u8g2.drawUTF8(0, 13, greekTitle);
  } else {
    u8g2.setFont(u8g2_font_ncenB10_tr);
    u8g2.drawStr(0, 13, englishTitle);
  }
}

void drawGreekCentered(
  int y,
  const char* text
) {

  u8g2.setFont(u8g2_font_cu12_t_greek);

  int width =
    u8g2.getUTF8Width(text);

  int x = 0;

  if (width < 128) {
    x =
      (128 - width) / 2;
  }

  // Keep rendering static.  If an exact translated label is wider than
  // the OLED, it is clipped at the right edge rather than continuously
  // refreshing the OLED.  This protects the timing-sensitive direct NEC
  // decoder from unnecessary I2C display traffic while the editor is open.
  u8g2.drawUTF8(
    x,
    y,
    text
  );
}

// Compact navigation symbols drawn directly with U8g2 primitives.
// They do not depend on the current font containing Unicode arrow glyphs.
void drawNavUpArrow(int x, int y) {
  u8g2.drawVLine(x, y + 1, 7);
  u8g2.drawLine(x, y, x - 3, y + 3);
  u8g2.drawLine(x, y, x + 3, y + 3);
}

void drawNavDownArrow(int x, int y) {
  u8g2.drawVLine(x, y, 7);
  u8g2.drawLine(x, y + 7, x - 3, y + 4);
  u8g2.drawLine(x, y + 7, x + 3, y + 4);
}

void drawNavLeftArrow(int x, int y) {
  u8g2.drawHLine(x + 1, y, 7);
  u8g2.drawLine(x, y, x + 3, y - 3);
  u8g2.drawLine(x, y, x + 3, y + 3);
}

void drawNavRightArrow(int x, int y) {
  u8g2.drawHLine(x - 7, y, 7);
  u8g2.drawLine(x, y, x - 3, y - 3);
  u8g2.drawLine(x, y, x - 3, y + 3);
}

// Compact bent return arrow used as the ENTER symbol.
void drawNavEnterArrow(int x, int y) {
  u8g2.drawHLine(x, y, 10);
  u8g2.drawVLine(x + 9, y, 8);
  u8g2.drawHLine(x + 2, y + 7, 8);
  u8g2.drawLine(x + 2, y + 7, x + 5, y + 4);
  u8g2.drawLine(x + 2, y + 7, x + 5, y + 10);
}

void drawSelectionNavigationIcons() {
  drawNavUpArrow(10, 54);
  drawNavDownArrow(26, 54);
  drawNavEnterArrow(108, 53);
}

void drawDigitEditNavigationIcons() {
  u8g2.setFont(u8g2_font_5x8_tf);
  u8g2.drawStr(0, 63, "0-9");
  drawNavLeftArrow(37, 59);
  drawNavRightArrow(53, 59);
  drawNavEnterArrow(108, 53);
}

void drawNumberEnterNavigationIcons() {
  u8g2.setFont(u8g2_font_5x8_tf);
  u8g2.drawStr(0, 63, "0-9");
  drawNavEnterArrow(108, 53);
}

void drawHolidayCheckbox(
  int x,
  int baselineY,
  bool checked
) {
  int top = baselineY - 7;

  u8g2.drawFrame(
    x,
    top,
    8,
    8
  );

  if (checked) {
    u8g2.drawLine(
      x + 1,
      top + 4,
      x + 3,
      top + 6
    );
    u8g2.drawLine(
      x + 3,
      top + 6,
      x + 7,
      top + 1
    );
  }
}

void drawHolidayOptionsPage() {

  drawLocalizedEditorTitle(
    "HOLIDAYS",
    "Αργίες"
  );

  // Keep navigation symbols on the title line so all four option rows
  // remain completely clear on the 64-pixel-high OLED.
  drawNavUpArrow(91, 3);
  drawNavDownArrow(103, 3);
  drawNavEnterArrow(116, 2);

  if (isGreekUI()) {

    static const int rowY[4] = {
      24, 35, 46, 58
    };

    u8g2.setFont(
      u8g2_font_cu12_t_greek
    );

    const char* labels[4] = {
      "Επίσημες",
      "Σάββατα",
      "Κυριακές",
      "Ώρα"
    };

    bool checks[3] = {
      editHolidayPublicDatesEnabled,
      editHolidaySaturdaysEnabled,
      editHolidaySundaysEnabled
    };

    for (int row = 0; row < 4; row++) {

      u8g2.drawStr(
        0,
        rowY[row],
        editorSelection == row ?
          ">" :
          " "
      );

      if (row < 3) {
        drawHolidayCheckbox(
          9,
          rowY[row],
          checks[row]
        );

        u8g2.drawUTF8(
          22,
          rowY[row],
          labels[row]
        );
      } else {
        u8g2.drawUTF8(
          22,
          rowY[row],
          labels[row]
        );
      }
    }

    // The time itself uses the compact Latin numeric font so HH:MM fits
    // comfortably on the same row as the Greek label.
    char buf[8];
    int h =
      editDigits[0] * 10 +
      editDigits[1];
    int m =
      editDigits[2] * 10 +
      editDigits[3];

    snprintf(
      buf,
      sizeof(buf),
      "%02d:%02d",
      h,
      m
    );

    u8g2.setFont(
      u8g2_font_6x10_tf
    );

    const int timeX = 74;
    const int timeY = 58;

    u8g2.drawStr(
      timeX,
      timeY,
      buf
    );

    if (editorSelection == 3) {
      static const int digitX[4] = {
        74, 80, 92, 98
      };

      u8g2.drawHLine(
        digitX[editorCursor],
        60,
        5
      );
    }

  } else {

    static const int rowY[4] = {
      23, 33, 43, 56
    };

    u8g2.setFont(
      u8g2_font_5x8_tf
    );

    const char* labels[4] = {
      "Public Holidays",
      "Saturdays",
      "Sundays",
      "Time"
    };

    bool checks[3] = {
      editHolidayPublicDatesEnabled,
      editHolidaySaturdaysEnabled,
      editHolidaySundaysEnabled
    };

    for (int row = 0; row < 4; row++) {

      u8g2.drawStr(
        0,
        rowY[row],
        editorSelection == row ?
          ">" :
          " "
      );

      if (row < 3) {
        drawHolidayCheckbox(
          9,
          rowY[row],
          checks[row]
        );

        u8g2.drawStr(
          22,
          rowY[row],
          labels[row]
        );
      } else {
        u8g2.drawStr(
          22,
          rowY[row],
          labels[row]
        );
      }
    }

    char buf[8];
    int h =
      editDigits[0] * 10 +
      editDigits[1];
    int m =
      editDigits[2] * 10 +
      editDigits[3];

    snprintf(
      buf,
      sizeof(buf),
      "%02d:%02d",
      h,
      m
    );

    u8g2.setFont(
      u8g2_font_6x10_tf
    );

    const int timeX = 74;
    const int timeY = 56;

    u8g2.drawStr(
      timeX,
      timeY,
      buf
    );

    if (editorSelection == 3) {
      static const int digitX[4] = {
        74, 80, 92, 98
      };

      u8g2.drawHLine(
        digitX[editorCursor],
        58,
        5
      );

      // Time row: indicate that LEFT/RIGHT move the active digit.
      drawNavLeftArrow(
        110,
        56
      );
      drawNavRightArrow(
        125,
        56
      );
    }
  }
}

void drawTimeDigitsLineAt(
  int x,
  int y,
  int offset,
  int cursorBase
) {

  char buf[24];
  int h = editDigits[offset] * 10 + editDigits[offset + 1];
  int m = editDigits[offset + 2] * 10 + editDigits[offset + 3];
  snprintf(buf, sizeof(buf), "%02d:%02d", h, m);
  u8g2.drawStr(x, y, buf);

  // Small cursor under the active digit.
  int xPos[4] = {
    x,
    x + 7,
    x + 21,
    x + 28
  };

  int local = editorCursor - cursorBase;
  if (local >= 0 && local < 4) {
    u8g2.drawHLine(xPos[local], y + 2, 6);
  }
}

void drawTimeDigitsLine(int y, int offset, int cursorBase) {
  drawTimeDigitsLineAt(29, y, offset, cursorBase);
}

void showEditor() {
  if (!editorActive) return;

  currentOLEDState = OLED_IR_COMMAND;
  oledOffTimer = millis();

  if (selectedMessageActive) {
    u8g2.setPowerSave(0);
    u8g2.clearBuffer();

    if (isGreekUI()) {
      drawGreekCentered(38, "ΕΠΙΛΕΧΘΗΚΕ");
    } else {
      u8g2.setFont(u8g2_font_ncenB14_tr);
      u8g2.drawStr(20, 38, "SELECTED");
    }

    u8g2.sendBuffer();
    return;
  }

  if (editorPage == EDITOR_MAIN) {
    static const char* itemsEnglish[] = {
      "Country",
      "Time Zone",
      "Time",
      "12/24 Hour",
      "Fonts",
      "Siesta",
      "Night Mode",
      "Holidays",
      "Dimmer",
      "AUTOSYNC",
      "SYNC NOW",
      "Mono Color",
      "Multi Color",
      "Language"
    };

    static const char* itemsGreek[] = {
      "Χώρα",
      "Ζώνη Ώρας",
      "Ώρα",
      "Εμφάνιση 12/24",
      "Γραμματοσειρές",
      "Ώρες Κοινής Ησυχίας",
      "Νυχτερινό",
      "Αργίες",
      "Φωτεινότητα",
      "Αυτοσυγχρονισμός",
      "Συγχρονισμός Τώρα",
      "Μονόχρωμο",
      "Πολύχρωμο",
      "Γλώσσα"
    };

    // The main menu title deliberately remains English in both languages.
    drawEditorTitle("SIESTA CLOCK");

    if (isGreekUI()) {
      u8g2.setFont(u8g2_font_cu12_t_greek);
    } else {
      u8g2.setFont(u8g2_font_6x10_tf);
    }

    int first = editorMenuIndex - 2;
    if (first < 0) first = 0;
    if (first > 10) first = 10;

    for (int row = 0; row < 4; row++) {
      int idx = first + row;
      if (idx >= 14) break;
      int y =
        (isGreekUI() ? 25 : 27) +
        row * 12;

      if (isGreekUI()) {
        // Exact Greek labels are rendered statically.  This avoids periodic
        // OLED refreshes that could disturb the timing-sensitive NEC decoder.
        u8g2.drawUTF8(
          6,
          y,
          itemsGreek[idx]
        );

        u8g2.drawStr(
          0,
          y,
          idx == editorMenuIndex ? ">" : " "
        );
      } else {
        // Preserve the established English coordinates exactly.
        u8g2.drawStr(
          2,
          y,
          idx == editorMenuIndex ? ">" : " "
        );
        u8g2.drawStr(
          12,
          y,
          itemsEnglish[idx]
        );
      }
    }

    u8g2.sendBuffer();
    return;
  }

  if (editorPage == EDITOR_COUNTRY) {
    drawLocalizedEditorTitle("COUNTRY", "Χώρα");

    char countryName[52];
    getCountryName((uint8_t)editorSelection, countryName, sizeof(countryName));

    if (isGreekUI() && editorSelection == 0) {
      u8g2.setFont(u8g2_font_cu12_t_greek);
      u8g2.drawUTF8(1, 34, "Ελλάδα");
    } else {
      u8g2.setFont(u8g2_font_5x8_tf);
      u8g2.drawStr(1, 34, countryName);
    }
    drawSelectionNavigationIcons();
  }
  else if (editorPage == EDITOR_TIMEZONE) {
    drawLocalizedEditorTitle("TIME ZONE", "Ζώνη Ώρας");

    char zoneName[52];
    getTimezoneName((uint16_t)editorSelection, zoneName, sizeof(zoneName));
    u8g2.setFont(u8g2_font_5x8_tf);
    u8g2.drawStr(1, 34, zoneName);
    drawSelectionNavigationIcons();
  }
  else if (editorPage == EDITOR_TIME) {
    drawLocalizedEditorTitle("TIME", "Ώρα");
    u8g2.setFont(u8g2_font_7x13_tf);
    drawTimeDigitsLine(38, 0, 0);

    if (isGreekUI()) {
      u8g2.setFont(u8g2_font_cu12_t_greek);
      drawDigitEditNavigationIcons();
    } else {
      u8g2.setFont(u8g2_font_5x8_tf);
      drawDigitEditNavigationIcons();
    }
  }
  else if (editorPage == EDITOR_HOUR_FORMAT) {
    drawLocalizedEditorTitle("12/24 HOUR", "Εμφάνιση 12/24");

    if (isGreekUI()) {
      drawGreekCentered(
        39,
        editorSelection == 0 ?
          "12 Ώρες" :
          "24 Ώρες"
      );
      u8g2.setFont(u8g2_font_cu12_t_greek);
      drawSelectionNavigationIcons();
    } else {
      u8g2.setFont(u8g2_font_ncenB12_tr);
      u8g2.drawStr(
        28,
        39,
        editorSelection == 0 ?
          "12 Hour" :
          "24 Hour"
      );
      u8g2.setFont(u8g2_font_6x10_tf);
      drawSelectionNavigationIcons();
    }
  }
  else if (editorPage == EDITOR_FONT) {
    char buf[20];
    drawLocalizedEditorTitle("FONTS", "Γραμματοσειρές");

    if (isGreekUI()) {
      snprintf(
        buf,
        sizeof(buf),
        "Στυλ %d",
        editorSelection + 1
      );
      drawGreekCentered(39, buf);
      u8g2.setFont(u8g2_font_cu12_t_greek);
      drawSelectionNavigationIcons();
    } else {
      u8g2.setFont(u8g2_font_ncenB12_tr);
      snprintf(
        buf,
        sizeof(buf),
        "Style %d",
        editorSelection + 1
      );
      u8g2.drawStr(27, 39, buf);
      u8g2.setFont(u8g2_font_6x10_tf);
      drawSelectionNavigationIcons();
    }
  }
  else if (editorPage == EDITOR_SIESTA) {
    drawSiestaEditorPage();
  }
  else if (editorPage == EDITOR_NIGHT) {
    drawLocalizedEditorTitle("NIGHT MODE", "Νυχτερινό");

    if (isGreekUI()) {
      u8g2.setFont(u8g2_font_cu12_t_greek);
      u8g2.drawUTF8(0, 28, "Έναρξη");
      u8g2.drawUTF8(0, 45, "Λήξη");

      u8g2.setFont(u8g2_font_7x13_tf);
      drawTimeDigitsLineAt(68, 28, 0, 0);
      drawTimeDigitsLineAt(68, 45, 4, 4);

      u8g2.setFont(u8g2_font_cu12_t_greek);
      drawDigitEditNavigationIcons();
    } else {
      u8g2.setFont(u8g2_font_6x10_tf);
      u8g2.drawStr(0, 28, "Start");
      u8g2.drawStr(0, 45, "End");
      drawTimeDigitsLine(28, 0, 0);
      drawTimeDigitsLine(45, 4, 4);
      u8g2.setFont(u8g2_font_5x8_tf);
      drawDigitEditNavigationIcons();
    }
  }
  else if (editorPage == EDITOR_HOLIDAYS) {
    drawLocalizedEditorTitle("HOLIDAYS", "Αργίες");

    if (isGreekUI()) {
      u8g2.setFont(u8g2_font_cu12_t_greek);
      drawGreekCentered(
        39,
        editorSelection == 0 ?
          "Όχι" :
          "Ναι"
      );
    } else {
      u8g2.setFont(u8g2_font_ncenB12_tr);
      u8g2.drawStr(
        editorSelection == 0 ?
          50 :
          52,
        39,
        editorSelection == 0 ?
          "Off" :
          "On"
      );
    }

    drawSelectionNavigationIcons();
  }
  else if (editorPage == EDITOR_HOLIDAY_OPTIONS) {
    drawHolidayOptionsPage();
  }
  else if (editorPage == EDITOR_DIMMER) {
    char buf[24];
    drawLocalizedEditorTitle("DIMMER", "Φωτεινότητα");
    u8g2.setFont(u8g2_font_ncenB14_tr);
    snprintf(buf, sizeof(buf), "%d", editorSelection);
    u8g2.drawStr(53, 40, buf);

    if (isGreekUI()) {
      u8g2.setFont(u8g2_font_cu12_t_greek);
      drawSelectionNavigationIcons();
    } else {
      u8g2.setFont(u8g2_font_6x10_tf);
      drawSelectionNavigationIcons();
    }
  }
  else if (editorPage == EDITOR_SYNC) {
    char buf[24];
    drawLocalizedEditorTitle("AUTOSYNC", "Αυτοσυγχρονισμός");
    u8g2.setFont(u8g2_font_ncenB14_tr);
    snprintf(buf, sizeof(buf), "%d h", editorSelection);
    u8g2.drawStr(53, 40, buf);

    if (isGreekUI()) {
      u8g2.setFont(u8g2_font_cu12_t_greek);
      drawSelectionNavigationIcons();
    } else {
      u8g2.setFont(u8g2_font_6x10_tf);
      drawSelectionNavigationIcons();
    }
  }
  else if (editorPage == EDITOR_MONO_COLOR) {
    drawLocalizedEditorTitle("MONO COLOR", "Μονόχρωμο");

    if (isGreekUI()) {
      drawGreekCentered(39, clockColorNamesGreek[editorSelection]);
      u8g2.setFont(u8g2_font_cu12_t_greek);
      drawSelectionNavigationIcons();
    } else {
      u8g2.setFont(u8g2_font_ncenB12_tr);
      u8g2.drawStr(1, 39, clockColorNames[editorSelection]);
      u8g2.setFont(u8g2_font_6x10_tf);
      drawSelectionNavigationIcons();
    }
  }
  else if (editorPage == EDITOR_MULTI_COLOR) {
    static const char* multiItemsEnglish[] = {
      "None",
      "Solid Color",
      "Colorful",
      "Funny",
      "Animated",
      "Dynamic"
    };

    static const char* multiItemsGreek[] = {
      "Κενό",
      "Συμπαγές Χρώμα",
      "Πολύχρωμο",
      "Αστείο",
      "Animated",
      "Δυναμικό"
    };

    drawLocalizedEditorTitle("MULTI COLOR", "Πολύχρωμο");

    if (isGreekUI()) {
      drawGreekCentered(39, multiItemsGreek[editorSelection]);
      u8g2.setFont(u8g2_font_cu12_t_greek);
      drawSelectionNavigationIcons();
    } else {
      u8g2.setFont(u8g2_font_ncenB10_tr);
      u8g2.drawStr(1, 39, multiItemsEnglish[editorSelection]);
      u8g2.setFont(u8g2_font_6x10_tf);
      drawSelectionNavigationIcons();
    }
  }
  else if (editorPage == EDITOR_LANGUAGE) {
    drawLocalizedEditorTitle("LANGUAGE", "Γλώσσα");

    if (editorSelection == UI_GREEK) {
      drawGreekCentered(39, "Ελληνικά");
    } else {
      u8g2.setFont(u8g2_font_ncenB12_tr);
      u8g2.drawStr(31, 39, "English");
    }

    if (isGreekUI()) {
      u8g2.setFont(u8g2_font_cu12_t_greek);
      drawSelectionNavigationIcons();
    } else {
      u8g2.setFont(u8g2_font_6x10_tf);
      drawSelectionNavigationIcons();
    }
  }

  u8g2.sendBuffer();
}

void fillDigitsFromTime(int offset, int hour, int minute) {
  editDigits[offset + 0] = hour / 10;
  editDigits[offset + 1] = hour % 10;
  editDigits[offset + 2] = minute / 10;
  editDigits[offset + 3] = minute % 10;
}

bool readEditedTime(int offset, int &hour, int &minute) {
  hour = editDigits[offset] * 10 + editDigits[offset + 1];
  minute = editDigits[offset + 2] * 10 + editDigits[offset + 3];
  return hour >= 0 && hour <= 23 && minute >= 0 && minute <= 59;
}

void fillDigitsFromDate(
  int day,
  int month
) {

  editDigits[0] = day / 10;
  editDigits[1] = day % 10;
  editDigits[2] = month / 10;
  editDigits[3] = month % 10;
}

bool readEditedDate(
  int &day,
  int &month
) {

  day =
    editDigits[0] * 10 +
    editDigits[1];

  month =
    editDigits[2] * 10 +
    editDigits[3];

  return isValidSeasonBoundary(day, month);
}

void drawDateDigitsLine(
  int y
) {

  char buf[12];

  int day =
    editDigits[0] * 10 +
    editDigits[1];

  int month =
    editDigits[2] * 10 +
    editDigits[3];

  snprintf(
    buf,
    sizeof(buf),
    "%02d/%02d",
    day,
    month
  );

  u8g2.drawStr(
    42,
    y,
    buf
  );

  static const int xPos[4] = {
    42, 49, 63, 70
  };

  if (editorCursor >= 0 && editorCursor < 4) {
    u8g2.drawHLine(
      xPos[editorCursor],
      y + 2,
      6
    );
  }
}

void drawSiestaEditorPage() {

  static const char* titlesEnglish[8] = {
    "SUMMER DAY",
    "SUMMER NIGHT",
    "SUMMER START",
    "SUMMER END",
    "WINTER DAY",
    "WINTER NIGHT",
    "WINTER START",
    "WINTER END"
  };

  static const char* titlesGreek[8] = {
    "Θερινό Ημέρας",
    "Θερινό Νύχτας",
    "Έναρξη Θερινού",
    "Λήξη Θερινού",
    "Χειμερινό Ημέρας",
    "Χειμερινό Νύχτας",
    "Έναρξη Χειμερινού",
    "Λήξη Χειμερινού"
  };

  if (isGreekUI()) {
    drawLocalizedEditorTitle(
      titlesEnglish[siestaEditStep],
      titlesGreek[siestaEditStep]
    );
  } else {
    drawEditorTitle(
      titlesEnglish[siestaEditStep]
    );
  }

  if (
    siestaEditStep == 0 ||
    siestaEditStep == 1 ||
    siestaEditStep == 4 ||
    siestaEditStep == 5
  ) {

    if (isGreekUI()) {
      u8g2.setFont(u8g2_font_cu12_t_greek);
      u8g2.drawUTF8(0, 28, "Έναρξη");
      u8g2.drawUTF8(0, 45, "Λήξη");

      u8g2.setFont(u8g2_font_7x13_tf);
      drawTimeDigitsLineAt(68, 28, 0, 0);
      drawTimeDigitsLineAt(68, 45, 4, 4);
    } else {
      u8g2.setFont(
        u8g2_font_6x10_tf
      );

      u8g2.drawStr(0, 28, "Start");
      u8g2.drawStr(0, 45, "End");

      drawTimeDigitsLine(28, 0, 0);
      drawTimeDigitsLine(45, 4, 4);
    }

  } else {

    u8g2.setFont(
      u8g2_font_7x13_tf
    );

    drawDateDigitsLine(38);
  }

  if (isGreekUI()) {
    u8g2.setFont(u8g2_font_cu12_t_greek);
    drawDigitEditNavigationIcons();
  } else {
    u8g2.setFont(
      u8g2_font_5x8_tf
    );

    drawDigitEditNavigationIcons();
  }
}

void loadSiestaEditStep() {

  editorCursor = 0;

  switch (siestaEditStep) {

    case 0:
      fillDigitsFromTime(0, editSummerSiesta.dayStartHour, editSummerSiesta.dayStartMin);
      fillDigitsFromTime(4, editSummerSiesta.dayEndHour, editSummerSiesta.dayEndMin);
      break;

    case 1:
      fillDigitsFromTime(0, editSummerSiesta.nightStartHour, editSummerSiesta.nightStartMin);
      fillDigitsFromTime(4, editSummerSiesta.nightEndHour, editSummerSiesta.nightEndMin);
      break;

    case 2:
      fillDigitsFromDate(editSummerSiesta.startDay, editSummerSiesta.startMonth);
      break;

    case 3:
      fillDigitsFromDate(editSummerSiesta.endDay, editSummerSiesta.endMonth);
      break;

    case 4:
      fillDigitsFromTime(0, editWinterSiesta.dayStartHour, editWinterSiesta.dayStartMin);
      fillDigitsFromTime(4, editWinterSiesta.dayEndHour, editWinterSiesta.dayEndMin);
      break;

    case 5:
      fillDigitsFromTime(0, editWinterSiesta.nightStartHour, editWinterSiesta.nightStartMin);
      fillDigitsFromTime(4, editWinterSiesta.nightEndHour, editWinterSiesta.nightEndMin);
      break;

    case 6:
      fillDigitsFromDate(editWinterSiesta.startDay, editWinterSiesta.startMonth);
      break;

    case 7:
      fillDigitsFromDate(editWinterSiesta.endDay, editWinterSiesta.endMonth);
      break;
  }
}

bool commitSiestaEditStep() {

  int sh, sm, eh, em;
  int day, month;

  switch (siestaEditStep) {

    case 0:
      if (!readEditedTime(0, sh, sm) || !readEditedTime(4, eh, em)) return false;
      if ((sh * 60 + sm) == (eh * 60 + em)) return false;
      editSummerSiesta.dayStartHour = sh;
      editSummerSiesta.dayStartMin = sm;
      editSummerSiesta.dayEndHour = eh;
      editSummerSiesta.dayEndMin = em;
      return true;

    case 1:
      if (!readEditedTime(0, sh, sm) || !readEditedTime(4, eh, em)) return false;
      if ((sh * 60 + sm) == (eh * 60 + em)) return false;
      editSummerSiesta.nightStartHour = sh;
      editSummerSiesta.nightStartMin = sm;
      editSummerSiesta.nightEndHour = eh;
      editSummerSiesta.nightEndMin = em;
      return true;

    case 2:
      if (!readEditedDate(day, month)) return false;
      editSummerSiesta.startDay = day;
      editSummerSiesta.startMonth = month;
      return true;

    case 3:
      if (!readEditedDate(day, month)) return false;
      editSummerSiesta.endDay = day;
      editSummerSiesta.endMonth = month;
      return true;

    case 4:
      if (!readEditedTime(0, sh, sm) || !readEditedTime(4, eh, em)) return false;
      if ((sh * 60 + sm) == (eh * 60 + em)) return false;
      editWinterSiesta.dayStartHour = sh;
      editWinterSiesta.dayStartMin = sm;
      editWinterSiesta.dayEndHour = eh;
      editWinterSiesta.dayEndMin = em;
      return true;

    case 5:
      if (!readEditedTime(0, sh, sm) || !readEditedTime(4, eh, em)) return false;
      if ((sh * 60 + sm) == (eh * 60 + em)) return false;
      editWinterSiesta.nightStartHour = sh;
      editWinterSiesta.nightStartMin = sm;
      editWinterSiesta.nightEndHour = eh;
      editWinterSiesta.nightEndMin = em;
      return true;

    case 6:
      if (!readEditedDate(day, month)) return false;
      editWinterSiesta.startDay = day;
      editWinterSiesta.startMonth = month;
      return true;

    case 7:
      if (!readEditedDate(day, month)) return false;
      editWinterSiesta.endDay = day;
      editWinterSiesta.endMonth = month;
      return true;
  }

  return false;
}

void enterEditor() {
  testInfoActive = false;
  editorActive = true;
  editorPage = EDITOR_MAIN;
  editorMenuIndex = 0;
  editorCursor = 0;
  editorLastActivity = millis();
  selectedMessageActive = false;

  isPurpleOverride = true;
  normalLightTimer = millis();

  if (isNightDimmingMode) currentBrightness = BRIGHTNESS_MIN;
  updateDisplay();
  showEditor();
}

void exitEditor() {
  editorActive = false;
  selectedMessageActive = false;
  isPurpleOverride = false;

  DateTime now = rtc.now();
  checkBrightnessMode(now.hour(), now.minute());
  updateDisplay();
  turnOLEDOff();

  // WiFi remains OFF between scheduled sessions.
  // A changed timezone is stored immediately and will be
  // applied at the next startup or scheduled NTP sync.
  pendingTimezoneSync = false;
}

void showSelectedAndExit() {
  selectedMessageActive = true;
  selectedUntil = millis() + 3000UL;
  showEditor();
}

void showInvalidEditorValue() {
  u8g2.setPowerSave(0);
  u8g2.clearBuffer();

  if (isGreekUI()) {
    drawGreekCentered(38, "ΜΗ ΕΓΚΥΡΟ");
  } else {
    u8g2.setFont(u8g2_font_ncenB12_tr);
    u8g2.drawStr(20, 38, "INVALID");
  }

  u8g2.sendBuffer();
  delay(700);
  showEditor();
}

void beginEditorItem() {
  editorCursor = 0;

  switch (editorMenuIndex) {
    case 0:
      editorPage = EDITOR_COUNTRY;
      editorSelection = countryIndex;
      break;

    case 1:
      editorPage = EDITOR_TIMEZONE;
      editorSelection = timezoneIndex;
      break;

    case 2: {
      editorPage = EDITOR_TIME;
      DateTime now = rtc.now();
      fillDigitsFromTime(0, now.hour(), now.minute());
      break;
    }

    case 3:
      editorPage = EDITOR_HOUR_FORMAT;
      editorSelection = use24HourFormat ? 1 : 0;
      break;

    case 4:
      editorPage = EDITOR_FONT;
      editorSelection = clockFontStyle;
      break;

    case 5:
      editorPage = EDITOR_SIESTA;
      editSummerSiesta = summerSiesta;
      editWinterSiesta = winterSiesta;
      siestaEditStep = 0;
      loadSiestaEditStep();
      break;

    case 6:
      editorPage = EDITOR_NIGHT;
      fillDigitsFromTime(0, dimStartHour, dimStartMin);
      fillDigitsFromTime(4, dimEndHour, dimEndMin);
      break;

    case 7:
      editorPage = EDITOR_HOLIDAYS;
      editorSelection = holidaysEnabled ? 1 : 0;
      break;

    case 8:
      editorPage = EDITOR_DIMMER;
      editorSelection = dayBrightness;
      break;

    case 9:
      editorPage = EDITOR_SYNC;
      editorSelection = syncIntervalHours;
      break;

    case 10:
      performSyncNow();
      return;

    case 11:
      editorPage = EDITOR_MONO_COLOR;
      editorSelection = clockColorIndex;
      break;

    case 12:
      editorPage = EDITOR_MULTI_COLOR;
      editorSelection = 0;
      break;

    case 13:
      editorPage = EDITOR_LANGUAGE;
      editorSelection = uiLanguage;
      break;
  }

  showEditor();
}

void handleEditorCommand(uint16_t commandCode) {
  editorLastActivity = millis();
  normalLightTimer = millis();

  if (selectedMessageActive) return;

  if (commandCode == IR_BACK || commandCode == IR_CANCEL) {
    exitEditor();
    return;
  }

  if (editorPage == EDITOR_MAIN) {
    if (commandCode == IR_UP) {
      editorMenuIndex = (editorMenuIndex + 13) % 14;
      showEditor();
    }
    else if (commandCode == IR_DOWN) {
      editorMenuIndex = (editorMenuIndex + 1) % 14;
      showEditor();
    }
    else if (commandCode == IR_ENTER) {
      beginEditorItem();
    }
    return;
  }

  if (editorPage == EDITOR_COUNTRY) {
    if (commandCode == IR_UP) {
      editorSelection = (editorSelection + NUM_COUNTRIES - 1) % NUM_COUNTRIES;
      showEditor();
    }
    else if (commandCode == IR_DOWN) {
      editorSelection = (editorSelection + 1) % NUM_COUNTRIES;
      showEditor();
    }
    else if (commandCode == IR_ENTER) {
      countryIndex = (uint8_t)editorSelection;
      saveHolidaySettings();
      invalidateHolidayCache();

      if (holidaysEnabled && holidayPublicDatesEnabled) {
        bool refreshOK = refreshPublicHolidayCache(true);
        showHolidayDownloadResult(refreshOK);
      }

      showSelectedAndExit();
    }
    return;
  }

  if (editorPage == EDITOR_TIMEZONE) {
    if (commandCode == IR_UP) {
      editorSelection = (editorSelection + NUM_TIMEZONES - 1) % NUM_TIMEZONES;
      showEditor();
    }
    else if (commandCode == IR_DOWN) {
      editorSelection = (editorSelection + 1) % NUM_TIMEZONES;
      showEditor();
    }
    else if (commandCode == IR_ENTER) {
      timezoneIndex = editorSelection;
      applyTimezone(timezoneIndex);
      saveTimezone();
      pendingTimezoneSync = true;
      showSelectedAndExit();
    }
    return;
  }

  if (editorPage == EDITOR_HOUR_FORMAT) {

    if (
      commandCode == IR_UP ||
      commandCode == IR_DOWN
    ) {

      editorSelection =
        editorSelection == 0 ?
        1 :
        0;

      showEditor();
    }
    else if (
      commandCode == IR_ENTER
    ) {

      use24HourFormat =
        editorSelection == 1;

      saveDisplaySettings();
      showSelectedAndExit();
    }

    return;
  }

  if (editorPage == EDITOR_FONT) {

    if (commandCode == IR_UP) {
      editorSelection =
        (
          editorSelection +
          NUM_CLOCK_FONT_STYLES -
          1
        ) %
        NUM_CLOCK_FONT_STYLES;

      showEditor();
    }
    else if (commandCode == IR_DOWN) {
      editorSelection =
        (
          editorSelection +
          1
        ) %
        NUM_CLOCK_FONT_STYLES;

      showEditor();
    }
    else if (commandCode == IR_ENTER) {

      clockFontStyle =
        (uint8_t)editorSelection;

      saveDisplaySettings();
      showSelectedAndExit();
    }

    return;
  }

  if (editorPage == EDITOR_SIESTA) {

    int maxCursor =
      (siestaEditStep == 2 ||
       siestaEditStep == 3 ||
       siestaEditStep == 6 ||
       siestaEditStep == 7) ?
      3 : 7;

    int digit =
      digitFromCommand(commandCode);

    if (digit >= 0) {
      editDigits[editorCursor] = digit;
      if (editorCursor < maxCursor) editorCursor++;
      showEditor();
      return;
    }

    if (commandCode == IR_MOVE_LEFT) {
      editorCursor--;
      if (editorCursor < 0) editorCursor = maxCursor;
      showEditor();
      return;
    }

    if (commandCode == IR_MOVE_RIGHT) {
      editorCursor++;
      if (editorCursor > maxCursor) editorCursor = 0;
      showEditor();
      return;
    }

    if (commandCode == IR_ENTER) {

      if (!commitSiestaEditStep()) {
        showInvalidEditorValue();
        return;
      }

      if (siestaEditStep < 7) {
        siestaEditStep++;
        loadSiestaEditStep();
        showEditor();
        return;
      }

      if (
        !validateSiestaConfiguration(
          editSummerSiesta,
          editWinterSiesta
        )
      ) {

        // Keep all entered values in RAM and restart at the first
        // Siesta screen so the user can correct the configuration.
        siestaEditStep = 0;
        loadSiestaEditStep();
        showInvalidEditorValue();
        return;
      }

      summerSiesta =
        editSummerSiesta;

      winterSiesta =
        editWinterSiesta;

      saveSiestaSettings();
      showSelectedAndExit();
      return;
    }

    return;
  }

  if (editorPage == EDITOR_HOLIDAYS) {
    if (commandCode == IR_UP || commandCode == IR_DOWN) {
      editorSelection =
        editorSelection == 0 ?
        1 :
        0;

      showEditor();
    }
    else if (commandCode == IR_ENTER) {

      if (editorSelection == 0) {

        holidaysEnabled = false;
        saveHolidaySettings();
        showSelectedAndExit();

      } else {

        // Edit transaction: nothing changes in the active configuration
        // until the user accepts the Time row with ENTER.
        editHolidayPublicDatesEnabled =
          holidayPublicDatesEnabled;

        editHolidaySaturdaysEnabled =
          holidaySaturdaysEnabled;

        editHolidaySundaysEnabled =
          holidaySundaysEnabled;

        fillDigitsFromTime(
          0,
          holidayOverrideHour,
          holidayOverrideMinute
        );

        editorPage =
          EDITOR_HOLIDAY_OPTIONS;

        editorSelection = 0;
        editorCursor = 0;

        showEditor();
      }
    }
    return;
  }

  if (editorPage == EDITOR_HOLIDAY_OPTIONS) {

    if (commandCode == IR_UP) {
      editorSelection--;
      if (editorSelection < 0) {
        editorSelection = 3;
      }
      showEditor();
      return;
    }

    if (commandCode == IR_DOWN) {
      editorSelection++;
      if (editorSelection > 3) {
        editorSelection = 0;
      }
      showEditor();
      return;
    }

    if (editorSelection < 3) {

      if (commandCode == IR_ENTER) {

        if (editorSelection == 0) {
          editHolidayPublicDatesEnabled =
            !editHolidayPublicDatesEnabled;
        }
        else if (editorSelection == 1) {
          editHolidaySaturdaysEnabled =
            !editHolidaySaturdaysEnabled;
        }
        else {
          editHolidaySundaysEnabled =
            !editHolidaySundaysEnabled;
        }

        showEditor();
      }

      return;
    }

    // Time row.
    int digit =
      digitFromCommand(
        commandCode
      );

    if (digit >= 0) {
      editDigits[editorCursor] =
        digit;

      if (editorCursor < 3) {
        editorCursor++;
      }

      showEditor();
      return;
    }

    if (commandCode == IR_MOVE_LEFT) {
      editorCursor--;

      if (editorCursor < 0) {
        editorCursor = 3;
      }

      showEditor();
      return;
    }

    if (commandCode == IR_MOVE_RIGHT) {
      editorCursor++;

      if (editorCursor > 3) {
        editorCursor = 0;
      }

      showEditor();
      return;
    }

    if (commandCode == IR_ENTER) {

      int h, m;

      if (
        !readEditedTime(
          0,
          h,
          m
        )
      ) {
        showInvalidEditorValue();
        return;
      }

      holidaysEnabled = true;

      holidayPublicDatesEnabled =
        editHolidayPublicDatesEnabled;

      holidaySaturdaysEnabled =
        editHolidaySaturdaysEnabled;

      holidaySundaysEnabled =
        editHolidaySundaysEnabled;

      holidayOverrideHour =
        (uint8_t)h;

      holidayOverrideMinute =
        (uint8_t)m;

      saveHolidaySettings();
      saveHolidayRuleSettings();

      // Internet is needed only when the Public Holidays checkbox is active.
      if (holidayPublicDatesEnabled) {
        bool refreshOK =
          refreshPublicHolidayCache(
            true
          );

        showHolidayDownloadResult(
          refreshOK
        );
      }

      showSelectedAndExit();
    }

    return;
  }

  if (editorPage == EDITOR_DIMMER) {
    if (commandCode == IR_UP) {
      if (editorSelection < BRIGHTNESS_MAX) editorSelection++;
      showEditor();
    }
    else if (commandCode == IR_DOWN) {
      if (editorSelection > BRIGHTNESS_MIN) editorSelection--;
      showEditor();
    }
    else if (commandCode == IR_ENTER) {
      dayBrightness = editorSelection;
      saveBrightness();
      if (!isNightDimmingMode) currentBrightness = dayBrightness;
      showSelectedAndExit();
    }
    return;
  }

  if (editorPage == EDITOR_SYNC) {
    if (commandCode == IR_UP) {
      if (editorSelection < 255) editorSelection++;
      showEditor();
    }
    else if (commandCode == IR_DOWN) {
      if (editorSelection > 1) editorSelection--;
      showEditor();
    }
    else if (commandCode == IR_ENTER) {
      syncIntervalHours = (uint8_t)editorSelection;
      saveSyncInterval();
      showSelectedAndExit();
    }
    return;
  }

  if (editorPage == EDITOR_MONO_COLOR) {
    if (commandCode == IR_UP) {
      editorSelection = (editorSelection + NUM_CLOCK_COLORS - 1) % NUM_CLOCK_COLORS;
      showEditor();
    }
    else if (commandCode == IR_DOWN) {
      editorSelection = (editorSelection + 1) % NUM_CLOCK_COLORS;
      showEditor();
    }
    else if (commandCode == IR_ENTER) {
      clockColorIndex = editorSelection;

      // Selecting a Mono Color explicitly returns normal daytime
      // operation to Mono Color.
      dayColorMode = DAY_COLOR_MONO;

      saveColorSetting();
      saveDayColorMode();
      showSelectedAndExit();
    }
    return;
  }

  if (editorPage == EDITOR_MULTI_COLOR) {
    if (commandCode == IR_UP) {
      editorSelection = (editorSelection + 5) % 6;
      showEditor();
    }
    else if (commandCode == IR_DOWN) {
      editorSelection = (editorSelection + 1) % 6;
      showEditor();
    }
    else if (commandCode == IR_ENTER) {

      // None only returns to the editor menu.  It changes nothing.
      if (editorSelection == 0) {
        editorPage = EDITOR_MAIN;
        showEditor();
        return;
      }

      if (editorSelection == 1) {
        dayColorMode = DAY_COLOR_SOLID;
      }
      else if (editorSelection == 2) {
        dayColorMode = DAY_COLOR_COLORFUL;
      }
      else if (editorSelection == 3) {
        dayColorMode = DAY_COLOR_FUNNY;
      }
      else if (editorSelection == 4) {
        dayColorMode = DAY_COLOR_ANIMATED;
      }
      else {
        dayColorMode = DAY_COLOR_DYNAMIC;
      }

      saveDayColorMode();

      dailyColorDateKey = 0;
      ensureDailyColors(
        rtc.now()
      );

      showSelectedAndExit();
    }
    return;
  }

  if (editorPage == EDITOR_LANGUAGE) {

    if (
      commandCode == IR_UP ||
      commandCode == IR_DOWN
    ) {
      editorSelection =
        editorSelection == UI_ENGLISH ?
        UI_GREEK :
        UI_ENGLISH;

      showEditor();
    }
    else if (commandCode == IR_ENTER) {
      uiLanguage =
        (uint8_t)editorSelection;

      saveLanguageSetting();
      showSelectedAndExit();
    }

    return;
  }

  int maxCursor = (editorPage == EDITOR_TIME) ? 3 : 7;
  int digit = digitFromCommand(commandCode);

  if (digit >= 0) {
    editDigits[editorCursor] = digit;
    if (editorCursor < maxCursor) editorCursor++;
    showEditor();
    return;
  }

  if (commandCode == IR_MOVE_LEFT) {
    editorCursor--;
    if (editorCursor < 0) editorCursor = maxCursor;
    showEditor();
    return;
  }

  if (commandCode == IR_MOVE_RIGHT) {
    editorCursor++;
    if (editorCursor > maxCursor) editorCursor = 0;
    showEditor();
    return;
  }

  if (commandCode != IR_ENTER) return;

  if (editorPage == EDITOR_TIME) {
    int h, m;
    if (!readEditedTime(0, h, m)) {
      showInvalidEditorValue();
      return;
    }
    DateTime now = rtc.now();
    rtc.adjust(DateTime(now.year(), now.month(), now.day(), h, m, 0));
    timeSyncSuccess = true;
    showSelectedAndExit();
    return;
  }

  if (editorPage == EDITOR_NIGHT) {
    int sh, sm, eh, em;
    if (!readEditedTime(0, sh, sm) || !readEditedTime(4, eh, em)) {
      showInvalidEditorValue();
      return;
    }
    dimStartHour = sh;
    dimStartMin = sm;
    dimEndHour = eh;
    dimEndMin = em;
    saveDimSettings();
    showSelectedAndExit();
    return;
  }
}

void showTestInfo() {
  char ipLine[32];
  snprintf(ipLine, sizeof(ipLine), "IP: %s", lastKnownIP.c_str());

  editorActive = false;
  selectedMessageActive = false;
  isPurpleOverride = false;
  testInfoActive = true;
  testInfoStarted = millis();
  currentOLEDState = OLED_IR_COMMAND;
  oledOffTimer = millis();

  u8g2.setPowerSave(0);
  u8g2.clearBuffer();
  if (isGreekUI()) {
    u8g2.setFont(u8g2_font_cu12_t_greek);
    u8g2.drawUTF8(0, 15, "ΠΛΗΡΟΦΟΡΙΕΣ");
    char zoneName[52];
    getTimezoneName(timezoneIndex, zoneName, sizeof(zoneName));
    u8g2.setFont(u8g2_font_5x8_tf);
    u8g2.drawStr(0, 36, zoneName);
    u8g2.drawStr(0, 54, ipLine);
  } else {
    u8g2.setFont(u8g2_font_ncenB10_tr);
    u8g2.drawStr(0, 15, "CLOCK INFO");
    u8g2.setFont(u8g2_font_6x10_tf);
    char zoneName[52];
    getTimezoneName(timezoneIndex, zoneName, sizeof(zoneName));
    u8g2.drawStr(0, 36, zoneName);
    u8g2.drawStr(0, 54, ipLine);
  }
  u8g2.sendBuffer();
}

// ============================================================
// IR REMOTE
// ============================================================

void handleIRCommands() {
  if (digitalRead(IR_PIN) == HIGH) return;

  NECResult r = readNECFrame();
  if (!r.valid) return;


  // Ignore NEC repeat frames for menu navigation / data entry.
  // A fresh full frame is required for each editor action.
  if (r.repeat) return;

  if (editorActive) {
    handleEditorCommand(r.commandCode);
    return;
  }

  if (r.commandCode == IR_MENU) {
    enterEditor();
    return;
  }

  if (r.commandCode == IR_TEST) {
    showTestInfo();
    return;
  }
}

// ============================================================
// SAVE DIM SETTINGS
// ============================================================

void saveDimSettings() {

  EEPROM.write(
    EEPROM_DIM_START_HOUR,
    dimStartHour
  );

  EEPROM.write(
    EEPROM_DIM_START_MIN,
    dimStartMin
  );

  EEPROM.write(
    EEPROM_DIM_END_HOUR,
    dimEndHour
  );

  EEPROM.write(
    EEPROM_DIM_END_MIN,
    dimEndMin
  );

  EEPROM.commit();
}

// ============================================================
// LOAD DIM SETTINGS
// ============================================================

void loadDimSettings() {

  dimStartHour =
    EEPROM.read(
      EEPROM_DIM_START_HOUR
    );

  dimStartMin =
    EEPROM.read(
      EEPROM_DIM_START_MIN
    );

  dimEndHour =
    EEPROM.read(
      EEPROM_DIM_END_HOUR
    );

  dimEndMin =
    EEPROM.read(
      EEPROM_DIM_END_MIN
    );

  if (
    dimStartHour > 23 ||
    dimStartMin > 59 ||
    dimEndHour > 23 ||
    dimEndMin > 59
  ) {

    dimStartHour = 23;
    dimStartMin = 0;

    dimEndHour = 7;
    dimEndMin = 0;

    saveDimSettings();
  }
}

// ============================================================
// EXTRA SETTINGS: SEASONAL SIESTA + COLOR
// ============================================================

void saveSiestaSettings() {

  EEPROM.write(EEPROM_SIESTA_SUM_DAY_SH, summerSiesta.dayStartHour);
  EEPROM.write(EEPROM_SIESTA_SUM_DAY_SM, summerSiesta.dayStartMin);
  EEPROM.write(EEPROM_SIESTA_SUM_DAY_EH, summerSiesta.dayEndHour);
  EEPROM.write(EEPROM_SIESTA_SUM_DAY_EM, summerSiesta.dayEndMin);
  EEPROM.write(EEPROM_SIESTA_SUM_NIGHT_SH, summerSiesta.nightStartHour);
  EEPROM.write(EEPROM_SIESTA_SUM_NIGHT_SM, summerSiesta.nightStartMin);
  EEPROM.write(EEPROM_SIESTA_SUM_NIGHT_EH, summerSiesta.nightEndHour);
  EEPROM.write(EEPROM_SIESTA_SUM_NIGHT_EM, summerSiesta.nightEndMin);
  EEPROM.write(EEPROM_SIESTA_SUM_START_D, summerSiesta.startDay);
  EEPROM.write(EEPROM_SIESTA_SUM_START_MO, summerSiesta.startMonth);
  EEPROM.write(EEPROM_SIESTA_SUM_END_D, summerSiesta.endDay);
  EEPROM.write(EEPROM_SIESTA_SUM_END_MO, summerSiesta.endMonth);

  EEPROM.write(EEPROM_SIESTA_WIN_DAY_SH, winterSiesta.dayStartHour);
  EEPROM.write(EEPROM_SIESTA_WIN_DAY_SM, winterSiesta.dayStartMin);
  EEPROM.write(EEPROM_SIESTA_WIN_DAY_EH, winterSiesta.dayEndHour);
  EEPROM.write(EEPROM_SIESTA_WIN_DAY_EM, winterSiesta.dayEndMin);
  EEPROM.write(EEPROM_SIESTA_WIN_NIGHT_SH, winterSiesta.nightStartHour);
  EEPROM.write(EEPROM_SIESTA_WIN_NIGHT_SM, winterSiesta.nightStartMin);
  EEPROM.write(EEPROM_SIESTA_WIN_NIGHT_EH, winterSiesta.nightEndHour);
  EEPROM.write(EEPROM_SIESTA_WIN_NIGHT_EM, winterSiesta.nightEndMin);
  EEPROM.write(EEPROM_SIESTA_WIN_START_D, winterSiesta.startDay);
  EEPROM.write(EEPROM_SIESTA_WIN_START_MO, winterSiesta.startMonth);
  EEPROM.write(EEPROM_SIESTA_WIN_END_D, winterSiesta.endDay);
  EEPROM.write(EEPROM_SIESTA_WIN_END_MO, winterSiesta.endMonth);

  EEPROM.write(
    EEPROM_SIESTA_SEASONS_MAGIC,
    SIESTA_SEASONS_MAGIC_VALUE
  );

  EEPROM.commit();
}

void setDefaultSiestaSettings() {

  summerSiesta = {
    15, 0, 17, 30,
    23, 0, 7, 0,
    1, 4, 30, 9
  };

  winterSiesta = {
    15, 30, 17, 30,
    22, 0, 7, 30,
    1, 10, 31, 3
  };
}

void loadSiestaSettings() {

  if (
    EEPROM.read(EEPROM_SIESTA_SEASONS_MAGIC) !=
    SIESTA_SEASONS_MAGIC_VALUE
  ) {

    setDefaultSiestaSettings();
    saveSiestaSettings();
    return;
  }

  summerSiesta.dayStartHour = EEPROM.read(EEPROM_SIESTA_SUM_DAY_SH);
  summerSiesta.dayStartMin = EEPROM.read(EEPROM_SIESTA_SUM_DAY_SM);
  summerSiesta.dayEndHour = EEPROM.read(EEPROM_SIESTA_SUM_DAY_EH);
  summerSiesta.dayEndMin = EEPROM.read(EEPROM_SIESTA_SUM_DAY_EM);
  summerSiesta.nightStartHour = EEPROM.read(EEPROM_SIESTA_SUM_NIGHT_SH);
  summerSiesta.nightStartMin = EEPROM.read(EEPROM_SIESTA_SUM_NIGHT_SM);
  summerSiesta.nightEndHour = EEPROM.read(EEPROM_SIESTA_SUM_NIGHT_EH);
  summerSiesta.nightEndMin = EEPROM.read(EEPROM_SIESTA_SUM_NIGHT_EM);
  summerSiesta.startDay = EEPROM.read(EEPROM_SIESTA_SUM_START_D);
  summerSiesta.startMonth = EEPROM.read(EEPROM_SIESTA_SUM_START_MO);
  summerSiesta.endDay = EEPROM.read(EEPROM_SIESTA_SUM_END_D);
  summerSiesta.endMonth = EEPROM.read(EEPROM_SIESTA_SUM_END_MO);

  winterSiesta.dayStartHour = EEPROM.read(EEPROM_SIESTA_WIN_DAY_SH);
  winterSiesta.dayStartMin = EEPROM.read(EEPROM_SIESTA_WIN_DAY_SM);
  winterSiesta.dayEndHour = EEPROM.read(EEPROM_SIESTA_WIN_DAY_EH);
  winterSiesta.dayEndMin = EEPROM.read(EEPROM_SIESTA_WIN_DAY_EM);
  winterSiesta.nightStartHour = EEPROM.read(EEPROM_SIESTA_WIN_NIGHT_SH);
  winterSiesta.nightStartMin = EEPROM.read(EEPROM_SIESTA_WIN_NIGHT_SM);
  winterSiesta.nightEndHour = EEPROM.read(EEPROM_SIESTA_WIN_NIGHT_EH);
  winterSiesta.nightEndMin = EEPROM.read(EEPROM_SIESTA_WIN_NIGHT_EM);
  winterSiesta.startDay = EEPROM.read(EEPROM_SIESTA_WIN_START_D);
  winterSiesta.startMonth = EEPROM.read(EEPROM_SIESTA_WIN_START_MO);
  winterSiesta.endDay = EEPROM.read(EEPROM_SIESTA_WIN_END_D);
  winterSiesta.endMonth = EEPROM.read(EEPROM_SIESTA_WIN_END_MO);

  if (
    !validateSiestaConfiguration(
      summerSiesta,
      winterSiesta
    )
  ) {

    setDefaultSiestaSettings();
    saveSiestaSettings();
  }
}

void saveColorSetting() {
  EEPROM.write(EEPROM_COLOR_INDEX, clockColorIndex);
  EEPROM.commit();
}

void saveDayColorMode() {
  EEPROM.write(
    EEPROM_DAY_COLOR_MODE,
    dayColorMode
  );

  EEPROM.write(
    EEPROM_COLOR_MODE_MAGIC,
    COLOR_MODE_MAGIC_VALUE
  );

  EEPROM.commit();
}

void loadDayColorMode() {
  int magic =
    EEPROM.read(
      EEPROM_COLOR_MODE_MAGIC
    );

  int mode =
    EEPROM.read(
      EEPROM_DAY_COLOR_MODE
    );

  if (
    magic == COLOR_MODE_MAGIC_VALUE &&
    mode >= DAY_COLOR_MONO &&
    mode <= DAY_COLOR_ANIMATED
  ) {

    dayColorMode =
      (uint8_t)mode;

  } else {

    // First startup with this firmware keeps established Mono Color.
    dayColorMode =
      DAY_COLOR_MONO;

    saveDayColorMode();
  }
}

void saveSyncInterval() {
  EEPROM.write(
    EEPROM_SYNC_HOURS,
    syncIntervalHours
  );
  EEPROM.commit();
}

void loadSyncInterval() {
  int value =
    EEPROM.read(
      EEPROM_SYNC_HOURS
    );

  if (
    value >= 1 &&
    value <= 255
  ) {
    syncIntervalHours =
      (uint8_t)value;
  } else {
    syncIntervalHours =
      DEFAULT_SYNC_HOURS;
    saveSyncInterval();
  }
}

unsigned long getSyncIntervalMs() {
  return (
    (unsigned long)syncIntervalHours *
    3600000UL
  );
}

void loadExtraSettings() {

  loadSiestaSettings();

  int ci = EEPROM.read(EEPROM_COLOR_INDEX);
  if (ci >= 0 && ci < (int)NUM_CLOCK_COLORS) {
    clockColorIndex = ci;
  } else {
    clockColorIndex = 0;
    saveColorSetting();
  }
}

// ============================================================
// DISPLAY FORMAT SETTINGS: 12/24 HOUR + CLOCK FONT
// ============================================================

void saveDisplaySettings() {

  EEPROM.write(
    EEPROM_HOUR_FORMAT_24,
    use24HourFormat ?
      1 :
      0
  );

  EEPROM.write(
    EEPROM_CLOCK_FONT_STYLE,
    clockFontStyle
  );

  EEPROM.write(
    EEPROM_DISPLAY_SETTINGS_MAGIC,
    DISPLAY_SETTINGS_MAGIC_VALUE
  );

  EEPROM.commit();
}

void loadDisplaySettings() {

  int magic =
    EEPROM.read(
      EEPROM_DISPLAY_SETTINGS_MAGIC
    );

  int storedFormat =
    EEPROM.read(
      EEPROM_HOUR_FORMAT_24
    );

  int storedFont =
    EEPROM.read(
      EEPROM_CLOCK_FONT_STYLE
    );

  if (
    magic ==
      DISPLAY_SETTINGS_MAGIC_VALUE &&
    (
      storedFormat == 0 ||
      storedFormat == 1
    ) &&
    storedFont >= 0 &&
    storedFont <
      NUM_CLOCK_FONT_STYLES
  ) {

    use24HourFormat =
      storedFormat == 1;

    clockFontStyle =
      (uint8_t)storedFont;

  } else {

    // Safe upgrade/default behavior exactly matches the established clock.
    use24HourFormat = false;
    clockFontStyle = 0;

    saveDisplaySettings();
  }
}

// ============================================================
// OLED UI LANGUAGE
// ============================================================

void saveLanguageSetting() {

  EEPROM.write(
    EEPROM_UI_LANGUAGE,
    uiLanguage
  );

  EEPROM.write(
    EEPROM_UI_LANGUAGE_MAGIC,
    UI_LANGUAGE_MAGIC_VALUE
  );

  EEPROM.commit();
}

void loadLanguageSetting() {

  int magic =
    EEPROM.read(
      EEPROM_UI_LANGUAGE_MAGIC
    );

  int storedLanguage =
    EEPROM.read(
      EEPROM_UI_LANGUAGE
    );

  if (
    magic == UI_LANGUAGE_MAGIC_VALUE &&
    (
      storedLanguage == UI_ENGLISH ||
      storedLanguage == UI_GREEK
    )
  ) {
    uiLanguage =
      (uint8_t)storedLanguage;
  } else {
    // Every existing English-only firmware upgrades safely to English.
    uiLanguage = UI_ENGLISH;
    saveLanguageSetting();
  }
}

// ============================================================
// COUNTRY + HOLIDAYS
// ============================================================

void saveHolidaySettings() {
  EEPROM.write(EEPROM_COUNTRY_INDEX, countryIndex);
  EEPROM.write(EEPROM_PUBLIC_HOLIDAYS_ENABLED, holidaysEnabled ? 1 : 0);
  EEPROM.write(EEPROM_PUBLIC_HOLIDAY_HOUR, holidayOverrideHour);
  EEPROM.write(EEPROM_PUBLIC_HOLIDAY_MIN, holidayOverrideMinute);
  EEPROM.write(EEPROM_HOLIDAY_SETTINGS_MAGIC, HOLIDAY_SETTINGS_MAGIC_VALUE);
  EEPROM.commit();
}

void loadHolidaySettings() {
  int magic = EEPROM.read(EEPROM_HOLIDAY_SETTINGS_MAGIC);
  int storedCountry = EEPROM.read(EEPROM_COUNTRY_INDEX);
  int storedEnabled = EEPROM.read(EEPROM_PUBLIC_HOLIDAYS_ENABLED);
  int storedHour = EEPROM.read(EEPROM_PUBLIC_HOLIDAY_HOUR);
  int storedMinute = EEPROM.read(EEPROM_PUBLIC_HOLIDAY_MIN);

  if (
    magic == HOLIDAY_SETTINGS_MAGIC_VALUE &&
    storedCountry >= 0 && storedCountry < NUM_COUNTRIES &&
    (storedEnabled == 0 || storedEnabled == 1) &&
    storedHour >= 0 && storedHour <= 23 &&
    storedMinute >= 0 && storedMinute <= 59
  ) {
    countryIndex = (uint8_t)storedCountry;
    holidaysEnabled = storedEnabled == 1;
    holidayOverrideHour = (uint8_t)storedHour;
    holidayOverrideMinute = (uint8_t)storedMinute;
  } else {
    countryIndex = 0;              // Greece
    holidaysEnabled = false;
    holidayOverrideHour = 8;
    holidayOverrideMinute = 0;
    saveHolidaySettings();
  }
}

void saveHolidayRuleSettings() {

  uint8_t mask = 0;

  if (holidayPublicDatesEnabled) {
    mask |= 0x01;
  }

  if (holidaySaturdaysEnabled) {
    mask |= 0x02;
  }

  if (holidaySundaysEnabled) {
    mask |= 0x04;
  }

  EEPROM.write(
    EEPROM_HOLIDAY_RULES_MASK,
    mask
  );

  EEPROM.write(
    EEPROM_HOLIDAY_RULES_MAGIC,
    HOLIDAY_RULES_MAGIC_VALUE
  );

  EEPROM.commit();
}

void loadHolidayRuleSettings() {

  int magic =
    EEPROM.read(
      EEPROM_HOLIDAY_RULES_MAGIC
    );

  if (
    magic ==
    HOLIDAY_RULES_MAGIC_VALUE
  ) {

    uint8_t mask =
      EEPROM.read(
        EEPROM_HOLIDAY_RULES_MASK
      );

    holidayPublicDatesEnabled =
      (mask & 0x01) != 0;

    holidaySaturdaysEnabled =
      (mask & 0x02) != 0;

    holidaySundaysEnabled =
      (mask & 0x04) != 0;

  } else {

    // Upgrade from v7: preserve exactly the behavior that already existed.
    // Public Holidays remains selected; weekends remain unselected.
    holidayPublicDatesEnabled = true;
    holidaySaturdaysEnabled = false;
    holidaySundaysEnabled = false;

    saveHolidayRuleSettings();
  }
}

void invalidateHolidayCache() {
  holidayCacheValid = false;
  holidayCacheYear = 0;
  holidayCacheCountry = countryIndex;
  memset(holidayBitmap, 0, sizeof(holidayBitmap));
  EEPROM.write(EEPROM_HOLIDAY_CACHE_MAGIC, 0x00);
  EEPROM.commit();
}

void loadHolidayCache() {
  int magic = EEPROM.read(EEPROM_HOLIDAY_CACHE_MAGIC);
  holidayCacheYear =
    (uint16_t)EEPROM.read(EEPROM_HOLIDAY_CACHE_YEAR_LO) |
    ((uint16_t)EEPROM.read(EEPROM_HOLIDAY_CACHE_YEAR_HI) << 8);
  holidayCacheCountry = EEPROM.read(EEPROM_HOLIDAY_CACHE_COUNTRY);

  for (int i = 0; i < HOLIDAY_BITMAP_BYTES; i++) {
    holidayBitmap[i] = EEPROM.read(EEPROM_HOLIDAY_BITMAP_START + i);
  }

  holidayCacheValid =
    magic == HOLIDAY_CACHE_MAGIC_VALUE &&
    holidayCacheYear >= 2000 &&
    holidayCacheCountry < NUM_COUNTRIES;
}

int dayOfYearIndex(int year, int month, int day) {
  static const uint16_t daysBeforeMonth[12] = {
    0,31,59,90,120,151,181,212,243,273,304,334
  };

  if (month < 1 || month > 12 || day < 1 || day > 31) return -1;

  int index = daysBeforeMonth[month - 1] + day - 1;
  bool leap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
  if (leap && month > 2) index++;
  if (index < 0 || index > 365) return -1;
  return index;
}

bool isPublicHolidayDate(const DateTime &now) {
  if (!holidayCacheValid) return false;
  if (holidayCacheYear != now.year()) return false;
  if (holidayCacheCountry != countryIndex) return false;

  int dayIndex = dayOfYearIndex(now.year(), now.month(), now.day());
  if (dayIndex < 0) return false;

  int byteIndex = dayIndex >> 3;
  uint8_t bitMask = (uint8_t)(1U << (dayIndex & 7));
  return (holidayBitmap[byteIndex] & bitMask) != 0;
}

bool isHolidayOverrideDate(const DateTime &now) {

  if (!holidaysEnabled) {
    return false;
  }

  // RTClib dayOfTheWeek(): Sunday = 0 ... Saturday = 6.
  uint8_t dayOfWeek =
    now.dayOfTheWeek();

  if (
    holidaySaturdaysEnabled &&
    dayOfWeek == 6
  ) {
    return true;
  }

  if (
    holidaySundaysEnabled &&
    dayOfWeek == 0
  ) {
    return true;
  }

  if (
    holidayPublicDatesEnabled &&
    isPublicHolidayDate(now)
  ) {
    return true;
  }

  return false;
}

bool downloadPublicHolidays(uint16_t year) {
  if (WiFi.status() != WL_CONNECTED) return false;

  char countryCode[3];
  getCountryCode(countryIndex, countryCode, sizeof(countryCode));

  String url = "https://date.nager.at/api/v3/PublicHolidays/";
  url += String(year);
  url += "/";
  url += countryCode;

  BearSSL::WiFiClientSecure client;
  client.setInsecure();

  HTTPClient https;
  https.setTimeout(12000);
  if (!https.begin(client, url)) return false;

  int httpCode = https.GET();
  if (httpCode != HTTP_CODE_OK) {
    https.end();
    return false;
  }

  String payload = https.getString();
  https.end();

  uint8_t newBitmap[HOLIDAY_BITMAP_BYTES] = {0};
  int searchFrom = 0;

  while (true) {
    int objectStart = payload.indexOf('{', searchFrom);
    if (objectStart < 0) break;
    int objectEnd = payload.indexOf('}', objectStart);
    if (objectEnd < 0) break;

    // Only nationwide entries override Night Mode. Regional holidays are
    // deliberately ignored because the Country menu has no subdivision.
    int globalPos = payload.indexOf("\"global\":true", objectStart);
    int publicTypePos = payload.indexOf("\"Public\"", objectStart);
    if (
      globalPos >= 0 && globalPos < objectEnd &&
      publicTypePos >= 0 && publicTypePos < objectEnd
    ) {
      int dateKey = payload.indexOf("\"date\":\"", objectStart);
      if (dateKey >= 0 && dateKey < objectEnd) {
        dateKey += 8;
        if (dateKey + 10 <= payload.length()) {
          int y = payload.substring(dateKey, dateKey + 4).toInt();
          int m = payload.substring(dateKey + 5, dateKey + 7).toInt();
          int d = payload.substring(dateKey + 8, dateKey + 10).toInt();
          if (y == year) {
            int dayIndex = dayOfYearIndex(y, m, d);
            if (dayIndex >= 0) {
              newBitmap[dayIndex >> 3] |= (uint8_t)(1U << (dayIndex & 7));
            }
          }
        }
      }
    }

    searchFrom = objectEnd + 1;
    yield();
  }

  memcpy(holidayBitmap, newBitmap, HOLIDAY_BITMAP_BYTES);
  holidayCacheYear = year;
  holidayCacheCountry = countryIndex;
  holidayCacheValid = true;

  EEPROM.write(EEPROM_HOLIDAY_CACHE_YEAR_LO, year & 0xFF);
  EEPROM.write(EEPROM_HOLIDAY_CACHE_YEAR_HI, (year >> 8) & 0xFF);
  EEPROM.write(EEPROM_HOLIDAY_CACHE_COUNTRY, countryIndex);
  for (int i = 0; i < HOLIDAY_BITMAP_BYTES; i++) {
    EEPROM.write(EEPROM_HOLIDAY_BITMAP_START + i, holidayBitmap[i]);
  }
  EEPROM.write(EEPROM_HOLIDAY_CACHE_MAGIC, HOLIDAY_CACHE_MAGIC_VALUE);
  EEPROM.commit();
  return true;
}

bool refreshPublicHolidayCache(bool connectIfNeeded) {
  if (
    !holidaysEnabled ||
    !holidayPublicDatesEnabled ||
    !rtcInitialized
  ) {
    return false;
  }

  bool openedConnection = false;
  if (WiFi.status() != WL_CONNECTED && connectIfNeeded) {
    openedConnection = reconnectSavedWiFi(20000UL);
    if (!openedConnection) return false;
  }

  bool ok = false;
  if (WiFi.status() == WL_CONNECTED) {
    ok = downloadPublicHolidays(rtc.now().year());
  }

  if (openedConnection) {
    disconnectWiFiCompletely();
  }

  return ok;
}

void showHolidayDownloadResult(bool ok) {
  u8g2.setPowerSave(0);
  u8g2.clearBuffer();

  if (isGreekUI()) {
    u8g2.setFont(u8g2_font_cu12_t_greek);
    drawGreekCentered(24, "Αργίες");
    drawGreekCentered(48, ok ? "ΕΝΗΜΕΡΩΘΗΚΑΝ" : "ΧΩΡΙΣ ΔΕΔΟΜΕΝΑ");
  } else {
    u8g2.setFont(u8g2_font_ncenB10_tr);
    u8g2.drawStr(30, 24, "HOLIDAYS");
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(18, 48, ok ? "DATA UPDATED" : "NO HOLIDAY DATA");
  }

  u8g2.sendBuffer();
  delay(900);
}

// ============================================================
// SAVE BRIGHTNESS
// ============================================================

void saveBrightness() {

  EEPROM.write(
    EEPROM_BRIGHTNESS,
    dayBrightness
  );

  EEPROM.commit();
}

// ============================================================
// LOAD BRIGHTNESS
// ============================================================

void loadBrightness() {

  int value =
    EEPROM.read(
      EEPROM_BRIGHTNESS
    );

  if (
    value >= BRIGHTNESS_MIN &&
    value <= BRIGHTNESS_MAX
  ) {

    dayBrightness =
      value;

  } else {

    dayBrightness =
      BRIGHTNESS_DEFAULT;
  }

  currentBrightness =
    dayBrightness;
}

// ============================================================
// SAVE / LOAD TIMEZONE
// ============================================================

void saveTimezone() {
  EEPROM.write(EEPROM_TIMEZONE_EXT_LO, timezoneIndex & 0xFF);
  EEPROM.write(EEPROM_TIMEZONE_EXT_HI, (timezoneIndex >> 8) & 0xFF);
  EEPROM.write(EEPROM_TIMEZONE_EXT_MAGIC, TIMEZONE_EXT_MAGIC_VALUE);
  EEPROM.commit();
}

void loadTimezone() {
  int magic = EEPROM.read(EEPROM_TIMEZONE_EXT_MAGIC);

  if (magic == TIMEZONE_EXT_MAGIC_VALUE) {
    int stored =
      EEPROM.read(EEPROM_TIMEZONE_EXT_LO) |
      (EEPROM.read(EEPROM_TIMEZONE_EXT_HI) << 8);

    if (stored >= 0 && stored < NUM_TIMEZONES) {
      timezoneIndex = stored;
      return;
    }
  }

  // Compact-table migration.
  //
  // Address 5 is the preserved timezone byte from the established
  // pre-expanded nine-zone firmware.  A v6 geographic-table index is not
  // reused because its numeric meaning is different in the compact table.
  // Instead the original nine-zone value is mapped directly to the matching
  // compact civil-time rule.
  //
  // 0 Greece          -> UTC+02/+03 EU DST
  // 1 UK              -> UTC+00/+01 EU DST
  // 2 Central Europe  -> UTC+01/+02 EU DST
  // 3 Eastern Europe  -> UTC+02/+03 EU DST
  // 4 Eastern USA     -> UTC-05/-04 US DST
  // 5 Central USA     -> UTC-06/-05 US DST
  // 6 Mountain USA    -> UTC-07/-06 US DST
  // 7 Pacific USA     -> UTC-08/-07 US DST
  // 8 Japan           -> UTC+09
  static const uint8_t legacyToCompact[9] = {
    0,   // Greece
    28,  // UK
    32,  // Central Europe
    0,   // Eastern Europe
    16,  // Eastern USA
    13,  // Central USA
    10,  // Mountain USA
    8,   // Pacific USA
    50   // Japan
  };

  int legacy = EEPROM.read(EEPROM_TIMEZONE);
  if (legacy < 0 || legacy > 8) legacy = 0;

  timezoneIndex = legacyToCompact[legacy];
  saveTimezone();
}

// ============================================================
// SET TIMEZONE
// ============================================================

bool setTimezone(
  int index
) {

  if (
    index < 0 ||
    index >= NUM_TIMEZONES
  ) {

    return false;
  }

  timezoneIndex =
    index;

  applyTimezone(
    timezoneIndex
  );

  saveTimezone();


  // ----------------------------------------------------------
  // IMMEDIATE NTP RESYNC
  // ----------------------------------------------------------

  if (
    WiFi.status() ==
    WL_CONNECTED
  ) {

    showOLEDMessage(
      OLED_TIME_SYNC
    );

    if (
      syncRTCWithNTP()
    ) {

      timeSyncSuccess =
        true;

      char zoneName[52];
      getTimezoneName(timezoneIndex, zoneName, sizeof(zoneName));
      showOLEDMessage(
        OLED_IR_COMMAND,
        zoneName
      );

      delay(1000);

      updateDisplay();

      return true;
    }
  }

  updateDisplay();

  return true;
}

// ============================================================
// CHANGE TIMEZONE
// ============================================================

void changeTimezone(
  int direction
) {

  int newIndex =
    timezoneIndex +
    direction;

  if (
    newIndex < 0
  ) {

    newIndex =
      NUM_TIMEZONES - 1;
  }

  if (
    newIndex >= NUM_TIMEZONES
  ) {

    newIndex = 0;
  }

  setTimezone(
    newIndex
  );
}

// ============================================================
// OLED MESSAGE
// ============================================================

void showOLEDMessage(
  uint8_t state,
  String customMessage
) {

  currentOLEDState =
    (OLEDState)state;

  oledOffTimer =
    millis();

  u8g2.setPowerSave(0);

  u8g2.clearBuffer();

  if (isGreekUI()) {

    u8g2.setFont(
      u8g2_font_cu12_t_greek
    );

    switch (state) {

      case OLED_WELCOME:
        u8g2.drawStr(0, 20, "WiFi Clock v2.0");
        u8g2.drawStr(0, 40, "DS3231 RTC");
        u8g2.drawUTF8(0, 60, "Εκκίνηση...");
        break;

      case OLED_WIFI_CONNECTING:
        u8g2.drawUTF8(0, 20, "Σύνδεση στο WiFi");
        u8g2.drawUTF8(0, 40, "Παρακαλώ περιμένετε");
        u8g2.drawStr(0, 58, WiFi.SSID().c_str());
        break;

      case OLED_WIFI_CONFIG:
        u8g2.drawUTF8(0, 15, "Ρύθμιση WiFi");
        u8g2.drawUTF8(0, 35, "1. Σύνδεση στο AP:");
        u8g2.drawStr(10, 50, "AutoConnectAP");
        u8g2.drawUTF8(0, 64, "2. Άνοιξε 192.168.4.1");
        break;

      case OLED_TIME_SYNC:
        u8g2.drawUTF8(0, 20, "Συγχρονισμός Ώρας");
        u8g2.drawUTF8(0, 40, "NTP / Ζώνη Ώρας");
        {
          char zoneName[52];
          getTimezoneName(timezoneIndex, zoneName, sizeof(zoneName));
          u8g2.setFont(u8g2_font_5x8_tf);
          u8g2.drawStr(0, 58, zoneName);
        }
        break;

      case OLED_IR_COMMAND:
        u8g2.drawUTF8(0, 20, "Εντολή IR");

        // The only normal custom IR-command message here is the
        // selected timezone after an immediate timezone sync.
        {
          char zoneName[52];
          getTimezoneName(timezoneIndex, zoneName, sizeof(zoneName));
          u8g2.setFont(u8g2_font_5x8_tf);
          u8g2.drawStr(0, 40, zoneName);
        }
        break;

      case OLED_DIM_EDITING: {
        u8g2.drawUTF8(0, 15, "Ρύθμιση Νυχτερινού");

        char buf[32];

        sprintf(
          buf,
          "%02d:%02d",
          dimStartHour,
          dimStartMin
        );
        u8g2.drawUTF8(0, 31, "Έναρξη:");
        u8g2.drawStr(72, 31, buf);

        sprintf(
          buf,
          "%02d:%02d",
          dimEndHour,
          dimEndMin
        );
        u8g2.drawUTF8(0, 45, "Λήξη:");
        u8g2.drawStr(72, 45, buf);

        drawDigitEditNavigationIcons();
        break;
      }

      case OLED_MANUAL_TIME:
        u8g2.drawUTF8(0, 20, "Χειροκίνητη Ώρα");
        u8g2.drawStr(0, 40, customMessage.c_str());
        drawNumberEnterNavigationIcons();
        break;

      case OLED_ERROR:
        u8g2.drawUTF8(0, 20, "ΣΦΑΛΜΑ");
        u8g2.drawUTF8(0, 40, "Σφάλμα RTC");
        break;

      default:
        break;
    }

    u8g2.sendBuffer();
    return;
  }

  // English path below is intentionally identical to the archived
  // FINAL ENGLISH OLED behavior.
  u8g2.setFont(
    u8g2_font_ncenB08_tr
  );

  switch (state) {

    case OLED_WELCOME:

      u8g2.drawStr(
        0,
        20,
        "WiFi Clock v2.0"
      );

      u8g2.drawStr(
        0,
        40,
        "DS3231 RTC"
      );

      u8g2.drawStr(
        0,
        60,
        "Starting..."
      );

      break;

    case OLED_WIFI_CONNECTING:

      u8g2.drawStr(
        0,
        20,
        "Connecting to WiFi"
      );

      u8g2.drawStr(
        0,
        40,
        "Please wait..."
      );

      u8g2.setFont(
        u8g2_font_6x10_tf
      );

      u8g2.drawStr(
        0,
        58,
        WiFi.SSID().c_str()
      );

      break;

    case OLED_WIFI_CONFIG:

      u8g2.drawStr(
        0,
        15,
        "WiFi Setup Required"
      );

      u8g2.setFont(
        u8g2_font_6x10_tf
      );

      u8g2.drawStr(
        0,
        35,
        "1. Connect to AP:"
      );

      u8g2.drawStr(
        10,
        50,
        "AutoConnectAP"
      );

      u8g2.drawStr(
        0,
        65,
        "2. Go to 192.168.4.1"
      );

      break;

    case OLED_TIME_SYNC:

      u8g2.drawStr(
        0,
        20,
        "Syncing Time"
      );

      u8g2.drawStr(
        0,
        40,
        "NTP / Timezone"
      );

      u8g2.setFont(
        u8g2_font_6x10_tf
      );

      {
        char zoneName[52];
        getTimezoneName(timezoneIndex, zoneName, sizeof(zoneName));
        u8g2.drawStr(0, 58, zoneName);
      }

      break;

    case OLED_IR_COMMAND:

      u8g2.drawStr(
        0,
        20,
        "IR Command"
      );

      u8g2.drawStr(
        0,
        40,
        customMessage.c_str()
      );

      break;

    case OLED_DIM_EDITING: {

      u8g2.drawStr(
        0,
        15,
        "Edit Dim Schedule"
      );

      u8g2.setFont(
        u8g2_font_6x10_tf
      );

      char buf[32];

      sprintf(
        buf,
        "Start: %02d:%02d",
        dimStartHour,
        dimStartMin
      );

      u8g2.drawStr(
        0,
        31,
        buf
      );

      sprintf(
        buf,
        "End:   %02d:%02d",
        dimEndHour,
        dimEndMin
      );

      u8g2.drawStr(
        0,
        45,
        buf
      );

      drawDigitEditNavigationIcons();

      break;
    }

    case OLED_MANUAL_TIME:

      u8g2.drawStr(
        0,
        20,
        "Manual Time Set"
      );

      u8g2.drawStr(
        0,
        40,
        customMessage.c_str()
      );

      drawNumberEnterNavigationIcons();

      break;

    case OLED_ERROR:

      u8g2.drawStr(
        0,
        20,
        "ERROR"
      );

      u8g2.drawStr(
        0,
        40,
        customMessage.c_str()
      );

      break;

    default:

      break;
  }

  u8g2.sendBuffer();
}

// ============================================================
// OLED OFF
// ============================================================

void turnOLEDOff() {

  currentOLEDState =
    OLED_OFF;

  u8g2.setPowerSave(1);
}

// ============================================================
// LOW-BRIGHTNESS STARTUP INDICATION
// ============================================================

void showStartupCheckmark() {

  static const int checkPoints[][2] = {
    {10, 4},
    {11, 5},
    {12, 6},
    {13, 7},
    {14, 6},
    {15, 5},
    {16, 4},
    {17, 3},
    {18, 2},
    {19, 1},
    {20, 0}
  };

  const int pointCount =
    sizeof(checkPoints) /
    sizeof(checkPoints[0]);

  matrix.setBrightness(
    BRIGHTNESS_MIN
  );

  clearDisplay();
  matrix.show();

  for (
    int i = 0;
    i < pointCount;
    i++
  ) {

    int index =
      matrixIndex(
        checkPoints[i][0],
        checkPoints[i][1]
      );

    if (
      index >= 0
    ) {

      matrix.setPixelColor(
        index,
        COLOR_WHITE
      );
    }

    matrix.show();

    delay(
      70
    );
  }

  delay(
    250
  );

  clearDisplay();
  matrix.show();
}

void showStartupGreen() {

  matrix.setBrightness(
    BRIGHTNESS_MIN
  );

  for (
    int i = 0;
    i < NUM_LEDS;
    i++
  ) {

    matrix.setPixelColor(
      i,
      COLOR_GREEN
    );
  }

  matrix.show();

  delay(
    500
  );

  clearDisplay();
  matrix.show();
}

// ============================================================
// WIFI POWER CONTROL
// ============================================================

void disconnectWiFiCompletely() {

  WiFi.setAutoReconnect(
    false
  );

  // Disconnect the station but KEEP the saved credentials.
  // The two-argument overload is important on ESP8266.
  WiFi.disconnect(
    false,
    false
  );

  delay(
    20
  );

  WiFi.mode(
    WIFI_OFF
  );

  WiFi.forceSleepBegin();

  delay(
    1
  );

}

bool reconnectSavedWiFi(
  unsigned long timeoutMs
) {

  // Wake the radio if it was switched off after the previous sync.
  WiFi.forceSleepWake();

  delay(
    1
  );

  WiFi.mode(
    WIFI_STA
  );

  // We want only the explicit connection attempt below.
  // Between sessions WiFi must remain off.
  WiFi.setAutoReconnect(
    false
  );

  // Reconnect using the station credentials already stored by
  // WiFiManager / ESP8266 SDK.  No configuration portal is opened.
  WiFi.begin();

  unsigned long start =
    millis();

  while (
    WiFi.status() != WL_CONNECTED &&
    millis() - start < timeoutMs
  ) {

    delay(
      100
    );

    yield();
  }

  if (
    WiFi.status() ==
    WL_CONNECTED
  ) {

    lastKnownIP =
      WiFi.localIP().toString();


    return true;
  }


  return false;
}

void attemptScheduledSync() {


  bool connected =
    reconnectSavedWiFi(
      20000UL
    );

  if (
    connected
  ) {

    if (
      syncRTCWithNTP()
    ) {

      timeSyncSuccess =
        true;

      if (holidaysEnabled && holidayPublicDatesEnabled) {
        downloadPublicHolidays(rtc.now().year());
      }

    } else {

      timeSyncSuccess =
        false;

    }

  } else {

    timeSyncSuccess =
      false;

  }

  disconnectWiFiCompletely();

  updateDisplay();
}


// ============================================================
// MANUAL SYNC NOW
// ============================================================
//
// Called from the OLED editor.
//
// Sequence:
//   1. Wake WiFi.
//   2. Connect using the already saved WiFi credentials.
//   3. Synchronize NTP using the selected timezone.
//   4. Write the synchronized local time into the DS3231 RTC.
//   5. Turn WiFi completely OFF again.
//   6. Return to the normal clock display.
//
// This does NOT change the AUTOSYNC interval setting.
//

void performSyncNow() {

  showOLEDMessage(
    OLED_TIME_SYNC
  );

  bool syncOK = false;

  if (
    reconnectSavedWiFi(
      20000UL
    )
  ) {

    syncOK =
      syncRTCWithNTP();

    if (syncOK && holidaysEnabled && holidayPublicDatesEnabled) {
      downloadPublicHolidays(rtc.now().year());
    }
  }

  disconnectWiFiCompletely();

  timeSyncSuccess =
    syncOK;

  u8g2.setPowerSave(
    0
  );

  u8g2.clearBuffer();

  if (isGreekUI()) {
    u8g2.setFont(u8g2_font_cu12_t_greek);
    u8g2.drawUTF8(0, 18, "Συγχρονισμός Τώρα");
    drawGreekCentered(45, syncOK ? "ΕΠΙΤΥΧΙΑ" : "ΑΠΟΤΥΧΙΑ");
  } else {
    u8g2.setFont(
      u8g2_font_ncenB10_tr
    );

    u8g2.drawStr(
      0,
      18,
      "SYNC NOW"
    );

    u8g2.setFont(
      u8g2_font_ncenB12_tr
    );

    u8g2.drawStr(
      0,
      45,
      syncOK ?
        "SUCCESS" :
        "FAILED"
    );
  }

  u8g2.sendBuffer();

  delay(
    1500
  );

  exitEditor();
}

// ============================================================
// WIFI MANAGER
// ============================================================

void setupWiFiManager() {

  WiFiManager wifiManager;

  u8g2.setPowerSave(0);

  u8g2.firstPage();

  do {

    if (isGreekUI()) {
      u8g2.setFont(u8g2_font_cu12_t_greek);
      u8g2.drawUTF8(0, 15, "Ρύθμιση WiFi");
      u8g2.drawUTF8(0, 35, "Σύνδεση στο AP:");
      u8g2.drawStr(0, 52, "AutoConnectAP");
      u8g2.drawStr(0, 64, "IP: 192.168.4.1");
    } else {
      u8g2.drawStr(
        0,
        15,
        "WiFi Setup Mode"
      );

      u8g2.drawStr(
        0,
        35,
        "Connect to AP:"
      );

      u8g2.drawStr(
        0,
        52,
        "AutoConnectAP"
      );

      u8g2.drawStr(
        0,
        64,
        "IP: 192.168.4.1"
      );
    }

  } while (
    u8g2.nextPage()
  );

  // In explicit setup mode it is acceptable to open the portal,
  // but give the connection attempt a finite timeout.
  wifiManager.setConnectTimeout(
    20
  );

  wifiManager.autoConnect(
    "AutoConnectAP"
  );

  u8g2.firstPage();

  do {

    if (isGreekUI()) {
      u8g2.setFont(u8g2_font_cu12_t_greek);
      u8g2.drawUTF8(0, 15, "Το WiFi ρυθμίστηκε!");
      u8g2.drawUTF8(0, 35, "Επανεκκίνηση...");
    } else {
      u8g2.drawStr(
        0,
        15,
        "WiFi Configured!"
      );

      u8g2.drawStr(
        0,
        35,
        "Rebooting..."
      );
    }

  } while (
    u8g2.nextPage()
  );

  delay(2000);

  turnOLEDOff();

  ESP.restart();
}
