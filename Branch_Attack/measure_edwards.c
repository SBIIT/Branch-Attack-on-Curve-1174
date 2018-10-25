#define _GNU_SOURCE 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <asm/unistd.h>
#include <linux/perf_event.h>
#include <time.h>
#include <assert.h>
#include <stdbool.h>



typedef unsigned long field_t;

#define WORDSIZE     16
#define SIZE	255
#define  WORDNUM ((SIZE+1)/WORDSIZE)
#define WORDLIMIT 2*WORDNUM + 1
#define MSB 16
#define BASE (1UL << MSB)
#define FIELD_P 7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff7
#define NUM 40000
#define LOOP(i,iter) for(i=0;i<iter;i++)
//
field_t CURVECONST_C[WORDLIMIT] = {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,16}; //1
field_t CURVECONST_D[WORDLIMIT] = {0xfb61, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0x7ff,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,16};
//7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffb61
field_t CURVE_PT_X[WORDLIMIT] = {0x9eda,0xbce2,0x3f27,0x1612,0xcd65,0x492e,0xd96a,0xc021,0xa190,0xc029,0xaee7,0x9343,0x8c47,0xea30,0xbb0c,0x37f,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,16};
// (0x37fbb0cea308c479343aee7c029a190c021d96a492ecd6516123f27bce29eda,
field_t CURVE_PT_Y[WORDLIMIT] = {0x360e,0x9b46,0xb1bf,0xa4cc,0x976b,0xaf3f,0xdee2,0x4fe2,0x0e0c,0x6984,0x8411,0x6656,0xb7cc,0xd47f,0x2f82,0x6b7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,16};
//0x6b72f82d47fb7cc6656841169840e0c4fe2dee2af3f976ba4ccb1bf9b46360e)
field_t CURVE_PT_Z[WORDLIMIT] = {0x1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,16};
field_t FP[WORDLIMIT] = {0xfff7, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0x7ff,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,16};
//7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff7



   // field_t k[WORDLIMIT]={0xffff,0x70f0,0xffff};
//bool k[1000] = {0,0,1,1,0,0,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1};




typedef struct _point_t{
	field_t x[WORDLIMIT];
	field_t y[WORDLIMIT];
	field_t z[WORDLIMIT];   // used only in projective coordinate system, else ignored
}point_t;
int temp[WORDNUM*WORDNUM + 4];
FILE * f1, * f2;
point_t * basepoint;
field_t t=0;
int trigger = 0;

void clear(int a[])
{
	int i;
	for(i=0;i<WORDNUM*WORDNUM;i++)
		a[i]=0;
}

void copy(int a[],int b[])
{
	int i;
	for(i=0;i<WORDNUM*WORDNUM;i++)
	{
		a[i]=b[i];
	}
}

void copy_element(field_t *d, field_t *s)
{
	int i;
	for(i=0;i<WORDLIMIT;i++)
        d[i]=s[i];
}

void init(field_t * a)
{
	int i;
	for(i=0;i<WORDLIMIT;i++)
		a[i]=0;
}

static void fillzeros(point_t *dest)
{
	memset(dest, 0, sizeof(point_t));
}

void copypoint(point_t *dest, point_t *src)
{
	memcpy(dest, src, sizeof(point_t));
}

static void print_element(field_t * x)
{
	int i;
	for(i=WORDLIMIT-1;i>=0;i--)
 		printf("%lx\t",x[i]);
	printf("\n");
}

static void printpoint(point_t * p)
{
	printf("x:");
	print_element(p->x);
	printf("\n");
	printf("y:");
	print_element(p->y);
	printf("\n");
	printf("z:");
	print_element(p->z);
	printf("\n");
}

#define ISEVEN(x)     ((x & 1ULL) == 0)

static bool less(field_t *a,field_t *b)
{
    	int i;
    	if(a[WORDLIMIT-1]<b[WORDLIMIT-1]){
        	return true;
    	}
    	else if(a[WORDLIMIT-1] == b[WORDLIMIT-1])
    	{
        	for(i=a[WORDLIMIT-1]-1;i>=0;i--)
        	{
           		if(a[i] < b[i]){
                		return true;
            		}
            		else if(a[i] > b[i]){
            			return false;
			}
			else
			{
			}
			
        	}
		return false;
    	}
    	else{
        	return false;
    	}
    
	
}

