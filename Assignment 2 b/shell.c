#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/dir.h>
#include <sys/param.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <stdint.h>
#include <fcntl.h>
#include <time.h>
#include <locale.h>
#include <langinfo.h>



#define BUFSIZE 1<<16


static char perms_buff[30];
char* input_file=NULL;
char* output_file=NULL;

void implement_cd(char *path)/*implementation of command cd*/
{
	int t;
	if(path)
		t=chdir(path);/*chdir is function to change directory*/
	else
	{

	}
	if(t==-1)
	{
		perror("Changing directory failed:");
	}
}
void implement_pwd()/*implementation of command pwd*/
{
	char current_dir[BUFSIZE];
	if(!getcwd(current_dir,BUFSIZE))/*getpwd is function to get present working directory*/
	{
		perror("Obtaining current directory failed");
	}
	else
	{
		strcat(current_dir,"\n");
		write(1,current_dir,strlen(current_dir));
	}
}

void implement_mkdir(char *path)/*implementation of command mkdir*/
{
	int t=mkdir(path,0777);
	if(t==-1)
	{
		perror("Creating directory failed:");
	}
}
void implement_rmdir(char *path)/*implementation of command rmdir*/
{
	
	int t=rmdir(path);
	if(t==-1)
	{
		perror("Removing directory failed:");
	}
}

void implement_exit()/*implementation of command exit*/
{
	exit(0);
}
static int filter_func (const struct dirent *temp)/*filter function used in implementation of ls -l*/
{
    if(temp->d_name[0]=='.')
        return 0;
    return 1;
}

void implement_ls()/*implement vanilla ls command*/
{
	
    struct dirent **filelist;
    char path[BUFSIZE];
    if(!getcwd(path, sizeof(path)))
        perror("Could not get pathname");
    int no_of_files = scandir(path, &filelist, filter_func, alphasort);
    int i;
    for(i=0;i<no_of_files;++i)
    {
    	printf("%s\t",filelist[i]->d_name);
    	free (filelist[i]);
    }
    free(filelist);
    printf("\n");


}
char *permissions(mode_t mode)/*prints permissions in ls -l command*/
{
 
    char ftype;
    if (S_ISFIFO(mode)) ftype = '|';
    else if (S_ISCHR(mode)) ftype = 'c';
    else if (S_ISBLK(mode)) ftype = 'b';
    else if (S_ISDIR(mode)) ftype = 'd';
    else if (S_ISLNK(mode)) ftype = 'l';
    else if (S_ISREG(mode)) ftype = '-';
    else ftype='?';
    char *permission_list;
    permission_list=(char *)(malloc(14*sizeof(char)));
    permission_list[0]=ftype;
    permission_list[1]=mode & S_IRUSR ? 'r' : '-';
    permission_list[2]=mode & S_IWUSR ? 'w' : '-';
    permission_list[3]=mode & S_IXUSR ? 'x' : '-';
    permission_list[4]=mode & S_IRGRP ? 'r' : '-';
    permission_list[5]=mode & S_IWGRP ? 'w' : '-';
    permission_list[6]=mode & S_IXGRP ? 'x' : '-';
    permission_list[7]=mode & S_IROTH ? 'r' : '-';
    permission_list[8]=mode & S_IWOTH ? 'w' : '-';
    permission_list[9]=mode & S_IXOTH ? 'x' : '-';
    permission_list[10]=' ';
    permission_list[11]=mode & S_ISUID ? 'U' : '-';
    permission_list[12]=mode & S_ISGID ? 'G' : '-';
    permission_list[13]=mode & S_ISVTX ? 'S' : '-';

    return permission_list; 
}
char *preprocess(char *buffer)/*May be used to preprocess strings without spaces*/
{
    int i,j;
    int l = strlen(buffer) + 1;
    char* dest  =  (char*)malloc(8192*sizeof(char));
    char* ptr=dest;
    i = 0;
    *ptr++ = buffer[i];
    for(i=1; i<l; i++)
    {
        if( (buffer[i]=='<' || buffer[i] == '>' || buffer[i] == '|' ) && (buffer[i-1] != ' '))
        {
            *ptr++ = ' ';
        }

        if((buffer[i-1]=='<' || buffer[i-1] == '>' || buffer[i-1] == '|' ) && (buffer[i] != ' '))
        {
            *ptr++ = ' ';
        }
        *ptr++ = buffer[i];
    }
    *ptr = '\0';
    return dest;
}
void implement_ls_l()/*implementation of ls -l*/
{
    struct dirent **filelist;
	struct stat     statbuf;
	struct passwd  *pwd;
	struct group   *grp;
	struct tm      *tm;
	char            datestring[256];
	char path[BUFSIZE];
	if(!getcwd(path, sizeof(path)))
       perror("Error: ");
	int count = scandir(path, &filelist, filter_func, alphasort);
	int i=0;
	printf("Total=%d\n",count);
    while (i<count) 
    {

    	
    	/* Get entry's information. */
    	if (stat(filelist[i]->d_name, &statbuf) == -1)
    	    continue;


    	/* Print out type, permissions, and number of links. */
    	printf("%15.15s", permissions (statbuf.st_mode));
    	printf("%4d",(int) statbuf.st_nlink); 


    	/* Print out owner's name if it is found using getpwuid(). */
    	if ((pwd = getpwuid(statbuf.st_uid)) != NULL)
    	    printf(" %-8.8s", pwd->pw_name);
    	else
    	    printf(" %-8d", statbuf.st_uid);


    	/* Print out group name if it is found using getgrgid(). */
    	if ((grp = getgrgid(statbuf.st_gid)) != NULL)
    	    printf(" %-8.8s", grp->gr_name);
    	else
    	    printf(" %-8d", statbuf.st_gid);

	    /* Print size of file. */
	    printf(" %9jd", (intmax_t)statbuf.st_size);


    	tm = localtime(&statbuf.st_mtime);


    	/* Get localized date string. */
    	strftime(datestring, sizeof(datestring), nl_langinfo(D_T_FMT), tm);


    	printf(" %s %s\n", datestring, filelist[i]->d_name);
    	++i;
	}	
    


	
}
void implement_cp(char *f1,char *f2)/*implementatio of command cp*/
{

	struct stat stat1,stat2;
	
	if( access( f1, F_OK ) == -1 ) {
        perror("Readfile non existent");
         return;
    }
    else if( access( f1, R_OK ) == -1 )
    {
         perror("No read permission on file 1");
         return;
    }
    else if( access( f2, W_OK ) == -1  &&  access( f2, F_OK ) != -1 )
    {
         perror("No write permission on file2");
         return;
    }
    else
    {
        stat(f1, &stat1);
        stat(f2, &stat2);
         // Time stamp comparison
        if(difftime(stat1.st_mtime,stat2.st_mtime)<0)
            {
            	printf("Cannot copy:the last modification time of file1 is not more recent than that of file2\n");
            	return;
            }

        int ifd = open(f1, O_RDONLY);
        int ofd = open(f2, O_CREAT | O_WRONLY |O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH | S_IXOTH);
        int no_read;char input[BUFSIZE];
        while(1)
        {
        	no_read=(read(ifd,input,BUFSIZE));
        	if(!no_read)
        		break;
        	if( write(ofd,input,no_read)<no_read)
        	{
        		perror("Error in file copy");
        	}
        }

    }

}



