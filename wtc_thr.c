/*
* Harshil Shah 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/time.h>
#include <pthread.h>

int *g = NULL;
int count;
int globalflag=0;
pthread_mutex_t m;
pthread_cond_t c;
typedef struct myArgs{
  int n;
  int start;
  int end;
  int p;
} myArgs;

void *trans(void* arg);

    int main(int argc, char ** argv){
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
   
   //allocating memory on heap for matrix
    g = (int*)malloc(sizeof(int)*n*n);
 
    int i=0;
    int j=0;
    for(i=0;i<n;i++){
      for(j=0;j<n;j++){   
	g[(i*n)+j]=0;       
      }
    }
  /*****   intializes the matrix while reading the file ******/
    while(fgets(buffer, 101,f)){
     if(strchr(buffer,' ')!=NULL){
      char *ic =(char*) strtok(buffer," ");
      int i = atoi(ic);
      ic =(char*) strtok(NULL," ");
      int j = atoi(ic);
     
      g[(i-1)*n+j-1] = 1;
	}
    }
   //thread variables
    pthread_t tid[p];
    pthread_mutex_init(&m,NULL);
    pthread_cond_init(&c,NULL);
    int ret[p];


    if(p>n){
		printf("Input Error!!\n");
		return 0;
    }
    int k = 0;
    int rows ;
    if(p==0)
    	rows = n;
    else
    	rows = (int)(n/p);
    int x = 0;

   count=0; 
  //time variables
   struct timeval start;
   struct timeval end;
   gettimeofday(&start, NULL);

 /** creating p threads and assigning each one what rows to do ***/
    for(x = 0; x < p-1; x++){
	//printf("main start:end %i:%i\n",x*rows,(x+1)*rows);
      myArgs *args = (myArgs*)malloc(sizeof(myArgs));
	args->n = n;
	args->start = x*rows;
	args->end = (x+1)*rows;
	args->p = p;
      
      ret[x] = pthread_create(&tid[x], NULL, trans, args);
    }
   // printf("main start:end %i:%i\n",x*rows,n);
   	myArgs *args = (myArgs*)malloc(sizeof(myArgs));
	args->n = n;
	args->start = (x)*rows;
	args->end = n;
	args->p = p;
	if(p==0){
		trans(args);
	}
	else
    	ret[p-1] = pthread_create(&tid[p-1], NULL, trans, args);
   
 /*******  main waits for all threds to finish **********/
    for(i=0; i < p; i++){
      pthread_join(tid[i], NULL);
    }
    gettimeofday(&end, NULL);
  
/****** calculating time ***********/
    int seconds = end.tv_sec - start.tv_sec;
    int suseconds = end.tv_usec - start.tv_usec;
    if(suseconds < 0){
      seconds--;
      suseconds = 1000000 + suseconds;
    } 
    printf( "Time to complete: %ld seconds %ld suseconds\n", seconds, suseconds);  
   
/***** prints matrix *************/
    printf("\n");
    for(i=0;i<n;i++){
      for(j=0;j<n;j++){ 
		if(g[(i*n) +j]==1)
			printf("%i %i\n",i+1,j+1);       
      }
    }
 printf("\n");
   
  }

/*********************************************************/
//main algorithm for graph traversal
    void *trans(void *args){
      int i;
      int j = 0;
      int k = 0;
      myArgs *a = (myArgs*)args;
      int n = a->n;
	int p = a->p;
	int flag=0;
      int start = a->start;
	int end = a->end;
	//printf("start:end %i:%i\n",start,end);
	  
      for(k=0; k < n; k++){
	  for(i=start; i < end;i++){
	    for(j=0;j < n;j++){    
	      pthread_mutex_lock(&m);
	      g[i*n +j]= g[i*n +j] || (g[i*n+k]&&g[k*n +j]); 
	      pthread_mutex_unlock(&m);
	    }
	  }
	//using the sense & local sense idea of barriers  
        flag=!flag;
	 if(p!=0){
	 pthread_mutex_lock(&m);
	  count++;
	  if(count==p){
	      count=0;
		globalflag=flag;
		pthread_cond_broadcast(&c);
	  }
	  else{
		pthread_cond_wait(&c,&m);
	  }
	 // printf("count: %i\n",count);
	  pthread_mutex_unlock(&m);
        
      }
     }
      
    }
/*****************************************************************/
