#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <papi.h>
#include "queue.c"

#define TOTAL_ITERATIONS 5000000

/**
 * generates a random number between 0 and 1.0 (both inclusive)
 * @return (double)0-1.0
 */
double randomProbability()
{
    return ((double)rand()) / RAND_MAX;
}

/**
 * Sum the surroundings of a point in a grid. (Ignores out of boundaries by not considering them in the sum)
 * Also does not include the center point in the sum.
 * @param subImage
 * @param rows
 * @param columns
 * @param rowCenter
 * @param columnCenter
 * @return
 */
int summer(char **subImage, int rows, int columns, int rowCenter, int columnCenter)
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
                        sum += (int)subImage[i][j];
                    }
                }
            }
        }
    }
    return sum;
};

int main(int argc, char **argv)
{

    srand(time(NULL));

    if (argc != 5)
    {
        fprintf(stderr, "Please, run the program as \n"
                        "\"sequential <input> <output> <beta> <pi>\"");
        return 1;
    }

    long_long papi_time_start, papi_time_stop;
    papi_time_start = PAPI_get_real_usec();

    double beta = atof(argv[3]);
    double pi = atof(argv[4]);
    double gammaValue = log((1 - pi) / pi) / 2;
    char *input = argv[1];
    char *output = argv[2];

    // region START

    FILE *inputFile, *outputFile;
    inputFile = fopen(input, "r");

    queue *rowQueue = newQueue();
    int rowCount = 0;
    int columnCount = 0;

    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&line, &len, inputFile)) != -1)
    {
        int i, cursor = 0, nextCursor, nextPixel;
        char *row = NULL;
        if (columnCount == 0)
        {
            queue *columnQueue = newQueue();
            while (sscanf(line + cursor, "%d%n", &nextPixel, &nextCursor) > 0)
            {
                cursor += nextCursor;
                ++columnCount;
                push(columnQueue, (void *)nextPixel);
            }
            row = (char *)malloc(columnCount * sizeof(char));
            for (i = 0; i < columnCount; ++i)
            {
                row[i] = (char)pop(columnQueue);
            }
            freeQueue(columnQueue);
        }
        else
        {
            row = (char *)malloc(columnCount * sizeof(char));
            i = 0;
            while (sscanf(line + cursor, "%d%n", &nextPixel, &nextCursor) > 0)
            {
                cursor += nextCursor;
                row[i++] = (char)nextPixel;
            }
        }
        ++rowCount;
        push(rowQueue, (void *)row);
    }

    char *finalResult[rowCount], *image[rowCount];
    int i;
    for (i = 0; i < rowCount; i++)
    {
        image[i] = (char *)pop(rowQueue);
        finalResult[i] = (char *)malloc(columnCount * sizeof(char));
        memcpy(finalResult[i], image[i], columnCount);
    }

    // region Calculations
    int iterations = TOTAL_ITERATIONS;

    while (iterations--)
    {
        if (iterations % 1000000 == 0)
        {
            printf("Iteration left: %d\n", iterations);
        }
        /* pick a random pixel */
        int rowPosition = rand() % rowCount;
        int columnPosition = rand() % columnCount;

        /* sum neighbour cells */
        int sum = summer(finalResult, rowCount, columnCount, rowPosition, columnPosition);

        /* calculate delta_e */
        // double deltaE = - 2 * finalResult[rowPosition][columnPosition] * (gammaValue * image[rowPosition][columnPosition] + beta * sum);
        double deltaE = -2 * gammaValue * image[rowPosition][columnPosition] * finalResult[rowPosition][columnPosition] - 2 * beta * finalResult[rowPosition][columnPosition] * sum;
        // printf("delta: %f exp delta %f\n", deltaE, exp(deltaE));
        // deltaE = log(accept_probability)  *** accept_probability can be bigger than 1, since we skipped Min(1, acc_prob) ***
        if (log(randomProbability()) <= deltaE)
        {
            // if accepted, flip the pixel
            finalResult[rowPosition][columnPosition] = -finalResult[rowPosition][columnPosition];
        }
    }
    // endregion

    printf("finished calculations, started writing to output\n");

    int rowNumber, columnNumber;
    outputFile = fopen(output, "w");
    for (rowNumber = 0; rowNumber < rowCount; ++rowNumber)
    {
        for (columnNumber = 0; columnNumber < columnCount; ++columnNumber)
        {
            fprintf(outputFile, "%d ", (int)finalResult[rowNumber][columnNumber]);
        }
        fprintf(outputFile, "\n");
    }
    papi_time_stop = PAPI_get_real_usec();
    printf("Running time %dus\n", papi_time_stop - papi_time_start);
    printf("finished successfully!\n");

    // endregion
}
