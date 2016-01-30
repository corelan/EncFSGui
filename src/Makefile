# change the following paths
WX_BUILD_DIR=/Users/corelanc0d3r/wxWidgets/wxWidgets-latest/build-release-static

COMPILER=g++
LINKER=g++
MIN_MACOSX_VERSION=-mmacosx-version-min=10.5
CPPFLAGS=`$(WX_BUILD_DIR)/wx-config --static=yes --cxxflags` -I$(CURL_INC_DIR) -DFUSE_USE_VERSION=26 $(MIN_MACOSX_VERSION) -DCURL_STATICLIB  -D__WXOSX_COCOA__  -DWXUSINGDLL -Wall -Wundef -Wunused-parameter -Wno-ctor-dtor-privacy -Woverloaded-virtual -Wno-deprecated-declarations  -D_FILE_OFFSET_BITS=64 -I$(WX_BUILD_DIR)/lib/wx/include/osx_cocoa-unicode-3.1 -I../../../include -DWX_PRECOMP -g -O0 -fno-common -fvisibility=hidden -fvisibility-inlines-hidden -I/usr/local/include
LDFLAGS=$(MIN_MACOSX_VERSION) `$(WX_BUILD_DIR)/wx-config --static=yes --libs` -lcurl

SOURCES=*.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=encfsgui
APPNAME=EncFSGui

all:	$(SOURCES) $(EXECUTABLE)
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
	$(LINKER) $(LDFLAGS) $(OBJECTS) -o $@ 
	SetFile -t APPL encfsgui
	@echo	    Step 2 Done

.cpp.o:
	@echo
	@echo	[+] Step 1: Compiling source
	@echo	----------------------------
	chmod +x bumpversion.sh
	./bumpversion.sh
	$(COMPILER) $(CPPFLAGS) -c $(SOURCES)
	rm -f *from*
	@echo	    Step 1 Done
	
clean:
	@echo	[+] Eating leftovers
	rm -rf *.o*
	rm -rf *.dSYM
	rm -rf *.d
	rm -rf .deps
	rm -rf encfsgui
	rm -rf *.app
	mkdir -p Build
	rm -rf Build/*
	mkdir -p ../release
	rm -rf ../release/*
	@echo
	@echo	Done eating.
	@echo	Run 'make' to compile the sources and create the application binary.
	@echo

release:
	@echo	[+] Creating app bundle $(APPNAME).app
	@echo	-----------------------------------------
	rm -rf *.app
	rm -rf Build/*
	rm -rf ../release/*
	SetFile -t APPL $(EXECUTABLE)
	mkdir -p $(APPNAME).app/Contents
	mkdir -p $(APPNAME).app/Contents/MacOS
	mkdir -p $(APPNAME).app/Contents/Resources
	chmod +x storeversion.sh
	./storeversion.sh $(APPNAME)
	/bin/echo "APPL????" >$(APPNAME).app/Contents/PkgInfo
	cp $(EXECUTABLE).icns $(APPNAME).app/Contents/Resources
	cp $(EXECUTABLE) $(APPNAME).app/Contents/MacOS
	rm -rf *.o*
	mv $(APPNAME).app Build/$(APPNAME).app
	@echo
	@echo Creating dmg into Releases folder
	mkdir -p ../release
	rm -rf ../release/*
	hdiutil create -fs HFS+ -srcfolder ./Build -volname EncFSGui EncFSGui.dmg
	mv EncFSGui.dmg ../release
	cp version.txt ../release 