void implement_executables(char **tokenlist)/*implementation of executable commands*/
{
	int fd=-1;
	int background_proc_flag=0;//flag for background process .1 if it is to be made a background process
	

	
	
	char *word=strdup(tokenlist[0]);
	int i=0;



	/*check for & (background process)*/
	while(tokenlist[i])
	{
		
		if(strcmp(tokenlist[i],"&")==0 )
		{
			background_proc_flag=1;
			tokenlist[i]=NULL;

			break;
		}	
		++i;
		
	}
	
int ifd,ofd;int flaginp=0,flagop=0,flagpipe=0;int pip_in[100];//max 100 pipes allowed
	for(i=0;tokenlist[i]!=NULL ;++i){
		if(strcmp(tokenlist[i],"<")==0 && tokenlist[i+1])//input file
		{
			ifd=open(tokenlist[i+1],O_RDONLY);
			if (ifd < 0) {
				//printf("%s\n",tokenlist[i+1] );
						perror("Unable to open input file in read mode...\n");
						exit(1);
					}
					flaginp=1;
		}
		if(strcmp(tokenlist[i],">")==0 && tokenlist[i+1])//output file
		{
				ofd = open(tokenlist[i+1], O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

			if (ofd < 0) {
						perror("Unable to open output file in write mode...\n");
						exit(1);
					}
					flagop=1;
		}
		if(strcmp(tokenlist[i],"|")==0 && tokenlist[i+1])//pipe
		{
			pip_in[flagpipe]=i;
			flagpipe++;
		}
		
	}

	if(flagpipe==0)//if there are no pipes
	{
		int id=fork();
		if(id==0)
		{
			if(flaginp)
			{
				close(0);// close stdin
				dup(ifd);//make file as stdin
			}
			if(flagop)
			{
				close(1);// close stdout
				dup(ofd);// make file as stdout
			}
			execvp(tokenlist[0],tokenlist);
			perror("Execvp failed");
		}
		else if(background_proc_flag==0){
							wait(NULL);
		}
		else
		{
			umask(0);
			if (setsid()==-1) 
			{
    			//perror("failed to make background process");
			}
		}

	}
	else
	{
		//pipe implementation

		int pipes[100][2];
		if(flagpipe>100)
		{
			printf("Too many  pipes!!\n");
			return;
		}
		for(int i1=0;i1<flagpipe;++i1)
		{
			if( pipe(pipes[i1])==-1)
			{
		  		perror("Error in creating pipe ");
		  		return;
			}


		}
		int id,pipe_n=0;
		int j=0;

		while(1)
		{
			if(tokenlist[j] == NULL) break;
    	    if(strcmp(tokenlist[j], "|") == 0)
    	    {
        		id = fork();
	  			if(id == -1)
	  			{
	    
	    			perror("Error in forking ");
	    			return;
	  			}
	  			else if(id == 0)
	  			{
	    
	    			tokenlist[j]=NULL;
            		if(pipe_n != 0)
            		{
	     
	      				if(dup2(pipes[pipe_n-1][0],0)==-1)//close stdin ,make output of previous pipe the standard input
	      				{
							perror("Error in dup2  "); 
							exit(0);
	      				}
	    			}
	   
	   				close(pipes[pipe_n][0]);
	   				

	    			if(dup2(pipes[pipe_n][1],1)==-1)//close stdout,make input of next pipe the stdout
	    			{
              			perror("Error in dup2 x "); 
              			exit(0);
	    			}
	    			if( execvp(tokenlist[0], tokenlist)==-1)
	    			{
	      			perror("Command not found");
	      			exit(0);
	     
	    			}
	    			
	  			}
	  			else 
	  			{
	  				close(pipes[pipe_n][1]);
	  				
							
							tokenlist += j+1;
	    			j = -1;
	    			pipe_n++;
	    			wait(NULL);

				}	
			}
			j++;
		}
		id=fork();
		if(id==0)
		{
      		if(dup2(pipes[pipe_n-1][0],0))
      		{
				perror("Error in dup2  "); 
				exit(0);
    		}
    		if(execvp(tokenlist[0], tokenlist)==-1)
    		{
				perror("Command not found");
				exit(0);
      		}
		}
		else
			wait(NULL);
      	

    }
  


}




void parse(char * input,char *tokenlist[])//parse string input
{
	
	
	
	 char* temp1 = (char*)malloc(sizeof(input));
	 strcpy(temp1,input);
	 	 
	 char *word=strtok(temp1," \n\t");
	 int n=0;
	 while(word)
	 {
	 	word=strtok(NULL," \n\t");
	 	++n;
	 }
	 free(word);

	 word=strtok(input," \n\t");
	 
	 int i,j;
	 for(i=0;word;++i)
	 {
	 	if(word==NULL)
	 		break;
	 	
	 	
	 	tokenlist[i]=(char *)malloc(strlen(word)*sizeof(char));
	 	strcpy(tokenlist[i],word);
	 	
	 	word=strtok(NULL," \n\t");
	 
	 	
	 }
	
	 tokenlist[n]=NULL;
	 
	 return;
	
}

int check_for_system_calls(char buffer[BUFSIZE],char *tokenlist[])//check for the system calls
{
	int background_proc_flag=0;//flag for background process .1 if it is to be made a background process
	

	
	
	
	int i=0;



	/*check for & (background process)*/
	while(tokenlist[i])
	{
		if(strcmp(tokenlist[i],"&")==0 )
		{
			background_proc_flag=1;
			tokenlist[i]=NULL;
			break;
		}	
		++i;
		
	}
	
	char *substring,*substring2,*substring3;
	substring=strtok(buffer," \n");
	if(substring==NULL)
	{
		return -2;
	}
	if(strcmp(substring,"cd")==0)
	{
		substring=strtok(NULL," \n");
		if(background_proc_flag==1)
		{
		int id=fork();
			if(id==0)
			{
				implement_cd(substring);
				exit(0);
					
			}
			else
			{
				umask(0);
				if (setsid()==-1) 
				{
    				//perror("failed to make background process");
				}
			}
		}
		
		else
		implement_cd(substring);
			return 1;

	}
	else if(strcmp(substring,"pwd")==0)
	{
		if(background_proc_flag==1)
		{
		int id=fork();
			if(id==0)
			{
				implement_pwd();
				exit(0);
					
			}
			else
			{
				umask(0);
				if (setsid()==-1) 
				{
    				//perror("failed to make background process");
				}
			}
		}
		
		else
		implement_pwd();
			return 1;

	}
	else if(strcmp(substring,"mkdir")==0)
	{
		substring=strtok(NULL," \n");
		if(background_proc_flag==1)
		{
		int id=fork();
			if(id==0)
			{
				implement_mkdir(substring);
				exit(0);
					
			}
			else
			{
				umask(0);
				if (setsid()==-1) 
				{
    				//perror("failed to make background process");
				}
			}
		}
		
		else
		implement_mkdir(substring);
			return 1;

	}
	else if(strcmp(substring,"rmdir")==0)
	{
		substring=strtok(NULL," \n");

		if(background_proc_flag==1)
		{
			int id=fork();
			if(id==0)
			{
				implement_rmdir(substring);
				exit(0);
					
			}
			else
			{
				umask(0);
				if (setsid()==-1) 
				{
    				//perror("failed to make background process");
				}
			}
		}
		else
			implement_rmdir(substring);
			return 1;

	}
	else if(strcmp(substring,"ls")==0)
	{
		substring=strtok(NULL," \n");
		if(substring==NULL)
		{
			//implementation for ls
			implement_ls();

  		}
		else if(strcmp(substring,"-l")==0)
		{
			substring=strtok(NULL," \n");
			if(substring==NULL )
			{
				//implementation for ls -l
				implement_ls_l();
			}
			else if(strcmp(substring,"&")==0 && strtok(NULL," \n")==NULL)
			{
				int id=fork();
				if(id==0)
				{
					implement_ls_l();
					exit(0);
				}
				else
				{
					umask(0);
					if (setsid()==-1) 
					{
    				//perror("failed to make background process");
					}
				}

			}
			else
			{
				
				perror("ls -l and ls are only supported options");

			}

		}
		else if(strcmp(substring,"&")==0 && strtok(NULL," \n")==NULL)
				{
					int id=fork();
					if(id==0)
					{
						implement_ls();
						exit(0);

					}
					else
					{
						umask(0);
						if (setsid()==-1) 
						{
    					//perror("failed to make background process");
						}

					}
				}
		else
		{
			perror("ls -l and ls are only supported options");
		}
			return 1;

		
	}
	if(strcmp(substring,"cp")==0)
	{
		substring=strtok(NULL," \n");char *substring4;
		if(!substring)
		{
			perror("Invalid format for cp");
		}

		else if(substring2=strtok(NULL," \n"))
		{
			if(substring4=strtok(NULL," \n"))
			{
				
				if( strtok(NULL," \n"))
				printf("Invalid format for cp\n");

				if(strcmp(substring4,"&")==0)
				{
					int id=fork();
					if(id==0)
					{
						implement_cp(substring,substring2);
						exit(0);
					}
					else
					{
						umask(0);
						if (setsid()==-1) 
						{
    					//perror("failed to make background process");
						}

					}
				}
				else
				{
					printf("Invalid format for cp\n");
				}

			}
			else
				implement_cp(substring,substring2);

		}
		else
		{
			printf("Invalid format for cp\n");

		}
			return 1;

	}
	else if(strcmp(substring,"exit")==0)
	{
		implement_exit();
		exit(0);
	}
	else return 0;



}

int main()
{
	char current_dir[BUFSIZE];
	char buffer[BUFSIZE];
	char temp[BUFSIZE];
	char *args[BUFSIZE];
   
   
    int i;
    int stdin_copy = dup(0);
    int stdout_copy = dup(1);

	while(1)
	{	

		if(getcwd(current_dir,sizeof(current_dir)))
		{
						printf("%s>",current_dir);//print working directory >

		}
		else
			perror("current directory error");
		fgets(buffer,BUFSIZE,stdin);

		strcpy(temp,buffer);
		char *tokenlist[50];
		parse(buffer,tokenlist);

		char *tokenlist1[50];
		int h=0;
		while(tokenlist[h])
		{
			tokenlist1[h]=strdup(tokenlist[h]);
			++h;
		}
		int z=check_for_system_calls(temp,tokenlist1);
		
		if(z==0)// if not a system call
		{
		
			
	
			
			
			
			implement_executables(tokenlist);
			
			

		}
	}


}