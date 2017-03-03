libtaigi

# libtaigi - The intelligent taigi input method library

## History
Start from libchewing, but goes too far

## Motivation
TODO

## Development Goal of libtaigi
TODO

## Status
Initialization state

### 1. System bridge integration

+ The active integration:
  - fcitx


### 2. support phonetic keyboard layout
  - only standard keyboard

### 3. External and unmerged projects
  - None


## Minimal Build Tools Requirement

The following tools are used to build libchewing. Not all tools are necessary
during building. For example, if the compiler you used is clang, gcc & Visual
Studio are not needed. The versions listed here is the minimal version known to
build libchewing. If any tools you use below this version, libchewing might not
be built.

+ Build tools:
   - autoconf >= 2.65
   - automake >= 1.11.6
   - libtool >= 2.4.2
   - cmake >= 2.8.8 (optional)
+ Toolchain / IDE:
   - clang >= 3.2
   - gcc >= 4.6.3
   - Visual Studio Express 2012
+ Documentation tools:
   - texinfo >= 4.12


## Prerequisite

Under ubuntu (verified) or debian based system:
	# sudo apt-get install dh-autoreconf libncursesw5-dev libsqlite3-dev libtool texinfo

## Build

	# cd libtaigi
	# ./autogen
	# ./configure --build=x86_64-linux-gnu --prefix=/usr --includedir=\${prefix}/include \
		--mandir=\${prefix}/share/man --infodir=\${prefix}/share/info --sysconfdir=/etc \
		--localstatedir=/var --disable-silent-rules --libdir=\${prefix}/lib/x86_64-linux-gnu \
		 --libexecdir=\${prefix}/lib/x86_64-linux-gnu --disable-maintainer-mode \
		--disable-dependency-tracking --includedir=/usr/include/x86_64-linux-gnu
	# make

## Installation
	# sudo make install


### Glossary
TODO

### Editing mode
Should be similar as libchiewing, but not verified


### Half/Full Shape sub-mode
Should be similar as libchiewing, but not verified


### Bypass mode
Should be similar as libchiewing, but not verified

## License

Except the following source code:

* thirdparty/sqlite-amalgamation/ contains sqlite3 source which is in public
  domain. See <http://www.sqlite.org/copyright.html> for more information.

* cmake/FindCurses.cmake is modified from CMake source, which is licensed under
   BSD 3-Clause.

All source code are licensed under GNU LGPL v2.1 (Lesser General Public License
v2.1). See "COPYING" for details.


## Authors & Contact Information

See "AUTHORS" for details.
