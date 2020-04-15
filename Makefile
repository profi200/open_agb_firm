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
SECTION0_TYPE := 0
SECTION0_FILE := arm9/$(TARGET)9.bin
SECTION1_ADR  := 0x1FF89000
SECTION1_TYPE := 1
SECTION1_FILE := arm11/$(TARGET)11.bin


export VERS_STRING := $(shell git describe --tags --match v[0-9]* --abbrev=8 | sed 's/-[0-9]*-g/-/i')
export VERS_MAJOR  := $(shell echo "$(VERS_STRING)" | sed 's/v\([0-9]*\)\..*/\1/i')
export VERS_MINOR  := $(shell echo "$(VERS_STRING)" | sed 's/.*\.\([0-9]*\).*/\1/')


.PHONY: checkarm9 checkarm11 clean release

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
all: checkarm9 checkarm11 $(TARGET).firm

#---------------------------------------------------------------------------------
checkarm9:
	@$(MAKE) -j4 --no-print-directory -C arm9

#---------------------------------------------------------------------------------
checkarm11:
	@$(MAKE) -j4 --no-print-directory -C arm11

#---------------------------------------------------------------------------------
$(TARGET).firm: arm9/$(TARGET)9.bin arm11/$(TARGET)11.bin
	firm_builder $(TARGET).firm $(ENTRY9) $(ENTRY11) $(SECTION0_ADR) $(SECTION0_TYPE) \
		$(SECTION0_FILE) $(SECTION1_ADR) $(SECTION1_TYPE) $(SECTION1_FILE)

#---------------------------------------------------------------------------------
arm9/$(TARGET)9.bin:
	@$(MAKE) -j4 --no-print-directory -C arm9

#---------------------------------------------------------------------------------
arm11/$(TARGET)11.bin:
	@$(MAKE) -j4 --no-print-directory -C arm11

#---------------------------------------------------------------------------------
clean:
	@$(MAKE) --no-print-directory -C arm9 clean
	@$(MAKE) --no-print-directory -C arm11 clean
	rm -f $(TARGET).firm *.7z

release: clean
	@$(MAKE) -j4 --no-print-directory -C arm9 NO_DEBUG=1
	@$(MAKE) -j4 --no-print-directory -C arm11 NO_DEBUG=1
	firm_builder $(TARGET).firm $(ENTRY9) $(ENTRY11) $(SECTION0_ADR) $(SECTION0_TYPE) \
		$(SECTION0_FILE) $(SECTION1_ADR) $(SECTION1_TYPE) $(SECTION1_FILE)
	@7z a -mx -m0=ARM -m1=LZMA $(TARGET)$(VERS_STRING).7z $(TARGET).firm
	@7z u -mx -m0=PPMD $(TARGET)$(VERS_STRING).7z LICENSE.txt README.md