void static sub(field_t *res,field_t *a,field_t *b)
{
	int i,flag;
	field_t b_w=0;
	field_t val;
	field_t atemp[WORDLIMIT];
	init(res);
	init(atemp);

	copy_element(atemp,a);
    	int size = atemp[WORDLIMIT-1];
	for(i=0; i<size; ++i)
	{
	    	if(atemp[i]<b[i])
	    	{
	        	b_w = 1UL << MSB;
            		res[i] = atemp[i] + b_w -b[i];
            		flag = (atemp[i+1] == 0) ? 1: 0;
            		atemp[i+1] = (flag == 1) ? (1UL << MSB) - 1 : atemp[i+1] - 1;
            	}
        	else if(flag == 1)
        	{
            		b_w = 0;
            		res[i] = atemp[i] - b[i];
            		flag = (atemp[i+1] == 0) ? 1: 0;
            		atemp[i+1] = (flag == 1) ? (1UL << MSB) - 1 : atemp[i+1] - 1;
        	}
        	else
        	{
            		b_w = 0;
            		res[i] = atemp[i] - b[i];
            		flag = 0;
        	}
	}
	res[WORDLIMIT-1]=WORDNUM;

}

static void add(field_t *res, field_t *a, field_t *b)
{

	int i;
	field_t c_y=0;
	field_t val;
	for(i=0; i<WORDNUM; ++i)
	{
		val = a[i] + b[i] + c_y;
		res[i] = val%BASE;
		c_y = val/BASE;
	}
	if(c_y == 1)
	{
		res[WORDNUM] = 1;
		res[WORDLIMIT-1] = WORDNUM+1;
	}
	else
        	res[WORDLIMIT-1]=WORDNUM;

}

static void subp(field_t *res, field_t *a, field_t *b)
{

	field_t a_1[WORDLIMIT];
	init(a_1);
    	copy_element(a_1,a);

	if(less(a,b) == true) //a < b
	{
		//printf("\n^^^^^^^^^^^^^^^^^^^^^^^^less\n");
        	while(less(a_1,b)==true){
	    		add(a_1,a_1,FP); // a=a+P
        	}
	   	sub(res,a_1,b);
	}
	else // a > b
	{
	    sub(res,a,b);
	}
	res[WORDLIMIT-1]=WORDNUM;

}

static void addp(field_t * res, field_t * a,field_t * b)
{
	field_t res1[WORDLIMIT];
	init(res1);
	add(res,a,b);

	if(less(res,FP) == false) //res > P
	{
		//printf("here");
		copy_element(res1,res);
		sub(res,res1,FP);
	}
	res[WORDLIMIT-1]=WORDNUM;
}


static void mulreducep(field_t * b,field_t * a)
{
	field_t T[WORDLIMIT], S_1[WORDLIMIT], S_2[WORDLIMIT], S_3[WORDLIMIT], S_4[WORDLIMIT], S_5[WORDLIMIT], S_6[WORDLIMIT];
	field_t res1[WORDLIMIT], res2[WORDLIMIT];
	int x,i;

	init(T);
	init(S_1);
	init(S_2);
	init(S_3);
	init(S_4);
	init(res1);
	init(b);

	LOOP(i,WORDNUM-1)
		T[i] = a[i];
	T[WORDNUM-1] = a[WORDNUM-1]&(0x7ff);

	LOOP(i,WORDNUM)
		S_1[i] = (a[i + WORDNUM]&(0x7ff))*32 + ((a[i + WORDNUM - 1]&(0xf800)) >> 11);
	//S_1[WORDNUM - 1] = (a[WORDNUM - 1]&(0x7ff))*32 + ((a[WORDNUM - 2]&(0xf800)) >> 11);

	for(i = (WORDNUM-1); i>0; i--)
		S_2[i] = (S_1[i] & (0x1fff))*8 + ((S_1[i-1]&(0xe000)) >> 13);
	S_2[0] = (S_1[0] & (0x1fff))*8;

	add(res1,S_2,S_1);
	add(S_3,res1,T);

	init(S_1);init(S_2);
	LOOP(i,WORDNUM-1)
		S_1[i] = S_3[i];
    	S_1[WORDNUM-1] = S_3[WORDNUM-1]&(0x7ff);

	S_2[0] = ((S_3[WORDNUM-1] & (0xf800))>>11)*9;

	add(S_4,S_1,S_2);
	subp(S_5,S_4,FP);
	subp(S_6,S_5,FP);
	copy_element(b,S_6);
}

