#!/bin/sh

. ./scripts/env.sh

if [ "$NO_COLOR" = "" ]; then
   GREEN="\033[32m";
   CYAN="\033[36m";
   YELLOW="\033[33m";
   BRIGHT_RED="\033[91m";
   RESET="\033[0m";
   BLUE="\033[34m";
else
   GREEN="";
   CYAN="";
   YELLOW="";
   BRIGHT_RED="";
   RESET="";
   BLUE="";
fi

update_docs=0;
for var in "$@"
do
case "$var" in --update-docs)
        update_docs=1;
        ;;
esac
done


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
      if [ -f ${gcno%.gcno}.gcda ] ; then
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
         printf "$GREEN%-20s$RESET $YELLOW%-10s$RESET $CYAN%s/%s$RESET\n" "${gcno%.gcno}.c" "$percent%" "$covered" "$lines";
      fi
   done
   cd ..;
done

echo "$BLUE====================================================================================================================$RESET";
codecov=`awk "BEGIN {print 100 * $coveredsum / $linessum}"`
codecov=`printf "%.2f" $codecov`;
timestamp=`date +%s`

printf "$GREEN%-20s$RESET $YELLOW%-10s$RESET $CYAN%i/%i$RESET\n" "Total Coverage" "$codecov%" $coveredsum $linessum;
export cc_final=$codecov


if [ $update_docs = 1 ]; then
   echo "$codecov" > /tmp/cc_final;
   cp ./.templates/README.md README.md
   perl -pi -e 's/CODE_COVERAGE/$ENV{cc_final}/g' README.md
   echo "$timestamp $codecov $coveredsum $linessum" >> ./docs/cc.txt
   ./scripts/update_code_coverage.sh
fi
