#  Master file included by all mkfiles in the libs tree

INC_DIR = `echo ${INC_DIR:-/usr/local/include/sdaniels}`
LIB_DIR = `echo ${LIB_DIR:-/usr/local/lib/sdaniels}`

#SRC_DIR = `echo ${SRC_ROOT:-..}`
MKSHELL = /bin/ksh
CC = `echo ${CC:-gcc}`

IFLAGS = `echo -I. -I$INC_DIR $IFLAGS`
CFLAGS = ${CFLAGS:--g}
LFLAGS = -L $LIB_DIR
BIN_DIR=`echo ${BIN_DIR:-$HOME/bin}`

NUKE = *.o *.a  *.eps *.png

# ===========================================================================

# the & meta character does not match / and this IS important!
&:      &.o
	$CC $IFLAGS $CFLAGS -o $target $prereq $LFLAGS

&.o:    &.c
	$CC $IFLAGS $CFLAGS -c $stem.c

&.html: &.xfm
	XFM_IMBED=$HOME/lib hfm ${prereq%% *} $target

&:	&.go
	echo "use go build, not mk, to build go source"

%.eps: %.fig
	fig2dev -L eps ${prereq} ${target}

%.png: %.fig
	fig2dev -L png ${prereq} ${target}

%_slides :  %_slides.xfm
	pfm -g 8ix11i ${prereq%% *} $target.ps

%.ps :  %.xfm
	pfm ${prereq%% *} $target

%.html :  %.xfm
	hfm ${prereq%% *} $target

%.pdf : %.ps
	gs -I/usr/local/share/ghostscript/fonts -dBATCH -dNOPAUSE -sDEVICE=pdfwrite -sOutputFile=$target ${prereq% *}

%.txt : %.xfm
	tfm ${prereq%% *} $target


# ===========================================================================

all:V:	$ALL

help:VQ:
	cat <<endKat
	Export any of the following variables to override mkfile settings: 
		LIB_DIR -- directory path where libraries are published    (/usr/local/lib/sdaniels/)
		INC_DIR -- directory path where header files are published (/usr/include/lib/sdaniels/)
		BIN_DIR -- directory where binaries are installed			($HOME/bin)
		CC      -- C compiler to use (gcc)
		CFLAGS  -- flags to pass on CC command line (-g)
		IFLAGS  -- any include flags to be added after "-I. -I $INC_DIR"
		
		
	Execute:
	    "mk publish" to publish libraries and header files in $LIB_DIR and $INC_DIR
	    "mk install" to build and install binaries in $BIN_DIR
	    "mk clean"   to perform a non-exhaustive reset in the current directory
	    "mk nuke"    to performa an exhaustive reset in the current directory
		
	See the mkfile in this direcory for specific targets
	endKat

clean:VQ:
	if [[ -n $CLEAN ]]
	then
		rm -f $CLEAN
	fi

nuke:VQ:
	rm -f $NUKE

# anything on the install list, of the form [path/]source[:[path/]target]
install:QV: publish
	for x in $INSTALL
	do
		source=${x%%:*}
		if [[ $x == *:* ]]
		then
			target=${x##*:}
		else
			target=$BIN_DIR/${source##*/}
		fi
	
		if [[ -d $target ]]
		then
			target=${target:-$BIN_DIR}/${source##*/}
		fi
	
		if [[ -f $target ]]
		then
			mv -f  $target $target-
			rm -f  $target-
		fi
	
		cp $source $target
		chmod 775 $target
		ls -al $target
	done
