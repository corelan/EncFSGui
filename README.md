# EncFSGui

Public repository for the encfsgui project, an OSX GUI wrapper around encfs

## Warning 
This application is still missing some features, but it should be quite usable already.<br>
It is still under heavy development and will be updated on a regular basis.<br>
Proper announcements will be made when the app is ready for use.<br>

If you feel brave enough, feel free to grab a compiled version of the app from here: https://github.com/corelan/EncFSGui/raw/master/release/EncFSGUI.dmg<br>
Note: To run the app, check the "Installing dependencies on OSX" section below<br>


## Donate
If you like this initiative and want to show your appreciation, please consider donating using [this link](https://www.paypal.com/cgi-bin/webscr?cmd=_donations&bn=EncFSGUI&business=peter.ve@corelan.be&currency_code=EUR&item_name=Support%20EncFSGui%20Project)<br>

(I could have decided to distribute this as a commercial closed-sourced project and sell it via the App Store... but I didn't, despite the fact that 'there is no free lunch')<br>


## Background
EncFSGUI is a wrapper around encfs, encfsctl and mount, and relies on OSXFuse to provide a filesystem.<br>
In other words, it relies entirely on those utilities, the ability to interact with those tools and to capture the output from those tools.<br>
As a result, the EncFSGUI source code is pretty easy to understand, as it does not contain any crypto or other black magic to do its job.<br>
The downside is that it is a wrapper and may break if tools start behaving in a different way.<br>

This application is written in C++, and uses the wxWidgets Cross-Platform Library.  Although the source probably compiles fine under Linux/Unix and Windows, it was written for OSX and contains hardcoded strings & paths that will certainly prevent the app from working on Windows.  It might actually work on Linux (but I haven't tested it myself)<br>
If you are looking for an EncFS Gui for Windows, check out the EncFSMP project on sourceforge.<br>
Also, this is my very first project in C++.  As I started learning C++ just a few weeks ago (self-study), I am fully aware that my C++ stinks. I am quite keen on learning & improving, and I am open for constructive advise and help.<br>

With that said, all positive contributions are more than welcome. If you want to contribute, check out the development setup section first.<br>

If you want to try the application, you'll need a number of dependencies installed on your OSX machine:
- encfs
- OSXFuse


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


### Before compiling EncFSGUI

1. Edit Makefile

  - update the `WX_BUILD_DIR` variable so it would contain the absolute path to the build-release-static folder on your own machine.


### Compiling & linking EncFSGUI

1. run `make clean` before compiling/linking

2. run `make` to compile and link


## Contribute

If you plan on contributing, please create a new branch first.  Do NOT submit pull requests against the master branch.


## TO DO

  - [ ] Add code to edit a (non-mounted) volume
    - [ ] Rename volume
    - [ ] Select a different mount point
    - [ ] Unsave password from Keychain (if saved)
    - [ ] Save password to Keychain (if not saved already)
    - [ ] Change password (routine already exists) (& update Keychain if needed)
  - [ ] Implement 'Start application at login'
  - [ ] Check how to launch as an icon and add as an option
    - [ ] Add context menu to icon, allow mount/unmount volumes
  - [ ] Implement overall error handling
  - [ ] Code cleanup & documentation
  - [ ] Prevent Command+C (Copy) to trigger 'Cancel' button in dialogs
  - [ ] Allow use of passwords that contain quotes (single and double)
  - [ ] Move installation instructions to wiki
  - [ ] Add pictures
  - [ ] Add license information everywhere, as needed
