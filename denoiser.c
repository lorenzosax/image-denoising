#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <mpi.h>
#include <pthread.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define THREADS 10

typedef struct finfo_t
{
    char *file_name;
    int start_index;
    int end_index;
    int8_t *matrix;
    int size;
    int id_thread; //for debug
} finfo_t;

int iterations;
double beta, pi, g;
MPI_Request request;
MPI_Status status;

void goto_line(FILE *, int);
void *fthread(void *);
void rscanf(char *, int8_t *, int);
void rprintf(char *, int8_t *, int);
void oprintf(int8_t *matrix, int size);
void initialize_matrix(int8_t *, int);
int master(int, int, int8_t *, int);
int slave(int, int);
int neighbour_adder(int8_t *, int, int, int, int);
double random_probability();

int main(int argc, char *argv[])
{
    srand(time(NULL));
    int world_size, world_rank, error;

    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    iterations = atoi(argv[2]);

    if (world_rank == 0)
    {
        //printf("--- Starting program (arguments: %d)...---\n", argc);

        double ti, tf;
        int i, j;
        int8_t *matrix;
        int size;
        char *input_file = NULL,
             *output_file = NULL;
        pthread_t threads[THREADS];

        size = atoi(argv[1]);
        if ((size % (world_size) != 0) && size != 1)
        {
            fprintf(stderr, "It's impossible go on, size is not divisible for all slave processes!\n");
            return -1;
        }

        if (argc == 3)
        {
            matrix = (int8_t *)malloc(size * size * sizeof(int8_t));
            initialize_matrix(matrix, size);
            //cprintf("matrix.txt", matrix, size);
            beta = 0.8,
            pi = 0.15;
        }
        else if (argc > 3 && argc == 7)
        {
            matrix = (int8_t *)malloc(size * size * sizeof(int8_t));
            input_file = argv[3];
            output_file = argv[4];
            beta = atof(argv[5]);
            pi = atof(argv[6]);
        }

        else
        {
            printf("Error! Example input: \"./main <size> <input> <output> <beta> <pi>\" or \"./main <size>\"\n");
            return -1;
        }

        g = log((1 - pi) / pi) / 2;
        //printf("beta=%f, pi=%f, gamma=%f\n", beta, pi, g);

        if (world_size < 2)
        {
            ti = MPI_Wtime();
            if (input_file != NULL)
                rscanf(input_file, matrix, size);
            tf = MPI_Wtime() - ti;
        }
        else
        {
            ti = MPI_Wtime();
            if (input_file != NULL)
                for (i = 0; i < THREADS; i++)
                {
                    finfo_t *finfo = (finfo_t *)malloc(sizeof(finfo_t));
                    finfo->file_name = input_file;
                    finfo->start_index = i * (size / THREADS);
                    finfo->end_index = finfo->start_index + (size / THREADS) - 1;
                    finfo->matrix = matrix;
                    finfo->size = size;
                    finfo->id_thread = i;

                    if (pthread_create(&threads[i], NULL, fthread, (void *)finfo) != 0)
                    {
                        printf("pthread_create failed!\n");
                        return EXIT_FAILURE;
                    }
                }
            tf = MPI_Wtime() - ti;
        }
        if (input_file != NULL)
	{
            printf("\ntime for read file: %fs\n\n", tf);

            for (i = 0; i < THREADS; i++)
            {
            	pthread_join(threads[i], NULL);
            }
	}
        ti = MPI_Wtime();

        if ((error = master(world_size, world_rank, matrix, size)))
        {
            printf("Error in master");
            return error;
        }

        tf = MPI_Wtime() - ti;

        if (output_file != NULL)
            rprintf(output_file, matrix, size);

        printf("\nrunning time for %d processes: %fs\n\n", world_size, tf);
    }
    else
    {
        if (argc == 3)
        {

            beta = 0.8,
            pi = 0.15;
        }
        else if (argc > 3 && argc == 7)
        {

            beta = atof(argv[5]);
            pi = atof(argv[6]);
        }

        g = log((1 - pi) / pi) / 2;
        //printf("beta=%f, pi=%f\n", beta, pi);

        if ((error = slave(world_size, world_rank)))
        {
            printf("Error in slave");
            return error;
        }
    }

    MPI_Finalize();
}

