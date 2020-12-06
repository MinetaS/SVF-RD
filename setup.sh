#!/bin/bash

export SVF_HOME=$(pwd)

# export LLVM_DIR
if [ -z "$LLVM_DIR" ] ; then
   export LLVM_DIR=$SVF_HOME/llvm-10.0.0.obj
fi
echo "[+] LLVM_DIR = $LLVM_DIR/"

for arg in "$@" ; do
  # export Z3_DIR
  if [[ "$arg" == "z3" ]] ; then
    if [ -z "$Z3_DIR" ] ; then
      export Z3_DIR=$SVF_HOME/z3.obj
    fi
    export SVF_Z3=1
    echo "[+] Z3_DIR = $Z3_DIR/"

  # export CTIR_DIR
  elif [[ "$arg" == "ctir" ]] ; then
    if [ -z "$CTIR_DIR" ] ; then
      export CTIR_DIR=$SVF_HOME/ctir.obj
    fi
    export SVF_CTIR=1
    echo "[+] CTIR_DIR = $CTIR_DIR/"

  # build config
  elif [[ "$arg" == "debug" ]] ; then
    export SVF_DEBUG=1

  else
    echo "[-] Unknown option: $arg"
  fi
done

# export BUILD_DIR
if [[ -z "$SVF_DEBUG" ]] ; then
  BUILD_DIR="$SVF_HOME/Release-build"
else
  BUILD_DIR="$SVF_HOME/Debug-build"
fi
if [ ! -d "$BUILD_DIR" ] ; then
  mkdir "$BUILD_DIR"
fi
echo "[+] BUILD_DIR = $BUILD_DIR/"

# Setup PATH
if [ -z "$PATH_OLD" ] ; then
  export PATH_OLD=$PATH
fi
export PATH=$BUILD_DIR/bin:$LLVM_DIR/bin:$PATH_OLD
echo "[+] PATH = $PATH"