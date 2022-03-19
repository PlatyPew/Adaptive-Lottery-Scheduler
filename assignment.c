#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE_LENGTH 512 // Max numer of bytes per line

// Tickets allocated based on long or short job
#define LONG_JOB_TICKETS = 1
#define SHORT_JOB_TICKETS = 10

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
size_t getLength(process* processes) {
    size_t length = 0;
    while ((processes + length)->pa->processNum != -1)
        length++;

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

int main(int argc, char** argv) {
    if (argc != 2)
        return 1;

    FILE* fp = fopen(argv[1], "r");
    if (fp == NULL) // Check if file can be opened
        return 1;

    process* processes = fileParse(fp);
    fclose(fp);
    size_t totalProcesses = getLength(processes);

    sortProcesses(processes, totalProcesses);
    printProcesses(processes, totalProcesses);

    free(processes);

    return 0;
}
