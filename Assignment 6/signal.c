#include "threads/thread.h"
#include <debug.h>
#include <stddef.h>
#include <random.h>
#include <stdio.h>
#include <string.h>
#include "threads/flags.h"
#include "threads/interrupt.h"
#include "threads/intr-stubs.h"
#include "threads/palloc.h"
#include "threads/switch.h"
#include "threads/synch.h"
#include "threads/vaddr.h"
#include "threads/signal.h"



void signal(enum signumber signum,enum sig_flags flag)//signal function
{
	if(signum==SIG_KILL)//no action for kill
	{
		return;
	}
	if(flag==SIG_DFL)//set mask for others
	{
		int k=15-1<<signum;
		int r=thread_current()->sigmask;
		r=r&k;
		thread_current()->sigmask=r;
		return;
	}
	int k=1<<signum;
	int r=thread_current()->sigmask;
	r=r|k;
	thread_current()->sigmask=r;
	return;
}
int kill(tid_t rec,enum signumber signum)//kill function.appropriately queue signals to deliver later
{

	struct thread *rt;

	if(!list_empty(&all_list)){
		struct list_elem *head = list_begin(&all_list);
	    struct list_elem *end = list_end(&all_list);
	    struct list_elem *temp = head;
	    while (temp != end){
	    	
	      struct thread *ttemp = list_entry (temp, struct thread, allelem);
	      if(ttemp)
	      {
	      	if (ttemp->tid == rec){
	      		rt=ttemp;
	      		break;
	       	} 
	  	   }

	      temp = list_next(temp);
	  	}
	}
	
	if(SIG_KILL==signum)
	{

		if(killchecker(thread_current()->tid,rec)==0)
			return -1;
		else
		{
		struct list_elem *head = list_begin(&other_list);
    	struct list_elem *end = list_end(&other_list);
    	struct list_elem *temp = head;
    	while (temp != end){
    	
    	  struct signal_book *ttemp = list_entry (temp, struct signal_book, sigelem);
    	  if(ttemp)
    	  {
      		if (ttemp->receiver == rec){
      		ttemp->sender=thread_current()->tid;
      		return;
       	} 
  	   }

      temp = list_next(temp);
  	}
			//queue up the kill signal appropriately
			struct signal_book *x = NULL;
			x = (struct signal_book *)malloc(sizeof(struct signal_book));
			x->signum = signum;
			x->sender = thread_current()->tid;
			x->receiver = rec;

			list_push_back(&other_list,&x->sigelem);
			
		}
	}
	else if(SIG_UNBLOCK == signum)
	{


		struct list_elem *head = list_begin(&unblock_list);
    	struct list_elem *end = list_end(&unblock_list);
    	struct list_elem *temp = head;
    	while (temp != end){
    	
    	  struct signal_book *ttemp = list_entry (temp, struct signal_book, sigelem);
    	  if(ttemp)
    	  {
      		if (ttemp->receiver == rec){
      		ttemp->sender=thread_current()->tid;
      		return;
       	} 
  	   }

      temp = list_next(temp);
  	}
		struct signal_book *y = NULL;
		y = (struct signal_book *)malloc(sizeof(struct signal_book));
		y->signum = signum;
		y->sender = thread_current()->tid;
		y->receiver = rec;

		list_push_back(&unblock_list,&y->sigelem);


	}
	else if(SIG_USER == signum){
			struct list_elem *head = list_begin(&other_list);
    	struct list_elem *end = list_end(&other_list);
    	struct list_elem *temp = head;
    	while (temp != end){
    	
    	  struct signal_book *ttemp = list_entry (temp, struct signal_book, sigelem);
    	  if(ttemp)
    	  {
      		if (ttemp->receiver == rec){
      		ttemp->sender=thread_current()->tid;
      		return;
       	} 
  	   }

      temp = list_next(temp);
  	}
		struct signal_book *z = NULL;
		z = (struct signal_book *)malloc(sizeof(struct signal_book));
		z->signum = signum;
		z->sender = thread_current()->tid;
		z->receiver = rec;
		

		list_push_back(&other_list,&z->sigelem);
		
	}

	else
	{
		return -1;
	}
	return 0;


}
int sigemptyset(int *set)
{
	(*set)=0;
	return 0;
}
int sigfillset(int *set)
{
	(*set)=15;
	return 0;
}
int sigaddset(int *set,int signum)
{
	(*set)=(*set)|(1<<signum);
	return 0;
}
int sigdelset(int *set,int signum)
{
	int m=15-(1<<signum);
	(*set)=(*set) & m;
	return 0;
}
int sigprocmask(enum sigpm_flags how,int *set,int *oldset)
{
	if(oldset!=NULL)
	{
		(*oldset)=(*set);
	}
	if(set==NULL)
	{
		return 0;
	}
	if(how==SIG_BLOCK)
	{
		int r=thread_current()->sigmask;
		r=r|(*set);
		thread_current()->sigmask=r;
	}
	else if(how==SIG_UNBLCK)
	{
		int r=thread_current()->sigmask;
		int k=15-(*set);
		r=r & k;
		thread_current()->sigmask=r;

	}
	else if(how==SIG_SETMASK)
	{
		thread_current()->sigmask=(*set);

	}
	else
		return -1;
	return 0;


}
void killhelper()
{
		printf("SIGKILL:Exiting thread %d...\n",thread_current()->tid );
		thread_exit();
}
void chldhelper()
{
	int tmask=thread_current()->sigmask;
	if((tmask>>SIG_CHLD)&1)
		return;
	thread_current()->numchild--;
	printf("SIG_CHLD: received by Thread %d :Number of children created =%d \t No of children alive=%d\n",thread_current()->tid,thread_current()->totalchild,thread_current()->numchild);

}
void userhelper(tid_t sender)
{
	int tmask=thread_current()->sigmask;
	if((tmask>>SIG_USER)&1)
		return;
	printf("SIGUSR:Sent by thread %d to thread %d\n",sender,thread_current()->tid );

}
void cpuhelper()
{
	int tmask=thread_current()->sigmask;
	
	if((tmask>>SIG_CPU)&1)
		return;
	
	printf("SIGCPU:Thread %d Max lifetime=%d \n",thread_current()->tid,thread_current()->max_ttl);
	thread_exit();
}
void unblockhelper(struct thread *t)
{
	int tmask=t->sigmask;
	if((tmask>>SIG_UNBLOCK)&1)
		return;
	printf("SIG_UNBLOCK:Unblocking %d\n",t->tid);
	if(t->status == THREAD_BLOCKED)
		thread_unblock(t);
}



