#!/bin/sh
# Run this to generate all the initial makefiles, etc.

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

REQUIRED_AUTOMAKE_VERSION=1.8

PKG_NAME="ekiga"

(test -f $srcdir/configure.ac \
  && test -f $srcdir/autogen.sh \
  && test -d $srcdir) || {
    echo -n "**Error**: Directory "\`$srcdir\'" does not look like the"
    echo " top-level $PKG_NAME directory"
    exit 1
}

DIE=0


# This is a bit complicated here since we can't use gnome-config yet.
# It'll be easier after switching to pkg-config since we can then
# use pkg-config to find the gnome-autogen.sh script.

gnome_autogen=
gnome_datadir=

ifs_save="$IFS"; IFS=":"
for dir in $PATH ; do
  test -z "$dir" && dir=.
  if test -f $dir/gnome-autogen.sh ; then
    gnome_autogen="$dir/gnome-autogen.sh"
    gnome_datadir=`echo $dir | sed -e 's,/bin$,/share,'`
    break
  fi
done
IFS="$ifs_save"

if test -z "$gnome_autogen" ; then
  echo "You need to install the gnome-common module and make"
  echo "sure the gnome-autogen.sh script is in your \$PATH."
  exit 1
fi

ACLOCAL_FLAGS="$ACLOCAL_FLAGS -I /usr/share/aclocal" GNOME_DATADIR="$gnome_datadir" USE_GNOME2_MACROS=1 . $gnome_autogen
