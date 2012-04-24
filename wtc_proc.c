/*
* Group 6
* Harshil Shah 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

int *mat = NULL;
int count= 0;

void *trans(int *shmaddr, int start, int end, int n, int p);

int main(int argc, char **argv)
{
    char c;
    int shmid;
    key_t key;
    char *shm, *s;
 
    /*
     * We'll name our shared memory segment
     * "5678".
     */
    key = 5678;

    /*
     * Create the segment.
     */
    if ((shmid = shmget(key, 10, IPC_CREAT | 0666)) < 0) {
        printf("shmget\n");
       return 0;
    }

    /*
     * Now we attach the segment to our data space.
     */
    if ((shm = shmat(shmid, NULL, 0)) == (char *) -1) {
        printf("shmat\n");
        return 0;
    }

    int pid;
    int semid; 
    int nsems = 1; 
    int nsops; 
    struct shmid_ds shmid_ds;

    semid  = semget(key,1, IPC_CREAT);
    semctl(semid, 0, SETVAL, 0);
    mat = (int*)shm;



    if(argc!=2){
      printf("Error!!! \n");
      return 0;
    }

    FILE *f = fopen(argv[1],"r");
   
    if(f==NULL){
      printf("Error!! \n");
    }
   
    char buffer[101];
    fgets(buffer, 101,f);
    int p = atoi(buffer);
   
    fgets(buffer, 101,f);
    int n = atoi(buffer);
    int i=0;
    int j=0;
    for(i=0;i<n;i++){
      for(j=0;j<n;j++){  
	mat[(i*n)+j]=0;       
      }
    }
    //readG
    while(fgets(buffer, 101,f)){
     if(strchr(buffer,' ')!=NULL){
      char *ic =(char*) strtok(buffer," ");
      int i = atoi(ic);
      ic =(char*) strtok(NULL," ");
      int j = atoi(ic);
     
      mat[(i-1)*n+j-1] = 1;
	}
    }
	
    if(p>n){
		printf("Input Error!!\n");
		return 0;
    }

	
    int k = 0;
    int rows;
    if(p==0)
    	rows=n;
    else
    	rows = (int)(n/p);
    
    int x = 0;
    pid_t ids[p];
    mat[n*n +1] = 0;
    mat[n*n+2] = 0;
   struct timeval start;
   struct timeval end;
   gettimeofday(&start, NULL);
    for(x = 0; x < p-1; x++){
      if((ids[x] = fork()) == 0){	
	//create processes
	//printf("main start:end %i:%i\n",x*rows,(x+1)*rows);
	int startR = x*rows;
	int endR = (x+1)*rows;
	trans((int*)shm, startR, endR, n, p);

	return 0;
      }
 
    }
    //printf("main start:end %i:%i\n",x*rows,n);
    int startR  = (x)*rows;
    if(p!=0){
    	if((ids[x] = fork())==0){
    		trans((int*)shm, startR, n, n, p);
    		return 0;
    	}
    }
    else
    	trans((int*)shm, startR, n, n, p);
    
    
 //create last process
 int status;
 for(x=0; x<p; x++){
   waitpid(ids[x], &status, 0);
 }

	gettimeofday(&end, NULL);
    int seconds = end.tv_sec - start.tv_sec;
    int suseconds = end.tv_usec - start.tv_usec;
    if(suseconds < 0){
      seconds--;
      suseconds = 1000000 + suseconds;
    }
    printf( "Time to complete: %ld seconds %ld suseconds\n", seconds, suseconds); 
    //prints matrix
printf("\n");
    for(i=0;i<n;i++){
      for(j=0;j<n;j++){ 
		if(mat[(i*n) +j]==1)
			printf("%i %i\n",i+1,j+1);       
      }
    }
 printf("\n");






   return 0;
}

void *trans(int *shmaddr, int start, int end, int n, int p){

  
      int i; 
      int j = 0;
      int k = 0;
      key_t key = 5678;
      int flag=0;
      int *globalflag=&shmaddr[n*n+2];
      //printf("start:end %i:%i\n",start,end);           //uncomment this to see which rows are assigned to the processes
      struct sembuf usops = {0,1,0};
      struct sembuf dsops = {0,-1,0};
	  
      for(k=0; k < n; k++){
	  for(i=start; i < end;i++){
	    for(j=0;j < n;j++){
	      semop(semget(key, 1, 0), &usops, 1);
	       shmaddr[i*n +j]= shmaddr[i*n +j] || (shmaddr[i*n+k]&&shmaddr[k*n +j]);
	      semop(semget(key, 1, 0), &dsops, 1);
	      
	    }
	  }
	  
        flag=!flag;
     if(p!=0){
	semop(semget(key, 1, 0), &usops, 1);	
	shmaddr[n*n +1]++;

	if(shmaddr[n*n +1]==p){
	  shmaddr[n*n +1]=0;
	  *globalflag=flag;
	}
	

	semop(semget(key, 1, 0), &dsops, 1);	 
	while(flag!=*globalflag){};
      }
      }
      
}
