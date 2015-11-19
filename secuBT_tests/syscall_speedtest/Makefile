
CFLAGS=-ggdb

all: speedtest

speedtest: speedtest.S
	gcc ${CFLAGS} -o speedtest speedtest.S
	
.PHONY: clean all

clean:
	rm -f speedtest

