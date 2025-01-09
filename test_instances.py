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
    # txt_filename = instance_name.replace(".instance.json", ".txt")
    txt_filename = instance_name.replace(".instance.json", "")
    txt_filename = txt_filename + "_" + method + "_" + "_".join(steiner_methods) + ".txt"
    print("txt_filename:", txt_filename)
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
        print("Data from",(method, (steiner_methods)),"--->",data_out)

    # Remove the temporary txt file
    os.remove(txt_filename)
    return (value, (method, (steiner_methods)))


# def run_method(method, instance_name):

#     steiner_methods = ["-proj"]
#     data1 = run_command(instance_name, method, steiner_methods)

#     steiner_methods = ["-proj", "-circum", "-merge"]
#     data2 = run_command(instance_name, method, steiner_methods)

#     steiner_methods = ["-centr", "-mid"]
#     data3 = run_command(instance_name, method, steiner_methods)

#     steiner_methods = ["-proj", "-centr", "-mid", "-circum", "-merge"]
#     data4 = run_command(instance_name, method, steiner_methods)

#     # Compare the values of the outputs
#     all_data = [data1, data2, data3, data4]
#     data = min(all_data, key=lambda x: x[0])
#     return data



def run_method(method, instance_name):
    steiner_methods_list = [
        ["-proj"],
        ["-proj", "-circum", "-merge"],
        ["-centr", "-mid"],
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


# ./opt_triangulation -i challenge_instances_cgshop25/simple-polygon_10_272aa6ea.instance.json -o outputs/output1.json -ant -proj -mid -centr -circum -merge

instance_name = "simple-polygon_10_272aa6ea.instance.json"

data_ls = run_method("-ls", instance_name)
print("ls data:", data_ls)
print("\n\n")

data_sa = run_method("-sa", instance_name)
print("sa data:", data_sa)
print("\n\n")

data_ant = run_method("-ant", instance_name)
print("ant data:", data_ant)
print("\n\n")


