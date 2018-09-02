#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#define BUFSIZE 100
int main(int argc, char *argv[])
{
   if (argc != 3)/*should have exactly 2 command line arguments*/
   {
      printf("usage: sort1 <filename1> <filename2>\n");
      exit(-1);
   }
   int ifd, ofd,fd1[2],fd2[2],flag,id,fileread;
   char input[BUFSIZE],line[2];
   

   /* Open input file descriptor */
   ifd = open(argv[1], O_RDONLY);
   if (ifd < 0) {
      perror("Unable to open input file in read mode \n");
      exit(1);
   } 
  /* Open output file descriptor */
   ofd = open(argv[2], O_CREAT | O_WRONLY |O_APPEND|O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
   if (ofd < 0) {
      perror("Unable to open output file in write mode...\n");
      exit(2);
   } 
   pipe(fd1);//create 1st pipe 
   pipe(fd2);//create 2nd pipe  
   flag=0;//flag to check whether the required fd's have been closed
   id=fork();
   while(1)
   {
      if(id==0)//in child process
      {
         if(flag==0)
         {
            close(fd1[1]);//close output for fd1
            close(fd2[0]);//close input for fd2
            flag=1;
         }
         int len=read(fd1[0],input,BUFSIZE);//read fd1 from pipe1 
         if(write(ofd,input,len)==-1)//write input read from pipe1 to output file
         {
             perror("Error in copying file...\n");
             write(fd2[1],"-1",2);//send -1 to pipe 2incase writing fails
             exit(-1);//exit

         }
         write(fd2[1],"0",1);//if succesful,send 0 to pipe2
         if(len<BUFSIZE)//if read no of characters are lesser than bufsize,file has been copied fully
         {
            printf("File copied successfully\n");
            exit(0);
         }


      }
      else
      {
         if(flag==0)//if not closed
         {
            close(fd1[0]);
            close(fd2[1]);
            flag=1;
         }
         fileread=read(ifd,input,BUFSIZE);//read from input file to pipe1
         write(fd1[1],input,fileread);//write file content to pipe1
         read(fd2[0],line,2);// read from pipe fd2
         if(strcmp("-1",line)==0 || (strcmp("0",line)==0)&& fileread<100)//if unsuccessful or completed
         {
            exit(3);
         }


      }
   }
}




