# remote-bt
a bittorrent client routed over ssh. this is still a wip, i don't know if it actually works or not yet.
## building
i don't like complicated build systems. my code will build with just a c compiler. if running on a unix/linux-based system, you can run the `build.sh` script. if on windows, run the `build.bat` script. the executables are placed in the `build` directory (will be created if it doesn't already exist).
### dependencies
this project dynamically links with libssh. hopefully this can be installed with your package manager on unix/linux. for building on windows, i used [vcpkg](https://github.com/microsoft/vcpkg) to compile `libssh[openssl]:x64-windows` and the build script expects to have an environment variable `VCPKG_X64_PATH` available to find the required headers and library files.
### configuring ssh
i did not want to code any complicated file parsing to configure how the program initializes the ssh connection. instead, this program requires an `ssh_config.c` file to exist or it fails to build. it's up to you to create the `ssh_config.c` file with your own settings and rebuild for the changes to take effect. use the `ssh_config.example.c` file as a template.
