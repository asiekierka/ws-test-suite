# SPDX-License-Identifier: CC0-1.0
#
# SPDX-FileContributor: Adrian "asie" Siekierka, 2023

WONDERFUL_TOOLCHAIN ?= /opt/wonderful
TARGET ?= wswan/small
include $(WONDERFUL_TOOLCHAIN)/target/$(TARGET)/makedefs.mk

INCLUDEDIRS	:= common
CBINDIRS	:= resources

# Defines passed to all files
# ---------------------------

DEFINES		:=

# Libraries
# ---------

LIBS		:= -lwsx -lws
LIBDIRS		:= $(WF_TARGET_DIR)

# Build artifacts
# ---------------

ifeq ($(TARGET),wswan/bootfriend)
    EXT_MONO := .bfb
    EXT_COLOR := .bfb
    BUILDDIR := build/wswan-bootfriend
    BIN2C_FLAGS :=
    DEFINES += -DBOOTFRIEND
else
    EXT_MONO := .ws
    EXT_COLOR := .wsc
    BUILDDIR := build/wswan-small
    BIN2C_FLAGS := --address-space __far
endif

# Verbose flag
# ------------

ifeq ($(V),1)
_V		:=
else
_V		:= @
endif

# Source files
# ------------

ifneq ($(CBINDIRS),)
    SOURCES_CBIN	:= $(shell find -L $(CBINDIRS) -name "*.bin")
    INCLUDEDIRS		+= $(addprefix $(BUILDDIR)/,$(CBINDIRS))
endif

TEST_MONO_DIRECTORIES	:= $(shell find -L src/mono -type f -name 'main.*' | sed 's#/[^/]*$$##')
TEST_COLOR_DIRECTORIES	:= $(shell find -L src/color -type f -name 'main.*' | sed 's#/[^/]*$$##')
TEST_MONO_DIRECTORIES	+= $(shell find -L src/tools -type f -name 'main.*' | sed 's#/[^/]*$$##')
TEST_MONO_DIRECTORIES	+= $(shell find -L src/wonderful -type f -name 'main.*' | sed 's#/[^/]*$$##')
TEST_ROMS		:= \
	$(addsuffix $(EXT_MONO),$(subst src,build/roms,$(TEST_MONO_DIRECTORIES))) \
	$(addsuffix $(EXT_COLOR),$(subst src,build/roms,$(TEST_COLOR_DIRECTORIES)))
TEST_SOURCES		:= $(shell find -L src -name "*.s") $(shell find -L src -name "*.c")
COMMON_SOURCES		:= $(shell find -L common -name "*.s") $(shell find -L common -name "*.c")

# Compiler and linker flags
# -------------------------

WARNFLAGS	:= -Wall

INCLUDEFLAGS	:= $(foreach path,$(INCLUDEDIRS),-I$(path)) \
		   $(foreach path,$(LIBDIRS),-isystem $(path)/include)

LIBDIRSFLAGS	:= $(foreach path,$(LIBDIRS),-L$(path)/lib)

ASFLAGS		+= -x assembler-with-cpp $(DEFINES) $(WF_ARCH_CFLAGS) \
		   $(INCLUDEFLAGS) -ffunction-sections

CFLAGS		+= -std=gnu11 $(WARNFLAGS) $(DEFINES) $(WF_ARCH_CFLAGS) \
		   $(INCLUDEFLAGS) -ffunction-sections -Os

LDFLAGS		:= -T$(WF_LDSCRIPT) $(LIBDIRSFLAGS) \
		   $(WF_ARCH_LDFLAGS) $(LIBS)

BUILDROMFLAGS	:=
#BUILDROMFLAGS	+= -v

# Intermediate build files
# ------------------------

OBJS_ASSETS	:= $(addsuffix .o,$(addprefix $(BUILDDIR)/,$(SOURCES_CBIN)))

HEADERS_ASSETS	:= $(patsubst %.bin,%_bin.h,$(addprefix $(BUILDDIR)/,$(SOURCES_CBIN)))

OBJS_TEST	:= $(addsuffix .o,$(addprefix $(BUILDDIR)/,$(TEST_SOURCES)))

