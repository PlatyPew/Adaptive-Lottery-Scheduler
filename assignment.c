#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE_LENGTH 32 // Max numer of bytes per line

// Tickets allocated based on long or short job
#define LONG_JOB_TICKETS 1
#define SHORT_JOB_TICKETS 10

// Seed
#define SEED 69420

// Attributes of a process
typedef struct {
    int processNum;    // Process number
    int arrivalTime;   // Time process arrives
    int burstTime;     // Burst time of process
    int remainingTime; // Remaining time process has left to run

    int waitTime;       // Time since last execution
    int exitTime;       // Time when program finishes
    int turnAroundTime; // Turnaround time
    int tickets;        // Number of lottery tickets process has
    int shortJob;       // Check if job is long or short
} processAttr;

// A process and the next process in the queue
typedef struct process {
    processAttr* pa; // Process attributes
    struct process* next;
} process;

int getAvgRemainingTime(process* queue);
int getTotalTickets(process* queue);
int mathCeil(float num);
process* enqueue(process* curr, process* prev);
process* fileParse(FILE* fp);
process* getTailFromQueue(process* queue);
process* getWinner(process* queue);
size_t getLengthProcesses(process* processes);
size_t getLengthQueue(process* queue);
void allocateTickets(process* queue, int avgBurstTime);
void deleteProcess(process** queue, process* p);
void freeProcesses(process* processes, size_t length);
void printProcesses(process* processes, size_t length);
void sortProcesses(process* processes, size_t length);

int main(int argc, char** argv) {
    if (argc != 2) { // Check if there is exactly 1 argument
        puts("Usage: ./assignment <input file>");
        return 1;
    }

    FILE* fp = fopen(argv[1], "r");
    if (fp == NULL) { // Check if file can be opened
        printf("File %s cannot be opened!\n", argv[1]);
        return 1;
    }

    srand(SEED); // Seed lottery numbers

    // Parse processes from file to array
    process* processes;
    if (!(processes = fileParse(fp))) { // Check if there is at least 1 process
        fclose(fp);
        return 1;
    }
    fclose(fp);

    size_t totalProcesses = getLengthProcesses(processes);

    sortProcesses(processes, totalProcesses); // Sorts processes by arrival time

    // Index pointer used to track when to add newly arrived processes
    process* indexPtr = processes + 1; // Add one because the first process is already enqueued
    // Queue means the process queue, which is using the linked list data structure
    process* queue = processes;

    // Starts of clock to arrival time of earliest process
    int timeElapsed = processes->pa->arrivalTime;

    // Add newly arrived processes to queue
    process* prev = processes;
    for (size_t i = 0; i < totalProcesses; i++) {
        process* curr = processes + i;
        // If processes has arrived and not first
        if (i != 0 && curr->pa->arrivalTime == processes->pa->arrivalTime) {
            prev = enqueue(curr, prev);
            indexPtr++; // Tracks process after the last arrived process within all processes
        }
    }

    // While there are still processes to run in the queue
    while (getLengthQueue(queue)) {
        // Determining average burst time
        int avgBurstTime = getAvgRemainingTime(queue);
        int timeSlice = avgBurstTime;

        // Allocating tickets
        allocateTickets(queue, avgBurstTime);

        // Selecting a process to run
        process* winner = getWinner(queue);

        // Check every 1 time unit
        for (int sec = 0; sec < timeSlice; sec++) {
            winner->pa->remainingTime--;
            timeElapsed++;

            // Check if new process arrived
            process* tmpPtr = indexPtr; // Temporary index pointer
            int newProcessArrived = 0;
            process* prev = getTailFromQueue(queue);
            for (size_t i = 0; i < getLengthProcesses(indexPtr); i++) {
                // If new process arrived
                processAttr* p = (indexPtr + i)->pa;
                if (timeElapsed >= p->arrivalTime) {
                    // Add arrived process to queue
                    prev = enqueue(indexPtr + i, prev);

                    // Increment index pointer reference
                    tmpPtr++;
                    newProcessArrived = 1;
                }
            }
            indexPtr = tmpPtr;

            // If process finished running
            if (winner->pa->remainingTime == 0) {
                // Calculate exit and turn around time
                winner->pa->exitTime = timeElapsed;
                winner->pa->turnAroundTime = winner->pa->exitTime - winner->pa->arrivalTime;
                winner->pa->waitTime = winner->pa->turnAroundTime - winner->pa->burstTime;

                // Remove winning process from queue
                deleteProcess(&queue, winner);
                break;
            }

            // Stop if new process arrives
            if (newProcessArrived)
                break;
        }

        /*
         * Guard against edge case: when all processes are completed in queue
         * but there's still processes that have yet to arrive
         * E.g. P1 arrives at time 1 with burst time 1 P2 arrives at time 10 with burst time 1
         */

        // If there's no more processes in the queue
        if (!getLengthQueue(queue)) {
            queue = indexPtr++; // First process is already in queue

            // Update time elapsed to arrival time of new process
            timeElapsed = queue->pa->arrivalTime;
        }
    }

    // Initailise info
    float avgTAT = 0;
    float avgWT = 0;
    int maxTAT = -1;
    int maxWT = -1;

    // Calculate averages and max timings
    for (size_t i = 0; i < totalProcesses; i++) {
        processAttr* p = (processes + i)->pa;
        avgTAT += p->turnAroundTime;
        avgWT += p->waitTime;

        if (p->turnAroundTime > maxTAT)
            maxTAT = p->turnAroundTime;

        if (p->waitTime > maxWT)
            maxWT = p->waitTime;
    }

    printProcesses(processes, totalProcesses); // Prints information on process

    freeProcesses(processes, totalProcesses); // Free memory

    // Ger averages
    avgTAT /= totalProcesses;
    avgWT /= totalProcesses;

    printf("\naverage turnaround time: %.2f\n", avgTAT);
    printf("maximum turnaround time: %d\n", maxTAT);
    printf("average waiting time: %.2f\n", avgWT);
    printf("maximum waitTime time: %d\n", maxWT);

    return 0;
}

