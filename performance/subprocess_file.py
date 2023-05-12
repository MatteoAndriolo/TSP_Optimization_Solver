import subprocess
import csv
import os
# Define the command to be executed
s = 'nng.em.nn'
command = ['./bin/main', '-f', './data/att48.tsp', '-n', '30', '-m', s]
num_points = s.count('.') + 1
if not os.path.exists('./performance/performance_data.csv'):
    with open('performance_data.csv', 'w', newline='') as file:
        writer = csv.writer(file)
        # Write the first row of the CSV
        row = [num_points] + ['algorithm_' + s.split('.')[i] for i in range(num_points)]
        writer.writerow(row)
else:
    with open('./performance/performance_data.csv', 'w', newline='') as file:
        writer = csv.writer(file)
        # Write the first row of the CSV
        row = [num_points] + ['algorithm_' + s.split('.')[i] for i in range(num_points)]
        writer.writerow(row)
# Launch the command
file.close()
subprocess.call(command)
command2 =  ['/bin/python3', 'fischetti/PerfProf/perfprof.py','./performance/performance_data.csv', './fischetti/PerfProf/pp.pdf', '-G'] 
subprocess.call(command2)