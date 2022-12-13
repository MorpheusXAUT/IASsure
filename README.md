# IASsure

I assure IAS - stop guessing wind corrections based on pilot reports. `IASsure` calculates indicated air speed (IAS) and Mach number of EuroScope radar targets based on their ground speed and (real-life) weather data.

This plugin allows you to make better informed decisions based on aircraft speed without relying on pilot reports - speed control just became a lot less guesswork.

## Table of Contents

- [Getting started](#getting-started)
  - [Prerequisites](#prerequisites)
  - [Verifying releases](#verifying-releases)
  - [Installation](#installation)
- [Usage](#usage)
  - [Basics](#basics)
  - [Tag items](#tag-items)
  - [Tag functions](#tag-functions)
  - [Chat commands](#chat-commands)
  - [Config](#config)
  - [Weather data](#weather-data)
- [Contributing](#contributing)
  - [Development setup](#development-setup)
- [Acknowledgments](#acknowledgments)
- [License](#license)

## Getting started

### Prerequisites

Since `IASsure` was developed as an EuroScope plugin, it requires a working installation [EuroScope](https://euroscope.hu/). The initial development was started using EuroScope version [`v3.2.1.34`](https://www.euroscope.hu/wp/2022/05/08/v3-2-1-34/), although the plugin should most likely also work fine with previous and later versions. As development continues, compatibility to the latest **beta** versions of EuroScope will be maintained as long as possible and feasible.

### Verifying releases

Each release includes a `checksums.txt` file, containing the `sha256` checksums of the downloadable artifacts. In order to verify the checksum's integrity, a detached `gpg` signature (`checksums.txt.sig`) is available for all official releases as well.  
All signatures are made using GPG key [`54930D654C505BA73BC06A0E4C685956F39779B2`](54930D654C505BA73BC06A0E4C685956F39779B2.asc).

In order to verify a release, perform the following steps:
- Import the signing key into your local keyring (only required once):
```bash
gpg --import 54930D654C505BA73BC06A0E4C685956F39779B2.asc
```
- Download the appropriate archive, `checksums.txt` and `checksums.txt.sig` from the [Releases](https://github.com/MorpheusXAUT/IASsure/releases) page
- Verify the integrity of the checksums file:
```bash
gpg --verify checksums.txt.sig checksums.txt
```
- Verify the checksum of the downloaded archive:
```bash
sha256sum --ignore-missing -c checksums.txt
```

### Installation

1. Download the latest release (`IASsure_vX.Y.Z.zip`) of `IASsure` from the [**Releases**](https://github.com/MorpheusXAUT/IASsure/releases/latest) section of this repository.
2. (Optional) [Verify downloaded release](#verifying-releases).
3. Extract the zip file to a location of your choosing (most likely somewhere inside your EuroScope sector file/profile setup, along with other plugins that are already set up). As a [config file](#config) is supported (and expected to reside within the same folder as the plugin's `.dll`), it's advised to create a new `IASsure` folder.
4. Start EuroScope and open the **Plug-ins ...** dialog in the settings menu (**OTHER SET**).
![Plug-ins dialog](https://i.imgur.com/SrVtRp9.png)
5. **Load** the plugins by navigating to the folder you extracted `IASsure` to and selecting the exacted `IASsure.dll`. Ensure the expected version is displayed.
![Load plugin](https://i.imgur.com/D1eawjA.png)
6. Close the plugin dialog by clicking **Close**.
7. Configure your tags via the **TAG editor ...** dialog in the settings menu (**OTHER SET**). See [tag items](#tag-items) and [tag functions](#tag-functions) below for a list of available options.
8. Close the tag editor dialog by clicking **OK**.

## Usage

## Basics

`IASsure` currently supports two calculated speed values for aircraft: indicated air speed (IAS) and Mach number. Both values are available in various representations (see [tag items](#tag-items) below).

All calculations are based on the ground speed of radar targets and can be performed with or without (real-life) weather data to correct for wind/temperature deviations. Note that even with accurate (real-life) wind data, some values might be off by quite a margin as pilots sometimes use different winds. Generally speaking, MSFS wind data seems to correlate with the calculated values quite well even though small inaccuracies are always to be expected.

In addition, `IASsure` lets you set the reported IAS/Mach number for aircraft. Selecting a value displays a difference to the calculated value and provides a quick overview of different winds/speed differences between aircraft, making speed based sequencing easier.

### Tag items

Tag items are used to display calculated IAS and Mach values for radar targets.  
Whilst only these two values are available at the moment, there's several representations to be picked from:

#### Calculated IAS

![Calculated IAS](https://i.imgur.com/vHiockm.png)  
Displays (full) calculated indicated air speed (in knots) including selected prefix (default `I`).  
Recommended function setup:
- Left click: [Open reported IAS menu](#open-reported-ias-menu)
- Right click: [Toggle calculated IAS](#toggle-calculated-ias)

![Calculated IAS setup](https://i.imgur.com/PRU8yVB.png)

#### Calculated IAS (togglable)

Displays (full) calculated indicated air speed (in knots) including selected prefix (default `I`), can be toggled on and off via [Toggle calculated IAS](#toggle-calculated-ias) tag item function. No/empty value is displayed by default until toggle has been triggered.  
To be used in reduced tags to selectively display calculated IAS without showing values for all aircraft.

#### Calculated IAS (abbreviated)

![Calculated IAS (abbreviated)](https://i.imgur.com/1OPKMVy.png)  
Displays abbreviated calculated indicated air speed (in tens of knots) without any prefix.  
Recommended function setup:
- Left click: [Open reported IAS menu](#open-reported-ias-menu)
- Right click: [Toggle calculated IAS (abbreviated)](#toggle-calculated-ias-abbreviated)

![Calculated IAS (abbreviated) setup](https://i.imgur.com/6n8Bp97.png)

#### Calculated IAS (abbreviated, togglable)

Displays abbreviated calculated indicated air speed (in tens of knots) without any prefix, can be toggled on and off via [Toggle calculated IAS (abbreviated)](#toggle-calculated-ias-abbreviated) tag item function. No/empty value is displayed by default until toggle has been triggered.  
To be used in reduced tags to selectively display calculated IAS without showing values for all aircraft.

#### Calculated Mach

![Calculated Mach](https://i.imgur.com/yQhdRJH.png)  
Displays calculated Mach number including selected prefix (default `M`) and configured precision (default 2 digits).  
Recommended function setup:
- Left click: [Open reported Mach menu](#open-reported-mach-menu)
- Right click: [Toggle calculated Mach](#toggle-calculated-mach)

![Calculated Mach setup](https://i.imgur.com/zTsfQoa.png)

#### Calculated Mach (togglable)

Displays calculated Mach number including selected prefix (default `M`) and configured precision (default 2 digits), can be toggled on and off via [Toggle calculated Mach](#toggle-calculated-mach) tag item function. No/empty value is displayed by default until toggle has been triggered.  
To be used in reduced tags to selectively display calculated Mach number without showing values for all aircraft.

#### Calculated Mach (above threshold)

![Calculated Mach (above threshold)](https://i.imgur.com/yQhdRJH.png)  
Displays calculated Mach number including selected prefix (default `M`) and configured precision (default 2 digits) for all aircraft above the desired flight level threshold (default FL245). Any aircraft below the threshold will have an empty tag.  
Recommended function setup:
- Left click: [Open reported Mach menu](#open-reported-mach-menu)
- Right click: [Toggle calculated Mach (above threshold)](#toggle-calculated-mach-above-threshold)

![Calculated Mach (above threshold) setup](https://i.imgur.com/GGftOFt.png)

#### Calculated Mach (above threshold, togglable)

Displays calculated Mach number including selected prefix (default `M`) and configured precision (default 2 digits) for all aircraft above the desired flight level threshold (default FL245). Any aircraft below the threshold will have an empty tag. This tag can be toggled on and off via [Toggle calculated Mach (above threshold)](#toggle-calculated-mach-above-threshold) tag item function. No/empty value is displayed by default until toggle has been triggered.  
To be used in reduced tags to selectively display calculated Mach number without showing values for all aircraft.

### Tag functions

#### Clear reported IAS

Triggering this tag function clears the selected reported indicated air speed for this radar target (if one has been set). This functionality is also available via the [reported IAS menu](#open-reported-ias-menu), but can be assigned separately if desired.

#### Clear reported Mach

Triggering this tag function clears the selected reported Mach number for this radar target (if one has been set). This functionality is also available via the [reported Mach menu](#open-reported-mach-menu), but can be assigned separately if desired.

#### Open reported IAS menu

![Open reported IAS menu](https://i.imgur.com/OvM6hxj.png)  
Triggering this tag function allows controllers to select the indicated air speed reported by the pilot (e.g. via voice). This will switch the [Calculated IAS](#calculated-ias) and [Calculated IAS (abbreviated)](#calculated-ias-abbreviated) (as well as their togglable counterparts) to display the difference between calculated and reported IAS, providing a quick overview in the spot winds for the selected aircraft. This allows for easier estimation of required speed differences in sequencing if two radar targets use different winds.

![IAS difference calculation](https://i.imgur.com/VOlX6QZ.png)

As shown above, the speed difference of 15kts (270 calculated, 255 reported) is displayed as `-02` (in abbreviated form, rounded to 20kts), indicating the aircraft is roughly 20kts slower than expected.

#### Open reported Mach menu

![Open reported Mach menu](https://i.imgur.com/VWiwpFk.png)  
Triggering this tag function allows controllers to select the Mach number reported by the pilot (e.g. via voice). This will switch the [Calculated Mach](#calculated-mach) and [Calculated Mach (above threshold)](#calculated-mach-above-threshold) (as well as their togglable counterparts) to display the difference between calculated and reported Mach number, providing a quick overview in the spot winds for the selected aircraft. This allows for easier estimation of required speed differences in sequencing if two radar targets use different winds.

![Mach difference calculation](https://i.imgur.com/QOxssn8.png)

As shown above, the speed difference of M0.01 (M0.80 calculated, M.081 reported) is displayed as `+01`, indicating the aircraft is roughly M0.01 faster than expected.

#### Toggle calculated IAS

Toggles the display of the [Calculated IAS (togglable)](#calculated-ias-togglable) for the selected aircraft, enabling the tag item's content. Executing this function for the same radar target again disables the togglable display value.

#### Toggle calculated IAS (abbreviated)

Toggles the display of the [Calculated IAS (abbreviated, togglable)](#calculated-ias-abbreviated-togglable) for the selected aircraft, enabling the tag item's content. Executing this function for the same radar target again disables the togglable display value.

#### Toggle calculated Mach

Toggles the display of the [Calculated Mach (togglable)](#calculated-mach-togglable) for the selected aircraft, enabling the tag item's content. Executing this function for the same radar target again disables the togglable display value.

#### Toggle calculated Mach (above threshold)

Toggles the display of the [Calculated Mach (above threshold, togglable)](#calculated-mach-above-threshold-togglable) for the selected aircraft, enabling the tag item's content. Executing this function for the same radar target again disables the togglable display value.

### Chat commands

Chat commands allow for some on-the-fly configuration of `IASsure`'s features. All commands are prefixed with `.ias` and provide some basic information about their usage when executed without arguments. Executing `.ias` (without any further commands/arguments) prints the version loaded and a list of available commands.

#### Toggle debug logging

`.ias debug`

Toggles debug logging, displaying more messages about the internal state as well as weather data retrieval.

This setting will be saved to the EuroScope settings upon exit.

#### Reset plugin state

`.ias reset`

Resets the plugin's internal state, clearing all reported IAS/Mach numbers as well as toggled tag items, also removing the currently stored weather information and causing it to be re-fetched.

#### Configure weather handling

`.ias weather`

Allows for configuration of `IASsure`'s weather handling via three subcommands:

##### Set weather data update interval

`.ias weather update <MIN>`

Changes the interval (in minutes) between automatic weather data updates. Setting this value to `0` disables automatic weather updates.

This setting will be saved to the EuroScope settings upon exit.

##### Set weather data update URL

`.ias weather url <URL>`

Configures URL for [weather data](#weather-data) file to be retrieved automatically.  
You **must** set a value (either via this chat command) or the [config](#config) in order to use (real-life) weather based calculations.

This setting will be saved to the EuroScope settings upon exit.

##### Clear weather data

`.ias weather clear`

Clears all weather data currently stored, falling back to windless speed calculations. Mostly useful for debugging or troubleshooting purposes if retrieved data seems to be faulty.

#### Toggle ground speed source

`.ias gs`

Toggles between using the ground speed reported by the pilot client (default) and the one calculated by EuroScope. You should only need to change this if a lot of values based on the reported speed are off by a large margin (e.g. due to faulty pilot clients).

This setting will be saved to the EuroScope settings upon exit.

#### Configure calculated IAS/Mach number prefixes

`.ias prefix`

`IASsure` lets you configure the prefixes displayed for [Calculated IAS](#calculated-ias) and [Calculated Mach](#calculated-mach) tag items (as well as their respective togglable parts). This allows you to customize your tag items to your FIR's likings.

##### Set calculated IAS prefix

`.ias prefix ias <PREFIX>`

Sets desired prefix for [Calculated IAS](#calculated-ias) (and togglable equivalent) tag item. Calling this function without any prefix (`.ias prefix ias`) will disable the prefix, only displaying the calculated value.  
Note that the [Calculated IAS (abbreviated)](#calculated-ias-abbreviated) does not contain the configured prefix.

This setting will be saved to the EuroScope settings upon exit.

##### Set calculated Mach number prefix

`.ias prefix mach <PREFIX>`

Sets desired prefix for [Calculated Mach](#calculated-mach) and [Calculated Mach (above threshold)](#calculated-mach-above-threshold) (and togglable equivalents) tag items. Calling this function without any prefix (`.ias prefix mach`) will disable the prefix, only displaying the calculated value.

This setting will be saved to the EuroScope settings upon exit.

#### Configure Mach number calculation settings

`.ias mach`

Changes settings related to Mach number calculations.

##### Set digit count for calculated Mach number

`.ias mach digits <DIGITS>`

Sets number of digits (precision) for displayed Mach numbers. Value provided must be between 1 and 5.

This setting will be saved to the EuroScope settings upon exit.

##### Set threshold for Mach number calculations

`.ias mach threshold <FLIGHTLEVEL>`

Sets threshold (as flight level) for [Calculated Mach (above threshold)](#calculated-mach-above-threshold) (and togglable equivalent). All radar targets above the selected threshold (245 by default) will have their Mach number calculated and displayed; all other aircraft will have an empty tag item instead. The provided threshold must be larger than 0 and provided in hundreds of ft (e.g. `.ias mach threshold 245`).

This setting will be saved to the EuroScope settings upon exit.

### Config

Aside from the configuration provided via [chat commands](#chat-commands), `IASsure`'s behavior can be configured using a config file, allowing for easy distribution of default settings for your FIR.  
The plugin searches for a `config.json` file (formatted as [JSON](https://www.json.org)) located within the same directory as the loaded `IASsure.dll`. If found, the file is read and parsed and settings are overwritten accordingly. Note that configuration loaded from the `config.json` file currently takes precedence over EuroScope-configured settings.

All settings available via config file are optional - if no value was found in the file, `IASsure` defaults to the value set via EuroScope settings.

#### Top level structure

The configuration file is expected to consist of a top level object with one or multiple of the following keys:

| Key       | Type     | Description                         |
| --------- | -------- | ----------------------------------- |
| `mach`    | `object` | Mach number calculation settings    |
| `prefix`  | `object` | Calculated IAS/Mach number prefixes |
| `weather` | `object` | Weather handling                    |

#### `mach` object

| Key           | Type     | Description                            |
| ------------- | -------- | -------------------------------------- |
| `digits`      | `int`    | Digit count for calculated Mach number |
| `thresholdFL` | `int`    | Threshold for Mach number calculations |

#### `prefix` object

| Key    | Type     | Description                   |
| ------ | -------- | ----------------------------- |
| `ias`  | `string` | Calculated IAS prefix         |
| `mach` | `string` | Calculated Mach number prefix |

#### `weather` object

| Key      | Type     | Description                  |
| -------- | -------- | ---------------------------- |
| `url`    | `string` | Weather data update URL      |
| `update` | `int`    | Weather data update interval |

### Weather data

As all barometric formulas used for IAS/Mach calculations require wind and temperature data, the best and most accurate results are achieved when using a (real-life) weather data source. All calculations can be performed without weather data as well (the plugin will fall back to this mode if no data can be retrieved), however the calculated results (especially the Mach number) will be inaccurate.

Note that weather data is only retrieved while the client is connected to VATSIM directly or via proxy - playback and sweatbox connections will not fetch weather information at all.

Since neither EuroScope nor VATSIM provide spot winds/enroute wind data, a data source for weather information is required in order to utilise wind-corrected data. The current weather implementation is based on [Windy](https://www.windy.com/)'s data (or anything related provided in identical format) and defines several strategic reference points within a FIR. These points should cover all relevant parts/major traffic routes of your FIR in order to provide best weather data coverage without over-complicating weather data retrieval.

The following screenshot shows an example weather reference point setup as defined for the LOVV FIR.  
![Example weather reference point setup LOVV](https://i.imgur.com/3YYbFwi.png)  
The corresponding weather data file can be found [here](https://weather.morpheusxaut.net/LOVV.json).

As most (if not all) weather APIs providing the required level of detail/coverage for wind and temperature data require recurring fees, no general weather data can be bundled into this plugin or offered by the author - you will need to set up your own weather source.

Feel free to get in contact via [discussions](https://github.com/MorpheusXAUT/IASsure/discussions) in this repository or via Discord if you would like to know more about setting up your own weather parser or need help doing so.

## Contributing

If you have a suggestion for the project or encountered an error, please open an [issue](https://github.com/MorpheusXAUT/IASsure/issues) on GitHub. Please provide a summary of your idea or problem, optionally with some logs or screenshots and ways to replicate for the latter.

[Pull requests](https://github.com/MorpheusXAUT/IASsure/pulls) are highly welcome, feel free to extend the plugin's functionality as you see fit and submit a request to this repository to make your changes available to everyone. Please keep in mind this plugin attempts to provide features in a relatively generic way so it can be used by vACCs with different needs - try refraining from "hard-coding" any features that might just apply to a specific airport or vACC.

### Development setup

`IASsure` currently has no external development dependencies that need to be configured manually aside from [Visual Studio](https://visualstudio.microsoft.com/vs/). Initial development started using Visual Studio 2022, although later versions should most likely remain compatible.

To allow for debugging, the project has been configured to launch EuroScope as its debug command. Since your installation path of EuroScope will most likely be different, you **must** set an environment variable `EUROSCOPE_ROOT` to the **directory** EuroScope is installed in (**not** the actual `EuroScope.exe` executable), for instance `C:\Program Files (x86)\EuroScope`.  
Note: if you're using [TopSky](https://vatsim-scandinavia.org/forums/forum/54-plugins/) in your sector file, triggering a breakpoint causes both EuroScope and Visual Studio to freak out, resulting in high resource usage and sluggish mouse movements due to the mouse wheel handling implemented in TopSky. To circumvent this issue, set `System_UseMouseWheel=0` in your `TopSkySettings.txt` before launching your debug session. This will prevent you from using your mouse wheel to scroll/zoom in TopSky, however makes debugging actually useful - don't forget to remove the setting before starting your next controlling session again.  
**NEVER** debug your EuroScope plugin using a live connection as halting EuroScope can apparently mess with the VATSIM data feed under certain circumstances.

`IASsure` is compiled using Windows SDK Version 10.0 with a platform toolset for Visual Studio 2022 (v143) using the ISO C++20 Standard.

This repository contains all third-party libraries used by the project in their respective `third_party` and `lib` folders:

- `EuroScope`: EuroScope plugin library
- `nlohmann/json`: [JSON for Modern C++](https://github.com/nlohmann/json/) ([v3.11.2](https://github.com/nlohmann/json/releases/tag/v3.11.2), [MIT License](https://github.com/nlohmann/json/blob/v3.11.2/LICENSE.MIT)), used for parsing the weather data and the optional config file

## Acknowledgements

Many thanks to colleagues at vACC Austria and the Belux vACC for their help with development of calculations and wind data parsing as well as endless testing sessions pestering pilots to gauge plugin accuracy and refine the used formulas.

## License

[MIT License](LICENSE)