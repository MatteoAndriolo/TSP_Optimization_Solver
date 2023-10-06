import argparse
import sys


def parse_arguments():
    parser = argparse.ArgumentParser(description="Description of your program.")

    # Add command line arguments
    addarg = parser.add_argument
    addarg("-i", "--input", type=str, help="Input file")
    addarg("-m", "--model", type=str, help="Model type")
    addarg("--seed", type=int, default=1234, help="Random seed")
    addarg("--grasp", type=str, default="1", help="Grasp probabilities")
    addarg("--timelimit", type=float, default=3600.0, help="Total time limit")
    addarg("-n", "--numinstances", type=int, help="Number of instances")
    addarg("--integercosts", action="store_true", help="Use integer costs")
    addarg("-p", "--plot", action="store_true", help="Enable plotting")
    addarg("-v", "--verbose", action="store_true", help="Enable verbose output")
    addarg("--ttenure", type=int, default=0, help="Tabu tenure")
    addarg("--tsize", type=int, default=0, help="Tabu size")
    addarg("--cplex_perchf", type=float, default=0.5, help="CPLEX percentual for Heuristic")

    args = parser.parse_args()

    # vheck if model input are given
    if args.input is None:
        print("Error: No insolutionput file given")
        sys.exit(1)
    if args.model is None:
        print("Error: No model given")
        sys.exit(1)

    # Create a dictionary to store the arguments and their values
    arg_dict = {}
    for arg in vars(args):
        value = getattr(args, arg)
        if value is not None:
            arg_dict[arg] = value
    print(arg_dict)
    return arg_dict


def argp():
    args_dict = parse_arguments()

    if args_dict.get("verbose", False):
        print("\n\n")
        print("-" * 30)
        print("{:<15} {:<15}".format("Argument", "Value"))
        print("-" * 30)
        for key, value in args_dict.items():
            if value is not None:
                print("{:<15} {:<15}".format(key, str(value)))
        if args_dict.get("grasp", None) is not None:
            print("{:<15} {:<15}".format("grasp", str(args_dict["grasp"])))
        print("-" * 30)
        print("\n\n")

    return args_dict


# Example usage
if __name__ == "__main__":
    args_dict = parse_arguments()
    print(args_dict)
