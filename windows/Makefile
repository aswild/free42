# MSYS2/MinGW Makefile wrapper for Free42

# don't let make do any parallel stuff
.NOTPARALLEL:

# wrapper batch file for msbuild
BUILD_VCXPROJ = cmd //c build-project.bat $(basename $(notdir $@))

# convenience targets
all: bin dec
bin: Free42Binary.exe
dec: Free42Decimal.exe
.PHONY: all bin dec

# VCX Project wrappers (built with msbuild)
Free42Binary.exe: free42.h cl111libbid.lib skins.cpp keymap.cpp VERSION.h
	$(BUILD_VCXPROJ)
	cp build/$(basename $@)/$@ $@

Free42Decimal.exe: free42.h cl111libbid.lib skins.cpp keymap.cpp VERSION.h
	$(BUILD_VCXPROJ)
	cp build/$(basename $@)/$@ $@

build/skin2cpp/skin2cpp.exe: skin2cpp.cpp skin2cpp.conf
	$(BUILD_VCXPROJ)

build/keymap2cpp/keymap2cpp.exe: keymap2cpp.cpp
	$(BUILD_VCXPROJ)

# git rev in version
VERSION.h: VERSION.h.in
	sed 's/\(FREE42_VERSION_1 .*\)"/\1 ($(shell ../gitversion.sh))"/' $< >$@

# rather than a "links" sentinel file, just look for free42.h
free42.h:
	cmd //c copy-files.bat

# Intel library gets its own script
cl111libbid.lib:
	./build-intel-lib.sh

# generated code
skins.cpp: build/skin2cpp/skin2cpp.exe
	$<

keymap.cpp: build/keymap2cpp/keymap2cpp.exe
	$<

dist: Free42Binary.exe Free42Decimal.exe README.txt
	version="$(shell ../gitversion.sh --full)"; \
		bsdtar -a -cvf Free42Windows-$${version}.zip -s "#^#Free42Windows-$${version}/#p" $^

clean:
	rm -rf build Free42Binary.exe Free42Decimal.exe keymap.cpp skins.cpp VERSION.h

distclean: clean
	rm -rf IntelRDFPMathLib20U1 cl111libbid.lib readtest.c readtest_lines.cpp \
		$(shell sed -n 's/^copy.*\\\(\S*\) \.$$/\1/p' copy-files.bat) \
		$(shell sed -n 's/^copy.* \(\S*[^.]\)$$/\1/p' copy-files.bat)

.PHONY: dist clean distclean
