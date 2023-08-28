# remote-bt
a bittorrent client routed over ssh. this is still a wip, i don't know if it actually works or not yet.
## building
i don't like complicated build systems. my code will build with just a c compiler. if running on a unix/linux-based system, you can run the `build.sh` script. if on windows, run the `build.bat` script. the executables are placed in the `build` directory (will be created if it doesn't already exist).
### dependencies
this project depends on libssh (which depends on openssl). the build scripts also try to build the dependencies from source, so they are included as submodules. when cloning this repo, be sure to use the `--recurse-submodules` flag.
### tools
unfortunately there are a number of tools required for building the dependencies. i will list the tools i ended up needing, but there is always the chance that my environment had some things that i didn't realize were necessary. so if the build scripts have problems building the dependencies, you can check the respective repos for build instructions.

ill list the dependencies i needed, but you can probably just run the build script and see what works or not.
#### unix/linux-based
- gcc: my build script will probably work with clang or something too, but im not sure about the dependencies
- make: this runs the build/install scripts for openssl and libssh
- cmake: this is needed for configuring the libssh build
#### windows
- visual studio: there isn't a visual studio solution or anything, but visual studio gives you the build environment scripts and some compilation tools (cl, nmake). the build script tries to run one of the visual studio environment scripts by checking for an environment variable `VS_PATH`, so be sure to define that somewhere in your windows environment settings (should be something like `C:\Program Files\Microsoft Visual Studio\2022\Community`)
- perl: this runs the configuration script for openssl. when i wrote this, openssl recommended installing [Strawberry Perl](http://strawberryperl.com). install it and add it to your `PATH`
- nasm: used by the openssl build script. [download](https://www.nasm.us) it and add it to your `PATH`
