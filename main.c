#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <math.h>
//#include <papi.h>
#include <string.h>

#define N 200
#define THREADS 10
#define THREADSWORKER 5
#define TOTAL_ITERATIONS 500000

typedef struct fileinfo
{
    char *file_name;
    int start_index;
    int end_index;
    int id;
} fileinfo;

typedef struct threadinfo
{
    int start_row;
    int end_row;
    int id;
} threadinfo;

int matrix[N][N];
int finalmatrix[N][N];
double beta, gammaValue;

void *thread(void *);
int gotospecificline(FILE *, int);
void matrixcopy (void * destmat, void * srcmat);
void *worker(void *);

int main(int argc, char **argv)
{
    /*long_long papi_time_start, papi_time_stop;
    papi_time_start = PAPI_get_real_usec();*/
    int i, j;
    char *file_name = argv[1];
    char *file_name_output = argv[2];
    beta = atof(argv[3]);
	double pi = atof(argv[4]);
	gammaValue = log((1 - pi) / pi) / 2;

    pthread_t threads[THREADS];
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    for (i = 0; i < THREADS; i++)
    {
        fileinfo *finfo = (fileinfo *)malloc(sizeof(fileinfo));
        finfo->file_name = file_name;
        finfo->start_index = i * (N / THREADS);
        finfo->end_index = finfo->start_index + (N / THREADS) - 1;
        finfo->id = i;

        if (pthread_create(&threads[i], NULL, thread, (void *)finfo) != 0)
        {
            printf("pthread_create failed!\n");
            return EXIT_FAILURE;
        }
    }

    for (i = 0; i < THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }


    matrixcopy(finalmatrix, matrix);

    pthread_t threadsworker[THREADSWORKER];

	for (i = 0; i < THREADSWORKER; i++)
	{
		threadinfo *tinfo = (threadinfo *)malloc(sizeof(threadinfo));
		tinfo->start_row = i * (N / THREADSWORKER);
		tinfo->end_row = tinfo->start_row + (N / THREADSWORKER) - 1;
		tinfo->id = i;
		if (pthread_create(&threadsworker[i], NULL, worker, (void *)tinfo) != 0)
		{
			printf("pthread_create failed!\n");
			return EXIT_FAILURE;
		}
	}

	for (i = 0; i < THREADSWORKER; i++)
	{
		pthread_join(threadsworker[i], NULL);
	}

    FILE *file = fopen(file_name_output, "w");
    for (i = 0; i < N; i++)
    {
        for (j = 0; j < N; j++)
        {
            if (finalmatrix[i][j] < 0)
            {
                fprintf(file, " %d", finalmatrix[i][j]);
            }
            else
            {
                fprintf(file, "  %d", finalmatrix[i][j]);
            }
        }
        fprintf(file, "\n");
    }
    fclose(file);
    /*papi_time_stop = PAPI_get_real_usec();
    printf("Running time: %dus\n", papi_time_stop - papi_time_start);*/

    pthread_exit(NULL);
}

int gotospecificline(FILE *file, int nextLine)
{
    int i;
    char *line = NULL;
    size_t len = 0;
    for (i = 0; i < nextLine; i++)
    {
        getline(&line, &len, file);
    }
    return 0;
}

void *thread(void *args)
{
    int i, j;
    char *line = NULL;
    ssize_t c;
    fileinfo *finfo = (fileinfo *)args;

    FILE *file = fopen(finfo->file_name, "r");

    size_t length = 0;

    if (c = gotospecificline(file, finfo->start_index) != 0)
    {
        printf("Error fseek");
    }

    for (i = finfo->start_index; i <= finfo->end_index; i++)
    {
        getline(&line, &length, file);
        char *tmp = NULL;

        for (j = 0; j < N; j++)
        {
            char val[3];
            val[0] = line[(j * 3)];
            val[1] = line[(j * 3) + 1];
            val[2] = line[(j * 3) + 2];

            int x = atoi(val);
            matrix[i][j] = x;
        }
    }

    fclose(file);
}

void matrixcopy (void * destmat, void * srcmat)
{
  memcpy(destmat, srcmat, N*N*sizeof(int));
}

int getrandom(int lower, int upper)
{
   return (rand() % (upper - lower + 1)) + lower;
}

int summer(int rows, int columns, int rowCenter, int columnCenter)
{
    int sum = 0;
    int i, j;
    for (i = rowCenter - 1; i <= rowCenter + 1; ++i)
    {
        if (i >= 0 && i < rows)
        { // if within row boundaries
            for (j = columnCenter - 1; j <= columnCenter + 1; ++j)
            {
                if (j >= 0 && j < columns)
                { // if within column boundaries
                    if (i != rowCenter || j != columnCenter)
                    { // skip center
                        sum += finalmatrix[i][j];
                    }
                }
            }
        }
    }
    return sum;
};

double randomProbability()
{
    return ((double)rand()) / RAND_MAX;
}

void *worker(void * arg)
{
	int count = 0;
	srand(time(NULL));
	threadinfo *tinfo = (threadinfo *)arg;
	int iterations = TOTAL_ITERATIONS;
	while (iterations--)
	{
		if (iterations % 1000000 == 0) {
			// printf("thread %d started a new millionth iteration (iterations left: %d)\n", tinfo->id, iterations);
		}
		/* pick a random pixel */
		int rowPosition = getrandom(tinfo->start_row, tinfo->end_row);
		int columnPosition = rand() % N;
		int rowCount = tinfo->end_row - tinfo->start_row + 1;

		/* sum neighbour cells */
		int sum = summer(rowCount, N, rowPosition, columnPosition);

		/* calculate delta_e */
		double deltaE = -2 * gammaValue * matrix[rowPosition][columnPosition] * finalmatrix[rowPosition][columnPosition]
			- 2 * beta * finalmatrix[rowPosition][columnPosition] * sum;

		if (iterations == 3000002) {
			printf("Thread id: %d ha delta pari a %d, e ha log(randomProbability):%d\n", tinfo->id, deltaE, log(randomProbability()));
		}

		if (log(randomProbability()) <= deltaE)
		{
			// flip the pixel
			count ++;
			finalmatrix[rowPosition][columnPosition] = -finalmatrix[rowPosition][columnPosition];
		}
	}
	printf("Thread id: %d ha fatto %d flip\n", tinfo->id, count);
}