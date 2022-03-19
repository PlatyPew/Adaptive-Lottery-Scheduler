#!/usr/bin/env python3
from random import randint
import argparse

parser = argparse.ArgumentParser(description='Generate test cases')
parser.add_argument('-p', type=int, help='Minimum processes', default=30)
parser.add_argument('-P', type=int, help='Maximum processes', default=50)

parser.add_argument('-a', type=int, help='Minimum arrival time', default=0)
parser.add_argument('-A', type=int, help='Maximum arrival time', default=1000)

parser.add_argument('-b', type=int, help='Minimum burst time', default=1)
parser.add_argument('-B', type=int, help='Maximum burst time', default=1000)

args = parser.parse_args()

START_FILE = 1
END_FILE = 30

MIN_PROCESSES = args.p
MAX_PROCESSES = args.P

MIN_AT = args.a
MAX_AT = args.A

MIN_BT = args.b
MAX_BT = args.B


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

        with open(f'testcase{i:03d}.txt', 'w') as f:
            f.write(processes)
