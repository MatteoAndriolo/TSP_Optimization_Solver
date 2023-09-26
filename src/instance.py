import time


def check_input_file(input):
    import os.path

    if input == "test":
        return input
    # if string does not ends with .tsp, add it
    if not input.endswith(".tsp"):
        input += ".tsp"

    if not os.path.isfile(input):
        if os.path.isfile("./data/tsp/" + input):
            return "./data/tsp/" + input

    raise Exception("Input file not found")


class Instance:
    def __init__(
        self,
        name,
        input,
        model,
        integer_costs=True,
        grasp=False,
        numinstances=5,
        timelimit=3600,
        plot=True,
        seed=12345,
        verbose=False,
    ):
        # check if file exists
        self.name=name
        self.input = check_input_file(input)

        self.model = model
        self.integer_costs = integer_costs
        self.timelimit = timelimit
        self.numinstances = numinstances

        self.randomseed = seed
        self.grasp = grasp
        self.verbose = verbose
        self.plot = plot

        # Global data
        self.tstart = None

        # log file is name model + timestamp
        # save time now in readable format
        timenow = time.strftime("%Y-%m-%d_%H-%M-%S", time.localtime())
        # self.path_log_file = "./out/log/" + model + "_" + timenow + ".log"
        self.path_sol_file = "./out/sol/" + model + "_" + timenow + ".out"
        # self.path_plot_file = "./out/plot/" + model + "_" + timenow + ".png"
        # self.log_file = None
        self.sol_file = None
        # self.out_file = None

    def open_file(self):
        # self.log_file = open(self.path_log_file, "w+")
        self.sol_file = open(self.path_sol_file, "w+")
        # self.out_file = open(self.path_plot_file, "w+")

    def close_file(self):
        # self.log_file.close()
        self.sol_file.close()
        # self.out_file.close()

    def print_values(self):
        print("\n" + "-" * 50)
        print("{:<20} {:<20}".format("Attribute", "Value"))
        print("-" * 50)

        attributes = vars(self)  # Get all attributes of the instance

        for attr, value in attributes.items():
            print("{:<20} {:<20}".format(attr, str(value)))

        print("-" * 50 + "\n")

    def set_input(self, input):
        self.input = check_input_file(input)

    def run(self):
        # run src/main.c
        import subprocess
        import sys
        import time

        # Create the command to run
        command = [
            "./bin/main",
            "-input",
            str(self.input),
            "-m",
            str(self.model),
            "--seed",
            str(self.randomseed),
            "--maxtime",
            str(self.timelimit),
            "-v",
            "--numinstances",
            str(self.numinstances),
            "--plot" if self.plot else "",
            "--grasp",
            str(self.grasp),

        ]
        if self.integer_costs:
            command.append("--int")

        # Run the command
        print("Running command: {}".format(" ".join(command)))
        self.tstart = time.time()
        process = subprocess.Popen(
            command, stdout=subprocess.PIPE, stderr=subprocess.PIPE
        )
        stdout, stderr = process.communicate()
        end_time = time.time()
        print("Time elapsed: {} seconds".format(end_time - self.tstart))

        # Check if the process was successful
        if process.returncode != 0:
            print("Error: {}".format(stderr.decode("utf-8")))
            sys.exit(1)

        # Print the output
        print(stdout.decode("utf-8"))

        # Save the solution to a file
        self.sol_file.write(stdout.decode("utf-8"))
