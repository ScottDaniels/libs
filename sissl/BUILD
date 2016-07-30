
Building this pacakge (libsissl.a) should be as simple as running mk in the current
directory. Once the library is built, 'sudo mk publish' can be executed to push a
copy of the library and header files into the $LIB_DIR an $INC_DIR directories. These
default to /usr/local/lib/sdaniels and /usr/local/include/sdaniels; override them by 
exporting the variables mentioned, or modifying the ../master.mk file to suit your needs.

It will be necessary to build and publish the headers and library in the ../si directory
as this depends on the socket_if.h header file which is installed in $LIB_DIR.

The mk utility is the Plan-9 mk application written by Andrew Hume and can be 
downloaded from several different places.  It migh also be available via apt-get, zypper,
or yum installation tools.
