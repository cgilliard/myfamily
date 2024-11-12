#!/bin/sh

. ./scripts/env.sh

if [ "$NO_COLOR" = "" ]; then
   GREEN="\x1b[32m";
   CYAN="\x1b[36m";
   YELLOW="\x1b[33m";
   BRIGHT_RED="\x1b[91m";
   RESET="\x1b[0m";
   BLUE="\x1b[34m";
else
   GREEN="";
   CYAN="";
   YELLOW="";
   BRIGHT_RED="";
   RESET="";
   BLUE="";
fi


linessum=0;
coveredsum=0;

for dir in $subdirs; do
echo "$BLUE--------------------------------"\
"--------------------------------"\
"--------------------------------"\
"--------------------$RESET";
printf "%-20s %-10s %s\n" "$dir" "percent" "lines";
echo "$BLUE--------------------------------"\
"--------------------------------"\
"--------------------------------"\
"--------------------$RESET";
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
      printf "$GREEN%-20s$RESET $YELLOW%-10s$RESET $CYAN%s$RESET\n" "${gcno%.gcno}.c" "$percent%" "$lines";
   done
   cd ..;
done

echo "[$BLUE===================================================================================================================$RESET]";
codecov=`awk "BEGIN {print 100 * $coveredsum / $linessum}"`
codecov=`printf "%.2f" $codecov`;
printf "$GREEN%-20s$RESET $YELLOW%-10s$RESET $CYAN%i$RESET\n" "Total Coverage" "$codecov%" $linessum;
