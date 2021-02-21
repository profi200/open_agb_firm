# open_agb_firm

open_agb_firm is a bare metal app for running GBA games and homebrew using the 3DS builtin GBA hardware.

## Disclaimer
open_agb_firm is currently in alpha. While open_agb_firm is relatively stable, there are few quirks that have yet to be fixed. See [Known issues](#known-issues) and [Troubleshooting](#troubleshooting) for more information.

Additionally, we are not responsible for any damage that may occur to your system as a direct or indirect result of you using open_agb_firm.

## How to build
To compile open_agb_firm, the following needs to be installed:
* [devkitARM](https://sourceforge.net/projects/devkitpro/)
* [Corelink DMA-330 assembler](https://github.com/profi200/dma330as)
* [CTR firm builder](https://github.com/derrekr/ctr_firm_builder)

Additionally, 7-Zip (or on Linux, p7zip) needs to be installed to make release builds. Also, make sure that the dma330as and CTR firm builder binaries are in the $PATH environment variable and accessible to the Makefile. Build open_agb_firm as a debug build via `make` or as a release build via `make release`.

## How to use
The process to set up and launch open_agb_firm is similar that of [GodMode9](https://github.com/d0k3/GodMode9).
* Download the [latest release](https://github.com/profi200/open_agb_firm/releases/latest) and extract it to obtain `open_agb_firm.firm`.
* Copy the `open_agb_firm.firm` file to your 3DS's SD card at `/luma/payloads` if using Luma3DS or elsewhere if using fastboot3DS.
* Launch open_agb_firm using Luma3DS by holding START while booting your 3DS or assign the firm to a slot if you're using fastboot3DS.
* After open_agb_firm launches, use the file browser to navigate to a .GBA ROM to run.

## Controls
A/B/L/R/START/SELECT - GBA buttons, respectively

SELECT+Y - Dump screen output to `/3ds/open_agb_firm/texture_dump.bmp`*\
*If the screen output freezes, press HOME to fix it. This is a hard to track down bug that will be fixed.

Hold the power button to turn off the 3DS.

## Configuration

Settings are stored in `/3ds/open_agb_firm/config.ini`.

### General
`u8 backlight` - Backlight brightness (default: `40`, max: `107`)

`bool biosIntro` - Show GBA BIOS intro at game startup (default: `true`)

### Video
`float inGamma` - Set gamma (default: `2.2`)

`float outGamma` - Specify screen gamma (default: `1.54`)*\
*Default setting based on o3DS LCD. For raw GBA colors, set to `2.2`.

`float contrast` - Set gain (default: `1.0`)

`float brightness` - Set lift (default: `0.0`)

## Known issues
This section is reserved for a listing of known issues. At present only this remains:
* Sleep mode is not fully implemented.
* Save type detection may still fail for certain games using EEPROM.
* Lack of settings (including brightness control).
* No cheats or other enhancements.

If you happen to stumble over another bug, please [open an issue](https://github.com/profi200/open_agb_firm/issues) or contact profi200 via other platforms.

## Troubleshooting
Known problems and their solutions.

Problem: The game crashes/shows white or blackscreens or shows a savegame corrupt message.\
Solution: Try to delete the savegame file. If this doesn't help, [report the issue](https://github.com/profi200/open_agb_firm/issues).\
Note: EEPROM saves made by some emulators are incompatible because they have every 8 byte block endian swapped. [This tool](https://gist.github.com/profi200/e06794d7561ed552c518b4b0b2f5f2f6) can fix affected saves.

## Hardware limitations
This is a list of limitations we can't solve in software or are very hard to work around. This doesn't mean it will never happen (unless stated otherwise).
* 64 MiB (512 mbit) games. Not possible to support.
* Games with extra hardware built into the cartridge (except Real-Time Clock). Patches are required.
* GBA Serial port (aka Link Cable).
* 64 KiB (512 kbit) SRAM (homebrew games/emulators). Not possible to support.
* Can't switch back to 3DS mode from GBA mode, requiring a reboot for booting a different game.
* Savestates. Very difficult to implement because no direct hardware access.

## License
You may use this under the terms of the GNU General Public License GPL v3 or under the terms of any later revisions of the GPL. Refer to the provided `LICENSE.txt` file for further information.

## Thanks to...
* **yellows8**
* **plutoo**
* **smea**
* **Normmatt**
* **WinterMute**
* **ctrulib devs**
* **Luma 3DS devs**
* **devkitPro**
* **ChaN** (fatfs)
* **benhoyt** (inih)
* **fastboot3DS project**
* **Wolfvak, Sono and all the other people in #GodMode9 on IRC/Discord**
* **endrift, Extrems and all the other people in #mgba on Freenode**
* ...everyone who contributed to **3dbrew.org**

Copyright (C) 2021 derrek, profi200, d0k3
