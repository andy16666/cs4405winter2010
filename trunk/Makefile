#!/usr/bin/make -f

CC = m6811-elf-gcc
OBJCOPY = m6811-elf-objcopy

CPPFLAGS = 
CFLAGS = $(DBGFLAGS) -O -mshort -msoft-reg-count=0
DBGFLAGS = -g
LDFLAGS = -Wl,-m,m68hc11elfb

OBJECTS = user.o lcd.o process.o semaphore.o fifo.o os.o

OBJFLAGS = --only-section=.text --only-section=.rodata --only-section=.vectors --only-section=.data --output-target=srec

BINFILE = os.elf
S19FILE = os.s19

all: $(BINFILE) $(S19FILE)

$(BINFILE): $(OBJECTS)
	@echo 'Building $@'
	$(CC) $(CFLAGS) $(LDFLAGS) $+ -o $@

$(S19FILE): $(BINFILE)
	@echo 'Building $@'
	$(OBJCOPY) $(OBJFLAGS) $+ $@

clean:
	-rm -f $(OBJECTS)
	-rm -f $(BINFILE) $(S19FILE)

.PHONY: all clean
