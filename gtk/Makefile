###############################################################################
# Free42 -- an HP-42S calculator simulator
# Copyright (C) 2004-2024  Thomas Okken
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License, version 2,
# as published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, see http://www.gnu.org/licenses/.
###############################################################################

PKG_CONFIG ?= pkg-config

CPPFLAGS += -MMD
CFLAGS ?= -O2

CFLAGS += \
	$(CPPFLAGS) \
	 -Wall \
	 -Wno-parentheses \
	 -Wno-write-strings \
	 -Wno-sign-compare \
	 -Wno-narrowing \
	 -Wno-format-truncation \
	 -pipe -g \
	 $(shell $(PKG_CONFIG) --cflags gtk+-3.0) \
	 -DGDK_DISABLE_DEPRECATION_WARNINGS -DGTK_DISABLE_SINGLE_INCLUDES -DGSEAL_ENABLE \
	 -DVERSION="\"$(shell cat VERSION) ($(shell ../gitversion.sh))\"" \
	 -DVERSION_PLATFORM="\"$(shell uname -s)\"" \
	 -DDECIMAL_CALL_BY_REFERENCE=1 \
	 -DDECIMAL_GLOBAL_ROUNDING=1 \
	 -DDECIMAL_GLOBAL_ROUNDING_ACCESS_FUNCTIONS=1 \
	 -DDECIMAL_GLOBAL_EXCEPTION_FLAGS=1 \
	 -DDECIMAL_GLOBAL_EXCEPTION_FLAGS_ACCESS_FUNCTIONS=1

ifeq ($(WERROR),1)
CFLAGS += -Werror
endif

CXXFLAGS = $(CFLAGS) \
	 -fno-exceptions \
	 -fno-rtti \
	 -D_WCHAR_T_DEFINED

HOSTCC       ?= $(CC)
HOSTCXX      ?= $(CXX)
HOSTCFLAGS   ?= $(CFLAGS)
HOSTCXXFLAGS ?= $(CXXFLAGS)
HOSTLDFLAGS  ?= $(LDFLAGS)

LIBS = -L$(INTEL_DIR)/LIBRARY -lbid $(shell $(PKG_CONFIG) --libs gtk+-3.0)

ifdef AUDIO_ALSA
LIBS += -lpthread -ldl
endif

ifneq "$(findstring 6162,$(shell echo ab | od -x))" ""
CFLAGS += -DF42_BIG_ENDIAN -DBID_BIG_ENDIAN
endif

ifeq "$(shell uname -m)" "arm64"
CFLAGS += -fsigned-char -DBID_SIZE_LONG=8
else ifeq "$(shell uname -m)" "aarch64"
CFLAGS += -fsigned-char -DBID_SIZE_LONG=8
else ifeq "$(shell uname -m | sed 's/^\(...\).*$$/\1/')" "arm"
CFLAGS += -fsigned-char -DBID_SIZE_LONG=4
endif

SRCS = shell_main.cc shell_skin.cc skins.cc keymap.cc shell_loadimage.cc \
	shell_spool.cc core_main.cc core_commands1.cc core_commands2.cc \
	core_commands3.cc core_commands4.cc core_commands5.cc \
	core_commands6.cc core_commands7.cc core_display.cc core_globals.cc \
	core_helpers.cc core_keydown.cc core_linalg1.cc core_linalg2.cc \
	core_math1.cc core_math2.cc core_phloat.cc core_sto_rcl.cc \
	core_tables.cc core_variables.cc
CORE_OBJS = shell_spool.o core_main.o core_commands1.o core_commands2.o \
	core_commands3.o core_commands4.o core_commands5.o \
	core_commands6.o core_commands7.o core_display.o core_globals.o \
	core_helpers.o core_keydown.o core_linalg1.o core_linalg2.o \
	core_math1.o core_math2.o core_phloat.o core_sto_rcl.o \
	core_tables.o core_variables.o
OBJS = shell_main.o shell_skin.o skins.o keymap.o shell_loadimage.o \
	$(CORE_OBJS)

BCD_MATH ?= 1

