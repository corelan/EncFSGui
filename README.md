# EncFSGui

Public repository for the encfsgui project, an OSX GUI wrapper around encfs

## Warning 
This application is NOT ready for use yet. <br>
It is still under heavy development and will be updated on a regular basis.<br>
Proper announcements will be made when the app is ready for use. <br><br>

## Donate

If you like this initiative and want to show your appreciation, please consider donating using [this link](https://www.paypal.com/cgi-bin/webscr?cmd=_donations&bn=EncFSGUI&business=peter.ve@corelan.be&currency_code=EUR&item_name=Support%20EncFSGui%20Project)<br>

(I could have decided to distribute this as a commercial closed-sourced project and sell it via the App Store... but I didn't, despite the fact that 'there is no free lunch')<br>


## Background
This application is written in C++, and uses the wxWidgets Cross-Platform Library.  Although the source probably compiles fine under Linux/Unix and Windows, it was written for OSX and contains hardcoded strings & paths that will certainly prevent the app from working on Windows.  It might actually work on Linux (but I haven't tested it myself)<br>
If you are looking for an EncFS Gui for Windows, check out the EncFSMP project on sourceforge.<br>
Also, this is my very first project in C++.  As I started learning C++ just a few weeks ago (self-study), I am fully aware that my C++ stinks. I am quite keen on learning & improving, and I am open for constructive advise and help.<br>

With that said, all positive contributions are more than welcome. If you want to contribute, check out the development setup section first.<br>

For now, if you want to try the application, you'll need a number of dependencies installed on your OSX machine:
- encfs
- OSXFuse

Note: The plan is to statically compile this application with the required dependencies (encfs,fuse, openssl), so you would not have to worry about installing dependencies yourself.


## Running EncFSGUi : Installing dependencies on OSX

(this procedure should work on Yosemite and El Capitan, as those are the 2 versions that I am using myself) 


1. Install the xcode development command line tools

  Simply run 'gcc', OSX should ask you if you want to install the xcode command lines tools

  ```
  gcc
  ```

  If you already have XCode installed (which is not needed by itself to compile this app), you can also run:

  ```
  xcode-select --install
  ```

  Verify that the command line tools are installed correctly:<br>
  `xcode-select -p` 	(should print out a path that ends with 'Developer', but this will only work if you have xcode installed)<br>
  `gcc -version`		(should print out version information)<br>


3. Install homebrew

  ```
  ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
  sudo chown -R $(whoami) /usr/local/lib
  brew doctor
  ```

4. Install OSXFuse

  Download the dmg image from http://sourceforge.net/projects/osxfuse/files/ and install


5. Install encfs

  ```
  brew update
  ```

  On El Capitan, you may have to overrule some permissions:

  ```
  sudo chown -R $USER:admin /usr/local/include
  sudo chown -R $USER:admin /usr/local/lib/pkgconfig
  brew link xz libtool boost rlog
  ```
  (install xz libtool etc first if needed)

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

### Install dependencies

1. Install the depencencies from the procedure listed above first

2. Clone the latest version of the wxWidgets repository from github to a folder on your local harddrive:

  ```
  git clone https://github.com/wxWidgets/wxWidgets.git wxWidgets-latest
  ```

3. Build the wxWidget library binaries for static linking  

  (Run the following commands from within the wxWidgets-latest folder)

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

4. Install libraries to build encfs

  ```
  brew install cmake
  brew install openssl
  brew install gettext
  ```


5. Clone the latest version of the encfs repository from github onto your harddrive (outside of the wxWidgets folder):
  ```
  git clone https://github.com/vgough/encfs.git
  ```

6. Build encfs

  (From within the new encfs folder)
  ```
  mkdir build
  cd build
  cmake .. -DOPENSSL_INCLUDE_DIR=/usr/local/opt/openssl/include -DOPENSSL_ROOT_DIR=/usr/local/opt/openssl
  make
  ```
  (Note: you may have to update the OPENSSL_INCLUDE_DIR and OPENSSL_ROOT_DIR directives, depending on where the files are on your system)

  If you prefer to use this very latest version of encfs instead of the one created via brew, simply run after completing the steps above
  ```
  make install
  ```


7. Copy encfs' cpp and h files into the EncFSGUI project

  Copy the 'encfs' folder from your local encfs clone into EncFSGUI/src folder (make sure to keep the encfs subfolder name)<br>
  Next, copy the config.h file from within the encfs 'build' folder into this newly created EncFSGUI/src/encfs subfolder<br>
  (This file will be created after running the cmake command in step 6 above)<br>


### Before compiling EncFSGUI

4. Edit Makefile

  - update the `WX_BUILD_DIR` variable so it would contain the absolute path to the build-release-static folder on your own machine.
  - check the paths for `OSXFUSE_INCLUDE_DIR`, `OPENSSL_INCLUDE_DIR` and `OPENSSL_ROOT_DIR`, make sure they contain the correct absolute path on your machine

### Compiling & linking EncFSGUI

1. run `make clean` before compiling/linking

2. run `make` to compile and link


## Contribute

If you plan on contributing, please create a new branch first.  Do NOT submit pull requests against the master branch.



