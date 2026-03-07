default: build

build: clean
	gcc -Wall -Wextra -Werror -o ask_littletown main.c gnl.c util.c libjson-c.a -lcurl 

clean:
	rm -rf curl 

test: build
	./curl https://freegeoip.app/json/
