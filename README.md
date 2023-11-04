# remote-bt
a bittorrent client routed over ssh. this is still a wip, i don't know if it actually works or not yet.
## building
just run `make`!

on windows, i installed `make` with winget. i've aliased `cc` to run `clang`, which you can install optionally as part of the "desktop development with c/c++" package when installing visual studio.
### dependencies
this project dynamically links with libssh. hopefully this can be installed with your package manager on unix/linux. for building on windows, i used [vcpkg](https://github.com/microsoft/vcpkg) to compile `libssh[openssl]`. then, you can add the necessary vcpkg directories to your system environment variables so `clang` can find the libraries.
### configuring ssh
i did not want to code any complicated file parsing to configure how the program initializes the ssh connection. instead, this program requires an `ssh_config.c` file to exist or it fails to build. it's up to you to create the `ssh_config.c` file with your own settings and rebuild for the changes to take effect. use the `ssh_config.example.c` file as a template.
