import subprocess
import os
from concurrent.futures import ThreadPoolExecutor

def run_command(instance_name, method, steiner_methods):
    # Command to run
    program = "./opt_triangulation"
    arg1 = "-i"
    arg2 = "challenge_instances_cgshop25/" + instance_name
    arg3 = "-o"
    arg4 = "outputs/output1.json "
    arg5 = method
    command = [program, arg1, arg2, arg3, arg4, arg5]
    for st_method in steiner_methods:
        command.append(st_method)

    # Create a temporary txt file named from arg2_2 (probably usefull to fill it with the value of the output file)
    txt_filename = instance_name.replace(".instance.json", "")
    txt_filename = txt_filename + "_" + method + "_" + "_".join(steiner_methods) + ".txt"
    with open(txt_filename, 'w') as file:
        pass
    # exit()

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
    with open(txt_filename, 'r') as file:
        data_out = file.read()
        value = data_out
        # print("Data from",(method, (steiner_methods)),"--->",data_out)

    # Remove the temporary txt file
    os.remove(txt_filename)
    return (value, (method, (steiner_methods)))



def run_method(method, instance_name):
    steiner_methods_list = [
        ["-proj"],
        ["-proj", "-circum", "-merge"],
        ["-proj", "-centr", "-mid"],
        ["-proj", "-centr", "-mid", "-circum", "-merge"],
    ]

    # Define a function to run a single command
    def run_single_command(steiner_methods):
        return run_command(instance_name, method, steiner_methods)

    # Execute the commands in parallel
    with ThreadPoolExecutor() as executor:
        results = list(executor.map(run_single_command, steiner_methods_list))

    # Compare the values of the outputs
    data = min(results, key=lambda x: x[0])
    return data

def find_variable_name(value):
    for var_name, var_value in globals().items():
        if var_value is value:
            return var_name

def run_instances(instances):
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
            data_ls = run_method("-ls", instance_name)
            # print("\t- ls data:", data_ls)
            data_sa = run_method("-sa", instance_name)
            # print("\t- sa data:", data_sa)
            data_ant = run_method("-ant", instance_name)
            # print("\t- ant data:", data_ant)

            methods = {
                "ls " + ", ".join(data_ls[1][1]): float(data_ls[0]),
                "sa " + ", ".join(data_sa[1][1]): float(data_sa[0]),
                "ant " + ", ".join(data_ant[1][1]): float(data_ant[0])
            }
            smallest_method = min(methods, key=methods.get)
            # smallest_value = methods[smallest_method]
            file.write(f"Smallest method: {smallest_method}\n")
            print("Smallest method:", smallest_method)
            # print("Smallest value:", smallest_value)
            file.write("\n")
            print("\n")

# Category A -> Convex Boundary - No Constraints
instances_A = ["point-set_10_4bcb7c21.instance.json", 
                "point-set_10_7451a2a9.instance.json", 
                "point-set_20_fa3fd7e0.instance.json", 
                "point-set_40_8cbf31aa.instance.json", 
                "point-set_40_9451c229.instance.json", 
                "point-set_60_27bc003d.instance.json", 
                "point-set_80_ff15444b.instance.json", 
                "point-set_80_d77fb670.instance.json", 
                "point-set_100_dd67678e.instance.json", 
                "point-set_150_982c9ab3.instance.json"]

# Category B -> Convex Boundary - Open Constrants
instances_B = ["simple-polygon-exterior-20_10_6fbd9669.instance.json",
                "simple-polygon-exterior-20_10_ce9152de.instance.json",
                "simple-polygon-exterior-20_20_4ddfa00e.instance.json",
                "simple-polygon-exterior-20_60_57858065.instance.json",
                "simple-polygon-exterior-20_100_512f0fc4.instance.json",
                "simple-polygon-exterior-20_100_8d1c2e30.instance.json",
                "simple-polygon-exterior-20_100_8ff7a64d.instance.json"]

# Category C -> Convex Boundary - Close Constraints
instances_C = ["simple-polygon-exterior_10_c5616894.instance.json",
                "simple-polygon-exterior-20_10_8c4306da.instance.json",
                "simple-polygon-exterior-20_60_28a85662.instance.json",
                "simple-polygon-exterior-40_785575e7.instance.json",
                "simple-polygon-exterior-40_ff947945.instance.json",
                "simple-polygon-exterior-60_ba2c82c0.instance.json",
                "simple-polygon-exterior-80_22d34c7e.instance.json",
                "simple-polygon-exterior-100_f1740925.instance.json",
                "simple-polygon-exterior-150_1301b82e.instance.json",
                "simple-polygon-exterior-250_a97729dd.instance.json"]

# Category D -> Not Convex Boundary - Parallel to Axes
instances_D = ["ortho_10_d2723dcc.instance.json",
                "ortho_20_5a9e8244.instance.json",
                "ortho_20_e2aff192.instance.json",
                "ortho_40_56a6f463.instance.json",
                "ortho_60_5c5796a0.instance.json",
                "ortho_60_c423f527.instance.json",
                "ortho_80_06ee55d4.instance.json",
                "ortho_100_bd1e4a14.instance.json",
                "ortho_150_53eb4022.instance.json",
                "ortho_250_3b977f7e.instance.json"]

# Category E -> Not Convex Boundary - No Rules
instances_E = ["simple-polygon_10_272aa6ea.instance.json",
                "simple-polygon_20_0dda68ed.instance.json",
                "simple-polygon_40_12969fc3.instance.json",
                "simple-polygon_60_17af118a.instance.json",
                "simple-polygon_80_7b8f6c4c.instance.json",
                "simple-polygon_100_6101abad.instance.json",
                "simple-polygon_100_cb23308c.instance.json",
                "simple-polygon_150_743d6b9c.instance.json",
                "simple-polygon_250_432b4814.instance.json",
                "simple-polygon_250_6e9d9c26.instance.json"]

# Main
# instances = []
# instance_name1 = "simple-polygon_10_272aa6ea.instance.json"
# instance_name2 = "ortho_10_d2723dcc.instance.json"
# instances.append(instance_name1)
# instances.append(instance_name2)
run_instances(instances_A)