#!/bin/sh

subdirs="base crypto"
deps_base='*.o';
deps_crypto='*.o ../base/*.o';

cc=gcc
cc_flags='-Wno-pointer-sign -g -O0 -DTEST'

