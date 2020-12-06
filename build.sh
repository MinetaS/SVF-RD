#!/bin/bash

# SVF build script
#
# Release build : ./build.sh
# Debug build   : ./build.sh debug
#
# Set the SVF_CTIR environment variable to build and run FSTBHC tests.
#   e.g. "./build.sh SVF_CTIR=1"
#
# If the LLVM_DIR variable is not set, LLVM will be downloaded.
# If the Z3_DIR variable is not set, Z3 will be downloaded (only if variable SVF_Z3 is set).
# If the CTIR_DIR variable is not set, ctir Clang will be downloaded (only if SVF_CTIR is set).

source ./setup.sh $@

# Setup variables
SVF_TESTS="Test-Suite"

if [[ $OSTYPE == "linux-gnu" ]] ; then
  LLVM_URL="https://github.com/llvm/llvm-project/releases/download/llvmorg-10.0.0/clang+llvm-10.0.0-x86_64-linux-gnu-ubuntu-18.04.tar.xz"
  Z3_URL="https://github.com/Z3Prover/z3/releases/download/z3-4.8.9/z3-4.8.9-x64-ubuntu-16.04.zip"
  CTIR_URL="https://github.com/mbarbar/ctir/releases/download/ctir-10.c3/ctir-clang-v10.c3-ubuntu18.04.zip"
elif [[ $OSTYPE == "darwin*" ]] ; then
  LLVM_URL="https://github.com/llvm/llvm-project/releases/download/llvmorg-10.0.0/clang+llvm-10.0.0-x86_64-apple-darwin.tar.xz"
  Z3_URL="https://github.com/Z3Prover/z3/releases/download/z3-4.8.9/z3-4.8.9-x64-osx-10.14.6.zip"
  CTIR_URL="https://github.com/mbarbar/ctir/releases/download/ctir-10.c3/ctir-clang-v10.c3-macos10.15.zip"
else
    echo "[-] Unsupported operating system. ($OSTYPE)"
fi

# Function for file download
function generic_download_file {
  if [ $# -ne 2 ] ; then
    echo "[-] $0: invalid argument set (requires 2)"
    exit 1
  fi

  if [[ "$OSTYPE" == "linux-gnu" ]] ; then
    curl -L "$1" -o "$2"
  elif [[ "$OSTYPE" == "darwin*" ]] ; then
    wget -c "$1" -O "$2"
  else
    exit 1
  fi
}

# Download LLVM
if [ ! -d "$LLVM_DIR" ] ; then
  echo "[+] Downloading LLVM ..."
  generic_download_file "$LLVM_URL" llvm.tar.xz
  mkdir -p "$LLVM_DIR" && tar -xf llvm.tar.xz -C "$LLVM_DIR" --strip-components 1
  rm llvm.tar.xz
fi

# Download Z3
if [[ -n "$SVF_Z3" && ! -d "$Z3_DIR" ]] ; then
  echo "[+] Downloading Z3 ..."
  generic_download_file "$Z3_URL" z3.zip
  unzip -q z3.zip && mv z3-* "$Z3_DIR"
  rm z3.zip
fi

# Download ctir Clang
if [[ -n "$SVF_CTIR" && ! -d "$CTIR_DIR" ]] ; then
  echo "[+] Downloading ctir Clang ..."
  generic_download_file "$CTIR_URL" ctir.zip
  mkdir -p "$CTIR_DIR" && unzip -q "ctir.zip" -d "$CTIR_DIR"
  rm ctir.zip
fi

# Download SVF test suite
if [[ "$OSTYPE" == "linux-gnu" && ! -d "$SVF_TESTS" ]] ; then
  echo "[+] Downloading SVF test suite ..."
  git clone "https://github.com/SVF-tools/Test-Suite.git"
  pushd "$SVF_TESTS" > /dev/null
  ./generate_bc.sh
  popd > /dev/null
fi

# Build phase
pushd "$BUILD_DIR" > /dev/null
if [[ -z "$SVF_DEBUG" ]] ; then
  cmake ..
else
  cmake -D CMAKE_BUILD_TYPE:STRING=Debug ..
fi
make -j $(nproc)

# Run ctest
if [[ "$OSTYPE" == "linux-gnu" ]] ; then
  ctest --v
fi

popd > /dev/null