import subprocess
import os

# Command to run
# TODO: fix this arguments, need to add the methods and steiner methods (remove -preselected_params)
program = "./opt_triangulation"
arg1 = "-i"
arg2_1 = "challenge_instances_cgshop25/"
arg2_2 = "simple-polygon_10_272aa6ea.instance.json"
arg2 = arg2_1 + arg2_2
arg3 = "-o"
arg4 = "outputs/output1.json "
arg5_method = "-ls"
command = [program, arg1, arg2, arg3, arg4, arg5_method]
steiner_methods = ["-proj", "-centr", "-mid", "-circum", "-merge"]
for method in steiner_methods:
    command.append(method)

# Create a temporary txt file named from arg2_2 (probably usefull to fill it with the value of the output file)
txt_filename = arg2_2.replace(".instance.json", ".txt")
with open(txt_filename, 'w') as file:
    pass
print(f"Empty file '{txt_filename}' created successfully.")

# Run the command
result = subprocess.run(command, capture_output=True, text=True)

# Print the output
print("stdout:", result.stdout)
print("stderr:", result.stderr)

# Open the file and print it's data
with open(txt_filename, 'r') as file:
    data_out = file.read()
    print("Data from temp file: " + data_out)
    

# Remove the temporary txt file
os.remove(txt_filename)
print(f"Temporary file '{txt_filename}' deleted successfully.")
