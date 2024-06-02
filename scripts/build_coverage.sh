#!/bin/bash

./scripts/coverage.sh

linessum=0;
coveredsum=0;
rm -f /tmp/gcov_cat.txt

for d in "log" "base" "main" "util"
do

cd $d

FILES=./*.c

for f in $FILES
do
	if [ $f != "./test.c" ]; then
		echo "calling gcov for $f"
		percent=`gcov $f | grep "^Lines" | head -1 | cut -f2 -d ' ' | cut -f2 -d ':' | cut -f1 -d '%' | tr -d \\n`;
		lines=`gcov $f | grep "^Lines" | head -1 | cut -f4 -d ' ' | tr -d \\n`;
		v=100;
		echo $percent;
		ratio=`awk "BEGIN {print $percent / 100}"`;
		covered=`awk "BEGIN {print int($ratio * $lines)}"`;
		linessum=`awk "BEGIN {print $linessum + $lines}"`;
		coveredsum=`awk "BEGIN {print $coveredsum + $covered}"`;

		cat $f.gcov >> /tmp/gcov_cat.txt

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
cp docs/code_coverage.html.template docs/code_coverage.html
export codecov;

entries=`cat docs/cc_summary.txt`;
rm -f /tmp/timestamps
rm -f /tmp/values
declare -a timestamps;
declare -a values;
i=0;
for entry in $entries
do
        if [ $(expr $i % 2) == 0 ]
        then
                echo "format_date($entry * 1000 )," >> /tmp/timestamps
        else
                last_value=`echo $entry`;
                echo "$entry," >> /tmp/values
        fi
        let i=i+1;
done

export coverage=`cat /tmp/values`;
export timestampsv=`cat /tmp/timestamps`;
export summary=`cat /tmp/gcov_cat.txt`;

perl -pi -e 's/REPLACECOVERAGE_SINGLE/$ENV{codecov}/g' docs/code_coverage.html
perl -pi -e 's/REPLACECOVERAGE/$ENV{coverage}/g' docs/code_coverage.html
perl -pi -e 's/REPLACETIMESTAMP/$ENV{timestampsv}/g' docs/code_coverage.html
perl -pi -e 's/REPLACESUMMARY/$ENV{summary}/g' docs/code_coverage.html

# update README
cp README.md.template README.md
perl -pi -e 's/CODE_COVERAGE/$ENV{codecov}/g' README.md
