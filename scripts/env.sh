#!/bin/sh

subdirs="base crypto core main"
deps_base='*.o';
deps_crypto='*.o ../base/*.o';
deps_core='*.o ../crypto/*.o ../base/*.o';
deps_main='../base/*.o';