static void mulp(field_t *res, field_t *a, field_t *b)
{
	//print_element(a);
	field_t c_y,i,j,val,prev_val;
	field_t hamm_dist;
    	field_t tmp[WORDLIMIT];
    	field_t op1[WORDLIMIT],op2[WORDLIMIT];

    	init(tmp);
    	init(op1);
    	init(op2);
    	copy_element(op1,a);
    	copy_element(op2,b);
    	init(res);
    	t=0;
	for(i=0;i<WORDNUM;i++)
	{
		c_y = 0;
		for(j=0;j<WORDNUM;j++)
		{
			val = op1[i]*op2[j] + res[i+j] + c_y;
			prev_val = res[i+j] + c_y;
			
			res[i+j] = val%BASE;
			c_y = val/BASE;
            
        	}
		res[i+WORDNUM] = c_y;
	}

    	copy_element(tmp,res);	
	init(res);
	mulreducep(res,tmp);	
}

static int equal_ele(field_t *x,field_t *y)
{
	int i;
	LOOP(i,WORDLIMIT)
	{
		if(x[i] != y[i])
		return 0;
	}
	return 1;
}

static int equal(point_t p1,point_t p2)
{
	if((equal_ele(p1.x,p2.x) && equal_ele(p1.y, p2.y) && equal_ele(p1.z,p2.z)) == 1)
		return 1;
	else
		return 0;
}


static void pt_add_after(point_t *pt1, point_t *pt2,point_t *pt3)
{
	field_t T_1[WORDLIMIT], T_2[WORDLIMIT], T_3[WORDLIMIT], T_4[WORDLIMIT], T_5[WORDLIMIT], T_6[WORDLIMIT], T_7[WORDLIMIT], T_8[WORDLIMIT], T_9[WORDLIMIT], 
	S_1[WORDLIMIT], S_2[WORDLIMIT],S_3[WORDLIMIT], S_4[WORDLIMIT],S_5[WORDLIMIT], S_6[WORDLIMIT],S_7[WORDLIMIT],X_3[WORDLIMIT],Y_3[WORDLIMIT], Z_3[WORDLIMIT];
	    int j;
	    init(T_1);
	    init(T_2);
	    init(T_3);
	    init(T_4);
	    init(T_5);
	    init(T_6);
	    init(T_7);
	    init(T_8);
	    init(T_9);
	    init(S_1);
	    init(S_2);
	    init(S_3);
	    init(S_4);
	    init(S_5);
	    init(S_6);
	    init(S_7);
	    init(X_3);
	    init(Y_3);
	    init(Z_3);
	

	mulp(S_1,pt1->x,pt2->y); 
	mulp(S_4,pt2->x,pt1->y);
	mulp(S_2,pt1->x,pt2->x);
	mulp(S_3,pt1->y,pt2->y);
	mulp(T_1,pt1->z,pt2->z); 
	
	
	addp(T_2,S_1,S_4);
	mulp(S_5,T_1,T_1);
	subp(T_3,S_3,S_2);
	mulp(S_6,T_1,T_2);
	mulp(S_7,T_1,T_3); 
	mulp(T_4,S_2,S_3);
	mulp(T_9,CURVECONST_D,T_4);
	
	
	copy_element(T_6,S_6);
	copy_element(T_8,S_7);
	subp(T_5,S_5,T_9);
	mulp(X_3,T_5,T_6);
	addp(T_7,S_5,T_9);
	mulp(Y_3,T_7,T_8);
	mulp(Z_3,T_5,T_7);
	
	init(pt3->x);
	init(pt3->y);
	init(pt3->z);
	
	copy_element(pt3->x,X_3);
	copy_element(pt3->y,Y_3);
	copy_element(pt3->z,Z_3);
}

