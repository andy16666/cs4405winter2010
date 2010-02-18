@echo on
m6811-elf-gcc -g -mshort -Wl,-m,m68hc11elfb -O -msoft-reg-count=0 user.c lcd.c process.c semaphore.c fifo.c os.c -o os.elf
m6811-elf-objcopy --only-section=.text --only-section=.rodata --only-section=.vectors --only-section=.data --output-target=srec os.elf os.s19