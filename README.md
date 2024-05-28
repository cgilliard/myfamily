[![Build Status](https://dev.azure.com/mwc-project/bitcoinmw/_apis/build/status/cgilliard.myfamily?branchName=main)](https://dev.azure.com/mwc-project/bitcoinmw/_build?definitionId=14)
[![Release Version](https://img.shields.io/github/v/release/cgilliard/myfamily.svg)](https://github.com/cgilliard/myfamily/releases)
[![Code Coverage](https://img.shields.io/static/v1?label=Code%20Coverage&message=84.76%&color=purple)](https://cgilliard.github.io/myfamily/code_coverage.html)
[![Docmentation](https://img.shields.io/static/v1?label=Documentation&message=Doxygen&color=red)](https://cgilliard.github.io/myfamily/)
[![License](https://img.shields.io/github/license/cgilliard/bitcoinmw.svg)](https://github.com/cgilliard/bitcoinmw/blob/master/LICENSE)

# Overview

My family is a work in progress. More to come.

# Build

1.) Prereqs

Required:

* make
* gcc or clang
* bash
* perl

Optional:

* doxygen - to build docs (https://www.doxygen.nl/index.html)
* criterion - to run tests (https://github.com/Snaipe/Criterion)



2.) Configure

```
# ./configure
```

If you want to be able to run the tests, specify the location of your criterion headers and libs.

```
# ./configure --with-criterion-include=<path/to/criterion/include/dir>\
              --with-criterion-lib=<path/to/criterion/lib/dir>
```

This might look something like this:
```
# ./configure --with-criterion-lib=/Users/USERNAME/homebrew/Cellar/criterion/2.4.2/lib\
              --with-criterion-include=/Users/USERNAME/homebrew/Cellar/criterion/2.4.2/include
```

3.) Build

```
# make
```

4.) Install
```
# sudo make install
```

# Documentation

To build the documentation run the following command:

```
# make doc
```

Documentation will be placed in the /doc directory.

# Tests

To run the tests, use the following command:

```
# make test
```

# Code coverage

For code coverage, run the following command:

```
./scripts/coverage.sh
```

To view coverage of a specific file cd into it's directory and run the gcov command. For example, for parser.c:

```
# cd parser
# gcov parser.c
```

The result will be stored in the parser.c.gcov file. Additional code coverage reporting will be available shortly.
