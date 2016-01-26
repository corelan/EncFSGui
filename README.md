# EncFSGui

Public repository for the EncFSGui project, an OSX GUI wrapper around encfs

## Warning 
This application is still missing some (minor) features, but it should now be ready for use.<br>
It is still under active development and will be updated on a regular basis.<br>


If you don't feel like compiling stuff yourself today, feel free to grab the most recent compiled version of the app from here: https://github.com/corelan/EncFSGui/blob/master/release/EncFSGUI.dmg<br>
Open the dmg file, and drag encfsgui.app into /Applications.<br>
Note: To run the app, check the "Installing dependencies on OSX" section below.  You must install the dependencies in order to use the app !!<br>


## Donate
If you like this initiative and want to show your appreciation, please consider donating using [this link](https://www.paypal.com/cgi-bin/webscr?cmd=_donations&bn=EncFSGui&business=peter.ve@corelan.be&currency_code=EUR&item_name=Support%20EncFSGui%20Project)<br>

(I could have decided to distribute this as a commercial closed-sourced project and sell it via the App Store... but I didn't, despite the fact that 'there is no free lunch')<br>


## What is EncFSGui ?
EncFSGui is a small tool that allows you to manage the encfs folders on your system, and to mount/dismount folders when needed. It is fully compatible with recent encfs versions.<br>
You can use EncFSGui to mount encfs folders created by BoxCryptor Classic or other similar tools that rely on encfs too.<br> 
EncFSGui was written for OSX, but it might compile/run on Linux as well (some changes may be needed though).<br>
If you are looking for an EncFS tool for Windows, check out the EncFSMP project on sourceforge.<br>
I am fully aware that EncFSMP runs on OSX too, but unfortunately the performance of the embedded file system engine was not so good on OSX.<br>


## How does it work ?
EncFSGui is a wrapper around encfs, encfsctl and mount, and relies on OSXFuse to provide a filesystem.<br>
In other words, it relies entirely on those utilities, the ability to interact with those tools and to capture the output from those tools.<br>
As a result, the EncFSGui source code is pretty easy to understand, as it does not contain any crypto or other black magic to do its job.<br>
The downside is that it is a wrapper and may break if tools start behaving in a different way.<br>

## Background
This application is written in C++, and uses the wxWidgets Cross-Platform Library.<br>  
Although the source probably compiles fine under Linux/Unix and Windows, it was written for OSX and contains hardcoded strings & paths that will certainly prevent the app from working on Windows.  It might actually work on Linux (but I haven't tested it myself)<br>
Also, this is my very first project in C++.  As I started learning C++ just a few weeks ago (self-study), I am fully aware that my C++ stinks. I am quite keen on learning & improving, and I am open for constructive advise and help.<br>

With that said, all positive contributions are more than welcome. If you want to contribute, check out the development setup section first.<br>

If you want to try the application, you'll need a number of dependencies installed on your OSX machine:
- encfs
- OSXFuse


## Running EncFSGui: Installing dependencies on OSX

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


### Before compiling EncFSGui

1. Edit Makefile

  - update the `WX_BUILD_DIR` variable so it would contain the absolute path to the build-release-static folder on your own machine.


### Compiling & linking EncFSGui

1. run `make clean` before compiling/linking

2. run `make` to compile and link


## Contribute

If you would like to contribute, please create a new branch first.  Do NOT submit pull requests against the master branch.<br>
All initiatives to improve the code / app are more than welcome.  Alternatively, please check the TO DO list below.


## TO DO

  - [X] Add code to edit a (non-mounted) volume
    - [X] Rename volume
    - [X] Select a different mount point
    - [X] Unsave password from Keychain (if saved)
    - [X] Save password to Keychain (if not saved already)
    - [ ] Change password of encfs volume (routine already exists) (& update Keychain if needed)
  - [X] Implement 'Start application at login'
  - [ ] Allow use of master password
  - [ ] Check how to launch as an icon and add as an option
    - [ ] Add context menu to icon, allow mount/unmount volumes
  - [ ] Implement overall error handling
  - [ ] Code cleanup & documentation
    - [ ] Create proper destructors where needed
    - [ ] Add proper documentation for functions & classes
    - [ ] Review variable names
    - [ ] Adopt a better coding style
    - [ ] Learn how to write proper C++
  - [ ] Prevent Command+C (Copy) to trigger 'Cancel' button in dialogs
  - [ ] Allow use of passwords that contain quotes (single and double)
  - [ ] Move installation instructions to wiki
  - [ ] Add pictures
  - [ ] Add license information everywhere, as needed
  - [ ] Updates
    - [ ] Code to check for updates
    - [ ] Option to "auto check for updates"
  - [ ] Check if it would be possible to statically compile the app with encfs, osxfuse and openssl
