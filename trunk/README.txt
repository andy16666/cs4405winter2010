This operating system is designed for the Rug Warrior robot running the
68HC11 processor.

For defining of user processes, we have provided user.c and user.h.
Initial processes creation can be done via ProcessInit() in user.c.

A make.bat file is included. Use this file to build the operating system
and create os.elf and os.s19. The compiler tools are expected to be on the
executable path. On Windows, if the compiler is not already on the path,
this may be done from the command prompt in a manner similar to the
following:
    set path=%path%;C:\usr\bin
Where C:\usr\bin is replaced by the full path to the bin directory of the
compiler installation.

After you have defined your processes, run make.bat. Once this finishes,
install os.s19 on the Rug Warrior robot using HCLoad.
All process management functions and definitions are located in process.c
and process.h

Files:

ports.h - contains definition and macros for accessing ports on the 68HC11. 

interrupts.h - defines interrupt vectors. 

os.c - implements the core API specified in os.h, with the exception of
fifo and semaphore functionality. 

FIFOs are implemented in fifo.c and fifo.h

Semaphores are implemented in semaphore.c and semaphore.h

Memory regions are defined by memory.x. 
