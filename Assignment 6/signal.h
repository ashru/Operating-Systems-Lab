#ifndef THREADS_SIGNAL_H
#define THREADS_SIGNAL_H


#include <debug.h>
#include <list.h>
#include <stdint.h>
#include "threads/malloc.h"

enum signumber
{
	SIG_CHLD=0,
	SIG_CPU ,
 	SIG_UNBLOCK ,
 	SIG_USER ,
 	SIG_KILL 

};
enum sig_flags
{
	SIG_DFL=0,
	SIG_IGN
};
enum sigpm_flags
{
	SIG_BLOCK,
	SIG_UNBLCK,
	SIG_SETMASK

};
struct signal_book
{
	enum signumber signum;
	tid_t sender;
	tid_t receiver;
	struct list_elem sigelem;
};
int sig_stat[5];                     // status of signals.init to 0 if masked set to 0
void signal(enum signumber signo,enum sig_flags flag);
int kill(tid_t rec,enum signumber signo);
void killcpu(tid_t tid);
int sigemptyset(int *set);
int sigfillset(int *set);
int sigaddset(int *set,int signum);
int sigdelset(int *set,int signum);
int sigprocmask(enum sigpm_flags how,int *set,int *oldset);
void killhelper();
void chldhelper();
void userhelper(tid_t sender);
void cpuhelper(void);
void unblockhelper(struct thread *t);
struct thread *get_thread(tid_t thread_id);


int killchecker(tid_t sender, tid_t rec);

extern struct list all_list;

void killchild(tid_t tid);

void update_parent(struct thread *t);
// struct list chld_list;
// struct list cpu_list;
struct list unblock_list;
// struct list user_list;
// struct list kill_list;

struct list other_list;



int sighandler_SIG_KILL_DFL();
int sighandler_SIG_CPU_DFL();
int sighandler_SIG_CHLD_DFL();
int sighandler_SIG_USER_DFL(tid_t sender);
int sighandler_SIG_UNBLOCK_DFL(tid_t receiver);


#endif