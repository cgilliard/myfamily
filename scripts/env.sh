#!/bin/sh

subdirs="base crypto core main"
deps_base='*.o';
deps_crypto='*.o ../base/*.o';
deps_core='*.o ../base/*.o';
deps_main='../base/*.o';


