#!/bin/bash

./scripts/coverage.sh

linessum=0;
coveredsum=0;

for d in "parser" "syn" "main" "util"
do

cd $d

FILES=./*.c

for f in $FILES
do
	if [ $f != "./test.c" ]; then
		echo "calling gcov for $f"
		percent=`gcov $f | grep "^Lines" | cut -f2 -d ' ' | cut -f2 -d ':' | cut -f1 -d '%'`;
		lines=`gcov $f | grep "^Lines" | cut -f4 -d ' '`;
		v=100;
		echo $percent;
		ratio=`awk "BEGIN {print $percent / 100}"`;
		covered=`awk "BEGIN {print int($ratio * $lines)}"`;
		linessum=`awk "BEGIN {print $linessum + $lines}"`;
		coveredsum=`awk "BEGIN {print $coveredsum + $covered}"`;

		echo "percent=$percent,lines=$lines,percent_dec=$ratio,covered=$covered";
	fi
done

cd ..

done

codecov=`awk "BEGIN {print 100 * $coveredsum / $linessum}"`
codecov=`printf "%.2f" $codecov`;
echo "linessum=$linessum,coveredsum=$coveredsum,codecov=$codecov";
timestamp=$(date +%s);
echo "$timestamp $codecov" >> ./docs/cc_summary.txt
