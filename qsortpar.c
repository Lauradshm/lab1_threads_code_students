/***************************************************************************
 *
 * Sequential version of Quick sort
 *
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>



#define KILO (1024)
#define MEGA (1024*1024)
#define MAX_ITEMS (64*MEGA)
#define swap(v, a, b) {unsigned tmp; tmp=v[a]; v[a]=v[b]; v[b]=tmp;}

//shared variables 
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// declare the functions
static void print_array(void);
static void init_array(void);
static unsigned partition(int *v, unsigned low, unsigned high, unsigned pivot_index);
void* thread_function(void*params);
static void quick_sort(int *v, unsigned low, unsigned high);
static void quick_sort_seq(int *v, unsigned low, unsigned high);
static int issorted(void);

static int *v;
static int threardcounter = 1;
static int MAX_THREADS = 8;
static unsigned int THRESHOLD = 3000000;
static int count = 0;
// static int count;
struct threadArgs {
    int *int_v;
	unsigned int low;
	unsigned int high;
};



static void
print_array(void)
{
    int i;
    for (i = 0; i < MAX_ITEMS; i++)
        printf("%d ", v[i]);
    printf("\n");
}

static void
init_array(void)
{
    int i;
    v = (int *) malloc(MAX_ITEMS*sizeof(int));
    for (i = 0; i < MAX_ITEMS; i++)
        v[i] = rand();
}

static unsigned
partition(int *v, unsigned low, unsigned high, unsigned pivot_index)
{
    /* move pivot to the bottom of the vector */
    if (pivot_index != low)
        swap(v, low, pivot_index);

    pivot_index = low;
    low++;

    /* invariant:
     * v[i] for i less than low are less than or equal to pivot
     * v[i] for i greater than high are greater than pivot
     */

    /* move elements into place */
    while (low <= high) {
        if (v[low] <= v[pivot_index])
            low++;
        else if (v[high] > v[pivot_index])
            high--;
        else
            swap(v, low, high);
    }

    /* put pivot back between two groups */
    if (high != pivot_index)
        swap(v, pivot_index, high);
    return high;
}

void*
thread_function(void*params){
    struct threadArgs *args= (struct threadArgs*) params;
    unsigned int low = args->low;
    unsigned int high = args->high;
    int *int_v = args->int_v;

    
    quick_sort(int_v, low, high);
    pthread_mutex_lock(&lock);
    threardcounter--;
    pthread_mutex_unlock(&lock);
    free(args);

    return NULL;
}

