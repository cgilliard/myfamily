# myfamily

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
# ./configure --with-criterion-include=<path/to/criterion/include/dir> --with-criterion-lib=<path/to/criterion/lib/dir>
```

This might look something like this:
```
# ./configure --with-criterion-lib=/Users/USERNAME/homebrew/Cellar/criterion/2.4.2/lib --with-criterion-include=/Users/USERNAME/homebrew/Cellar/criterion/2.4.2/include
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
./coverage.sh
```

To view coverage of a specific file cd into it's directory and run the gcov command. For example, for parser.c:

```
# cd parser
# gcov parser.c
```

The result will be stored in the parser.c.gcov file.