void smul(field_t *k)
{
	int word,msb;
	point_t q;
	int i,j;

	word = WORDLIMIT - 1;
	msb  = sizeof(field_t)*2 - 1;
	//printf("\nmsb : %d\n",msb);
	while(1)
	{
		if(k[word] >> msb)
		{
			copypoint(&q, basepoint);
			break;
		}

		msb--;
		if(msb < 0)
		{
			msb = sizeof(field_t)*2-1;
			word--;
			if(word < 0)
			{
				/* We have not found a leading MSB so fill q with 0s */
				fillzeros(&q);
				break;
			}
		}
	}
	
	//right to left no branching always double and add algorithm
	bool b;
	point_t R[2];
	fillzeros(&R[0]);
	fillzeros(&R[1]);
	fillzeros(&q);
	//Initializing
	copypoint(&R[0],basepoint);
	copypoint(&R[1],basepoint);


	j=1;
	for(i=0; i< word; i++ )
	{
		for(; (j<MSB); j++)
		{	
			b = !((k[i] >> j)&1);
			pt_add_after(&R[b],&R[b],&q);
			pt_add_after(&R[!b],&q,&R[b]);
		} j=0;
	}
	j=0;
	if(i==0) j=1;
	for(; j<msb+1; j++)
	{
		fillzeros(&q);
		b = !((k[i] >> j)&1);
		pt_add_after(&R[b],&R[b],&q);
		pt_add_after(&R[!b],&q,&R[b]);
	}
	//copypoint(&q,&r0);
	//printpoint(&R[0]);
}

int count = 0;	//To count the number of interrupts due to sampling mode execution
long long sample_counter;	//To read branch_misses values at each interrupt
int fm;	//File descriptor of perf_event related to branch_misses
FILE *fp;	//File to write the final values of branch_misses

//Function to open perf_events via system call
long perf_event_open(struct perf_event_attr* event_attr, pid_t pid, int cpu, int group_fd, unsigned long flags) {
    return syscall(__NR_perf_event_open, event_attr, pid, cpu, group_fd, flags);
}

//Function to handle overflow interrupts
static void perf_event_handler(int signum, siginfo_t* info, void* ucontext) {
    	if(info->si_code != POLL_HUP) {	//Only POLL_HUP should happen
        	exit(EXIT_FAILURE);
    	}
    	count++;
    	read(fm, &sample_counter, sizeof(long long));	//Read branch_misses value
    	ioctl(fm, PERF_EVENT_IOC_RESET, 0);	//Reset the branch_misses counter to zero
    	fprintf(fp, "%lld ", sample_counter);
    	ioctl(info->si_fd, PERF_EVENT_IOC_REFRESH, 1);	//Refresh the instruction count buffer
}

unsigned int timestamp(void)
{
    unsigned int bottom;
    unsigned int top;
    asm volatile("xorl %%eax,%%eax\n cpuid \n" ::: "%eax", "%ebx", "%ecx", "%edx"); // flush pipeline
    asm volatile("rdtsc\n" : "=a" (bottom), "=d" (top) );                           // read rdtsc
    asm volatile("xorl %%eax,%%eax\n cpuid \n" ::: "%eax", "%ebx", "%ecx", "%edx"); // flush pipeline again
    return bottom;
}

