#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
// #include <papi.h>
#include <string.h>

#define N 8
#define THREADS 2

typedef struct fileinfo 
{
    char* file_name;
    int start_index;
    int end_index;
    int id;
} fileinfo;

pthread_mutex_t mutex;
int matrix[N][N];

void *thread(void *);

int main(int argc, char **argv)
{
    int i, j;

    pthread_t threads[THREADS];
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_mutex_init(&mutex, NULL);

    for (i = 0; i < THREADS; i++) {
        fileinfo *finfo = (fileinfo *)malloc(sizeof(fileinfo));
        char file_name[] = "file.txt";
        finfo->file_name = file_name;
        finfo->start_index = i*(N/THREADS);
        finfo->end_index = finfo->start_index+(N/THREADS)-1;
        finfo->id = i;
        printf("%d %d\n", finfo->start_index, finfo->end_index);

        if (pthread_create(&threads[i], NULL, thread, (void *)finfo) != 0) {
            printf("pthread_create failed!\n");
            return EXIT_FAILURE;
        }

    }

    for(i = 0; i < THREADS; i++) {
    	pthread_join(threads[i], NULL);
    }

	printf("\nECCO\n");
    for(i = 0; i < N; i++) {
    	for(j = 0; j < N; j++) {
		 	printf("%d ", matrix[i][j]);
		}
		printf("\n");
	}

	pthread_exit(NULL);
    // return 0;
}

void *thread(void *args)
{
    int i, j;
    ssize_t c;
    fileinfo *finfo = (fileinfo *) args;

	pthread_mutex_lock(&mutex);

    printf("\nThread\n");

	FILE* file = fopen(finfo->file_name, "r");

	int t = 3*N*finfo->start_index;
	if (finfo->id == 1) {
		t += 8;
		printf("t = %d \n", t);
	}
   	if(c = fseek (file , t, SEEK_SET) != 0) {
        printf("Error fseek");
	}

    char *line = NULL;
    size_t length = 0;

	printf("%d %d\n", finfo->start_index, finfo->end_index);
    for(i = finfo->start_index; i <= finfo->end_index; i++)
    {
        getline(&line, &length, file);
		printf("%s\n", line);
        char *tmp = NULL;
        
       	for(j = 0; j < N; j++) {
       		char val[3];
            val[0] = line[(j*3)];
            val[1] = line[(j*3)+1];
            val[2] = line[(j*3)+2];

			int x = atoi(val);
            matrix[i][j] = x;
        }
    }
    
    fclose(file);

    pthread_mutex_unlock(&mutex);
    // pthread_exit(NULL);
}