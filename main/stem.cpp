#include <stdio.h>
#include "util/porter.h"
static char * s;         /* a char * (=string) pointer; passed into b above */

#define INC 50           /* size units in which s is increased */
static int i_max = INC;  /* maximum offset in s */

void increase_s()
{  i_max += INC;
   {  char * new_s = (char *) malloc(i_max+1);
      { int i; for (i = 0; i < i_max; i++) new_s[i] = s[i]; } /* copy across */
      free(s); s = new_s;
   }
}

#define UC(ch) (ch <= 'Z' && ch >= 'A')
#define LC(ch) (ch <= 'z' && ch >= 'a')
#define LETTER(ch) (UC(ch) || LC(ch))
#define FORCELC(ch) (ch-('A'-'a'))

void stemfile(FILE * f)
{  while(TRUE)
   {  int ch = getc(f);
      if (ch == EOF) return;
      if (LETTER(ch))
      {  int i = 0;
         while(TRUE)
         {  if (i == i_max) increase_s();

            if UC(ch) ch = FORCELC(ch);
            /* forces lower case. Remove this line to make the program work
               exactly like the Muscat stemtext command. */

            s[i] = ch; i++;
            ch = getc(f);
            if (!LETTER(ch)) { ungetc(ch,f); break; }
         }
         s[stem(s,0,i-1)+1] = 0;
         /* the pevious line calls the stemmer and uses its result to
            zero-terminate the string in s */
         printf("%s",s);
      }
      else putchar(ch);
   }
}

int main(int argc, char * argv[])
{  int i;
   s = (char *) malloc(i_max+1);
   for (i = 1; i < argc; i++)
   {  FILE * f = fopen(argv[i],"r");
      if (f == 0) { fprintf(stderr,"File %s not found\n",argv[i]); exit(1); }
      stemfile(f);
   }
   free(s);
   return 0;
}
