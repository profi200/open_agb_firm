# open_agb_firm

open_agb_firm is a bare metal app for running GBA homebrew/games using the 3DS builtin GBA hardware.

## Disclaimer
We are not responsible for any damage that may occur to your system as a direct or indirect result of you using open_agb_firm.

## How to build
To compile open_agb_firm you need
* [devkitARM](https://sourceforge.net/projects/devkitpro/)
* [Corelink DMA-330 assembler](https://github.com/profi200/dma330as)
* [CTR firm builder](https://github.com/derrekr/ctr_firm_builder)

installed in your system. Additionally you need 7-Zip or on Linux p7z installed to make release builds. Also make sure the CTR firm builder and dma330as binaries are in your $PATH environment variable and accessible to the Makefile. Build open_agb_firm as debug build via `make` or as release build via `make release`.

## Known issues
This section is reserved for a listing of known issues. At present only this remains:
* Sleep mode is not fully implemented.
* Save type detection may still fail for certain games using EEPROM.
* No settings (including brightness control), no cheats and other enhancements.

If you happen to stumble over another bug, please open an issue in the [official open_agb_firm repo on GitHub](https://github.com/profi200/open_agb_firm/issues) or contact me via other platforms.

## Hardware limitations
This is a list of limitations we can't solve in software or are very hard to work around. This doesn't mean it will never happen (unless stated otherwise).
* 64 MiB (512 mbit) games. Not possible to support.
* Games with extra hardware built into the cartridge (except Real-Time Clock). Patches are required.
* GBA Serial port (aka. Link Cable).
* 64 KiB (512 kbit) SRAM (homebrew games/emulators). Not possible to support.
* Can't switch back to 3DS mode from GBA mode requiring a reboot for booting a different game.
* Savestates. Very difficult to implement because no direct hardware access.

## Troubleshooting
Known problems and the solution.

Problem: The game crashes/shows white or blackscreens or shows a savegame corrupt message.\
Solution: Try to delete the savegame file. If this doesn't help report the issue.\
Note: EEPROM saves made by some emulators are incompatible because they have every 8 bytes block endian swapped.

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
* **ChaN**
* **fastboot3DS project**
* **Wolfvak, Sono and all the other people on GodMode9 IRC/Discord**
* ...everyone who contributed to **3dbrew.org**

Copyright (C) 2020 derrek, profi200, d0k3
