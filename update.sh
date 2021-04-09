#!/bin/bash
set -euo pipefail

# delete everything to detect deleted/renamed files
rm -r QtWebApp Demo{1,2,3}

# download the new upstream sources
wget -qO- http://stefanfrings.de/qtwebapp/QtWebApp.zip \
	| bsdtar xf - --strip-components 1
