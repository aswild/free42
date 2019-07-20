# MSYS2/MinGW Makefile wrapper for Free42

# don't let make do any parallel stuff
.NOTPARALLEL:

# wrapper batch file for msbuild
BUILD_VCXPROJ = cmd //c build-project.bat $(patsubst Release/%.exe,%,$@)

# convenience targets
all: bin dec
bin: Free42Binary.exe
dec: Free42Decimal.exe
.PHONY: all bin dec

# move output out of Release/ dir for convenience
Free42%.exe: Release/Free42%.exe
	mv $< $@

# VCX Project wrappers (built with msbuild)
Release/Free42Binary.exe: free42.h cl111libbid.lib skins.cpp keymap.cpp
	$(BUILD_VCXPROJ)

Release/Free42Decimal.exe: free42.h cl111libbid.lib skins.cpp keymap.cpp
	$(BUILD_VCXPROJ)

Release/skin2cpp.exe: skin2cpp.cpp skin2cpp.conf
	$(BUILD_VCXPROJ)

Release/keymap2cpp.exe: keymap2cpp.cpp
	$(BUILD_VCXPROJ)

# rather than a "links" sentinel file, just look for free42.h
free42.h:
	cmd //c link-files.bat

# Intel library gets its own script
cl111libbid.lib:
	./build-intel-lib.sh

# generated code
skins.cpp: Release/skin2cpp.exe
	$<

keymap.cpp: Release/keymap2cpp.exe
	$<
