# EncFSGui

Public repository for the encfsgui project, an OSX GUI wrapper around encfs

## Warning 
This application is NOT ready for use yet. <br>
It is still under heavy development and will be updated on a regular basis.<br>
Proper announcements will be made when the app is ready for use. <br><br>

## Background
This application is written in C++, and uses the wxWidgets Cross-Platform Library.  Although the source probably compiles fine under Linux/Unix and Windows, it was written for OSX and contains hardcoded strings & paths that will certainly prevent the app from working on Windows.  It might actually work on Linux (but I haven't tested it myself)<br>
If you are looking for an EncFS Gui for Windows, check out the EncFSMP project on sourceforge.<br>
Also, this is my very first project in C++.  As I started learning C++ just a few weeks ago (self-study), I am fully aware that my C++ stinks. I am quite keen on learning & improving, and I am open for constructive advise and help.<br>

With that said, all positive contributions are more than welcome. If you want to contribute, check out the development setup section first.<br>
If you want to try the application, you'll need a number of dependencies installed on your OSX machine:
- encfs
- OSXFuse


## Development setup: Installing dependencies on OSX

(this procedure should work on Yosemite and El Capitan, the 2 versions that I am using myself) 

1. Install XCode 7

(Look for it in the app store yo)


2. Install command line tools

```
xcode-select --install
```

Verify that the command line tools are installed correctly:
`xcode-select -p` 	(should print out a path that ends with 'Developer')
`gcc -version`		(should print out version information)


3. Install homebrew

```
ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"

brew doctor
```

4. Install OSXFuse

Download the dmg image from http://sourceforge.net/projects/osxfuse/files/ and install



5. Install encfs

```
brew update
```


On El Capitan, fix permissions:

```
sudo chown -R $USER:admin /usr/local/include
sudo chown -R $USER:admin /usr/local/lib/pkgconfig
brew link xz libtool boost rlog
```

Then check if everything is ok and finally install encfs

```
brew doctor
brew install homebrew/fuse/encfs
```

Check if encfs works:
```
encfs
```



## Development setup

1. Clone the latest version of the wxWidgets repository from github:

```
git clone https://github.com/wxWidgets/wxWidgets.git wxWidgets-latest
```

2. Build the wxWidget library binaries for static linking  

(from within the wxWidgets-latest folder)

```
mkdir build-release-static
cd build-release-static
export PATH=$(pwd):$PATH
../configure --enable-optimise --prefix="$(pwd)" --enable-stl --enable-unicode --enable-threads --enable-static --disable-shared --enable-monolithic --enable-graphics_ctx
make
# Build the samples and demos (optional)
cd samples; make;cd ..
cd demos;   make;cd ..
```

3. Edit the Makefile and update the WX_BUILD_DIR variable to make it reflect the absolute folder structure/path on your own machine
4. run `make clean` before compiling/linking
5. run `make` to compile and link




