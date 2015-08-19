# Installing `libgptk` #

## Installing dependencies ##

Prior to compiling `libgptk`, we need to install several dependencies required by `libgptk`. These include IT++, GSL and Gnuplot. The instructions below should work on Ubuntu systems.

### IT++ ###
In Ubuntu 8.10, IT++ can be installed directly from the repository, using the following command (in a terminal):
```
$ sudo apt-get install libitpp6gf libitpp-dev libitpp6-dbg
```

### GNU scientific library (GSL) ###
Also available from the repository:
```
$ sudo apt-get install libgsl0ldbl libgsl0-dbg libgsl0-dev
```

### GNUplot ###
GNU plot should be installed for the plotting features to be available. This can be done from the repository:
```
$ sudo apt-get install gnuplot
```


### Xerces XML parser ###
`libptk` requires the Xerces XML Parser C++ library version 3.0.1. The Ubuntu repository only provides version 2.8.0, so we'll need to download and install the library.

For 32-bit linux systems, this can be done by running the `install-required-libraries.sh` script:
```
$ chmod +x install-required-libraries.sh
$ ./install-required-libraries.sh
```
This will install Xerces C in the `lib` folder.

Alternatively, you can install the library manually.

  1. Navigate to the `lib` directory
```
$ cd gptk/lib
```
  1. Download the library binaries for Linux from the Xerces website:
```
$ wget http://mirror.fubra.com/ftp.apache.org/xerces/c/3/binaries/xerces-c-3.0.1-x86-linux-gcc-3.4.tar.gz
```
  1. Extract the library and create a symbolic link to it.
```
$ tar -xzvf xerces-c-3.0.1-x86-linux-gcc-3.4.tar.gz
$ ln -s xerces-c-3.0.1-x86-linux-gcc-3.4 xercesc
```

On 64-bit systems, an alternative binary can be download from the [Xerces website](http://xerces.apache.org/xerces-c/download.cgi). Save the appropriate file in the `gptk/lib` folder and resume at step 3 (replacing "`xerces-c-3.0.1-x86-linux-gcc-3.4`" with the name of the downloaded binary).

## Building libgptk ##
  1. Download the latest source: <br />
```
$ svn checkout https://gptk.googlecode.com/svn/trunk/ gptk --username <your_username>
```
  1. Run the `bootstrap` script to generate the files needed by automake:
```
$ cd gptk
$ ./bootstrap
```
  1. Run the configure script, using the `--prefix` option to specify the directory where you want the libraries and test programs to be installed:
```
$ ./configure --prefix=$HOME/gptk/
```
> You can safely ignore the warnings reporting missing configuration information in some folders.
  1. Compile and install the library, tests and examples:
```
$ make && make install
```
> There will be some complaints about redefining several variables. This is because both GPTK and IT++ try to define their own package version, name, etc. This is not critical and can be ignored for now.

If everything went fine, the installation should have created a `bin`, a `lib` and an `include` directories in your prefix directory. The `bin` directory contains the examples and tests, while the `lib` directory contains the actual libraries. C++ headers can be found in the `include` directory.