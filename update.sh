#!/bin/bash
set -euo pipefail

# delete everything to detect deleted/renamed files
for dir in QtWebApp Demo{1,2,3}; do
	rm -r $dir || echo "WARNING: Failed to remove $dir"
done

# download the new upstream sources
sourcezip=$(mktemp)
wget -O $sourcezip http://stefanfrings.de/qtwebapp/QtWebApp.zip
bsdtar xf $sourcezip --strip-components 1
rm $sourcezip
