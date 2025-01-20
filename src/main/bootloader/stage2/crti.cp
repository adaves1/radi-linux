package "radi.linux.pkg" with ".boot.stage2";

class .init {
  global _init;
  loop _init;
  l:_init {
    DEF << push REG ebp;
    DEF << move REG ebp : esp;
  }
}

class .fini {
  global _fini;
  loop _fini;
  l:_fini {
    DEF << push REG ebp;
    DEF << move REG ebp : REG esp
  }
}
