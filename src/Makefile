TARGET = makeip

VERSION = 2.0.0

OBJECTS = utils.o vector.o crc.o mr.o field.o ip.o main.o

CC = gcc

CFLAGS = -O2 -Wall -DMAKEIP_VERSION=\"$(VERSION)\"
LDFLAGS = -s

INSTALLDIR = $(KOS_BASE)/../bin

EXECUTABLEEXTENSION =
ifeq ($(shell echo $(OS)),Windows_NT)
  EXECUTABLEEXTENSION := .exe
endif

OUTPUT = $(TARGET)$(EXECUTABLEEXTENSION)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(OUTPUT) $(OBJECTS)

install:
	mkdir -p $(INSTALLDIR)
	cp $(OUTPUT) $(INSTALLDIR)

.PHONY: clean	
clean:
	-rm -f $(OUTPUT) *.o
