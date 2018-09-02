#include <stdio.h>
#include <stdlib.h>
int main(int argc, char *argv[])
{
	
	
	if (argc != 2)
	{
		printf("usage: sort1 <filename>\n");
		exit(-1);
	}
	FILE *file = fopen(argv[1], "r");
    int A[1000];// maximum number of integers in file will not exceed 1000
    int i=0,j,min,num,n,temp;
    while(fscanf(file, "%d", &num) > 0) //Input from file
    {
        A[i] = num;
        i++;
    }
    fclose(file);
    n=i;
    for(i=1;i<=n-1;++i)//Insertion sort
    {
    	j=i;
    	while(j>0 && A[j]<A[j-1])
    	{
    		temp=A[j];
    		A[j]=A[j-1];
    		A[j-1]=temp;
    		--j;
    	}
    }
    for(i=0;i<n;++i)
    	printf("%d\n",A[i]);
}

 