#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>




int main(int argc, char *argv[])
{
  int id, status = 0,id1,status1=0;
     
  
  id = fork();
  if (id == 0)
  {   
      
      char *argv2[4];//the array of sstring to pass to execvp
      argv2[0] = (char *)malloc(strlen("xterm")+1);
      argv2[1] = (char *)malloc(strlen("-e")+1);
      argv2[2] = (char *)malloc(strlen("./shell")+1);
     
      strcpy(argv2[0], "xterm");
      strcpy(argv2[1], "-e");//flag for executable command
      strcpy(argv2[2], "./shell");
      argv2[3]=NULL;
      

      execvp("xterm", argv2);//call execvp
      perror("execvp failed:");
      exit(-1);
      
  }
  else
  {
       wait(&status);         

  }
}
