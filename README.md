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

