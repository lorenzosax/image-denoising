#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <papi.h>
#include <string.h>

#define N 8
#define THREADS 4

typedef struct fileinfo 
{
    int **matrix;
    char* file_name;
    int start_index;
    int end_index;
} fileinfo;

pthread_mutex_t mutex;

void *thread(void *);

int main(int argc, char **argv)
{

    int matrix[N][N], i;
    
    pthread_t threads[THREADS];
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    //pthread_mutex_init(&mutex, NULL);

    for (i = 0; i < THREADS; i++) {

        fileinfo *finfo = (fileinfo *)malloc(sizeof(fileinfo));
        finfo->matrix = matrix;
        char file_name[] = "file.txt";
        finfo->file_name = file_name;

        finfo->start_index = i*(N/THREADS);
        finfo->end_index = finfo->start_index+((N/THREADS)-1);
        printf("%d %d\n", finfo->start_index, finfo->end_index);

        if (pthread_create(&threads[i], NULL, thread, (void *) finfo) != 0) {
            
            printf("pthread_create failed!\n");
            return EXIT_FAILURE;
        }

    }

    
    for(i = 0; i < THREADS; i++) {
         pthread_join(threads[i], NULL);
     }

     pthread_exit(NULL);



    //return 0;
}

void *thread(void *args)
{
    //pthread_mutex_lock(&mutex);    

    int i, j;
    ssize_t c;
    fileinfo *finfo = (fileinfo *) args;

    printf("\nThread\n");
    

    FILE* file = fopen(finfo->file_name, "r");

//      if(c = fseek (file , 3*N, SEEK_SET) != 0) {
//          printf("Error fseek");
//  } 
//      char *line = NULL;
//      size_t length = 0;

//      getline(&line, &length, file);

//      printf("%s", line);

    //printf("%d", 3*N*finfo->start_index);

   if(c = fseek (file , 3*N*finfo->start_index, SEEK_SET) != 0) {
        printf("Error fseek");
    }

    char *line = NULL;
    size_t length = 0;

    for(i = finfo->start_index; i <= finfo->end_index; i++)
    {
        getline(&line, &length, file);

        char *tmp = NULL;
        
       for(j = 0; j < N; j++) {
            
            char val[3];
            printf("  ");
            for(int k = 0; k < 3; k++) {
                val[k] = line[(j*3)+k];
                  
            }

            printf("%d ", atoi(val));
            //printf("%s\n", val);
            //memcpy(tmp, line, 3);

            //printf("%s", tmp);
            //printf("%n%d", line[1]);
            //finfo->matrix[i][j] = atoi
        }
    }
    
    fclose(file);

    //pthread_mutex_unlock(&mutex);
    pthread_exit(NULL);
}


// void method() {
//     int c = 1;
//     int row4thread = N/THREADS;
//     char *array[row4thread];

//     while(getline(line,l,file)) {
//         if(row4thread == c) {

//         } else {
//             line
//         }
//         c++;
//     }
// }