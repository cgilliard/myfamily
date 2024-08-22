/**
 * @mainpage
 *
 * Welcome to the My Family Documentation.
 *
 * For module level documentation, refer to:
 * - \ref core "Core Library Documentation"
 * - \ref main "Main Executable Documentation"
 *
 * \section Build
 *
 * 1.) Prerequisites
 * * make
 * * gcc or cc (clang)
 * * bash (build scripts including in makefile)
 * * perl (replacing parameters in the configure script)
 *
 * Optional: (for test and documentation only)
 *
 * * doxygen - to build docs (https://www.doxygen.nl/index.html)
 * * criterion - to run tests (https://github.com/Snaipe/Criterion)
 *
 * 2.) Configure
 *
 * ```
 * # ./configure
 * ```
 *
 * If you want to be able to run the tests, and the criterion library/include
 * directories are not in your search pathes, specify the location of your
 * criterion headers and libs as shown below.
 *
 * ```
 * # ./configure --with-criterion-include=<path/to/criterion/include/dir> \
 *              --with-criterion-lib=<path/to/criterion/lib/dir>
 * ```
 *
 * This might look something like this:
 * ```
 * # ./configure \
 * --with-criterion-lib=/Users/USERNAME/homebrew/Cellar/criterion/2.4.2/lib \
 * --with-criterion-include=/Users/USERNAME/homebrew/Cellar/criterion/2.4.2/include
 * ```
 * 3.) Build
 *
 * ```
 * # make
 * ```
 *
 * 4.) Install
 * ```
 * # sudo make install
 * ```
 *
 * # Documentation
 *
 * To build the documentation run the following command:
 *
 * ```
 * # make doc
 * ```
 *
 * Documentation will be placed in the /doc directory.
 *
 * # Tests
 *
 * To run the tests, use the following command:
 *
 * ```
 * # make test
 * ```
 *
 * # Code coverage
 *
 * For code coverage, run the following command:
 *
 * ```
 * # make coverage
 * ```
 *
 * A summary will be displayed after tests run. To view coverage of a specific
 * file cd into its directory and run the gcov command. For example, for heap.c
 *
 * ```
 * # cd core
 * # gcov heap.c
 * ```
 *
 * The result will be stored in the heap.c.gcov file. Additional code coverage
 * reporting is available by clicking on the documentation link from the repo's
 * README.
 */
