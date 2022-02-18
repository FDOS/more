#!/bin/sh

if [ x"${COMPILER}" = "xgcc" ] ; then
  # Note requires installation of libi86-ia16-elf DOS compat library
  export CC="ia16-elf-gcc"
  export CFLAGS="-Wall -mcmodel=small -o "
  export LDFLAGS="-li86 -Wl,-Map=more.map"

elif [ x"${COMPILER}" = "xwatcom" ] ; then
  if [ -z "${WATCOM}" ] ; then
    # Make sure this is set correctly for your system before running
    # this script, or we default to this
    export WATCOM="/opt/watcom"
  fi
  export PATH=${PATH}:${WATCOM}/binl64
  export INCLUDE=${WATCOM}/h
  export CC="wcl"
  export CFLAGS="-q -bt=DOS -bcl=DOS -D__MSDOS__ -ms -lr -fe="
  export LDFLAGS=""

elif [ x"${COMPILER}" = "xwatcom-emu" ] ; then
  dosemu -q -td -K . -E "build.bat watcom"
  exit $?

elif [ x"${COMPILER}" = "xtcc-emu" ] ; then
  dosemu -q -td -K . -E "build.bat tcc"
  exit $?

else
  echo "Please set the COMPILER env var to one of"
  echo "Cross compile           : 'watcom' or 'gcc'"
  echo "Native compile (Dosemu) : 'watcom-emu' or 'tcc-emu'"
  exit 1
fi

export EXTRA_OBJS=

export EXTRA_OBJS="${EXTRA_OBJS} tnyprntf.obj"
# if you want to build without tnyprntf comment the above and uncomment
# the following
# export CFLAGS="-DNOPRNTF ${CFLAGS}"

export EXTRA_OBJS="${EXTRA_OBJS} kitten.obj"
# if you want to build without kitten comment the above and uncomment
# the following
# export CFLAGS="-DNOCATS ${CFLAGS}"

export UPXARGS="upx --8086 --best"
# if you don't want to use UPX set
#     UPXARGS=true
# if you use UPX: then options are
#     --8086 for 8086 compatibility
#   or
#     --best for smallest

make -C src
