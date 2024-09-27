#!/bin/bash

CRIT=`brew --prefix criterion`

./configure $1 $2\
	--with-criterion-include=${CRIT}/include \
	--with-criterion-lib=${CRIT}/lib \
