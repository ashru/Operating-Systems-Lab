#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>	/* Include this to use semaphores */
#include <sys/shm.h>	
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

                       
#define wait_sem(s) semop(s, &pop, 1)  /* pop is the structure we pass for doing
				   the wait(s) operation */
#define signal_sem(s) semop(s, &vop, 1)  /* vop is the structure we pass for doing
				   the signal(s) operation */


int full, empty,mutex ;
int buffer_index,shared_buffer,read_count,sum;
struct sembuf pop, vop ;





void producer()
{

	int i;
	int *index = shmat(buffer_index,0,0);
	int *mybuf = shmat(shared_buffer,0,0);


	for(i=1;i<=50;++i)
	{

		wait_sem(empty);
		wait_sem(mutex);
		//write i to buffer,increment index
		
		mybuf[index[0]]=i;

		
		index[0]++;
		
		signal_sem(mutex);
		signal_sem(full);
	}

	shmdt(index);//detach memory
	shmdt(mybuf);
	
}



void consumer(int m)
{


	int *index = shmat(buffer_index,0,0);
	int *mybuf = shmat(shared_buffer,0,0);
	int *countread = shmat(read_count,0,0);
	long long int *mysum = shmat(sum,0,0);
	
	while(countread[0]<m*50)//while read counter<m*50
	{
		wait_sem(full);
		if(countread[0]>=m*50)
		{
			signal_sem(full);//if all values read unblock process waiting to read
			signal_sem(mutex);//if all values read unblock process waiting to read

		}
		wait_sem(mutex);
		//decrement index,read  from buffer,add to sum,increment read count
		if(countread[0]>=m*50)
		{
			break;//if all values read break out

		}
		index[0]--;
		int k = mybuf[index[0]];
		mysum[0] = mysum[0]+k;
		//printf("%d\n",mysum[0]);
		countread[0]++;
		if(countread[0]>=m*50)
		{
			signal_sem(full);//if all values read unblock process waiting to read
		}
		signal_sem(mutex);
		signal_sem(empty);
	}
	shmdt(index);//detach memory
	shmdt(mybuf);
	shmdt(countread);
	shmdt(mysum);
}



int main()
{
	//create shared memory for buffer of 20 variables
	shared_buffer = shmget(IPC_PRIVATE, 20*sizeof(int), 0777|IPC_CREAT);
	// shared memory for index of buffer
	buffer_index = shmget(IPC_PRIVATE, 1*sizeof(int), 0777|IPC_CREAT);
	//shared memory for variable sum
	sum = shmget(IPC_PRIVATE, 1*sizeof(long long int), 0777|IPC_CREAT);
	//shared memory for variable storing the number of total values that have been read already
	read_count = shmget(IPC_PRIVATE, 1*sizeof(int), 0777|IPC_CREAT);
	//create semaphores full, empty and mutuallyexclusive
	full = semget(IPC_PRIVATE, 1, 0777|IPC_CREAT);
	empty = semget(IPC_PRIVATE, 1, 0777|IPC_CREAT);
	mutex = semget(IPC_PRIVATE, 1, 0777|IPC_CREAT);


	int m,n;
	printf("Enter the number of producers=");
	scanf("%d",&m);
	printf("Enter the number of consumers=");
	scanf("%d",&n);
	int i;int *q,*r;long long int *p;
	int *id1=(int *)(malloc(m*sizeof(int)));
	int *id2=(int *)(malloc(n*sizeof(int)));
	semctl(full, 0, SETVAL, 0);//initializing semaphores
	semctl(empty, 0, SETVAL, 20);
	semctl(mutex, 0, SETVAL, 1);
	pop.sem_num = vop.sem_num = 0;//defining operations
	pop.sem_flg = vop.sem_flg = 0;
	pop.sem_op = -1 ; vop.sem_op = 1 ;
	p = (long long int *)shmat(sum, 0, 0);//attaching semaphores to memory
	p[0]=0;
	shmdt(p);//detach memory
	q= (int *)shmat(buffer_index,0,0);
	q[0]=0;
	shmdt(q);//detach memory
	r= (int *)shmat(read_count,0,0);
	r[0]=0;
	shmdt(r);//detach memory
	
		for(i=0;i<m;++i)
		{
			id1[i]=fork();
			if(id1[i]==0)
			{

				producer();//call producer function

				exit(0);

				
			}

		}
		
		for(i=0;i<n;++i)
		{
			id2[i]=fork();
			if(id2[i]==0)
			{
				consumer(m);//call consumer function
				exit(0);
			}
		}
		int y;
		for(i=0;i<m;++i)
			waitpid(id1[i],&y,0);
		for(i=0;i<n;++i)
			waitpid(id2[i],&y,0);
	
		long long int *result=(long long int *)shmat(sum,0,0);
		printf("Sum=%lld\n",result[0]);
		shmdt(result);
		//print sum
		semctl(full, 0, IPC_RMID, 0);//remove semaphores and shared memory
		semctl(empty, 0, IPC_RMID, 0);
		semctl(mutex, 0, IPC_RMID, 0);
		shmctl(read_count, IPC_RMID, 0);
		shmctl(shared_buffer, IPC_RMID, 0);
		shmctl(buffer_index, IPC_RMID, 0);
		shmctl(sum, IPC_RMID, 0);
		return 0;
	
	
}