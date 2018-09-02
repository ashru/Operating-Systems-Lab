#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

/* This program should be used in conjunction with the calculator
   program sort1.c. The executable file sort1 should be present
*/

int main(int argc, char *argv[])
{
  int id, status = 0,id1,status1=0;
     
  if (argc != 2)
  {
      printf("usage: xsort <filename>\n");
      exit(-1);
  }
  id = fork();
  if (id == 0)
  {   
      char *argv2[6];
      argv2[0] = (char *)malloc(strlen("xterm")+1);
      argv2[1] = (char *)malloc(strlen("-hold")+1);
      argv2[2] = (char *)malloc(strlen("-e")+1);
      argv2[3] = (char *)malloc(strlen("./sort1")+1);
      argv2[4] = (char *)malloc(100);  // assuming max 100 characters
      strcpy(argv2[0], "xterm");
      strcpy(argv2[1], "-hold");//flag reqd to hold the xterm screen on console
      strcpy(argv2[2], "-e");//flag for executable command
      strcpy(argv2[3], "./sort1");
      strcpy(argv2[4], argv[1]);//passing filename as command line argument for sort1
      argv2[5]=NULL;
      execvp("xterm", argv2);
      perror("execvp failed:");
      exit(-1);
      
  }
  else
  {
       wait(&status);         

  }


   
          
}
      
