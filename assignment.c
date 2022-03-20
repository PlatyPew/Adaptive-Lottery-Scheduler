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
 * getLength(): returns number of processes
 * @processes: array of processes
 *
 * Return: number of processes
 */
size_t getLengthProcesses(process* processes) {
    size_t length = 0;
    while ((processes + length)->pa->processNum != -1)
        length++;

    return length;
}

size_t getLengthQueue(process* queueHead) {
    size_t length = 0;
    while (queueHead != NULL) {
        length++;
        queueHead = queueHead->next;
    }

    return length;
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

    do {
        processAttr* pa = queueHead->pa;

        printf("%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n", pa->processNum, pa->arrivalTime,
               pa->burstTime, pa->remainingTime, pa->waitTime, pa->exitTime, pa->turnAroundTime,
               pa->tickets, pa->shortJob);

        queueHead = queueHead->next;
    } while (queueHead != NULL);
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
    do {
        processAttr* p = queue->pa;
        // Short job if remaining burst time <= average remaining burst time
        if (p->remainingTime <= avgBurstTime) {
            p->tickets = SHORT_JOB_TICKETS;
            p->shortJob = 1;
        } else { // Else long job
            p->tickets = LONG_JOB_TICKETS;
            p->shortJob = 0;
        }

        queue = queue->next; // Move to next process in queue
    } while (queue != NULL);
}

/**
 * getTotalTickets(): returns all tickets in the queue
 * @queue: head of the queue
 *
 * Return: all tickets in the queue
 */
int getTotalTickets(process* queue) {
    int totalTickets = 0;
    do {
        processAttr* p = queue->pa;
        totalTickets += p->tickets;

        queue = queue->next; // Move to next process in queue
    } while (queue != NULL);

    return totalTickets;
}

int main(int argc, char** argv) {
    if (argc != 2)
        return 1;

    FILE* fp = fopen(argv[1], "r");
    if (fp == NULL) // Check if file can be opened
        return 1;

    process* processes = fileParse(fp);
    fclose(fp);
    size_t totalProcesses = getLengthProcesses(processes);

    sortProcesses(processes, totalProcesses);

    // Index pointer used to track when to add newly arrived processes
    process* indexPtr = processes;
    process* queue = processes;

    // Add newly arrived processes to queue
    process* prev = processes;
    for (int i = 0; i < totalProcesses; i++) {
        process* curr = (processes + i);
        // If processes has arrived
        if (i != 0 && curr->pa->arrivalTime == processes->pa->arrivalTime) {
            prev->next = curr;
            prev = curr;
            indexPtr++; // Tracks last item on the queue
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

        // Calculating max tickets
        int ticketInSystem = getTotalTickets(queue);

        break; // TODO: Remove
    }

    return 0;
}
