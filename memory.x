MEMORY
{
  page0 (rwx) : ORIGIN = 0x0000, LENGTH = 0x0020
  lcdram (rwx): ORIGIN = 0x0020, LENGTH = 0x00D0
  ports (rw)  : ORIGIN = 0x1000, LENGTH = 0x0034
  text (rx)   : ORIGIN = 0x8000, LENGTH = 0x3FBF
  vectors (rw): ORIGIN = 0xBFC0, LENGTH = 0x0040
  stacks (rw) : ORIGIN = 0xC000, LENGTH = 0x1000
  data (rwx)  : ORIGIN = 0xD000, LENGTH = 0x2E00
  lcdbuf (rwx): ORIGIN = 0xFE00, LENGTH = 0x00D0

}

SECTIONS
{
    .lcd :
       {
         *(.lcd)
       } > lcdbuf
}

PROVIDE (_stack = 0xFFBF);