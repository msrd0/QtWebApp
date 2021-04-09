#!/bin/busybox ash
set -eo pipefail

check=n
if [ "$1" == "--check" ]
then
	check=y
fi

set -u

format_file() {
	local tmpfile
	tmpfile=$(mktemp)
	clang-format "$1" >$tmpfile
	
	if [ $check == y ]
	then
		diff "$1" $tmpfile
	else
		mv $tmpfile "$1"
	fi
}

format_dir() {
	find "$1" \( -name '*.h' -or -name '*.cpp' \) -not -path '*build*' \
		| while read file; do
			format_file "$file"
		done
}

format_dir Demo1
format_dir Demo2
format_dir QtWebApp
