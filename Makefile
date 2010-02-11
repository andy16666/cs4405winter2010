CC = m68hc11-gcc
GENS19 = m68hc11-objcopy
S19FLAGS = --only-section=.text --only-section=.rodata --only-section=.vectors --only-section=.data --output-target=srec
CPPFLAGS = 
CFLAGS = -g -O3 -mshort
SOFLAGS = -Wl,-m,m68hc11elfb
LDFLAGS = -mshort -msoft-reg-count=0 -static
OBJECTS = process.o semaphore.o fifo.o os.o

robot: kernel
	$(GENS19) $(S19FLAGS) $<.elf $<.s19

kernel: $(OBJECTS)
	$(CC) $(LDFLAGS) $(SOFLAGS) $^ -o $@.elf

$(OBJECTS): %.o: %.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -o $@

clean:
	-rm *.elf $(OBJECTS)

.PHONY: clean
