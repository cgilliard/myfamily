#!/bin/sh

. ./scripts/env.sh

for dir in $subdirs; do
echo "--------------------------------"\
"--------------------------------"\
"--------------------------------"\
"--------------------";
printf "%-20s %-10s %s\n" "$dir" "percent" "lines";
echo "--------------------------------"\
"--------------------------------"\
"--------------------------------"\
"--------------------";
   cd $dir;
   for gcno in *.gcno; do
      percent=`gcov $gcno | grep "^Lines" | head -1 | cut -f2 -d ' ' | cut -f2 -d ':' | cut -f1 -d '%' | tr -d \\n`;
      if [ "$percent" = "" ]; then
         percent=0.00;
      fi;
      lines=`gcov $gcno | grep "^Lines" | head -1 | cut -f4 -d ' ' | tr -d \\n`; \
      if [ "$lines" = "" ]; then
         lines="0";
      fi
      printf "%-20s %-10s %s\n" "${gcno%.gcno}.c" "$percent%" "$lines";
   done
   cd ..;
done

# ${file%.c}.o
