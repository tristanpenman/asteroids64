#include <nusys.h>

beginseg
  name    "code"
  flags   BOOT OBJECT
  entry   nuBoot
  address NU_SPEC_BOOT_ADDR
  stack   NU_SPEC_BOOT_STACK
  include "codesegment.o"
  include "$(ROOT)/usr/lib/PR/rspboot.o"
  include "$(ROOT)/usr/lib/PR/aspMain.o"
  include "$(ROOT)/usr/lib/PR/n_aspMain.o"
  include "$(ROOT)/usr/lib/PR/gspF3DEX2.fifo.o"
  include "$(ROOT)/usr/lib/PR/gspL3DEX2.fifo.o"
  include "$(ROOT)/usr/lib/PR/gspF3DEX2.Rej.fifo.o"
  include "$(ROOT)/usr/lib/PR/gspF3DEX2.NoN.fifo.o"
  include "$(ROOT)/usr/lib/PR/gspF3DLX2.Rej.fifo.o"
  include "$(ROOT)/usr/lib/PR/gspS2DEX2.fifo.o"
endseg

beginseg
  name "sfxbank"
  flags RAW
  include "se.ctl"
endseg

beginseg
  name "sfxtable"
  flags RAW
  include "se.tbl"
endseg

beginwave
  name "asteroids"
  include "code"
  include "sfxbank"
  include "sfxtable"
endwave
