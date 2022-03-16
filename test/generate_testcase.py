#!/usr/bin/env python3
from random import randint

START_FILE = 9
END_FILE = 30

MIN_PROCESSES = 1
MAX_PROCESSES = 50

MIN_AT = 0
MAX_AT = 50

MIN_BT = 1
MAX_BT = 50


def gen_processes(num_of_proceeses):
    processes = ''
    for _ in range(num_of_proceeses):
        at = randint(MIN_AT, MAX_AT)
        bt = randint(MIN_BT, MAX_BT)
        processes += f'{at} {bt}\r\n'

    return processes


if __name__ == '__main__':
    for i in range(START_FILE, END_FILE + 1):
        processes = gen_processes(randint(MIN_PROCESSES, MAX_PROCESSES))

        with open(f'testcase{i}.txt', 'w') as f:
            f.write(processes)
