# bin2c util
# (c)2000 Dan Potter

all: makeip

bin2c:
	gcc -o makeip makeip.c

clean:
	-rm -f makeip

