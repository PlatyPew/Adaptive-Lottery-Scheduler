#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE_LENGTH 512 // Max numer of bytes per line

// Tickets allocated based on long or short job
#define LONG_JOB_TICKETS 1
#define SHORT_JOB_TICKETS 10

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
            *(line + charCounter - 2) = '\0'; // Strip \r\n
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
            p->waitTime = 0;
            p->exitTime = 0;
            p->turnAroundTime = 0;
            p->tickets = 0;
            p->shortJob = 0;

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
    for (int checkingIndex = 1; checkingIndex < length; checkingIndex++) {
        for (int count = checkingIndex; count > 0; count--) {
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
    if (queue == NULL)
        return 0;

    return 1 + getLengthQueue(queue->next);
}

/**
 * printProcesses(): prints all processes
 * @processes: array of processes
 */
void printProcesses(process* processes, size_t length) {
    puts("PNum\tAT\tBT\tRT\tWT\tET\tTAT\tTickets\tShort");

    for (int i = 0; i < length; i++) {
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

    processAttr* p = queue->pa;
    p->tickets += p->waitTime; // Calculating max tickets

    return p->tickets + getTotalTickets(queue->next);
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
        counter += p->tickets + p->waitTime;
        if (counter > winningTicket) {
            winner = queue;
            break;
        }

        queue = queue->next;
    } while (queue != NULL);

    return winner;
}

/**
 * bumpWaitTime(): increase wait time of each process that is not the winner
 * @queue: head of queue
 * @winner: winning process
 */
void bumpWaitTime(process* queue, process* winner) {
    if (queue == NULL)
        return;

    if (queue != winner)
        queue->pa->waitTime += 1;

    bumpWaitTime(queue->next, winner);
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

int main(int argc, char** argv) {
    if (argc != 2)
        return 1;

    FILE* fp = fopen(argv[1], "r");
    if (fp == NULL) // Check if file can be opened
        return 1;

    srand(1234); // Seed lottery numbers

    process* processes = fileParse(fp);
    fclose(fp);
    size_t totalProcesses = getLengthProcesses(processes);

    sortProcesses(processes, totalProcesses);

    // Index pointer used to track when to add newly arrived processes
    process* indexPtr = processes + 1; // Add one because the first process is already enqueued
    // Queue means the process queue, which is using the linked list data structure
    process* queue = processes;

    // Add newly arrived processes to queue
    process* prev = processes;
    for (int i = 0; i < totalProcesses; i++) {
        process* curr = processes + i;
        // If processes has arrived and not first
        if (i != 0 && curr->pa->arrivalTime == processes->pa->arrivalTime) {
            prev = enqueue(curr, prev);
            indexPtr++; // Tracks process after the last arrived process within all processes
        }
    }

    int timeElapsed = 0;

    // While there are still processes to run in the queue
    while (getLengthQueue(queue) != 0) {
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

            // Increase waiting time for all processes that is not selected to run
            bumpWaitTime(queue, winner);

            // Check if new process arrived
            process* tmpPtr = indexPtr; // Temporary index pointer
            int newProcessArrived = 0;
            process* prev = indexPtr - 1;
            for (int i = 0; i < getLengthProcesses(indexPtr); i++) {
                // If new process arrived
                processAttr* p = (indexPtr + i)->pa;
                if (timeElapsed >= p->arrivalTime) {
                    // Update new process waiting time
                    p->waitTime = timeElapsed - p->arrivalTime;

                    // Add arrived process to queue
                    process* curr = indexPtr + i;
                    prev = enqueue(curr, prev);

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
                winner->pa->turnAroundTime = timeElapsed - winner->pa->arrivalTime;

                // Remove winning process from queue
                deleteProcess(&queue, winner);
                break;
            }

            // Stop if new process arrives
            if (newProcessArrived)
                break;
        }
    }

    return 0;
}
