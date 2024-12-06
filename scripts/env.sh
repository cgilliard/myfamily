#!/bin/sh

subdirs="base core main"
deps_base='*.o';
deps_core='*.o ../base/*.o';
deps_main='../base/*.o ../core/*.o';


