# change the following paths
WX_BUILD_DIR=/Users/corelanc0d3r/wxWidgets/wxWidgets-latest/build-release-static

COMPILER=g++
LINKER=g++
MIN_MACOSX_VERSION=-mmacosx-version-min=10.5
COMPFLAGS=`$(WX_BUILD_DIR)/wx-config --static=yes --cxxflags` -DFUSE_USE_VERSION=26 $(MIN_MACOSX_VERSION) -D__WXOSX_COCOA__  -DWXUSINGDLL -Wall -Wundef -Wunused-parameter -Wno-ctor-dtor-privacy -Woverloaded-virtual -Wno-deprecated-declarations  -D_FILE_OFFSET_BITS=64 -I$(WX_BUILD_DIR)/lib/wx/include/osx_cocoa-unicode-3.1 -I../../../include -DWX_PRECOMP -g -O0 -fno-common -fvisibility=hidden -fvisibility-inlines-hidden -I/usr/local/include
LINKFLAGS=$(MIN_MACOSX_VERSION) `$(WX_BUILD_DIR)/wx-config --static=yes --libs`
SOURCES=*.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=encfsgui

all:	$(SOURCES) $(EXECUTABLE)
	@echo	[+] Step 3: Creating app bundle
	@echo	-------------------------------
	rm -rf $(EXECUTABLE).app
	SetFile -t APPL $(EXECUTABLE)
	mkdir -p $(EXECUTABLE).app/Contents
	mkdir -p $(EXECUTABLE).app/Contents/MacOS
	mkdir -p $(EXECUTABLE).app/Contents/Resources
	cp Info.plist.in $(EXECUTABLE).app/Contents/Info.plist
	/bin/echo "APPL????" >$(EXECUTABLE).app/Contents/PkgInfo
	cp $(EXECUTABLE).icns $(EXECUTABLE).app/Contents/Resources
	cp $(EXECUTABLE) $(EXECUTABLE).app/Contents/MacOS
	rm -rf *.o*
	mv $(EXECUTABLE).app Build
	@echo
	@echo Creating dmg into Releases folder
	mkdir -p ../release
	rm -rf ../release/*
	hdiutil create -fs HFS+ -srcfolder ./Build -volname EncFSGui EncFSGui.dmg
	mv EncFSGui.dmg ../release 
	@echo
	@echo	----------------------------------------
	@echo	 The EncFSGUI project has been compiled
	@echo	 Enjoy
	@echo	----------------------------------------
	@echo

$(EXECUTABLE): $(OBJECTS) 
	@echo
	@echo	[+] Step 2: Linking file to executable
	@echo	--------------------------------------
	$(LINKER) $(LINKFLAGS) $(OBJECTS) -o $@ 
	SetFile -t APPL encfsgui
	@echo	    Step 2 Done

.cpp.o:
	@echo
	@echo	[+] Step 1: Compiling source
	@echo	----------------------------
	$(COMPILER) $(COMPFLAGS) -c $(SOURCES)
	rm -f *from*
	@echo	    Step 1 Done
	
clean:
	@echo	[+] Eating leftovers
	rm -rf ./encfs*.o*
	rm -rf *.o*
	rm -rf *.dSYM
	rm -rf *.d
	rm -rf .deps
	rm -rf encfsgui
	mkdir -p Build
	rm -rf Build/*
	@echo
	@echo	Done eating.
	@echo	Run 'make' to compile the sources and create the application binary.
	@echo