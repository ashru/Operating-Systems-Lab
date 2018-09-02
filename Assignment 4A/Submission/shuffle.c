#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>

struct thread_data
{
	int thread_index;//index of thread stored in thread data
	int thread_count;//count of no of rows or columns the thread  needs to shift
};
int global_counter=0;// global counter variable
pthread_t *thread_id;// thread pointer
int k,x,n;// global variables k,x,n
int **matrix;// global matrix
pthread_mutex_t mutex;//the mutex
pthread_cond_t condition_var;//the condition variable
/*

Synchronization between threads:
No thread can start a column shift until all threads have
finished a row shift.Also no thread can start a row shift until all threads have finished the previous column shift.
I have used 1 mutex variable and 1 condition variable.
I have kept a global counter variable initialized to zero. After every thread shifts a set of columns or rows by 1 place 
 this counter increments by 1.Since there are x threads, in the loop for l+1th shift I make a thread wait till wait until 
 global counter=2*l*x(as l*2 row+column shifts(l for row+l for column) for x threads). 
 After shifting row ,iand incremeting the counter finds that the global counter has reached value x+2*l*x it signals all
  the other threads(Since this value indicates each of x processes have shifted l+1 set of rows
 and l set of columns). Now the thread waits till the global counter reaches this value of x+2*l*x before it can shift column.
 Again after shifting column and incrementing counter if the thread finds counter has reached value 2*x+2*l*x it signals all other threads 
 as it incdicates all threads have finished shifting l+1 set of rows and l+1 set of columns
*/
void *start_routine(void *parameter)
{
	struct thread_data *TD=(struct thread_data *) parameter;//thread data
	int index=(*TD).thread_index;//index of thread stored in thread data
	int count=(*TD).thread_count;//count of no of rows or columns the thread  needs to shift
	int i,j,l=0;

	while(l<k)// no of iterations ie shuffles
	{
		pthread_mutex_lock(&mutex);//lock mutex

		while (global_counter <2*l*x)
		{

			pthread_cond_wait(&condition_var, &mutex);// wait till all processes havefinished shifting previous columns
		}
		pthread_mutex_unlock(&mutex);// unlock mutex
		for(i=(n/x)*index;i<(n/x)*index+count;++i)//shift rows
		{
			int temp=matrix[i][0];
			for(j=0;j<n-1;++j)
				matrix[i][j]=matrix[i][(j+1)%n];
			matrix[i][n-1]=temp;


		}

		pthread_mutex_lock(&mutex);//lock mutex
		global_counter++;//increment counter

		if(global_counter == (x)+2*(l)*x )
		{
			pthread_cond_broadcast(&condition_var);// signal all threads that all processes have finished shifting rows
		}
		// Unlock the mutex
		pthread_mutex_unlock(&mutex);
				
		pthread_mutex_lock(&mutex);

		while (global_counter <2*l*x+x)
		{

			pthread_cond_wait(&condition_var, &mutex);// wait till all processes have finished shifting previous rows
		}
		pthread_mutex_unlock(&mutex);
		

		for(i=(n/x)*index;i<(n/x)*index+count;++i)// shift columns
		{
			int temp=matrix[n-1][i];
			for(j=n-1;j>0;--j)
				matrix[j][i]=matrix[(j-1)%n][i];
			matrix[0][i]=temp;

		}
		//wait for all column shifts
		pthread_mutex_lock(&mutex);
		global_counter++;// increment counter

		
		
		if(global_counter == (2*x)+2*(l)*x )
		{
			pthread_cond_broadcast(&condition_var);// signal all threads that all processes have finished shifting columns
		}

		// Unlock the mutex

		pthread_mutex_unlock(&mutex);
		++l;
	}

}
int main()
{

	printf("Enter n:");
	scanf("%d",&n);
	matrix=(int **)(malloc(n*sizeof(int *)));
	int i,j;
	for(i=0;i<n;++i)
		matrix[i]=(int *)(malloc(n*sizeof(int)));// dynamic allocation of memory for matrix
	for(i=0;i<n;++i)
		for(j=0;j<n;++j)
			scanf("%d",&matrix[i][j]);
	printf("\nInitial Matrix\n\n");
	for(i=0;i<n;++i)
	{
		for(j=0;j<n;++j)
			printf("%d ",matrix[i][j]);
		printf("\n");
	}
	printf("Enter k:");
	scanf("%d",&k);
	printf("Enter x:");
	scanf("%d",&x);
	thread_id=(pthread_t *)(malloc(x*sizeof(pthread_t)));// dynamic allocation of memory for threads
	struct thread_data *TD;
	TD=(struct thread_data*)(malloc(x*sizeof(struct thread_data)));
	pthread_mutex_init(&mutex, NULL);// initialize mutex
	pthread_cond_init(&condition_var, NULL);// initialize condition variable
	for(i=0; i<=x-1; i++)//for all threads set index and count
	{
		 TD[i].thread_index = i;
		 if(i==x-1)
		 	TD[i].thread_count=n/x+n%x;
		 else
		 	TD[i].thread_count=n/x;
		 pthread_create(&thread_id[i], NULL, start_routine, (void *) &TD[i]);// create threads
	}
	for(i=0; i<x; i++)//wait till all threads terminate
	{
		pthread_join(thread_id[i], NULL);
	}
	printf("\nFinal Matrix\n\n");//print final result
	for(i=0;i<n;++i)
	{
		for(j=0;j<n;++j)
			printf("%d ",matrix[i][j]);
		printf("\n");
	}
	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&condition_var);


	return 0;
}
