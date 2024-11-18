#!/bin/sh

usage="Usage: fam [ all | test ] [options]";

for var in "$@"; do
	case "$var" in
	-s)
		s=1
		;;
	--update-docs)
		update_docs=1
		;;
	--skip-final)
		skip_final=1
		;;
	--with-asan)
		with_asan=1
		;;
	--with-cc=*)
		cc=${var#*=}
		;;
	--with-extra=*)
		extra=${var#*=}
		;;
	--filter=*)
		filter=${var#*=}
		;;
	--target=*)
		target=${var#*=}
		;;
	all)
		if [ "$test" = "1" ] || [ "$clean" = "1" ] || [ "$coverage" = "1" ] || [ "$fasttest" = "1" ]; then
                        echo "Multiple options specified";
                        echo $usage;
                        exit;
                fi
		all=1;
		;;
	fasttest)
		if [ "$all" = "1" ] || [ "$clean" = "1" ] || [ "$coverage" = "1" ] || [ "$test" = "1" ]; then
                        echo "Multiple options specified";
                        echo $usage;
                        exit;
                fi
		fasttest=1;
		;;
	clean)
	if [ "$all" = "1" ] || [ "$test" = "1" ] || [ "$coverage" = "1" ] || [ "$fasttest" = "1" ]; then
                        echo "Multiple options specified";
                        echo $usage;
                        exit;
                fi
		clean=1;
		;;
	coverage)
		if [ "$all" = "1" ] || [ "$clean" = "1" ] || [ "$test" = "1" ] || [ "$fasttest" = "1" ]; then
                        echo "Multiple options specified";
                        echo $usage;
                        exit;
                fi
		coverage=1;
		;;
	test)
		if [ "$all" = "1" ] || [ "$clean" = "1" ] || [ "$coverage" = "1" ] || [ "$fasttest" = "1" ]; then
			echo "Multiple options specified";
			echo $usage;
			exit;
		fi
		test=1;
		;;
	*)
		echo "Unrecognized option: '$var'"
		echo $usage;
		exit;
	esac
done

if [ "$test" != "1" ] && [ "$clean" != "1" ] && [ "$coverage" != "1" ] && [ "$fasttest" != "1" ]; then
	all=1;
fi

