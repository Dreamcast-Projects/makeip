
all: makeip

makeip:
	gcc -o makeip makeip.c

clean:
	-rm -f makeip

