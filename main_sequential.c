#include <stdio.h>
#include <stdlib.h>
#include <time.h>
// #include <papi.h>
#include <string.h>

#define N 8
int matrix[N][N];

int main(int argc, char **argv)
{
    int i = 0, j;
	char *file_name = argv[1];
	char *file_name_output = argv[2];

	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	FILE* fileR = fopen(file_name, "r");

    while((read = getline(&line, &len, fileR)) != -1)
	{
		for(j = 0; j < N; j++) {
			char val[3];
			val[0] = line[(j*3)];
			val[1] = line[(j*3)+1];
			val[2] = line[(j*3)+2];

			int x = atoi(val);
			matrix[i][j] = x;
		}
		i++;
	}
	fclose(fileR);

	FILE* fileW = fopen(file_name_output, "w");
	for(i = 0; i < N; i++) {
		for(j = 0; j < N; j++) {
			if (matrix[i][j] < 0) {
				fprintf(fileW, " %d", matrix[i][j]);
			} else {
				fprintf(fileW, "  %d", matrix[i][j]);
			}
		}
		fprintf(fileW, "\n");
	}
	fclose(fileW);

    return 0;
}