int main(int argc, char** argv)
{
	int i,j;

	//Setting up the curve constants
	LOOP(i,WORDNUM*WORDNUM + 4)
		temp[i]=0;
	basepoint = (point_t *) malloc(sizeof(point_t));
	// initilize  basepoint
	copy_element(basepoint->x, CURVE_PT_X);
	copy_element(basepoint->y, CURVE_PT_Y);
	copy_element(basepoint->z, CURVE_PT_Z);
	//field_t k[WORDLIMIT]={0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xff,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	field_t k[WORDLIMIT]={0x0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	//field_t k[WORDLIMIT]= {0xc971,0xd166 ,0xd45f,0x8944,0x0734 ,0xdfd3,0x65c4 ,0xf779,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0x1ff,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	////////////////////////////////////////////////////////////////////////////////
	char filename[100];
    	sprintf(filename, "to_match/unknown");
	fp = fopen(filename, "a");
	printf("%s\n", filename);

	//printf("%d\n",argc);
	//reading k from outside
	for(i = 1;i<argc - 1;i++)
	{
		k[i-1] = strtol(argv[i],NULL,16);
	}

	//print and see what k reads
	//for(i=0;i< WORDLIMIT;i++)
	//	printf("%x ",k[i]);
	
	///////////////////////////////////////////////////////////////////////////////
    	//Configure the signal handler
	struct sigaction sa;
    	memset(&sa, 0, sizeof(struct sigaction));
    	sa.sa_sigaction = perf_event_handler; //Specify what function to execute on interrupt
    	sa.sa_flags = SA_SIGINFO;
    
    	//Setup signal handler
    	if (sigaction(SIGIO, &sa, NULL) < 0) {
        	fprintf(stderr,"Error setting up signal handler\n");
        	perror("sigaction");
        	exit(EXIT_FAILURE);
    	}

    	//Configure perf_event_attr struct
    	struct perf_event_attr pe, pm;    
    	memset(&pe, 0, sizeof(struct perf_event_attr));
    	pe.type = PERF_TYPE_HARDWARE;
    	pe.size = sizeof(struct perf_event_attr);
    	pe.config = PERF_COUNT_HW_INSTRUCTIONS;	//Event to be monitored for sampling mode
    	pe.disabled = 1;		//Event is initially disabled
    	pe.sample_type = PERF_SAMPLE_IP;
    	pe.sample_period = 1600;	//Number of counts of the monitored event on which interrupt would occur
    	pe.exclude_kernel = 1;	// excluding events that happen in kernel space
    	pe.exclude_hv = 1;	// excluding events that happen in hypervisor

    	pid_t pid = 0;	//measure on current process or thread
    	int cpu = -1;	//measure on any cpu
    	int group_fd = -1;
    	unsigned long flags = 0;

    	int fd = perf_event_open(&pe, pid, cpu, group_fd, flags);
    	if (fd == -1) {
        	fprintf(stderr, "Error opening leader %llx\n", pe.config);
        	perror("perf_event_open");
        	exit(EXIT_FAILURE);
    	}
	   	
    	//Setup event handler for overflow signals
    	fcntl(fd, F_SETFL, O_NONBLOCK|O_ASYNC);
    	fcntl(fd, F_SETSIG, SIGIO);
    	fcntl(fd, F_SETOWN, getpid());
    
	memset(&pm, 0, sizeof(struct perf_event_attr));
    	pm.type = PERF_TYPE_HARDWARE;
    	pm.size = sizeof(struct perf_event_attr);
    	pm.config = PERF_COUNT_HW_BRANCH_MISSES;	//Count branch_misses during the program execution
    	pm.disabled = 1;
    	pm.exclude_kernel = 1;
    	pm.exclude_hv = 1;
    	fm = perf_event_open(&pm, pid, cpu, group_fd, flags);
    	if (fm == -1) {
        	fprintf(stderr, "Error opening leader %llx\n", pm.config);
        	perror("perf_event_open");
        	exit(EXIT_FAILURE);
    	}	
	
	//Enabling perf_events for counting
	ioctl(fd, PERF_EVENT_IOC_RESET, 0);
	ioctl(fd, PERF_EVENT_IOC_REFRESH, 1);  //printf("I am here\n");

	ioctl(fm, PERF_EVENT_IOC_RESET, 0);
	ioctl(fm, PERF_EVENT_IOC_ENABLE, 0);
	ioctl(fm, PERF_EVENT_IOC_RESET, 0);	//Reset the branch_misses counter to zero
	
	//Start Monitoring  	
	smul(k);
	//End Monitoring
    
    	//Disable perf_events
    	ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);
    	ioctl(fm, PERF_EVENT_IOC_DISABLE, 0);
    
	//print statistics
    	long long counter;
    	read(fd, &counter, sizeof(long long)); // Read event counter value
    	printf("%lld Instructions Executed..\n", counter);
    	close(fd);
	close(fm);
    	printf("Sampling count = %d\n", count);


	fprintf(fp, "\n");
    	fclose(fp);
}
