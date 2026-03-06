default: build

build: clean
	gcc -Wall -o curl main.c gnl.c util.c libjson-c.a -lcurl 

clean:
	rm -rf curl 

test: build
	./curl https://freegeoip.app/json/
