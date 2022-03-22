#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE_LENGTH 32 // Max numer of bytes per line

// Tickets allocated based on long or short job
#define LONG_JOB_TICKETS 1
#define SHORT_JOB_TICKETS 10

// Seed
#define SEED 69420

typedef struct {
    int processNum;
    int arrivalTime;
    int burstTime;
    int remainingTime; // Remaining time process has left to run

    int waitTime; // Time since last execution
    int exitTime;
    int turnAroundTime;
    int tickets;  // Lottery tickets
    int shortJob; // Check if job is long or short
} processAttr;

typedef struct process {
    processAttr* pa;
    struct process* next;
} process;

/**
 * mathCeil(): returns the ceiling of a float
 * @num: the float to be ceiled
 *
 * Return: the ceiling of the float
 */
int mathCeil(float num) {
    int n = (int)num;
    if (n == num)
        return n;

    return n + 1;
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
    while ((ch = fgetc(fp)) != EOF) {
        char line[LINE_LENGTH];
        line[charCounter++] = ch; // Append character to line

        if (ch == '\n') {
            *(line + charCounter) = '\0'; // Strip \r\n
            charCounter = 0;

            // Append to all processes
            (processes + processesCounter)->pa = (processAttr*)malloc(sizeof(processAttr));
            processAttr* p = (processes + processesCounter)->pa;

            p->processNum = processesCounter;
            // Split by space and ceiling the numbers
            p->arrivalTime = mathCeil(atof(strtok(line, " ")));
            p->burstTime = mathCeil(atof(strtok(NULL, " ")));

            p->remainingTime = p->burstTime; // Remaining time is burst time

            // Setting the remaining attributes to 0
            p->waitTime = -1;
            p->exitTime = -1;
            p->turnAroundTime = -1;
            p->tickets = -1;
            p->shortJob = -1;

            (processes + processesCounter)->next = NULL;

            // Allocate memory for the next process
            processes = (process*)realloc(processes, sizeof(process) * (++processesCounter + 1));
        }
    }

    (processes + processesCounter)->pa = (processAttr*)malloc(sizeof(processAttr));
    (processes + processesCounter)->pa->processNum = -1;

    return processes;
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
    if (queue == NULL || queue->pa->remainingTime == 0) // Check if head is last node
        return 0;

    return 1 + getLengthQueue(queue->next);
}

/**
 * printProcesses(): prints all processes
 * @processes: array of processes
 */
void printProcesses(process* processes, size_t length) {
    puts("PNum\tAT\tBT\tRT\tWT\tET\tTAT\tTickets\tShort");

    for (size_t i = 0; i < length; i++) {
        processAttr* p = (processes + i)->pa;
        printf("%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n", p->processNum, p->arrivalTime, p->burstTime,
               p->remainingTime, p->waitTime, p->exitTime, p->turnAroundTime, p->tickets,
               p->shortJob);
    }
}

/**
 * printQueue(): prints all processes in the queue
 * @queueHead: the head of the queue
 */
void printQueue(process* queueHead) {
    puts("PNum\tAT\tBT\tRT\tWT\tET\tTAT\tTickets\tShort");

    while (queueHead != NULL) {
        processAttr* pa = queueHead->pa;

        printf("%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n", pa->processNum, pa->arrivalTime,
               pa->burstTime, pa->remainingTime, pa->waitTime, pa->exitTime, pa->turnAroundTime,
               pa->tickets, pa->shortJob);

        queueHead = queueHead->next;
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
    if (queue == NULL)
        return;

    processAttr* p = queue->pa;
    // Short job if remaining burst time <= average remaining burst time
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
    if (queue == NULL)
        return 0;

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

    do {
        processAttr* p = queue->pa;
        counter += p->tickets;
        if (counter > winningTicket) {
            winner = queue;
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
void freeProcessors(process* processes, size_t length) {
    // Include additional memory added as a form of null terminated array
    for (size_t i = 0; i < length + 1; i++) {
        free((processes + i)->pa);
    }
    free(processes);
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

int main(int argc, char** argv) {
    if (argc != 2)
        return 1;

    FILE* fp = fopen(argv[1], "r");
    if (fp == NULL) // Check if file can be opened
        return 1;

    srand(SEED); // Seed lottery numbers

    process* processes = fileParse(fp);
    fclose(fp);
    size_t totalProcesses = getLengthProcesses(processes);

    sortProcesses(processes, totalProcesses);

    // Index pointer used to track when to add newly arrived processes
    process* indexPtr = processes + 1; // Add one because the first process is already enqueued
    // Queue means the process queue, which is using the linked list data structure
    process* queue = processes;

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

            process* prev = queue;

            // Check for remaining processes
            for (size_t i = 0; i < getLengthProcesses(queue); i++) {
                process* curr = processes + i;
                // If processes has arrived and not first
                if (i != 0 && curr->pa->arrivalTime == timeElapsed) {
                    prev = enqueue(curr, prev);
                    indexPtr++; // Tracks process after the last arrived process within all
                                // processes
                }
            }
        }
    }

    float avgTAT = 0;
    float avgWT = 0;
    int maxTAT = -1;
    int maxWT = -1;

    for (size_t i = 0; i < totalProcesses; i++) {
        processAttr* p = (processes + i)->pa;
        avgTAT += p->turnAroundTime;
        avgWT += p->waitTime;

        if (p->turnAroundTime > maxTAT)
            maxTAT = p->turnAroundTime;

        if (p->waitTime > maxWT)
            maxWT = p->waitTime;
    }

    printProcesses(processes, totalProcesses);

    freeProcessors(processes, totalProcesses);

    avgTAT /= totalProcesses;
    avgWT /= totalProcesses;

    printf("average turnaround time: %.2f\n", avgTAT);
    printf("maximum turnaround time: %d\n", maxTAT);
    printf("average waiting time: %.2f\n", avgWT);
    printf("maximum waitTime time: %d\n", maxWT);

    return 0;
}
