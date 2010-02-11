CC = m68hc11-gcc
CPPFLAGS = 
CFLAGS = -g -O3 -mshort
SOFLAGS = -Wl,-m,m68hc11elfb
LDFLAGS = -mshort -msoft-reg-count=0 -static
OBJECTS = process.o semaphore.o fifo.o os.o

kernel: $(OBJECTS)
	$(CC) $(LDFLAGS) $(SOFLAGS) $^ -o $@.elf

$(OBJECTS): %.o: %.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -o $@


clean:
	-rm *.elf $(OBJECTS)

.PHONY: clean
