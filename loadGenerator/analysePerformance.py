#!/usr/bin/python3

import os, time
import matplotlib.pyplot as plot
import argparse

parser = argparse.ArgumentParser(description ='Load generator and graph plotter.')
parser.add_argument('--host',
                    type = str, required=True,
                    help ='host: The IP address or hostname of the server.')
parser.add_argument('--port',
                    type = int, required=True,
                    help ='port: The port number on which the server is running.')
parser.add_argument('--min',
                    type = int, required=True,
                    help ='min_concurrent_users: The minimum number of concurrent users to simulate.')
parser.add_argument('--max',
                    type = int, required=True,
                    help ='max_concurrent_users: The maximum number of concurrent users to simulate.')
parser.add_argument('--jump',
                    type = int, required=False,
                    help ='jump-next: Next concurrent user count will be (current+jumpnext). For instance if cuurent user count is 100 and jump is of 50 then next usercount will be 150. Helps in creating smooth graph plots.', default=100)

parser.add_argument('--thinktime',
                    type = float, required=True,
                    help ='thinktime: Time delay (in seconds) between consecutive requests by a single user.')

parser.add_argument('--testingperiod',
                    type = int, required=True,
                    help ='testing_period: The duration (in seconds) over which to conduct the test.')


args = parser.parse_args()
host = args.host
port = args.port
min = args.min
max = args.max
jump = args.jump
thinktime = args.thinktime
testingperiod = args.testingperiod

noOfUsers = list()
throughputs = list()
rtts = list()
os.system("gcc loadGenerator.c -o loadGen")

for i in range(min, max, jump):
    print("Starting for " + str(i) + " number of users")
    os.system(f"taskset -c 1 ./loadGen {host} {port} {str(i)} {thinktime} {testingperiod} >> /dev/null")
    file = open("load_gen.log", "r")
    for line in file:
        if "Number of Users" in line:
            noOfUsers.append(float(line.split(": ")[-1].split('\n')[0].strip()))
        elif "Average Throughput" in line:
            throughputs.append(float(line.split(": ")[-1].split('\n')[0].strip()))
        elif "Average Response Time" in line:
            rtts.append(float(line.split(": ")[-1].split('\n')[0].strip()))
        #plot.show()
    file.close()
    

plot.subplot(211)
plot.plot(noOfUsers, throughputs, label='Throughput', ls='-', color='red', marker='+', markersize=3, mew=2, linewidth=2)
plot.xlabel("Number of Users (n)")
plot.ylabel("Throughput (no of requests/sec)")
plot.grid(visible='on')
plot.legend()
plot.subplot(212)
plot.plot(noOfUsers, rtts, label='Response Time', ls='--', color='green',  marker='o', markersize=3, mew=2, linewidth=2)
plot.xlabel("Number of Users (n)")
plot.ylabel("Response Time (sec)")
plot.grid(visible='on')
plot.legend()
plot.savefig("output.png")