void goto_line(FILE *file, int final_line)
{
    int i;
    char *line = NULL;
    size_t length = 0;
    for (i = 0; i < final_line && getc(file) != EOF; i++)
        getline(&line, &length, file);
}

void *fthread(void *args)
{
    finfo_t *finfo = (finfo_t *)args;
    int i, j, size = finfo->size;
    char *line = NULL;
    FILE *file = fopen(finfo->file_name, "r");
    size_t length = 0;

    goto_line(file, finfo->start_index);

    for (i = finfo->start_index; i <= finfo->end_index; i++)
    {

        getline(&line, &length, file);
        char *tmp = NULL;

        for (j = 0; j < size; j++)
        {
            char val[3];
            val[0] = line[(j * 3)];
            val[1] = line[(j * 3) + 1];
            val[2] = line[(j * 3) + 2];

            int8_t one = atoi(val);
            finfo->matrix[i * size + j] = one;
        }
    }

    fclose(file);
}

void rscanf(char *file_name, int8_t *matrix, int size)
{
    int i, j;
    FILE *file = fopen(file_name, "r");

    for (i = 0; i < size; i++)
    {
        //printf("\n");
        for (j = 0; j < size; j++)
        {
            fscanf(file, "%d", &matrix[i * size + j]);
            //printf("%d\t", matrix[i*size+j]);
        }
    }
    fclose(file);
}

void rprintf(char *file_name, int8_t *matrix, int size)
{
    int i, j;
    FILE *file = fopen(file_name, "w");

    for (i = 0; i < size; i++)
    {
        for (j = 0; j < size; j++)
        {
            if (matrix[i * size + j] < 0)
            {
                fprintf(file, " %d", matrix[i * size + j]);
            }
            else
            {
                fprintf(file, "  %d", matrix[i * size + j]);
            }
        }
        fprintf(file, "\n");
    }
    fclose(file);
}

void oprintf(int8_t *matrix, int size)
{
    int i, j;

    for (i = 0; i < size; i++)
    {
        for (j = 0; j < size; j++)
        {
            if (matrix[i * size + j] < 0)
            {
                fprintf(stdout, " %d", matrix[i * size + j]);
            }
            else
            {
                fprintf(stdout, "  %d", matrix[i * size + j]);
            }
        }
        fprintf(stdout, "\n");
    }
}

void initialize_matrix(int8_t *matrix, int size)
{
    int i, j;
    for (i = 0; i < size; i++)
    {
        for (j = 0; j < size; j++)
        {
            if (rand() % 2 == 0)
                matrix[i * size + j] = 1;
            else
                matrix[i * size + j] = -1;
        }
    }
}

