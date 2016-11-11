# this makefile uploads the built packages

first: push

push: mount clean copy sign gen signrel

mount:
	mkdir mount
	sshfs alarmpi-debian@msrd0.duckdns.org:/srv/debian mount

clean:
	rm mount/pool/main/all/libq/libqtwebapp* || true
	rm mount/pool/main/amd64/libq/libqtwebapp* || true

copy:
	cp libqtwebapp*all*.deb mount/pool/main/all/libq/
	cp libqtwebapp*amd64*.deb mount/pool/main/amd64/libq/

sign:
	ssh alarmpi-debian@msrd0.duckdns.org -o BatchMode=yes -v \
		/srv/debian/bin/sign.sh

gen:
	mount/bin/genpackages.sh mount
	mount/bin/genrelease.sh mount

signrel:
	ssh alarmpi-debian@msrd0.duckdns.org -o BatchMode=yes -v \
		/srv/debian/bin/signreleases.sh
