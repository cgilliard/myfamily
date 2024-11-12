#!/bin/sh

. ./scripts/env.sh

linessum=0;
coveredsum=0;

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
      lines=`gcov $gcno | grep "^Lines" | head -1 | cut -f4 -d ' ' | tr -d \\n`;
      if [ "$lines" = "" ]; then
         lines="0";
      fi
      ratio=`awk "BEGIN {print $percent / 100}"`;
      covered=`awk "BEGIN {print int($ratio * $lines)}"`;
      linessum=`awk "BEGIN {print $linessum + $lines}"`;
      coveredsum=`awk "BEGIN {print $coveredsum + $covered}"`;
      printf "%-20s %-10s %s\n" "${gcno%.gcno}.c" "$percent%" "$lines";
   done
   cd ..;
done

echo "--------------------------------"\
"--------------------------------"\
"--------------------------------"\
"--------------------";
codecov=`awk "BEGIN {print 100 * $coveredsum / $linessum}"`
codecov=`printf "%.2f" $codecov`;
printf "%-20s %-10s %i\n" "Total Coverage" "$codecov%" $linessum;
