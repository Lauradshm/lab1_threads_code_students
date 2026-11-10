/***************************************************************************
 *
 * Sequential version of Gaussian elimination
 *
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h> // pthread types and functions

#define MAX_SIZE 4096

typedef double matrix[MAX_SIZE][MAX_SIZE];

int	N;		/* matrix size		*/
int	maxnum;		/* max number of element*/
char	*Init;		/* matrix init type	*/
int	PRINT;		/* print switch		*/
matrix	A;		/* matrix A		*/
double	b[MAX_SIZE];	/* vector b             */
double	y[MAX_SIZE];	/* vector y             */

/* forward declarations */
void* threadwork(void* params);
void work(void);
void Init_Matrix(void);
void Print_Matrix(void);
void Init_Default(void);
int Read_Options(int, char **);

struct threadArgs {
	unsigned int startrow;
	unsigned int endrow;
    unsigned int k; /* pivot index		*/
};

int
main(int argc, char **argv)
{
    int i, timestart, timeend, iter;
    Init_Default();		/* Init default values	*/
    Read_Options(argc,argv);	/* Read arguments	*/
    Init_Matrix();		/* Init the matrix	*/
    work();
    if (PRINT == 1)
	   Print_Matrix();
}

void
work(void)
{
    unsigned int j, k, rowsToUpdate, rowsPerThread, remainingRows, activeThreads;
    pthread_t* children;
    struct threadArgs* args;//argument buffer
    unsigned int numThreads = 4;
    children = malloc(numThreads * sizeof(pthread_t));
    
    /* Gaussian elimination algorithm, Algo 8.4 from Grama */
    for (k = 0; k < N; k++) { /* Outer loop */
	    for (j = k+1; j < N; j++)
	       A[k][j] = A[k][j] / A[k][k]; /* Division step */
	    y[k] = b[k] / A[k][k];
	    A[k][k] = 1.0;
        rowsToUpdate = N -k -1;
        rowsPerThread = rowsToUpdate / numThreads;
        remainingRows = rowsToUpdate % numThreads;
        activeThreads = (rowsToUpdate < numThreads)? rowsToUpdate : numThreads;
    

    unsigned int startRow, endRow;
    for(unsigned int id = 0; id < activeThreads; id++){
    args = malloc(sizeof(struct threadArgs));
    startRow = k + 1 + id * rowsPerThread;
    if (id < remainingRows)
        startRow += id;
    else
        startRow += remainingRows;


    endRow = startRow + rowsPerThread - 1;
    if(id < remainingRows)
        endRow++;

    
    args->startrow = startRow;
    args->endrow = endRow;
    args->k = k;

    pthread_create(&(children[id]), NULL, threadwork, (void*) args);
    }

    for (unsigned int id = 0; id < activeThreads; id++){
        pthread_join(children[id], NULL);
    }
    
    }

    free(children);
}


void*
threadwork(void* params){
    int i, j;
    struct threadArgs *args = (struct threadArgs*) params;
    unsigned int start = args->startrow;
    unsigned int end = args->endrow;
    unsigned int pivot = args->k;
     
    /* Gaussian elimination algorithm, Algo 8.4 from Grama */
    for (i = start; i <= end; i++) {
        for (j = pivot+1; j < N; j++)
            A[i][j] = A[i][j] - A[i][pivot]*A[pivot][j]; /* Elimination step */
        b[i] = b[i] - A[i][pivot]*y[pivot];
        A[i][pivot] = 0.0;
    }
    
    free(args);
    return NULL;
}

void
Init_Matrix()
{
    int i, j;

    printf("\nsize      = %dx%d ", N, N);
    printf("\nmaxnum    = %d \n", maxnum);
    printf("Init	  = %s \n", Init);
    printf("Initializing matrix...\n");

    if (strcmp(Init,"rand") == 0) {
        for (i = 0; i < N; i++){
            for (j = 0; j < N; j++) {
                if (i == j){ /* diagonal dominance */
                    A[i][j] = (double)(rand() % maxnum) + 5.0;
                }
                else{
                    A[i][j] = (double)(rand() % maxnum) + 1.0;
                }
            }
        }
    }
    if (strcmp(Init,"fast") == 0) {
        for (i = 0; i < N; i++) {
            for (j = 0; j < N; j++) {
                if (i == j) /* diagonal dominance */
                    A[i][j] = 5.0;
                else
                    A[i][j] = 2.0;
            }
        }
    }

    /* Initialize vectors b and y */
    for (i = 0; i < N; i++) {
        b[i] = 2.0;
        y[i] = 1.0;
    }

    printf("done \n\n");
    if (PRINT == 1)
        Print_Matrix();
}

void
Print_Matrix()
{
    int i, j;

    printf("Matrix A:\n");
    for (i = 0; i < N; i++) {
        printf("[");
        for (j = 0; j < N; j++)
            printf(" %5.2f,", A[i][j]);
        printf("]\n");
    }
    printf("Vector b:\n[");
    for (j = 0; j < N; j++)
        printf(" %5.2f,", b[j]);
    printf("]\n");
    printf("Vector y:\n[");
    for (j = 0; j < N; j++)
        printf(" %5.2f,", y[j]);
    printf("]\n");
    printf("\n\n");
}

void
Init_Default()
{
    N = 2048;
    Init = "rand";
    maxnum = 15.0;
    PRINT = 0;
}

int
Read_Options(int argc, char **argv)
{
    char    *prog;

    prog = *argv;
    while (++argv, --argc > 0)
        if (**argv == '-')
            switch ( *++*argv ) {
                case 'n':
                    --argc;
                    N = atoi(*++argv);
                    break;
                case 'h':
                    printf("\nHELP: try sor -u \n\n");
                    exit(0);
                    break;
                case 'u':
                    printf("\nUsage: gaussian [-n problemsize]\n");
                    printf("           [-D] show default values \n");
                    printf("           [-h] help \n");
                    printf("           [-I init_type] fast/rand \n");
                    printf("           [-m maxnum] max random no \n");
                    printf("           [-P print_switch] 0/1 \n");
                    exit(0);
                    break;
                case 'D':
                    printf("\nDefault:  n         = %d ", N);
                    printf("\n          Init      = rand" );
                    printf("\n          maxnum    = 5 ");
                    printf("\n          P         = 0 \n\n");
                    exit(0);
                    break;
                case 'I':
                    --argc;
                    Init = *++argv;
                    break;
                case 'm':
                    --argc;
                    maxnum = atoi(*++argv);
                    break;
                case 'P':
                    --argc;
                    PRINT = atoi(*++argv);
                    break;
                default:
                    printf("%s: ignored option: -%s\n", prog, *argv);
                    printf("HELP: try %s -u \n\n", prog);
                    break;
            }
}