int killchecker(tid_t sender, tid_t rec)//check by thread ids whether one is parent of other
{
	if(list_empty(&all_list)){
		return 0;
	}
	struct thread *temp1;
	struct list_elem *head = list_begin(&all_list);
    struct list_elem *end = list_end(&all_list);
    struct list_elem *temp = head;
    while (temp != end){
    	
      struct thread *ttemp = list_entry (temp, struct thread, allelem);
      if(ttemp)
      {
      	if (ttemp->tid == rec){
      		temp1=ttemp;
      		break;
       	} 
  	   }

      temp = list_next(temp);
  	}
	
	
	if(temp1->parent->tid==sender)
			return 1;
	
		return 0;
}


void killchild(tid_t tid){  //helper for queing sig_child 
	
		struct list_elem *head = list_begin(&other_list);
    	struct list_elem *end = list_end(&other_list);
    	struct list_elem *temp = head;
    	while (temp != end){
    	
    	  struct signal_book *ttemp = list_entry (temp, struct signal_book, sigelem);
    	  if(ttemp)
    	  {
      		if (ttemp->receiver == tid){
      		ttemp->sender=thread_current()->tid;
      		return;
       	} 
  	   }

      temp = list_next(temp);
  	}
	struct signal_book *x = NULL;
	x = (struct signal_book *)malloc(sizeof(struct signal_book));
	x->signum = SIG_CHLD;
	x->receiver = tid;
	list_push_back(&other_list,&x->sigelem);
}




//Signal-handlers

int sighandler_SIG_KILL_DFL(){
	killhelper();
	return 0;
}


int sighandler_SIG_CPU_DFL(){
	cpuhelper();
	return 0;
}

int sighandler_SIG_CHLD_DFL(){
	chldhelper();
	return 0;
}

int sighandler_SIG_USER_DFL(tid_t sender){
	userhelper(sender);
	return 0;
}

int sighandler_SIG_UNBLOCK_DFL(tid_t receiver){

	struct thread *t = NULL;	
	if(!list_empty(&all_list)){
		struct list_elem *head = list_begin(&all_list);
	    struct list_elem *end = list_end(&all_list);
	    struct list_elem *temp = head;
		while (temp != end){
			struct thread *ttemp = list_entry (temp, struct thread, allelem);
			if(ttemp->tid == receiver){
				t = ttemp;
				break;
			}
			temp = temp->next;
	  	}   
  	}

  	if(t){
		unblockhelper(t);
		return 0;
	}
	else 
		return -1;
}


