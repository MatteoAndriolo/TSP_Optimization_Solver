import subprocess

# Define a list of parameters to execute the executable with
executable_adress = "bin/main"
data_adress = "data/att48.tsp"
num_instances = 20

params_list = [
    {
        "-f": data_adress,
        "-m": "nn",
        "-n": num_instances,
    },
    {
        "-f": data_adress,
        "-m": "nn.2opt",
        "-n": num_instances,
    },
    {
        "-f": data_adress,
        "-m": "nng",
        "-n": num_instances,
        "-g": "7.2.1",
    },
    {
        "-f": data_adress,
        "-m": "em",
        "-n": num_instances,
    },
    {
        "-f": data_adress,
        "-m": "em.2opt",
        "-n": num_instances,
    },
]

subprocess.check_output("make")
for params in params_list:
    cmd = [executable_adress] 
    for arg, val in params.items():
        cmd += [arg, str(val)]
    cmd_str = " ".join(cmd)

    output = subprocess.check_output(cmd, universal_newlines=True)
