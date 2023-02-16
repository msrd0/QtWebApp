#!/bin/bash
set -euo pipefail

# delete everything to detect deleted/renamed files
for dir in QtWebApp Demo{1,2,3}; do
	rm -r $dir || echo "WARNING: Failed to remove $dir"
done

# download the new upstream sources
wget -qO- http://stefanfrings.de/qtwebapp/QtWebApp.zip \
	| bsdtar xf - --strip-components 1
