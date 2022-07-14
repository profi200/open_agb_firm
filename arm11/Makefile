#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

TOPDIR ?= $(CURDIR)
include $(DEVKITARM)/base_rules
include $(TOPDIR)/../libn3ds/libn3ds11.mk

#---------------------------------------------------------------------------------
# TARGET is the name of the output
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# DATA is a list of directories containing data files
# INCLUDES is a list of directories containing header files
#---------------------------------------------------------------------------------
#TARGET		:=	$(notdir $(CURDIR))
BUILD		:=	build
SOURCES		+=	../source/arm11 ../libraries/inih
DATA		:=
INCLUDES	+=	../include ../libraries
DEFINES		:=	-DARM11 -D_3DS -DVERS_STRING=\"$(VERS_STRING)\" \
				-DVERS_MAJOR=$(VERS_MAJOR) -DVERS_MINOR=$(VERS_MINOR)
ASSETS		:=

ifneq ($(strip $(NO_DEBUG)),)
	DEFINES += -DNDEBUG
endif

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
ARCH	:=	-march=armv6k+vfpv2 -mtune=mpcore -mfloat-abi=hard -mtp=soft -marm -mthumb-interwork -masm-syntax-unified

CFLAGS	:=	$(ARCH) -std=c17 -O2 -gdwarf-4 -flto -mword-relocations \
			-ffunction-sections -fno-math-errno -Wall -Wextra
CFLAGS	+=	$(INCLUDE) $(DEFINES)

CXXFLAGS	:=	$(ARCH) -std=c++17 -O2 -gdwarf-4 -flto -fno-rtti -fno-exceptions \
				-mword-relocations -ffunction-sections -fno-math-errno -Wall -Wextra
CXXFLAGS	+=	$(INCLUDE) $(DEFINES)

ASFLAGS	:=	$(ARCH) -gdwarf-4 -flto $(INCLUDE) $(DEFINES)
LDFLAGS	=	$(ARCH) -gdwarf-4 -flto -specs=../arm11.specs -Wl,-Map,$(notdir $*.map) -nostartfiles

ifeq ($(strip $(NO_DEBUG)),)
	CFLAGS	:=	$(subst -flto,,$(CFLAGS)) -fstack-protector-strong -fno-inline
	CXXFLAGS	:=	$(subst -flto,,$(CXXFLAGS)) -fstack-protector-strong -fno-inline
	ASFLAGS	:=	$(subst -flto,,$(ASFLAGS))
	LDFLAGS	:=	$(subst -flto,,$(LDFLAGS)) -fstack-protector-strong -fno-inline -Wl,-wrap=malloc,-wrap=calloc,-wrap=free
endif

LIBS	:=	-lm

#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS	:=


#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------

export OUTPUT	:=	$(CURDIR)/$(TARGET)
export TOPDIR	:=	$(CURDIR)

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
					$(foreach dir,$(DATA),$(CURDIR)/$(dir)) \
					$(foreach dir,$(ASSETS),$(CURDIR)/$(dir))

export DEPSDIR	:=	$(CURDIR)/$(BUILD)

CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
ASSETFILES	:=	$(foreach dir,$(ASSETS),$(notdir $(wildcard $(dir)/*.png)))
DMA330FILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.dma330)))
BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))

#---------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#---------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
#---------------------------------------------------------------------------------
	export LD	:=	$(CC)
#---------------------------------------------------------------------------------
else
#---------------------------------------------------------------------------------
	export LD	:=	$(CXX)
#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------

export OFILES_SOURCES 	:=	$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)

export OFILES_BIN	:=	$(addsuffix .o,$(BINFILES)) $(ASSETFILES:.png=.spla.o)

export OFILES := $(OFILES_BIN) $(OFILES_SOURCES)

export HFILES	:=	$(ASSETFILES:.png=_spla.h) $(addsuffix .h,$(subst .,_,$(BINFILES))) \
					$(addsuffix .h,$(subst .,_,$(DMA330FILES)))

export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
					$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
					-I$(CURDIR)/$(BUILD)

export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib)


.PHONY: $(BUILD) all clean

#---------------------------------------------------------------------------------
all: $(BUILD) $(DEPSDIR)
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

$(BUILD):
	@mkdir -p $@

ifneq ($(DEPSDIR),$(BUILD))
$(DEPSDIR):
	@mkdir -p $@
endif

#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	@rm -fr $(BUILD) $(OUTPUT)11.bin $(OUTPUT)11.elf

#---------------------------------------------------------------------------------
else

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
$(OUTPUT)11.bin	:	$(OUTPUT)11.elf
	@$(OBJCOPY) -O binary $< $@
	@echo built ... $(notdir $@)

$(OFILES_SOURCES) : $(HFILES)

$(OUTPUT)11.elf	:	$(OFILES)

#---------------------------------------------------------------------------------
# you need a rule like this for each extension you use as binary data
#---------------------------------------------------------------------------------
%.bin.o	%_bin.h :	%.bin
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)

#---------------------------------------------------------------------------------
# rules for conversion of .png to .spla
#---------------------------------------------------------------------------------
define splashConv
	$(eval CURBIN := $*.spla)
	$(eval DEPSFILE := $(DEPSDIR)/$*.spla.d)
	echo "$(CURBIN).o: $< $1" > $(DEPSFILE)
	echo "extern const u8" `(echo $(CURBIN) | sed -e 's/^\([0-9]\)/_\1/' | tr . _)`"_end[];" > `(echo $(CURBIN) | tr . _)`.h
	echo "extern const u8" `(echo $(CURBIN) | sed -e 's/^\([0-9]\)/_\1/' | tr . _)`"[];" >> `(echo $(CURBIN) | tr . _)`.h
	echo "extern const u32" `(echo $(CURBIN) | sed -e 's/^\([0-9]\)/_\1/' | tr . _)`_size";" >> `(echo $(CURBIN) | tr . _)`.h
	splashtool -f RGB565 -r $1 $(CURBIN)
	bin2s $(CURBIN) | $(AS) -o $*.spla.o
endef

%.spla.o %_spla.h : %.png
	@echo $(notdir $^)
	@$(call splashConv,$^)

#---------------------------------------------------------------------------------
# Rule for conversion of .dma330 to .h
#---------------------------------------------------------------------------------
%_dma330.h :	%.dma330
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@dma330as $< $@

#---------------------------------------------------------------------------------
%.elf:
#---------------------------------------------------------------------------------
	@echo linking $(notdir $@)
	@$(LD) $(LDFLAGS) $(OFILES) $(LIBPATHS) $(LIBS) -o $@
	@$(NM) -CSn $@ > $(notdir $*.lst)

-include $(DEPSDIR)/*.d

#---------------------------------------------------------------------------------------
endif
