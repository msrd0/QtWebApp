# this makefile uploads the built packages

first: push

push: mount clean copy sign gen signrel

mount:
	mkdir mount
	sshfs alarmpi-debian@msrd0.duckdns.org:/srv/debian mount

clean:
	rm mount/pool/main/all/i/intranet* || true
	rm mount/pool/main/amd64/i/intranet* || true

copy:
	cp intranet*all*.deb mount/pool/main/all/i/
	cp intranet*amd64*.deb mount/pool/main/amd64/all/i/

sign:
	ssh alarmpi-debian@msrd0.duckdns.org -o BatchMode=yes -v \
		/srv/debian/bin/sign.sh

gen:
	mount/bin/genpackages.sh mount
	mount/bin/genrelease.sh mount

signrel:
	ssh alarmpi-debian@msrd0.duckdns.org -o BatchMode=yes -v \
		/srv/debian/bin/signreleases.sh
