#!/bin/sh

# eval "deps_base='*.o'"
# eval "deps_crypto='*.o ../base/*.o'"

. ./scripts/env.sh

if [ "$1" = "clean" ]; then
   rm -f */*.o
elif [ "$1" = "test" ]; then
   ./make || exit 1;
   echo "[===================================================================================================================]";
   for dir in $subdirs; do
      cd ${dir};
      if [ -f "test.c" ]; then
         mkdir -p bin
         eval "deps=\$deps_${dir}"
         ${cc} -I.. ${cc_flags} test.c -o bin/test ${deps} || exit 1;
         ./bin/test || exit 1;
      fi
      cd ..;
   done
   echo "All tests passed!";
else
   for dir in $subdirs
   do
      cd ${dir}
         for file in *.c
         do
            newer_header=0;
            for header in *.h; do
               if [ ${header} -nt ${file%.c}.o ]; then
                  newer_header=1;
               fi
            done

            if [ ${file} -nt ${file%.c}.o ] || [ $newer_header == 1 ]; then
               if [ ${file} != "test.c" ]; then
                  echo "${cc} -I.. ${cc_flags} -o ${file%.c}.o -c ${file}"
                  ${cc} -I.. ${cc_flags} -o ${file%.c}.o -c ${file} || exit 1;
               fi
            fi
         done
      cd ..
   done
fi
