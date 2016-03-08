# EncFSGui

Public repository for the EncFSGui project, an OSX GUI wrapper around encfs

## Warning 
This application is still missing some (minor) features, but it should now be ready for use.<br>
It is still under active development and will be updated on a regular basis.<br>


If you don't feel like compiling stuff yourself today, feel free to grab the most recent compiled version of the app from here: https://github.com/corelan/EncFSGui/raw/master/release/EncFSGui.dmg<br>
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

You can find more information about EncFSGui at https://www.corelan.be/index.php/2016/01/31/encfsgui-gui-wrapper-around-encfs-for-osx/ 


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

4. Install libcurl & build for static linking

  - Go to http://curl.haxx.se/download.html and download the latest 'source archive'. (Make sure to get the tar.gz version).  At the time of writing, the latest version is 7.47.0 (https://github.com/bagder/curl/releases/download/curl-7_47_0/curl-7.47.0.tar.gz)
  - Extract into a folder (e.g. /Users/corelanc0d3r/dev/curl-7.47.0)
  - From within the curl-7.47.0 folder, build static libraries, using OSX SSL modules
  ```
  export MACOSX_DEPLOYMENT_TARGET="10.6"
  ./configure --with-darwinssl --disable-shared
  make
  make test
  sudo make install
  ```

  The `make install` command will put the header files into
  ```
  /usr/local/include/curl
  ```
  The libraries will be stored as
  ```
  /usr/local/lib/libcurl.a
  /usr/local/lib/libcurl.dylib
  ```

  Run `curl --version` to confirm that everything works correctly.


### Before compiling EncFSGui: update paths

1. Edit Makefile

  - update the `WX_BUILD_DIR` variable, so it would contain the absolute path to the `build-release-static` folder on your own machine.


### Compiling & linking EncFSGui

1. run `make clean` before compiling/linking

2. run `make` to compile and link


### After upgrading from Yosemite to El Capitan

If you have upgraded your development machine from Yosemite to El Capitan, you may need to run the fix some permissions:

```
sudo chown -R $(whoami) /usr/local/lib
sudo chown -R $USER:admin /usr/local/include
sudo chown -R $USER:admin /usr/local/lib/pkgconfig
```

It's also a good idea to update brew at the same time
```
brew update

```


## Contribute

If you would like to contribute, please create a new branch first.  Do NOT submit pull requests against the master branch.<br>
All initiatives to improve the code / app are more than welcome.  Alternatively, please check the TO DO list below.


## TO DO

  - [ ] Change password of encfs volume (routine already exists) (& update Keychain if needed)
  - [ ] Allow use of master password
  - [ ] Implement overall error handling
  - [ ] Code cleanup & documentation
    - [ ] Create proper destructors where needed
    - [ ] Add proper documentation for functions & classes
    - [ ] Review variable & function naming conventions
    - [ ] Adopt a better coding style (a.k.a rewrite the entire thing damnit)
    - [ ] Learn how to write proper C++ (hah - this will never happen!)
  - [ ] Prevent Command+C (Copy) to trigger 'Cancel' button in dialogs (known issue, so it seems: http://trac.wxwidgets.org/ticket/14954 // http://trac.wxwidgets.org/ticket/15678)
  - [ ] Allow use of passwords that contain quotes (single and double)
  - [ ] Move installation instructions to wiki
  - [ ] Add license information everywhere, as needed
  - [ ] Check if it would be possible to statically compile the app with encfs, osxfuse and openssl, without giving up on the ability to close the app (without unmounting volumes)

### DONE

  - [X] Updates
    - [X] Code to check for updates
    - [X] Option to "auto check for updates"
    - [X] improve mechanism to compare version info (only report on higher version numbers, ignore lower numbers)
  - [X] Add code to edit a (non-mounted) volume
    - [X] Rename volume
    - [X] Select a different mount point
    - [X] Unsave password from Keychain (if saved)
    - [X] Save password to Keychain (if not saved already)
  - [X] Check how to launch as an icon and add as an option
    - [X] Add context menu to icon, allow mount/unmount volumes
  - [X] Implement 'Start application at login'
  - [X] Remove temp files when they are no longer needed
  - [X] Add right-click context menu to List Control
