## NOTE: The current version seems to be broken! Use commit b33e41b7db92a258bc01b23b227ae3dcac0c77d1

After downloading the git repository, do `git checkout b33e41b7db92a258bc01b23b227ae3dcac0c77d1`. This is the version before I merged some updates from the original library, which somehow broke the sending of the body of web pages.

# QtWebApp [![Build Status](https://img.shields.io/travis/msrd0/QtWebApp/master.svg)](https://travis-ci.org/msrd0/QtWebApp) [![License](https://img.shields.io/badge/license-GPL3-blue.svg)](https://github.com/msrd0/QtWebApp/blob/master/LICENSE)

QtWebApp is a HTTP server library in C++, inspired by Java Servlets. This is a fork of
the original version by Stefan Frings that can be found at
[stefanfrings.de](http://stefanfrings.de/qtwebapp/index-en.html).

Note that this version is only tested with Qt5, Qt4 might not be supported anymore. Also,
I have no capability to test this on Windows, although Qt should ensure that the code is
platform-independent.

There is an unstable version available on the [http2](https://github.com/msrd0/QtWebApp/tree/http2)
branch that aims to add HTTP/2.0 support to the library, the master version only supports
HTTP/1.1.

## License

This code is released under the GPL version 3 (the code by Stefan Frings was developed
under the LGPL version 2). The license can be found [here](https://github.com/msrd0/QtWebApp/blob/master/LICENSE).

## Usage

To use this library, you either need to link your application against it or compile
the source files into your application. If you don't mind huge binary files, go with
the second option. Therefore, just include the .pri files you need.

### Link against the Library

To link against the library, I recommend to use the subdirectory-function of qmake. If
your operating system comes with this library (I don't think anyone does) you can of
course also use that version. To use subdirs, first clone the library:
```
# if you use git, add it as a submodule
git submodule init && git submodule add https://github.com/msrd0/QtWebApp.git
# if you don't use git, just clone it (which once requires git)
git clone https://github.com/msrd0/QtWebApp.git
```
Now, create your parent .pro file, containing something like this:
```
TEMPLATE = subdirs

SUBDIRS += \
    QtWebApp \
    YourProject

YourProject.depends = QtWebApp
```
Now, create the folder YourProject and add an .pro file as normal and add the library
and the include path to it:
```
LIBS += -L ../QtWebApp -lQtWebApp
INCLUDEPATH += ../QtWebApp/httpserver ../QtWebApp/logging ../QtWebApp/qtservice ../QtWebApp/templateengine
```