OBJS_COMMON	:= $(addsuffix .o,$(addprefix $(BUILDDIR)/,$(COMMON_SOURCES)))

OBJS		:= $(OBJS_ASSETS) $(OBJS_TEST) $(OBJS_COMMON)

DEPS		:= $(OBJS:.o=.d)

# Targets
# -------

.PHONY: all clean

all: $(TEST_ROMS) compile_commands.json

clean:
	@echo "  CLEAN"
	$(_V)$(RM) $(ROM) $(BUILDDIR)

compile_commands.json: $(OBJS) | Makefile
	@echo "  MERGE   compile_commands.json"
	$(_V)$(WF)/bin/wf-compile-commands-merge $@ $(patsubst %.o,%.cc.json,$^)

# Rules
# -----

build/%.ws : $(OBJS)
	@echo "  ROMLINK $@"
	@$(MKDIR) -p $(@D)
	@$(MKDIR) -p build/elfs/$(@D)
	$(_V)$(CC) -r -o build/elfs/$@_stage1.elf $(OBJS_ASSETS) $(OBJS_COMMON) $(filter $(subst build/roms,$(BUILDDIR)/src,$(subst .ws,,$@))%, $(OBJS_TEST)) $(WF_CRT0) $(LDFLAGS)
	$(_V)$(BUILDROM) -o $@ -c $(subst build/roms,src,$(subst .ws,/wfconfig.toml,$@)) --output-elf build/elfs/$@.elf $(BUILDROMFLAGS) build/elfs/$@_stage1.elf

build/%.wsc : $(OBJS)
	@echo "  ROMLINK $@"
	@$(MKDIR) -p $(@D)
	@$(MKDIR) -p build/elfs/$(@D)
	$(_V)$(CC) -r -o build/elfs/$@_stage1.elf $(OBJS_ASSETS) $(OBJS_COMMON) $(filter $(subst build/roms,$(BUILDDIR)/src,$(subst .wsc,,$@))%, $(OBJS_TEST)) $(WF_CRT0) $(LDFLAGS)
	$(_V)$(BUILDROM) -o $@ -c $(subst build/roms,src,$(subst .wsc,/wfconfig.toml,$@)) --output-elf build/elfs/$@.elf $(BUILDROMFLAGS) build/elfs/$@_stage1.elf

build/%.bfb : $(OBJS)
	@echo "  BFBLINK $@"
	@$(MKDIR) -p $(@D)
	@$(MKDIR) -p build/elfs/$(@D)
	$(_V)$(CC) -r -o build/elfs/$@_stage1.elf $(OBJS_ASSETS) $(OBJS_COMMON) $(filter $(subst build/roms,$(BUILDDIR)/src,$(subst .bfb,,$@))%, $(OBJS_TEST)) $(WF_CRT0) $(LDFLAGS)
	$(_V)$(BUILDBFB) -o $@ --output-elf build/elfs/$@.elf $(BUILDROMFLAGS) build/elfs/$@_stage1.elf

$(BUILDDIR)/%.s.o : %.s
	@echo "  AS      $<"
	@$(MKDIR) -p $(@D)
	$(_V)$(CC) $(ASFLAGS) -MMD -MP -MJ $(patsubst %.o,%.cc.json,$@) -c -o $@ $<

$(BUILDDIR)/%.c.o : %.c
	@echo "  CC      $<"
	@$(MKDIR) -p $(@D)
	$(_V)$(CC) $(CFLAGS) -MMD -MP -MJ $(patsubst %.o,%.cc.json,$@) -c -o $@ $<

$(BUILDDIR)/%.bin.o $(BUILDDIR)/%_bin.h : %.bin
	@echo "  BIN2C   $<"
	@$(MKDIR) -p $(@D)
	$(_V)$(WF)/bin/wf-bin2c -a 2 $(BIN2C_FLAGS) $(@D) $<
	$(_V)$(CC) $(CFLAGS) -MMD -MP -c -o $(BUILDDIR)/$*.bin.o $(BUILDDIR)/$*_bin.c

# Include dependency files if they exist
# --------------------------------------

-include $(DEPS)
