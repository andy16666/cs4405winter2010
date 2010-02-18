This operating system is designed for the Rug Warrior robot running the 68HC11 processor.

All process management functions and definitions are located in process.c and process.h

The core operating system is defined in os.c and os.h

FIFOs are implemented in fifo.c and fifo.h

Semaphores are implemented in semaphore.c and semaphore.h

When defining user processes, all definitions are expected to be in user.c and user.h.  Initial processes to be created are to be defined in ProcessInit() in user.c.

A make.bat file is included.  Use this file to build the operating system and create a S19 file from the resulting executable.  The compiler tools are expected to be on the executable path.  On Windows, if the compiler is not already on the path, this may be done from the command prompt in a manner similar to the following:
    set path=%path%;C:\usr\bin
Where C:\usr\bin is replaced by the full path to the bin directory of the compiler installation.

After make.bat finishes execution, there will be os.elf and os.s19.  To install the operating system on the Rug Warrior robot, use HCLoad to copy os.s19 to the robot's memory.
