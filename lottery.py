from collections import deque
from random import randint


# deleting the nth item in the queue
def delete_nth(d: deque, n):
    d.rotate(-n)
    d.popleft()
    d.rotate(n)


# represents a process node
class Process():

    def __init__(self, process_number, arrival_time, burst_time):
        self.process_number = process_number
        self.arrival_time = arrival_time
        self.burst_time = burst_time
        self.r_b_time = burst_time
        self.exit_time = 0
        self.wait_time = 0
        self.turn_around_time = 0
        self.tickets = 0
        self.isShortJob = False

    def __str__(self):
        return f"P_Num: {self.process_number}\n\
                A_Time: {self.arrival_time}\n\
                B_Time: {self.burst_time}\n\
                R_B_Time: {self.r_b_time}\n\
                W_Time: {self.wait_time}\n\
                TA_Time: {self.turn_around_time}\n\n"


# global queue
q = deque()

filename = input("Enter filename: ")
f = open(filename, "r")
# f = open("input3.txt", "r")

# Global reference to all processes in input file
processes = []
process_no = 0

# constructing each process node
for line in f:
    process_no += 1
    l = line.strip("\n")
    l = list(l.split(" "))
    a_time = int(l[0])
    b_time = int(l[1])
    p = Process(process_no, a_time, b_time)
    processes.append(p)

# sort process by arrival time
processes.sort(key=lambda process: process.arrival_time)

# index pointer used to track when to add newly arrived processes
index_ptr = 0

# add the newly arrived processes to queue
for p in processes:
    if p.arrival_time == processes[0].arrival_time:
        q.append(p)
        index_ptr += 1

current_time = 0

# while there is still process to run in the queue
while (len(q) != 0):

    avg_burst_time = 0
    time_slice = 0
    tickets_in_sys = 0

    # determining avg_burst_time
    for process in q:
        avg_burst_time += process.r_b_time

    avg_burst_time = avg_burst_time // len(q)
    time_slice = avg_burst_time

    # allocating tickets
    for process in q:
        if process.r_b_time <= avg_burst_time:
            process.tickets = 10
            process.isShortJob = True
        else:
            process.tickets = 1
            process.isShortJob = False

    # calculating max tickets
    for process in q:
        tickets_in_sys += process.tickets + process.wait_time

    # selecting process to run
    winning_ticket = randint(1, tickets_in_sys)
    winner = 0
    counter = 0
    for i in range(len(q)):
        counter = counter + q[i].tickets + q[i].wait_time
        if counter > winning_ticket:
            winner = i
            break

    # checking for each second
    for sec in range(1, time_slice + 1):
        q[winner].r_b_time -= 1
        current_time += 1

        # increasing the waiting time for all processes that is not selected to run
        for p in q:
            if p is not q[winner]:
                p.wait_time += 1

        # check if new process arrived
        is_new_process_arrived = False
        for i in range(index_ptr, len(processes)):

            # if new process arrived
            if current_time >= processes[i].arrival_time:

                # update new process waiting time
                processes[i].wait_time = current_time - processes[i].arrival_time

                # add arrived process to queue
                q.append(processes[i])

                # increment index pointer reference
                index_ptr += 1
                is_new_process_arrived = True

        # if process finished running
        if q[winner].r_b_time == 0:
            q[winner].exit_time = current_time
            delete_nth(q, winner)
            break

        if (is_new_process_arrived):
            break

avg_TAT = 0
avg_WT = 0
max_TAT = -1
max_WT = -1
for p in processes:
    process_TAT = p.burst_time + p.wait_time
    avg_TAT += process_TAT
    avg_WT += (p.wait_time)
    if process_TAT > max_TAT:
        max_TAT = process_TAT
    if p.wait_time > max_WT:
        max_WT = p.wait_time

avg_TAT = avg_TAT / len(processes)
avg_WT = avg_WT / len(processes)

print(f"Average turnaround time: {avg_TAT:.2f}")
print(f"Average waiting time: {avg_WT:.2f}")
print(f"Maximum turnaround time: {max_TAT}")
print(f"Maximum waiting time: {max_WT}")

f.close()
