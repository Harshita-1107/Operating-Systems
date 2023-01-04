#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
//#include <error.h> // commented for Mac OS X support
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <pthread.h>

/*
 * Starts a reader or writer thread. Pass one of the READER/WRITER aliases as
 * type to specify which.
 */
#define READER 1
#define WRITER 2
/* CONFIGURABLE: Comment or uncomment */
#define THREAD_VERBOSE // if defined, it will log when threads start/end
pthread_t start_thread(unsigned type);

/* The locks used by the solution. See functions reader and writer. */
pthread_mutex_t rw;         // the writers lock
pthread_mutex_t r;          // the rc resource lock
int rc = 0;                 // reader count

void *reader(void*);        // the reader thread routine
void *writer(void*);        // the writer thread routine

char *filename; // the file for the readers/writers to read/write to
#define HASH_SIZE 1000      // used to calculate hash of file
void read_file(int id);     // function to define how the reader reads

#define MAX_WRITE_SIZE 1024 // the write_file writes this many random chars
void write_file(int id);    // function to define how the writer writes

/* IMPLEMENTATION SECTION */
pthread_t start_thread(unsigned type) {
    // counts readers/writers and assigns them a more readable id than tid
    static unsigned rc = 0;
    static unsigned wc = 0;

    // build thread
    pthread_t tid;
    pthread_attr_t attr;
    pthread_attr_init(&attr); // defaults are fine

    // start thread
    int *id = malloc(sizeof(*id));
    switch (type) {
    case READER:
        *id = ++rc;
        pthread_create(&tid, &attr, reader, id);
        break;
    case WRITER:
        *id = ++wc;
        pthread_create(&tid, &attr, writer, id);
        break;
    // Assume function call is valid..
    }
    return tid;
}

void *reader(void *arg) {
    int id = *((int*)arg);
#ifdef THREAD_VERBOSE
    printf("[reader: %d] Thread started.\n", id);
#endif

    pthread_mutex_lock(&r);
    if (rc++==0) pthread_mutex_lock(&rw);
    pthread_mutex_unlock(&r);

    read_file(id);
    pthread_mutex_lock(&r);
    if (--rc==0) pthread_mutex_unlock(&rw);
    pthread_mutex_unlock(&r);

#ifdef THREAD_VERBOSE
    printf("[reader: %d] Thread exiting.\n", id);
#endif
    free(arg);
    pthread_exit(NULL);
}

void *writer(void *arg) {
    int id = *((int*)arg);
#ifdef THREAD_VERBOSE
    printf("[writer: %d] Thread started.\n", id);
#endif

    pthread_mutex_lock(&rw);
    write_file(id);
    pthread_mutex_unlock(&rw);

#ifdef THREAD_VERBOSE
    printf("[writer: %d] Thread exiting.\n", id);
#endif
    free(arg);
    pthread_exit(NULL);
}

void read_file(int id) {
    printf("[reader: %d] Started reading.\n", id);
    /* Open the file */
    FILE *fd = fopen(filename, "r");
    if (fd == NULL) {
        perror("Unable to open the file for reading");
        exit(1);
    }
    /* Read file and calculate hash */
    int hash = 0;
    for (int c = fgetc(fd); c!=EOF; c=fgetc(fd)) {
        /* Check for read error */
        if (ferror(fd)) {
            perror("Error while reading the file");
            exit(1);
        }
        hash += c;
    }
    hash %= HASH_SIZE;
    /* Close the file */
    if (fclose(fd)!=0) {
        perror("Unable to close the file opened by reader");
        exit(1);
    }
    printf("[reader: %d] Finished reading file with hash: %d\n", id, hash);
}

void write_file(int id) {
    printf("[writer: %d] Started writing.\n", id);
    FILE *fd = fopen(filename, "w+");
    if (fd == NULL) {
        perror("Unable to open the file for writing");
        exit(1);
    }
    /* Write to the file and calculate hash */
    int hash = 0;
    for (unsigned i = 0; i < MAX_WRITE_SIZE; i++) {
        unsigned char c = rand();
        fputc((char)c, fd);
        /* Check for write error */
        if (ferror(fd)) {
            perror("Error while writing to the file");
            exit(1);
        }
        hash += (int)c;
    }
    hash %= HASH_SIZE;
    /* Close the file */
    if (fclose(fd)!=0) {
        perror("Unable to close the file opened by writer");
        exit(1);
    }
    printf("[writer: %d] Write to file with hash: %d\n", id, hash);
    printf("[writer: %d] Done writing.\n", id);
}

void simple_test() {
    printf("Starting reader thread...\n");
    pthread_t rtid = start_thread(READER);
    printf("Started reader thread.\n");

    printf("Starting writer thread...\n");
    pthread_t wtid = start_thread(WRITER);
    printf("Started writer thread.\n");
    
    printf("Joining reader thread...\n");
    pthread_join(rtid, NULL);
    printf("Joined reader thread.\n");

    printf("Joining writer thread...\n");
    pthread_join(wtid, NULL);
    printf("Joined writer thread.\n");
}

void startrw(unsigned r, unsigned w) {
    pthread_t threads[r+w];
    printf("Starting %u writers.\n", w);
    for (unsigned i = 0; i < w; i++) threads[r+i] = start_thread(WRITER);
    printf("Starting %u readers.\n", r);
    for (unsigned i = 0; i < r; i++) threads[i] = start_thread(READER);
    for (unsigned i = 0; i < r+w; i++) pthread_join(threads[i], NULL);
    printf("All readers and writers' threads are done executing.\n");
}

void startx(unsigned x) {
    pthread_t threads[x*2];
    printf("Starting %u readers and writers total.\n", x*2);
    for (unsigned i = 0; i < x; i++) {
        threads[i*2+1] = start_thread(WRITER);
        threads[i*2] = start_thread(READER);
    }
    for (unsigned i = 0; i < x*2; i++) pthread_join(threads[i], NULL);
    printf("All readers and writers' threads are done executing.\n");
}

/*
 * This will demonstrate the starving of writers.
 */
void starve_writer() {
    printf("Starting 10 readers.\n");
    pthread_t threads[100];
    for (int i = 0; i < 10; i++) threads[i] = start_thread(READER);
    printf("Starting writer.\n");
    threads[10] = start_thread(WRITER);
    printf("Starting remaining readers.\n");
    for (int i = 11; i < 100; i++) threads[i] = start_thread(READER);
    for (unsigned i = 0; i < 100; i++) pthread_join(threads[i], NULL);
}

/* MAIN */
int main(int argc, char *argv[]) {
    // rand() used by write_file
    srand(time(NULL));
    // Buffers make the output stream of multithreading inaccurate.
    setbuf(stdout, NULL);
    // Initialize the mutex locks.
    pthread_mutex_init(&r, NULL);
    pthread_mutex_init(&rw, NULL);

    /* Check args */
    if (argc != 2) { // Incorrect number of arguments
        fprintf(stderr, "Usage: %s file\n", argv[0]);
        return 1;
    } else { // File doesn't exist
        filename = argv[1];
        if (access(filename, R_OK|W_OK) == -1) {
            perror("Please provide existing file with rw permissions");
            return 1;
        }
    }

    /* Start implementation, call test function */
    /* CONFIGURABLE: Call any of the test functions above here  */
    startx(10);         // start 10 readers/writers
    starve_writer();    // prove that writer can be starved
}