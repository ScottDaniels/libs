#  Master file included by all mkfiles in the libs tree

SRC_DIR = `echo ${SRC_ROOT:-..}`
MKSHELL = /bin/ksh
CC = gcc
IFLAGS = -I. -I$SRC_DIR/include
CFLAGS = -g
LFLAGS = 
TARGET_BIN=`echo ${TARGET_BIN:-$HOME/bin}`

NUKE = *.o *.a  *.eps *.png

backup_dir = $HOME/backup

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

nuke:
	rm -f $NUKE


# anything on the install list, of the form [path/]source[:[path/]target]
install:QV:
	for x in $INSTALL
	do
		source=${x%%:*}
		if [[ $x == *:* ]]
		then
			target=${x##*:}
		else
			target=$TARGET_BIN/${source##*/}
		fi
	
		if [[ -d $target ]]
		then
			target=${target:-$TARGET_BIN}/${source##*/}
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

# ---------------- making of backup stuff ------------------------
tar_contents=`ls *.c *.h *.ksh *.java mkfile $ADD2BACKUP 2>/dev/null`
tarf=`echo ${PWD##*/}.tgz`
$tarf::	$tar_contents
	tar -cf - ${tar_contents:-foo} | gzip -c >$tarf

tar:V:	$tarf
backup:QV:	$tarf
	ls -al $tarf
	mv $tarf $backup_dir
