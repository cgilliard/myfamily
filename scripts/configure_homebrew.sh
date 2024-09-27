#!/bin/bash

CRIT=`brew --prefix criterion`

./configure --minimal=false \
	--with-criterion-include=${CRIT}/include \
	--with-criterion-lib=${CRIT}/lib \
	--with-cc=cc
