# Folder Structure #

The GP Toolkit file/folder structure is as follows:

```
gptk/
 libgptk/       The GPTK source/library folder
 examples/      Examples folder
 lib/           Other required libraries (xerces-c)
 tests/         Tests folder
 Makefile       Main Makefile (to build lib, examples and tests)
 install-required-libraries Script to automate installation of required
                libraries
```

Note that this requires the Xerces C library, which is expected to be found in `lib/xercesc/lib`. Corresponding header files are looked for in `lib/xercesc/include`.