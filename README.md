# open_agb_firm

open_agb_firm is a very experimental, bare metal implementation of the 3DS GBA mode. Not ready for serious use yet.

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
* Drops a single frame roughly every 5 seconds. This is due to different refresh rates of the LCDs and the builtin GBA hardware.
* Leaving GBA sleep mode or pressing HOME can cause persistent glitches (double scanlines/shifted picture).
* No scaling for now. This requires the GPU due to timing constraints.
* Save type hardcoded to 32 KiB (256k) SRAM for now.

If you happen to stumble over another bug, please open an issue in the [official open_agb_firm repo on GitHub](https://github.com/profi200/open_agb_firm/issues) or contact me via other platforms.

## License
You may use this under the terms of the GNU General Public License GPL v3 or under the terms of any later revisions of the GPL. Refer to the provided `LICENSE.txt` file for further information.

## Thanks to...
* **yellows8**
* **plutoo**
* **smea**
* **Normmatt** (for sdmmc code)
* **WinterMute** (for console code)
* **ctrulib devs** (for HID code)
* **Luma 3DS devs** (for fmt.c/gfx code)
* **devkitPro** (for the toolchain/makefiles)
* **ChaN** (for the FATFS library)
* **fastboot3DS project** (heavily based on its codebase)
* ...everyone who contributed to **3dbrew.org**

Copyright (C) 2020 derrek, profi200, d0k3
