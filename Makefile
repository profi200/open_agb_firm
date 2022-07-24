#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

export TARGET := open_agb_firm
ENTRY9        := 0x08000040
ENTRY11       := 0x1FF89034
SECTION0_ADR  := 0x08000040
ifeq ($(strip $(USE_FIRMTOOL)),1)
SECTION0_TYPE := NDMA
else
SECTION0_TYPE := 0
endif
SECTION0_FILE := arm9/$(TARGET)9.bin
SECTION1_ADR  := 0x1FF89000
ifeq ($(strip $(USE_FIRMTOOL)),1)
SECTION1_TYPE := XDMA
else
SECTION1_TYPE := 1
endif
SECTION1_FILE := arm11/$(TARGET)11.bin


export VERS_STRING := $(shell git describe --tags --match v[0-9]* --abbrev=8 | sed 's/-[0-9]*-g/-/i')
export VERS_MAJOR  := $(shell echo "$(VERS_STRING)" | sed 's/v\([0-9]*\)\..*/\1/i')
export VERS_MINOR  := $(shell echo "$(VERS_STRING)" | sed 's/.*\.\([0-9]*\).*/\1/')
NPROC := $(shell nproc)


.PHONY: checkarm9 checkarm11 clean release

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
all: checkarm9 checkarm11 $(TARGET).firm

#---------------------------------------------------------------------------------
checkarm9:
	@$(MAKE) -j$(NPROC) --no-print-directory -C arm9

#---------------------------------------------------------------------------------
checkarm11:
	@$(MAKE) -j$(NPROC) --no-print-directory -C arm11

#---------------------------------------------------------------------------------
$(TARGET).firm: arm9/$(TARGET)9.bin arm11/$(TARGET)11.bin
ifeq ($(strip $(USE_FIRMTOOL)),1)
	firmtool build $(TARGET).firm -n $(ENTRY9) -e $(ENTRY11) -A $(SECTION0_ADR) $(SECTION1_ADR) \
		-D $(SECTION0_FILE) $(SECTION1_FILE) -C $(SECTION0_TYPE) $(SECTION1_TYPE)
else
	firm_builder $(TARGET).firm $(ENTRY9) $(ENTRY11) $(SECTION0_ADR) $(SECTION0_TYPE) \
		$(SECTION0_FILE) $(SECTION1_ADR) $(SECTION1_TYPE) $(SECTION1_FILE)
endif

#---------------------------------------------------------------------------------
arm9/$(TARGET)9.bin:
	@$(MAKE) -j$(NPROC) --no-print-directory -C arm9

#---------------------------------------------------------------------------------
arm11/$(TARGET)11.bin:
	@$(MAKE) -j$(NPROC) --no-print-directory -C arm11

#---------------------------------------------------------------------------------
clean:
	@$(MAKE) --no-print-directory -C arm9 clean
	@$(MAKE) --no-print-directory -C arm11 clean
	rm -fr $(TARGET).firm *.7z nightly

#---------------------------------------------------------------------------------
release: clean
	@$(MAKE) -j$(NPROC) --no-print-directory -C arm9 NO_DEBUG=1
	@$(MAKE) -j$(NPROC) --no-print-directory -C arm11 NO_DEBUG=1
ifeq ($(strip $(USE_FIRMTOOL)),1)
	firmtool build $(TARGET).firm -n $(ENTRY9) -e $(ENTRY11) -A $(SECTION0_ADR) $(SECTION1_ADR) \
		-D $(SECTION0_FILE) $(SECTION1_FILE) -C $(SECTION0_TYPE) $(SECTION1_TYPE)
else
	firm_builder $(TARGET).firm $(ENTRY9) $(ENTRY11) $(SECTION0_ADR) $(SECTION0_TYPE) \
		$(SECTION0_FILE) $(SECTION1_ADR) $(SECTION1_TYPE) $(SECTION1_FILE)
endif
	@7z a -mx -m0=ARM -m1=LZMA $(TARGET)$(VERS_STRING).7z $(TARGET).firm
	@7z u -mx -m0=LZMA $(TARGET)$(VERS_STRING).7z resources/gba_db.bin
	@7z u -mx -m0=PPMD $(TARGET)$(VERS_STRING).7z libn3ds/libraries/fatfs/LICENSE.txt libraries/inih/LICENSE.txt LICENSE.txt README.md
	@7z rn $(TARGET)$(VERS_STRING).7z resources/gba_db.bin 3ds/open_agb_firm/gba_db.bin libn3ds/libraries/fatfs/LICENSE.txt LICENSE_FatFs.txt libraries/inih/LICENSE.txt LICENSE_inih.txt

#---------------------------------------------------------------------------------
nightly: clean
	@$(MAKE) -j$(NPROC) --no-print-directory -C arm9 NO_DEBUG=1
	@$(MAKE) -j$(NPROC) --no-print-directory -C arm11 NO_DEBUG=1
ifeq ($(strip $(USE_FIRMTOOL)),1)
	firmtool build $(TARGET).firm -n $(ENTRY9) -e $(ENTRY11) -A $(SECTION0_ADR) $(SECTION1_ADR) \
		-D $(SECTION0_FILE) $(SECTION1_FILE) -C $(SECTION0_TYPE) $(SECTION1_TYPE)
else
	firm_builder $(TARGET).firm $(ENTRY9) $(ENTRY11) $(SECTION0_ADR) $(SECTION0_TYPE) \
		$(SECTION0_FILE) $(SECTION1_ADR) $(SECTION1_TYPE) $(SECTION1_FILE)
endif
	@mkdir -p nightly/3ds/open_agb_firm
	@cp -t nightly $(TARGET).firm LICENSE.txt README.md
	@cp resources/gba_db.bin nightly/3ds/open_agb_firm
	@cp libn3ds/libraries/fatfs/LICENSE.txt nightly/LICENSE_FatFs.txt
	@cp libraries/inih/LICENSE.txt nightly/LICENSE_inih.txt
