# M5ez Settings Persistence

M5ez provides UI for managing a number of settings which are persisted in NVS (Non-Volatile Storage) memory, and are accessible from other programs.
All settings share a single NVS namespace:

> NVS NameSpace:  "M5ez"

## Wifi Settings

The WiFi settings include Autoconnect, SSIDs, and passwords. SSIDs and passwords are grouped in pairs, but stored as individual strings.
The association between SSID and password is established by a numeric suffix, starting at one and incrementing for each pair.

| Key Name       | Type    | Purpose                                         |
| :------------- | :------ | :---------------------------------------------- |
| autoconnect_on | boolean | True if ezWiFi should connect to strongest SSID |
| SSID1          | String  | Name of first stored network (if any)           |
| key1           | String  | Password of first stored network (if any)       |
| ...                                                                        |
| SSIDn          | String  | Name of last stored network                     |
| keyn           | String  | Password of last stored network                 |

## Battery Settings

The Battery settings contains only a single flag which indicates whether or not the battery icon is displayed in the header.

| Key Name        | Type    | Purpose                                           |
| :-------------- | :------ | :------------------------------------------------ |
| battery_icon_on | boolean | True if battery icon is to be displayed in header |

## Clock Settings

The Clock settings include 4 items. The timezone names are described  [here](https://en.wikipedia.org/wiki/List_of_tz_database_time_zones).

| Key Name        | Type    | Purpose                                            |
| :-------------- | :------ | :------------------------------------------------- |
| clock_on        | boolean | True if time is to be displayed in header          |
| timezone        | String  | Name or description of selected timezone           |
| clock12         | boolean | True if time should be displayed in 12 hr format   |
| ampm            | boolean | True if am/pm should be displayed after 12 hr time |

## Backlight Settings

The Backlight settings are stored as two unsigned characters (0 - 255) representing brightness and inactivity timeout.

| Key Name        | Type    | Purpose                                                    |
| :-------------- | :------ | :--------------------------------------------------------- |
| brightness      | UChar   | Lcd brightness. Default is 128.                            |
| inactivity      | UChar   | Time until Lcd is blanked. 0 = never, else n * 30 seconds. |

## FACES Settings

The Faces settings group currently contains a single member.

| Key Name        | Type    | Purpose                                                    |
| :-------------- | :------ | :--------------------------------------------------------- |
| faces_on        | boolean | True if the keyboard Face should be used for text input.   |

## Theme Settings

The Theme settings group also contains a single member.

| Key Name        | Type    | Purpose                                                    |
| :-------------- | :------ | :--------------------------------------------------------- |
| theme           | String  | Name of the selected theme (if any.)                       |