INTEL_DIR = IntelRDFPMathLib20U1
INTEL_LIB = $(INTEL_DIR)/LIBRARY/libbid.a

ifeq ($(BCD_MATH),1)
CXXFLAGS += -DBCD_MATH
EXE = free42dec
else
EXE = free42bin
endif

ifdef FREE42_FPTEST
CFLAGS += -DFREE42_FPTEST
SRCS += readtest.c readtest_lines.cc
OBJS += readtest.o readtest_lines.o
endif

ifdef AUDIO_ALSA
# Note: the name of the libasound shared library that is usually compiled into
# the executable is defined in the corresponding *.la file, in the 'dlname'
# property. I suppose it would be better to grab it from there at build time,
# rather than hard-coding it into the Makefile like this, but as long as the
# build environment doesn't change (currently Ubuntu 12.04), this is fine.
CXXFLAGS += -DAUDIO_ALSA -DALSALIB="\"libasound.so.2\""
SRCS += audio_alsa.cc
OBJS += audio_alsa.o
endif

# silent make rules
V ?= 0
_V_CC_0        = @echo "  CC      " $@;
_V_CXX_0       = @echo "  CXX     " $@;
_V_LD_0        = @echo "  LD      " $@;
_V_HOSTCC_0    = @echo "  HOSTCC  " $@;
_V_HOSTCXX_0   = @echo "  HOSTCXX " $@;
_V_HOSTLD_0    = @echo "  HOSTLD  " $@;
_V_GEN_0       = @echo "  GEN     " $@;
_V_CLEAN_0     = @echo "  CLEAN";
_V_DISTCLEAN_0 = @echo "  DISTCLEAN";

$(EXE): $(OBJS)
	$(_V_LD_$(V))$(CXX) -o $(EXE) $(CXXFLAGS) $(LDFLAGS) $(OBJS) $(LIBS)

$(SRCS) skin2cc.cc keymap2cc.cc skin2cc.conf: .symlinks_done $(INTEL_LIB)

.cc.o:
	$(_V_CXX_$(V))$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

txt2raw: txt2raw.o $(CORE_OBJS)
	$(_V_LD_$(V))$(CXX) -o txt2raw $(CXXFLAGS) $(LDFLAGS) txt2raw.o $(CORE_OBJS) $(LIBS)

raw2txt: raw2txt.o $(CORE_OBJS)
	$(_V_LD_$(V))$(CXX) -o raw2txt $(CXXFLAGS) $(LDFLAGS) raw2txt.o $(CORE_OBJS) $(LIBS)

readtest.o: readtest.c
	$(_V_CC_$(V))$(CC) $(CFLAGS) -I $(INTEL_DIR)/TESTS -D__intptr_t_defined -DLINUX -c -o $@ $<

skin2cc: skin2cc.cc
	$(_V_HOSTCXX_$(V))$(HOSTCXX) $(HOSTCXXFLAGS) $(HOSTLDFLAGS) -o skin2cc skin2cc.cc

skins.cc: skin2cc skin2cc.conf
	$(_V_GEN_$(V))./skin2cc

keymap2cc: keymap2cc.cc
	$(_V_HOSTCXX_$(V))$(CXX) $(HOSTCXXFLAGS) $(HOSTLDFLAGS) -o keymap2cc keymap2cc.cc

keymap.cc: keymap2cc keymap.txt
	$(_V_GEN_$(V))./keymap2cc

symlinks: .symlinks_done
.symlinks_done:
	find ../common -type f -exec ln -s {} . \;
	@touch $@

$(INTEL_LIB):
	+sh ./build-intel-lib.sh

CLEAN_FILES = skin2cc skins.cc keymap2cc keymap.cc readtest_lines.cc
CLEAN_FILES += raw2txt txt2raw
CLEAN_FILES += .symlinks_done *.o *.d 
CLEANER_FILES = free42bin free42dec

clean:
	find . -type l -delete
	rm -f $(CLEAN_FILES)

cleaner: clean
	rm -f $(CLEANER_FILES)
	rm -rf $(INTEL_DIR)

distclean: cleaner
.PHONY: clean cleaner distclean

-include $(OBJS:.o=.d)
