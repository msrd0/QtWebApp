#!/bin/busybox ash
set -euo pipefail

format_file() {
	echo "Formatting file $1"
	local tmpfile
	tmpfile=$(mktemp)
	clang-format "$1" >$tmpfile
	mv $tmpfile "$1"
}

format_dir() {
	find . \( -name '*.h' -or -name '*.cpp' \) -not -path '*build*' \
		| while read file; do
			format_file "$file"
		done
}

format_dir Demo1
format_dir Demo2
format_dir QtWebApp
