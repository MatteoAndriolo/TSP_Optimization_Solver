from argp import arpg
from instance import Instance
import re


def get_opt_solutions(filename="data/solutions.csv"):
    import csv

    solutions = {}

    with open(filename, "r") as csv_file:
        # Create a CSV reader
        csv_reader = csv.reader(csv_file)
        next(csv_reader)  # skip the header

        # Loop through each row in the CSV file
        for row in csv_reader:
            # Assuming the first column is the instance name and the second column is the value
            instance = row[0]
            value = row[1]
            solutions[instance] = value

    return solutions


def check_datafiles_and_solutions(solutions, data_dir="./data/tsp/", extension=".tsp"):
    import os.path

    remove = []

    # check if all file names in solutions.keys() are in the data/tsp folder
    for instance in solutions.keys():
        if not os.path.isfile(data_dir + instance + extension):
            print("Instance {} not found in data/tsp folder".format(instance))
            remove.append(instance)

    # and also the opposite: print file names in data/tsp folder that are not in solutions.keys()
    for filename in os.listdir(data_dir):
        if filename.endswith(extension):
            instance = filename[:-4]
            if instance not in solutions.keys():
                print("Instance {} not found in solutions.csv".format(instance))
                remove.append(instance)

    # remove all instances that are not in both
    for instance in remove:
        del solutions[instance]
    return solutions


def analyze_run(solution, instance):
    instance: Instance
    solution: dict

    with open(instance.sol_file, "r") as f:
        lines = f.readlines()
        # match pattern "tourlenght 12345.12314" in lines and store values
        # if no match, return None
        pattern = re.compile(r"tourlenght (\d+.\d+)")
        matches = [pattern.findall(line) for line in lines]
        matches = [float(item) for sublist in matches for item in sublist]
        if len(matches) == 0:
            print("nothing")
            return None
        else:
            print(
                "mean is ", sum(matches) / len(matches), " over ", len(matches), " runs"
            )


if __name__ == "__main__":
    solutions = get_opt_solutions()
    solutions = check_datafiles_and_solutions(solutions)

    args = arpg()

    instance = Instance(
        input=args["input"],
        model=args["model"],
        seed=args["seed"],
        integer_costs=args["integercosts"],
        timelimit=args["timelimit"],
        numinstances=args["numinstances"],
        plot=args["plot"],
    )
    instance.open_file()
    if args["input"] == "test":
        for file in solutions.keys():
            instance.set_input(file)
            instance.run()
    else:
        instance.run()
    instance.close_file()

    # print solution for args["input"]
    print("Solution for {}: {}".format(args["input"], solutions[args["input"]]))

    analyze_run(solutions, instance)