/**
 * mathCeil(): returns the ceiling of a float
 * @num: the float to be ceiled
 *
 * Return: the ceiling of the float
 */
int mathCeil(float num) {
    int n = (int)num; // Floors the float
    if (n == num)
        return n;

    return n + 1; // Ceilings the float if floor(num) != num
}

/**
 * fileParse(): parses file and stores data into a process struct
 * @fp: file pointer of file to be parsed
 *
 * Return: array of processes
 */
process* fileParse(FILE* fp) {
    size_t processesCounter = 0;
    process* processes = (process*)malloc(sizeof(process));

    // Read file until EOF
    char ch;
    int charCounter = 0;
    while ((ch = fgetc(fp)) != EOF) { // Get each character until EOF
        char line[LINE_LENGTH];       // Maximum bytes read for each line
        line[charCounter++] = ch;     // Append character to line

        // Handle non-number characters
        if (!(ch == 0x0a || ch == 0x0d || ch == 0x20 || ch == 0x2e || (ch >= 0x30 && ch <= 0x39))) {
            puts("File contains illegal characters!");
            freeProcesses(processes, processesCounter);
            return 0;
        }

        if (ch == '\n') {
            *(line + charCounter) = '\0'; // Separate by newline
            if (strlen(line) <= 2)        // Check if file has ended at new line
                break;

            charCounter = 0;

            // Append to all processes
            (processes + processesCounter)->pa = (processAttr*)malloc(sizeof(processAttr));
            processAttr* p = (processes + processesCounter)->pa;

            p->processNum = processesCounter; // Process counter starts at 0

            // Split by space delimeter and ceiling the numbers to handle floats
            p->arrivalTime = mathCeil(atof(strtok(line, " ")));
            p->burstTime = mathCeil(atof(strtok(NULL, " ")));

            p->remainingTime = p->burstTime; // Remaining time is burst time

            // Setting the remaining attributes to -1
            p->waitTime = -1;
            p->exitTime = -1;
            p->turnAroundTime = -1;
            p->tickets = -1;
            p->shortJob = -1;

            (processes + processesCounter)->next = NULL; // Set pointer to next item to NULL

            // Allocate memory for the next process
            processes = (process*)realloc(processes, sizeof(process) * (++processesCounter + 1));
        }
    }

    // Creates a dummy process to use as a form of null-terminated queue
    (processes + processesCounter)->pa = (processAttr*)malloc(sizeof(processAttr));
    (processes + processesCounter)->pa->processNum = -1;

    // Added the below to initialize values that weren't before
    // Since many parts of the code relies on checking EOF node values
    (processes + processesCounter)->pa->remainingTime = -1;
    (processes + processesCounter)->next = NULL;

    if (!getLengthProcesses(processes)) {
        puts("File does not contain any processes!");
        freeProcesses(processes, 0); // Free null-terminated process in queue
        return 0;
    }

    return processes; // Returns an array of processes
}

/**
 * sortProcesses(): insertion sort processes by arrival time
 * @processes: array of processes
 *
 */
void sortProcesses(process* processes, size_t length) {
    // Does insertion sort
    for (size_t checkingIndex = 1; checkingIndex < length; checkingIndex++) {
        for (size_t count = checkingIndex; count > 0; count--) {
            if ((processes + count)->pa->arrivalTime < (processes + count - 1)->pa->arrivalTime) {
                // Swap values
                process tmp = processes[count];
                processes[count] = processes[count - 1];
                processes[count - 1] = tmp;
            }
        }
    }
}

