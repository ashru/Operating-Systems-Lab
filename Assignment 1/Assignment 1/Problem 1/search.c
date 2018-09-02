#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>


int main()
{
	char filename[1000];
	printf("Enter the filename:\n");
	scanf("%s",filename);
    FILE *file = fopen(filename, "r");
    int A[1000000];// I have assumed that maximum number of integers in file will not exceed 1000000
    int i=0,num,k,n,mid,j,low,high,m;
    while(fscanf(file, "%d", &num) > 0) //Input from file
    {
        A[i] = num;
        i++;
    }
    fclose(file);

    int success=0,init_id;
    init_id=getpid();//save the id of the initial process.It shall be required later on
    
    low=0;//lower index for subarray to search
    high=i-1;//higher index for subarray to searh
    while(1)
    {
    	printf("Enter k:");
    	scanf("%d",&k);
    	if(k<=0)
    		exit(0);
    	int flag=0;
    	while(1)
    	{
    		if((high-low)<=9)//if number of elements in array are not greater than 10
    		{
    			
    			int flag=0;
    			for(m=low;m<=high;++m)//linear search
    			{
    				if(A[m]==k)
    				{
    					flag=1; 
    					break;

    				}
    				
    			}
    			
    			if(getpid()==init_id)//if initial process id matches current, ie array has less than 10 elements, print the result and break
    			{
    				if(flag==0)
    					printf(" Not found\n");
    				else printf(" Found\n");
    				
    				break;
    			}
    			else//else exit with return value flag which is 1 if found and 0 otherwise
    			{
    				exit(flag);
    			}


    		}
    		else//if no of elements >10
    		{
    			
    			int id[2],status[2];//id and status variables for 2 children processes
    			id[0]=0;
    			id[1]=0;
    			for(j=0;j<2;++j)
    			{
    				id[j]=fork();//creating child process
    				
    				if(id[j]==0)
    				{
    					if(j==0)
    					{
    						low=low;
    						high=low+(high-low)/2;//for 1st child process, the first half of array is considered
    					}
    					else
    					{
    						high=high;
    						low=low+(high-low)/2+1;//for 2nd child process, the second half of array is considered
    						
    					}   	
    					break;				
    						
    				}  				
    				
    			}
    			if(id[0]!=0 && id[1]!=0)//if it is the parent process
    			{
    				waitpid(id[0],&status[0],0);//wait till both the children processes get over
    				waitpid(id[1],&status[1],0);
    				if(getpid()!=init_id)//if it is not the initial process
    				{

    					if(status[0]==0 && status[1]==0)// if element is not found in either subarray,return 0
    					{
    						
    						
    						exit(0);

    					}
    					else //otherwise return 1
    					{
    						
    						exit(1);
    					}
    				}
    				else// if it is the initial process,print the result and move on to next iteration
    				{
    					if(status[0]==0 && status[1]==0)
    						printf(" Not found\n");
    					else printf(" Found\n");
    					break;
    				}
    			}
    		}
    		



    	}
    	


    }
   

}