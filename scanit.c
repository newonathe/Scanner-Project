
/* this program repeatedly calls gettoken() and prints id and lexeme */

#include <stdio.h>
#include <string.h>
#include "scan.h"

int main(int argc, char** argv)
{
   char filename[20];
   strcpy(filename,"test1.txt");	
   if (argc >= 2)
      strcpy(filename,argv[1]);
   openfile(filename);
   struct token t = gettoken();
   while ( t.id != TokenEndOfFile )
   {
      printf("%s %s\n", tokennames[t.id], t.lexeme);
      t = gettoken();
   }
   return 0;
}

