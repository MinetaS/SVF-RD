#!/bin/bash

if [ -d "Release-build/" ] ; then
  rm -rf Release-build/
fi

if [ -d "Debug-build" ] ; then
  rm -rf Debug-build/
fi