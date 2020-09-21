#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
// #include <papi.h>
#include <string.h>

#define N 200
#define THREADS 20

typedef struct fileinfo 
{
    char* file_name;
    int start_index;
    int end_index;
    int id;
} fileinfo;

int matrix[N][N];

void *thread(void *);

int main(int argc, char **argv)
{
    int i, j;
	char *file_name = argv[1];
	char *file_name_output = argv[2];

    pthread_t threads[THREADS];
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    for (i = 0; i < THREADS; i++) {
        fileinfo *finfo = (fileinfo *)malloc(sizeof(fileinfo));
        finfo->file_name = file_name;
        finfo->start_index = i*(N/THREADS);
        finfo->end_index = finfo->start_index+(N/THREADS)-1;
        finfo->id = i;
        // printf("%d %d\n", finfo->start_index, finfo->end_index);

        if (pthread_create(&threads[i], NULL, thread, (void *)finfo) != 0) {
            printf("pthread_create failed!\n");
            return EXIT_FAILURE;
        }
    }

    for(i = 0; i < THREADS; i++) {
    	pthread_join(threads[i], NULL);
    }

	printf("\nPRINTING TO FILE...\n");
	FILE* file = fopen(file_name_output, "w");
    for(i = 0; i < N; i++) {
    	for(j = 0; j < N; j++) {
    		if (matrix[i][j] < 0) {
    			fprintf(file, " %d", matrix[i][j]);
    		} else {
    			fprintf(file, "  %d", matrix[i][j]);
    		}
		}
		fprintf(file, "\n");
	}
	fclose(file);

	pthread_exit(NULL);
}

int gotospecificline(FILE* file, int nextLine){
	int i;
	char *line = NULL;
	size_t len = 0;
	for (i=0; i < nextLine; i++){
		getline(&line, &len, file);
	}
	return 0;
}

void *thread(void *args)
{
    int i, j;
    char *line = NULL;
    ssize_t c;
    fileinfo *finfo = (fileinfo *) args;

    // printf("\nThread\n");

	FILE* file = fopen(finfo->file_name, "r");

    size_t length = 0;

   	if(c = gotospecificline(file, finfo->start_index) != 0) {
        printf("Error fseek");
	}

	printf("%d %d\n", finfo->start_index, finfo->end_index);
    for(i = finfo->start_index; i <= finfo->end_index; i++)
    {
        getline(&line, &length, file);
		printf("Line: %s\n", line);
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
}