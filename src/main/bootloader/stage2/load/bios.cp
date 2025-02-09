CPLUS CODE_BITS_AT_COMPILE = 32;

#ifndef
#DEFINE BIOS 101;
#endif

// THE BIOS YAYYYY

class Functions;
cls:Functions.properties(nocall = true);

class Var;
cls:Var.properties(nocall = true);

class Var extends {
  new string CUR;
}

class Functions extends {
  function displaytext(font:"Fonts/Radibike.font", text) {
    fnt = from:font\12:find:alltext;
    for raw "\n" in text {
      trace(newline);
    }
    fnt.fnt[a-a-1-1].trace(text); 
  }
}

if (CPLUS CODE_RUN_TYPE == "CPLUS-CODERUNFIRSTTIME") {
  void dt(*any) = displaytext(*any);
  dt("Radi Linux\n\n");
  dt("Installation");
  dt("\nSelect your normal cursor mode.\nTop2BottomCut (Press 1)\nTop2BottomCutNot (Press 2)");
  DEF << newstr cmd TAKEINPUT(None):for done[nL];
  if (cmd[0] == "1") {
    DEF << MOUSECURSOR == "Cursors/Top2BottomCut.cur";
    CUR = 1;
  }
  else if (cmd[0] == "2") {
    DEF << MOUSECURSOR == "Cursors/Top2BottomNotCut.cur";
    CUR = 2;
  }
  else {
    dt("Option not recognized!\nResetting Installation");
    CPLUS CODE_RUN_TIMES += 1;
  }
}