int master(int world_size, int world_rank, int8_t *final_matrix, int size)
{
    printf("I am the master and a random value (memory index %d) of matrix is: %d\n", rand() % (size * size), final_matrix[rand() % (size * size)]);

    if (world_size == 1)
    {
        int8_t *matrix = (int8_t *)malloc(size * size * sizeof(int8_t));

        memcpy(matrix, final_matrix, size * size);
        while (iterations--)
        {
            if (iterations % 1000000 == 0)
            {
                printf("Iterations left (process %d): %d\n", world_rank, iterations);
            }
            int row_position = rand() % size;
            int column_position = rand() % size;

            int neighbour_sum = neighbour_adder(final_matrix, size, size, row_position, column_position);

            double delta_e = -2 * g * matrix[row_position * size + column_position] * final_matrix[row_position * size + column_position] - 2 * beta * final_matrix[row_position * size + column_position] * neighbour_sum;

            if (log(random_probability()) <= delta_e)
            {
                final_matrix[row_position * size + column_position] = -final_matrix[row_position * size + column_position];
            }
        }
    }
    else
    {
        MPI_Bcast(&size, 1, MPI_INT, 0, MPI_COMM_WORLD);

        int i, iterations_left, partition = size * (size / world_size);
        int8_t *matrix = (int8_t *)malloc(partition * sizeof(int8_t));
        int8_t *matrixcpy = (int8_t *)malloc(partition * sizeof(int8_t));

        MPI_Scatter(final_matrix, partition, MPI_BYTE, matrix, partition, MPI_BYTE, 0, MPI_COMM_WORLD);

        memcpy(matrixcpy, matrix, partition);

        iterations_left = iterations;

        while (iterations_left--)
        {
            if (iterations_left % 1000000 == 0)
            {
                printf("Iterations left (process %d): %d\n", world_rank, iterations_left);
            }

            int row_position = rand() % size / world_size;
            int column_position = rand() % size;

            int neighbour_sum = neighbour_adder(matrix, size, size, row_position, column_position);

            double delta_e = -2 * g * matrix[row_position * size + column_position] * matrixcpy[row_position * size + column_position] - 2 * beta * matrixcpy[row_position * size + column_position] * neighbour_sum;

            if (log(random_probability()) <= delta_e)
            {
                matrixcpy[row_position * size + column_position] = -matrixcpy[row_position * size + column_position];
            }
        }

        memcpy(final_matrix, matrixcpy, partition);
        for (i = 1; i < world_size; i++)
        {
            MPI_Recv(final_matrix + (partition * i), partition, MPI_BYTE, i, 1, MPI_COMM_WORLD, &status);
        }
    }

    //printf("--- End program. ---\n");
    return 0;
}

int slave(int world_size, int world_rank)
{

    printf("I am the slave number %d and beta=%f, g=%f\n", world_rank, beta, g);

    int size, partition, iterations_left;

    MPI_Bcast(&size, 1, MPI_INT, 0, MPI_COMM_WORLD);
    partition = size * (size / world_size);

    int8_t *matrix = (int8_t *)malloc(partition * sizeof(int8_t));
    int8_t *matrixcpy = (int8_t *)malloc(partition * sizeof(int8_t));

    MPI_Scatter(NULL, 0, 0, matrix, partition, MPI_BYTE, 0, MPI_COMM_WORLD);

    memcpy(matrixcpy, matrix, partition);

    iterations_left = iterations;

    while (iterations_left--)
    {
        if (iterations_left % 1000000 == 0)
        {
            printf("Iterations left (process %d): %d\n", world_rank, iterations_left);
        }

        int row_position = rand() % size / world_size;
        int column_position = rand() % size;

        int neighbour_sum = neighbour_adder(matrix, size, size, row_position, column_position);

        double delta_e = -2 * g * matrix[row_position * size + column_position] * matrixcpy[row_position * size + column_position] - 2 * beta * matrixcpy[row_position * size + column_position] * neighbour_sum;

        if (log(random_probability()) <= delta_e)
        {
            matrixcpy[row_position * size + column_position] = -matrixcpy[row_position * size + column_position];
        }
    }

    MPI_Send(matrixcpy, partition, MPI_BYTE, 0, 1, MPI_COMM_WORLD);

    return 0;
}

int neighbour_adder(int8_t *matrix, int rows, int columns, int row_center, int column_center)
{
    int i, j, sum = 0;

    for (i = row_center - 1; i <= row_center + 1; ++i)
    {
        if (i >= 0 && i < rows) // if within row boundaries
        {
            for (j = column_center - 1; j <= column_center + 1; ++j)
            {
                if (j >= 0 && j < columns) // if within row boundaries
                {
                    if (i != row_center || j != column_center)
                    { // skip center
                        sum += (int)matrix[i * columns + j];
                        //columns instead of rows because they are constants
                    }
                }
            }
        }
    }
    return sum;
}

double random_probability()
{
    return ((double)rand()) / RAND_MAX;
}