/**
 * enqueue(): appends process to end of queue
 * @curr: current process
 * @prev: previous process
 *
 * Return: previous pointer
 */
process* enqueue(process* curr, process* prev) {
    prev->next = curr;
    return curr;
}

/**
 * getLength(): returns number of processes
 * @processes: array of processes
 *
 * Return: number of processes
 */
size_t getLengthProcesses(process* processes) {
    if (processes->pa->processNum == -1)
        return 0;

    return 1 + getLengthProcesses(processes + 1); // Recursively gets the next process
}

/**
 * getLengthQueue(): returns length of queue
 * @queue: head of the queue
 *
 * Return: length of queue
 */
size_t getLengthQueue(process* queue) {

    if (queue == NULL || queue->pa->remainingTime <= 0) // Check if head is last node
        return 0;

    return 1 + getLengthQueue(queue->next); // Recursively gets the next process
}

/**
 * printProcesses(): prints all processes
 * @processes: array of processes
 */
void printProcesses(process* processes, size_t length) {
    puts("Process Number   Arrival Time   Burst Time   Turnaround Time   Waiting Time");
    for (size_t i = 0; i < length; i++) {
        processAttr* p = (processes + i)->pa;
        printf("%14d   %12d   %10d   %15d   %12d\n", p->processNum, p->arrivalTime, p->burstTime,
               p->turnAroundTime, p->waitTime);
    }
}

/**
 * getAvgRemainingTime(): returns the average of all remaining time of processes in the queue
 * @queue: head of the queue
 *
 * Return: average remaining time
 */
int getAvgRemainingTime(process* queue) {
    int avgBurstTime = 0;
    size_t length = getLengthQueue(queue);

    // Loops through all processes in the queue
    do {
        processAttr* p = queue->pa;
        avgBurstTime += p->remainingTime;

        queue = queue->next; // Move to next process in queue
    } while (queue != NULL);

    return avgBurstTime / length;
}

/**
 * allocateTickets(): allocate tickets based on long or short job
 * @queue: head of the queue
 * @avgBurstTime: average to compare to
 */
void allocateTickets(process* queue, int avgBurstTime) {
    // Base case when end of queue
    if (queue == NULL)
        return;

    // Short job if remaining burst time <= average remaining burst time
    processAttr* p = queue->pa;
    if (p->remainingTime <= avgBurstTime) {
        p->tickets = SHORT_JOB_TICKETS;
        p->shortJob = 1;
    } else { // Else long job
        p->tickets = LONG_JOB_TICKETS;
        p->shortJob = 0;
    }

    allocateTickets(queue->next, avgBurstTime); // Recursively go to the next process in the queue
}

/**
 * getTotalTickets(): returns all tickets in the queue
 * @queue: head of the queue
 *
 * Return: all tickets in the queue
 */
int getTotalTickets(process* queue) {
    // Base case when end of queue
    if (queue == NULL)
        return 0;

    // Recursively gets the tickets of next process
    return queue->pa->tickets + getTotalTickets(queue->next);
}

/**
 * getWinner(): returns the pointer to the winning process
 * @queue: head of the queue
 *
 * Return: pointer to winning process
 */
process* getWinner(process* queue) {
    // Select random ticket between 1 and all tickets inclusively
    int winningTicket = rand() % getTotalTickets(queue) + 1;
    process* winner = queue;
    int counter = 0;

    // Loop through all processes in queue
    do {
        processAttr* p = queue->pa;
        counter += p->tickets;
        if (counter > winningTicket) {
            winner = queue; // Get a random winning process
            break;
        }

        queue = queue->next;
    } while (queue != NULL);

    return winner;
}

/**
 * deleteProcess(): deletes process from queue
 * @queue: address to start of the queue
 * @p: process to remove
 */
void deleteProcess(process** queue, process* p) {
    // If node is head
    if (*queue == p) {
        *queue = p->next;
        return;
    }

    // Loops through processes and deletes it
    process* tmp = *queue;
    do {
        if (tmp->next == p) {
            tmp->next = p->next;
            return;
        }

        tmp = tmp->next;
    } while (tmp != NULL);
}

/**
 * freeProcessors(): free all used memory
 * @processes: array of processes
 * @length: length of processes
 */
void freeProcesses(process* processes, size_t length) {
    // Include additional memory added as a form of null terminated array
    for (size_t i = 0; i < length + 1; i++) {
        free((processes + i)->pa); // Clear the process attributes
    }
    free(processes); // Free process struct itself
}

/**
 * getTailFromQueue(): returns tail of queue
 * @queue: pointer to head of queue
 *
 * Return: tail node of the queue
 */
process* getTailFromQueue(process* queue) {
    while (queue->next != NULL)
        queue = queue->next;

    return queue;
}
