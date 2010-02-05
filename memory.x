/* memory.x*/

MEMORY
{
  page0 (rw) : ORIGIN = 0x0, LENGTH = 0x0100
  ports (rw) : ORIGIN = 0x1000, LENGTH = 0x003F
  text (rx)  : ORIGIN = 0x8000, LENGTH = 0x6000
  eeprom        : ORIGIN = 0xb600, LENGTH = 512  
  vectors (rwx) : ORIGIN = 0xFFC0, LENGTH = 0x0040
  data          : ORIGIN = 0xB700, LENGTH = 0x5000
}

 	

SECTIONS
{
  ports : { *(ports) } > ports
  vectors : { *(vectors) } > vectors
}



PROVIDE (_stack = 0xF000);

/*PROVIDE (Ports = 0x1000);
PROVIDE (IV = 0xBFC0);*/
