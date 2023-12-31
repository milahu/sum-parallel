#include <stdio.h> // printf

#include <stdlib.h> // fopen, fseek, fread

// https://stackoverflow.com/questions/3908031/how-to-multithread-c-code/52453354#52453354
#include <threads.h>
#include <stdatomic.h>

// TODO dynamic, use "nproc" (number of CPU cores) and input file size
#define NUM_THREADS 4

#define CHUNK_START_SIZE 100

// TODO dynamic. parse command line arguments
//int debug = 1;
int debug = 0;

atomic_long sum;

struct worker_data_t {
    int worker_id;
    char* input_path;
    long chunk_start;
    long chunk_end;
    long worker_result;
};

typedef struct worker_data_t worker_data_t;

int worker(void* thread_data) {

    worker_data_t* worker_data = (worker_data_t*) thread_data;

    debug && printf("worker %d: starting\n", worker_data->worker_id);

    FILE *fptr;
    fptr = fopen(worker_data->input_path, "r");
    if (fptr == NULL) {
        fprintf(stderr, "worker %d: error: fopen failed\n", worker_data->worker_id);
        exit(1);
    }

  // no, seeking back is more complex
  if (0) {
  /*

    long chunk_start = worker_data->chunk_start;
    if (chunk_start > 0) {
        // seek back to find start of first number
        fseek(fptr, (worker_data->chunk_start - CHUNK_START_SIZE), SEEK_SET);
        char* chunk_start[CHUNK_START_SIZE];
        size_t chunk_start_size = fread(chunk_start, CHUNK_START_SIZE, 1, fptr);
        // find last whitespace
        int i;
        for (i = (chunk_start_size - 1); i > 0; i--) {
            if (chunk_start[i] == '\n') {
                chunk_start -= (chunk_start_size - i);
                break;
            }
        }
        if (i == 0) {
            fprintf(stderr, "worker %d: error: failed to find start of first number before chunk_start %ld\n", worker_data->worker_id, worker_data->chunk_start);
            exit(1);
        }
    }
    fseek(fptr, chunk_start, SEEK_SET);

  */
  } // if (0)
  else {

    if (worker_data->chunk_start > 0) {
        // seek forward to find start of first number
        debug && printf("worker %d: seeking forward to find start of first number\n", worker_data->worker_id);
        fseek(fptr, worker_data->chunk_start, SEEK_SET);
        char c;
        if (fread(&c, 1, 1, fptr) != 1) {
            fprintf(stderr, "worker %d: error: fread failed at chunk_start %ld\n", worker_data->worker_id, worker_data->chunk_start);
            exit(1);
        }
        if ('0' <= c && c <= '9') {
            // this chunk starts with a digit
            // undo the last fread
            fseek(fptr, -1, SEEK_CUR);
            // seek to the next number
            int n = 0;
            if (fscanf(fptr, "%d", &n) != 1) {
                fprintf(stderr, "worker %d: error: fscanf failed at chunk_start %ld\n", worker_data->worker_id, worker_data->chunk_start);
                exit(1);
            }
            debug && printf("worker %d: ok: ignoring number on start of chunk: %d\n", worker_data->worker_id, n);
        }
    }

  }

    long worker_result = 0;

    // 1 and 10 are really slow
    // 100 is still slow, and more is not faster
    //#define FSCANF_RESULT_SIZE 1
    //#define FSCANF_RESULT_SIZE 10
    #define FSCANF_RESULT_SIZE 100
    //#define FSCANF_RESULT_SIZE 1000
    //#define FSCANF_RESULT_SIZE 10000

    int fscanf_result[FSCANF_RESULT_SIZE];
    int fscanf_result_size = 0;
    // fill buffer
    // TODO use only one fread(fptr) or fgets(fptr) to fill a string buffer of about 16*1024 bytes
    //   then use sscanf(bufptr) to read numbers
    //   https://stackoverflow.com/a/25142013/10440128
    // or fread(fptr) ahead to store the file contents in the CPU cache
    //   then use fscanf(fptr)
    while (ftell(fptr) <= worker_data->chunk_end) {
        debug && printf("worker %d: reading numbers from offset %ld\n", worker_data->worker_id, ftell(fptr));
        fscanf_result_size = FSCANF_RESULT_SIZE;
        for (int i = 0; i < FSCANF_RESULT_SIZE; i++) {
            if (fscanf(fptr, "%d", &fscanf_result[i]) != 1) {
                fscanf_result_size = i;
                break;
            }
            // FIXME now this is broken
            if (ftell(fptr) > worker_data->chunk_end) {
                // usually, this will stop after chunk_end
                // so if (chunk_start > 0), we skip the first number
                // because it was consumed by another thread
                fscanf_result_size = i + 1;
                break;
            }
        }
        if (fscanf_result_size == 0) {
            break;
        }
        // summarize
        debug && printf("worker %d: adding %d numbers from %d to %d\n", worker_data->worker_id, fscanf_result_size, fscanf_result[0], fscanf_result[fscanf_result_size - 1]);
        for (int i = 0; i < fscanf_result_size; i++) {
            int n = fscanf_result[i];
            worker_result += n;
            //debug && printf("worker %d: ok: adding number: %d\n", worker_data->worker_id, n);
        }
    }

    fclose(fptr);

    worker_data->worker_result = worker_result;

    return 0;
}

// parallel processing summarize numbers from text file

int main(int argc, char** argv) {

    if (argc != 2) {
        fprintf(stderr, "usage: sum input.txt\n");
        return 1;
    }

    char* input_path = argv[1];

    // get input file size
    FILE* fptr;
    long input_size = 0;
    fptr = fopen(input_path, "r");
    if (fptr == NULL) {
        fprintf(stderr, "error: fopen failed\n");
        return 1;
    }
    fseek (fptr, 0, SEEK_END); // not portable
    input_size = ftell(fptr);
    fclose(fptr);
    debug && printf("input size: %ld\n", input_size);

    // +1: round up
    long chunk_size = (input_size / NUM_THREADS) + 1;

    thrd_t threads[NUM_THREADS];

    worker_data_t worker_data_arr[NUM_THREADS];

    // create threads
    for(int i = 0; i < NUM_THREADS; ++i) {
        worker_data_t* worker_data = &worker_data_arr[i];

        worker_data->worker_id = i;
        worker_data->input_path = input_path;
        worker_data->chunk_start = i * chunk_size;

        //worker_data->chunk_end = (i + 1) * chunk_size; // TODO off by one?
        worker_data->chunk_end = (i + 1) * chunk_size - 1; // TODO off by one?

        worker_data->worker_result = 0;

        debug && printf("starting worker %d for chunk %ld-%ld\n", i, worker_data->chunk_start, worker_data->chunk_end);

        thrd_create(&threads[i], worker, (void*) worker_data);
    }

    // wait for threads to finish
    for(int i = 0; i < NUM_THREADS; ++i) {
        thrd_join(threads[i], NULL);
    }

    // collect results
    long sum = 0;
    for(int i = 0; i < NUM_THREADS; ++i) {
        worker_data_t* worker_data = &worker_data_arr[i];
        sum += worker_data->worker_result;
    }
    printf("%ld\n", sum);
    return 0;
}
