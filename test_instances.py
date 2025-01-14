import subprocess
import os
from concurrent.futures import ThreadPoolExecutor

def run_command(instance_name, method, steiner_methods, dictionary_after_random, dictionary_before_random, output = False):
    # Command to run
    name = instance_name.replace(".instance.json", "")
    program = "./opt_triangulation"
    arg1 = "-i"
    arg2 = "challenge_instances_cgshop25/" + instance_name
    arg3 = "-o"
    arg4 = ""
    if (output):
        arg4 = f"outputs/test_instances/{name}.json"
    else:
        arg4 = "outputs/temp.json"
    arg5 = method
    command = [program, arg1, arg2, arg3, arg4, arg5]
    for st_method in steiner_methods:
        command.append(st_method)

    # Create a temporary txt file to fill it with the comparison value of the subprocess later 
    txt_filename = name
    txt_filename = txt_filename + "_" + method + "_" + "_".join(steiner_methods) + ".txt"
    with open(txt_filename, 'w') as file:
        pass

    # Run the command
    result = subprocess.run(command, capture_output=True, text=True)

    # Print details if we have an error
    if result.stderr:
        print("stdout:", result.stdout)
        print("stderr:", result.stderr)
        print("\n\n\n")
        print("instance_name:", instance_name)
        print("method:", method)
        print("steiner_methods:", steiner_methods)

    # Open the file and print it's data
    value = ""
    value_pre_random = ""
    compare_method = ""
    with open(txt_filename, 'r') as file:
        data_out = file.read()
        value_rate = data_out.split()[0]
        value_energy = data_out.split()[1]
        value_pre_random = data_out.split()[2]
        if value_rate == "-1":
            compare_method = "energy"
            value = value_energy
        else:
            compare_method = "rate"
            value = value_rate
        # print("Data from",(method, (steiner_methods)),"--->",data_out)

    # Remove the temporary txt file
    os.remove(txt_filename)
    dictionary_after_random[instance_name + method + "".join(steiner_methods)] = value
    dictionary_before_random[instance_name + method + "".join(steiner_methods)] = value_pre_random
    return (value, (method, (steiner_methods)), compare_method)

def run_method(method, instance_name, dictionary_after_random, dictionary_before_random):
    steiner_methods_list = [
        ["-proj"],
        ["-proj", "-circum", "-merge"],
        ["-proj", "-centr", "-mid"],
        ["-proj", "-centr", "-mid", "-circum", "-merge"]
    ]

    # Define a function to run a single command
    def run_single_command(steiner_methods):
        return run_command(instance_name, method, steiner_methods, dictionary_after_random, dictionary_before_random)

    # Execute the commands in parallel
    with ThreadPoolExecutor() as executor:
        results = list(executor.map(run_single_command, steiner_methods_list))

    # 
    print(f"Results for {method}:")
    for i in results:
        print(i)
    # 

    rate_items = [item for item in results if item[2] == "rate"]
    if rate_items:
        return max(rate_items, key=lambda x: x[0])
    else:
        return min(results, key=lambda x: x[0])

def find_variable_name(value):
    for var_name, var_value in globals().items():
        if var_value is value:
            return var_name

def run_instances(instances, dictionary_after_random, dictionary_before_random):
    category = find_variable_name(instances)
    output_dir = "test_categories"
    output_file = f"{output_dir}/{category}.txt"
    os.makedirs(output_dir, exist_ok=True)
    with open(output_file, "w") as file:
        print("Category:", category)
        file.write(f"Category: {category}\n")
        for instance_name in instances:
            file.write(f"Instance: {instance_name}\n")
            print("Instance:", instance_name)

            # Run the methods
            methods_list = [
                ["-ls"],
                ["-sa"],
                ["-ant"]
            ]

            # Define a function to run a single command
            def run_single_method(methods):
                print("Running method:", methods[0])
                return run_method(methods[0], instance_name, dictionary_after_random, dictionary_before_random)

            # Execute the methods in parallel
            with ThreadPoolExecutor() as executor:
                results = list(executor.map(run_single_method, methods_list))
            data_ls, data_sa, data_ant = results


            data_methods = [data_ls, data_sa, data_ant]
            rate_items = [item for item in data_methods if item[2] == "rate"]

            # 
            print("data_methods:")
            for i in data_methods:
                print(i)
            if len(rate_items) > 0:
                print("\nrate_items:")
                for i in rate_items:
                    print(i)
            # 

            best_method = None
            if rate_items:
                biggest = max(rate_items, key=lambda x: x[0])
                best_method = biggest[1][0]
                for st_method in biggest[1][1]:
                    best_method += " " + st_method + ","
                best_method = best_method[:-1]
            else:
                smallest = min(data_methods, key=lambda x: x[0])
                best_method = smallest[1][0]
                for st_method in smallest[1][1]:
                    best_method += " " + st_method + ","
                best_method = best_method[:-1]

            # methods = {
            #     "-ls " + ", ".join(data_ls[1][1]): float(data_ls[0]),
            #     "-sa " + ", ".join(data_sa[1][1]): float(data_sa[0]),
            #     "-ant " + ", ".join(data_ant[1][1]): float(data_ant[0])
            # }
            # best_method = min(methods, key=methods.get)
            file.write(f"Best method: {best_method}\n")
            print("Best method", best_method)
            file.write("\n")
            print("\n")
            
            parts = best_method.split(" ", 1)
            method_used = parts[0]
            sp_methods = [flag.strip() for flag in parts[1].split(",")]
            # print("instance_name:", instance_name)
            # print("method_used:", method_used)
            # print("sp_methods:", sp_methods)
            run_command(instance_name, method_used, sp_methods, dictionary_after_random, dictionary_before_random, True)




# Category A -> Convex Boundary - No Constraints
instances_A = None
with open("tests_categorized/A.txt", "r") as file:
    instances_A = [line.strip() for line in file]

# Category B -> Convex Boundary - Open Constrants
instances_B = None
with open("tests_categorized/B.txt", "r") as file:
    instances_B = [line.strip() for line in file]

# Category C -> Convex Boundary - Close Constraints
instances_C = None
with open("tests_categorized/C.txt", "r") as file:
    instances_C = [line.strip() for line in file]

# Category D -> Not Convex Boundary - Parallel to Axes
instances_D = None
with open("tests_categorized/D.txt", "r") as file:
    instances_D = [line.strip() for line in file]

# Category E -> Not Convex Boundary - No Rules
instances_E = None
with open("tests_categorized/E.txt", "r") as file:
    instances_E = [line.strip() for line in file]



# Main
dictionary_after_random = {}
dictionary_before_random = {}
instances = instances_E
run_instances(instances, dictionary_after_random, dictionary_before_random)

# run_command("simple-polygon_10_272aa6ea.instance.json", "-ls", ["-proj"], dictionary_after_random, dictionary_before_random)
# run_command("point-set_10_4bcb7c21.instance.json", "-ls", ["-proj", "-merge"], dictionary_after_random)

category = find_variable_name(instances)
output_dir = "instances_combinations_values"
output_file = f"{output_dir}/{category}.txt"
os.makedirs(output_dir, exist_ok=True)
with open(output_file, "w") as file:
    for key, value in dictionary_after_random.items():
        file.write(f"{key}: {value} {dictionary_before_random[key]}\n")