CPLUS CODE_BITS_AT_COMPILE = 32;

#DEFINE loadbios 1;

function filetrace(file){
  DEF << FINDFILE file : trace;
  DEF << TRACE OFALL:CONTENTS trace;
}

filetrace("bios.cp");