static void
quick_sort(int *v, unsigned low, unsigned high)
{
    unsigned pivot_index;
    unsigned left_size;
    unsigned right_size;

    /* no need to sort a vector of zero or one element */
    if (low >= high){
        return;
    }

    if((high - low+1) < THRESHOLD){
        quick_sort_seq(v, low, high);
        return;
    }

    /* select the pivot value */
    pivot_index = (low+high)/2;

    /* partition the vector */
    

    pivot_index = partition(v, low, high, pivot_index);

    left_size = pivot_index -low;
    right_size = high - pivot_index;
    // printf("left_size = %d\n", left_size);
    // printf("right_size = %d\n", right_size);


    pthread_t* children;
    struct threadArgs* argsleft;
    struct threadArgs* argsright;
    unsigned int numThreads = 2;
    children = malloc(numThreads * sizeof(pthread_t));
    unsigned int childcount = 0;
    unsigned int can_create = 0;
    if(children != NULL){

        if(left_size >= right_size){
            // printf("left_size  \n");
            int result;
            if(low < pivot_index){
                argsleft = malloc(sizeof(struct threadArgs));
                if(argsleft != NULL){
                    argsleft->low = low;
                    argsleft->high = pivot_index -1;
                    argsleft->int_v = v;
                    pthread_mutex_lock(&lock);
                    if(threardcounter < MAX_THREADS){
                        threardcounter++;
                        can_create = 1;
                    }
                    pthread_mutex_unlock(&lock);
                    if(can_create == 1){
                        result = pthread_create(&(children[childcount]), NULL, thread_function, (void*)argsleft);

                        if(result == 0){
                            childcount ++;
                            pthread_mutex_lock(&lock);
                            count ++;
                            pthread_mutex_unlock(&lock);
                        }else{
                            free(argsleft);
                            pthread_mutex_lock(&lock);
                            threardcounter --;
                            pthread_mutex_unlock(&lock);
                            quick_sort_seq(v, low, pivot_index-1);
                            
                        }
                    }else{
                        free(argsleft);
                        quick_sort_seq(v, low, pivot_index-1);
                    }

                }else{
                    quick_sort_seq(v, low, pivot_index-1);
                    
                }
            }
            if(pivot_index < high){
            
                argsright = malloc(sizeof(struct threadArgs));
                if(argsright != NULL){
                    argsright->low = pivot_index +1;
                    argsright->high = high;
                    argsright->int_v = v;

                    can_create = 0;
                    pthread_mutex_lock(&lock);
                    if(threardcounter < MAX_THREADS){
                        threardcounter++;
                        can_create = 1;
                    }
                    pthread_mutex_unlock(&lock);

                    if(can_create == 1){

                        result= pthread_create(&(children[childcount]), NULL, thread_function, (void*)argsright);
                        if(result == 0){
                            childcount ++;
                            pthread_mutex_lock(&lock);
                            count ++;
                            pthread_mutex_unlock(&lock);
                        }
                        else{
                            free(argsright);
                            pthread_mutex_lock(&lock);
                            threardcounter --;
                            pthread_mutex_unlock(&lock);
                            quick_sort_seq(v, pivot_index+1, high);
                            
                        }
                    }else{
                        free(argsright);
                        quick_sort_seq(v, pivot_index+1, high);
                    }

                }else{
                    quick_sort_seq(v, pivot_index+1, high);
                    
                }
            
            }
            
            for(unsigned int id = 0; id < childcount; id++){
                pthread_join(children[id], NULL);
            }

        }else{
            // printf("right_size  \n");
            int result;
            if(pivot_index < high){
                argsright = malloc(sizeof(struct threadArgs));
                if(argsright != NULL){
                    argsright->low = pivot_index +1;
                    argsright->high = high;
                    argsright->int_v = v;

                    can_create = 0;
                    pthread_mutex_lock(&lock);
                    if(threardcounter < MAX_THREADS){
                        threardcounter++;
                        can_create = 1;
                    }

                    pthread_mutex_unlock(&lock);

                    if(can_create == 1){

                        result= pthread_create(&(children[childcount]), NULL, thread_function, (void*)argsright);
                        if(result == 0){
                            childcount ++;
                            pthread_mutex_lock(&lock);
                            count ++;
                            pthread_mutex_unlock(&lock);
                        }
                        else{
                            free(argsright);
                            pthread_mutex_lock(&lock);
                            threardcounter --;
                            pthread_mutex_unlock(&lock);
                            quick_sort_seq(v, pivot_index+1, high);
                            
                        }
                    }else{
                        free(argsright);
                        quick_sort_seq(v, pivot_index+1, high);
                    }

                }else{
                    quick_sort_seq(v, pivot_index+1, high);
                    
                }


            }

            if(low < pivot_index){
                argsleft = malloc(sizeof(struct threadArgs));
                if(argsleft != NULL){
                    argsleft->low = low;
                    argsleft->high = pivot_index -1;
                    argsleft->int_v = v;

                    can_create = 0;
                    pthread_mutex_lock(&lock);
                    if(threardcounter < MAX_THREADS){
                        threardcounter++;
                        can_create = 1;
                    }
                    pthread_mutex_unlock(&lock);
                    if(can_create == 1){
                        result = pthread_create(&(children[childcount]), NULL, thread_function, (void*)argsleft);

                        if(result == 0){
                            childcount ++;
                            pthread_mutex_lock(&lock);
                            count ++;
                            pthread_mutex_unlock(&lock);
                        }else{
                            free(argsleft);
                            pthread_mutex_lock(&lock);
                            threardcounter --;
                            pthread_mutex_unlock(&lock);
                            quick_sort_seq(v, low, pivot_index-1);
                            
                        }
                    }else{
                        free(argsleft);
                        quick_sort_seq(v, low, pivot_index-1);
                    }

                }else{
                    quick_sort_seq(v, low, pivot_index-1);
                    
                }
            }

            for(unsigned int id = 0; id < childcount; id++){
                pthread_join(children[id], NULL);
            }

        }
    }else{
        if (low < pivot_index)
            quick_sort_seq(v, low, pivot_index-1);
        if (pivot_index < high)
            quick_sort_seq(v, pivot_index+1, high);
    }
    free(children);

}

static void
quick_sort_seq(int *v, unsigned low, unsigned high)
{
    unsigned pivot_index;

    /* no need to sort a vector of zero or one element */
    if (low >= high)
        return;

    /* select the pivot value */
    pivot_index = (low+high)/2;

    /* partition the vector */
    pivot_index = partition(v, low, high, pivot_index);

    /* sort the two sub arrays */
    if (low < pivot_index)
        quick_sort_seq(v, low, pivot_index-1);
    if (pivot_index < high)
        quick_sort_seq(v, pivot_index+1, high);
}


static int issorted(void){
    for (int i = 0; i< (MAX_ITEMS - 1); i++){
        if(v[i] > v[i+1]){
            return 0;
        }
    }

    return 1;
}


int
main(int argc, char **argv)
{
    struct timespec start, end;
    // count = 0;
    init_array();
    // print_array();
    clock_gettime(CLOCK_MONOTONIC, &start);
    quick_sort(v, 0, MAX_ITEMS-1);

    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed;
    elapsed = (end.tv_sec -start.tv_sec) + (end.tv_nsec - start.tv_nsec)/1e9;
    printf("elapsed par= %lf\n", elapsed);
    printf("count = %d\n", count);
    if(issorted()== 1){
        printf("the array is sorted\n");
    }else{
        printf("the array is not sorted\n");
    }
    // printf("-------------------------------------------------------------------------------------------\n");
    // print_array();
    free(v);
    
}
