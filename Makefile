#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

TOPDIR ?= $(CURDIR)
include $(DEVKITARM)/3ds_rules

TARGET		:=	legowb3ds
BUILD		:=	build
SOURCES		:=	source
DATA		:=
INCLUDES	:=	include
GRAPHICS	:=	gfx
ROMFS		:=	romfs
GFXBUILD	:=	$(ROMFS)/gfx

ARCH	:=	-march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft

CFLAGS	:=	-g -Wall -O2 -mword-relocations -ffunction-sections $(ARCH)
CFLAGS	+=	$(INCLUDE) -D__3DS__
# Record source paths relative to the project rather than absolutely, so the
# build does not depend on where it was checked out. Without this the debug
# information carries the full path, which ends up inside the CIA, and the same
# sources built in two different directories produce two different CIAs. The
# 3dsx is unaffected either way, since it carries no debug information.
#
# TOPDIR rather than CURDIR, because this makefile re-invokes itself inside
# build/ where CURDIR is the build directory. Under MSYS this does nothing:
# make reports /d/project while gcc records D:/project, and the two forms do
# not match. It works where the paths agree, which includes CI.
CFLAGS	+=	-ffile-prefix-map=$(TOPDIR)=.
CXXFLAGS	:= $(CFLAGS) -fno-rtti -fno-exceptions -std=gnu++11
ASFLAGS	:=	-g $(ARCH)
LDFLAGS	=	-specs=3dsx.specs -g $(ARCH) -Wl,-Map,$(notdir $*.map)

LIBS	:= -lcitro2d -lcitro3d -lctru -lm
LIBDIRS	:= $(CTRULIB)

APP_TITLE := LEGO World Builder 3DS
APP_DESCRIPTION := LEGO World Builder 3DS Port
APP_AUTHOR := gameLab
APP_ICON := $(TOPDIR)/icon.png

ifneq ($(BUILD),$(notdir $(CURDIR)))

export OUTPUT	:=	$(CURDIR)/$(TARGET)
export TOPDIR	:=	$(CURDIR)
export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir))
export VPATH	+=	$(foreach dir,$(GRAPHICS),$(CURDIR)/$(dir))
export DEPSDIR	:=	$(CURDIR)/$(BUILD)

CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
GFXFILES	:=	$(foreach dir,$(GRAPHICS),$(notdir $(wildcard $(dir)/*.t3s)))
BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))

ifeq ($(strip $(CPPFILES)),)
	export LD	:=	$(CC)
else
	export LD	:=	$(CXX)
endif

export OFILES_SOURCES := $(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)
export T3XFILES	:=	$(GFXFILES:.t3s=.t3x)
export OFILES_BIN	:=	$(addsuffix .o,$(BINFILES)) $(if $(filter $(BUILD),$(GFXBUILD)),$(addsuffix .o,$(T3XFILES)))
export OFILES		:=	$(OFILES_BIN) $(OFILES_SOURCES)
export HFILES		:=	$(addsuffix .h,$(subst .,_,$(BINFILES))) $(GFXFILES:.t3s=.h)

export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
			$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
			-I$(CURDIR)/$(BUILD)
export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib)

export _3DSXDEPS	:=	$(OUTPUT).smdh
export _3DSXFLAGS	+=	--smdh=$(CURDIR)/$(TARGET).smdh --romfs=$(CURDIR)/$(ROMFS)

# Overridable so a build on something other than Windows can point at its own
# copies. The bundled ones are Windows binaries.
BANNERTOOL	?=	$(CURDIR)/tools/bannertool.exe
MAKEROM		?=	$(CURDIR)/tools/makerom.exe

.PHONY: all clean

all:
	@mkdir -p $(BUILD) $(GFXBUILD)
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile $(T3XFILES)
# The atlases have to be in romfs/gfx before the 3dsx is packed, not after.
# They used to be copied afterwards, which worked only because the directory
# still held the previous build's copies: on a clean tree the first 3dsx came
# out with no graphics in it at all, a third of the size it should be. The CIA
# was unaffected, since makerom reads the directory later still.
	@cp -f $(BUILD)/*.t3x $(GFXBUILD)/ 2>/dev/null || true
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile
	@echo Building CIA banner...
	@$(BANNERTOOL) makebanner -i $(CURDIR)/banner.png -a $(CURDIR)/banner.wav -o $(CURDIR)/banner.bnr
	@echo Building CIA...
	@$(MAKEROM) -f cia -o $(OUTPUT).cia -target t -desc app:2.50 \
		-rsf $(CURDIR)/$(TARGET).rsf -elf $(OUTPUT).elf \
		-icon $(OUTPUT).smdh -banner $(CURDIR)/banner.bnr \
		-DDIR_ROMFS=$(CURDIR)/$(ROMFS)

clean:
	@echo clean ...
	@rm -fr $(BUILD) $(TARGET).3dsx $(OUTPUT).smdh $(TARGET).elf $(TARGET).cia banner.bnr

else

$(OUTPUT).3dsx: $(OUTPUT).elf $(_3DSXDEPS)

$(OFILES_SOURCES): $(HFILES)

$(OUTPUT).elf: $(OFILES)

%.bin.o %_bin.h: %.bin
	@echo $(notdir $<)
	@$(bin2o)

.PRECIOUS: %.t3x
%.t3x.o %_t3x.h: %.t3x
	@$(bin2o)

%.t3x %.h: %.t3s $(wildcard $(TOPDIR)/$(GRAPHICS)/*.png)
	@echo $(notdir $<)
	@tex3ds -i $< -H $*.h -d $*.d -o $*.t3x

-include $(DEPENDS)

endif
