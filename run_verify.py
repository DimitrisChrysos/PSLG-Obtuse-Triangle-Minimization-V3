import os
import subprocess

folder_input = "challenge_instances_cgshop25"
files_input = [file for file in os.listdir(folder_input) if file.endswith(".json") and os.path.isfile(os.path.join(folder_input, file))]

folder_output = "outputs/test_instances"
files_output = [file for file in os.listdir(folder_output) if file.endswith(".json") and os.path.isfile(os.path.join(folder_output, file))]

program = "python3"
arg1 = "verify.py"
count = 0
for output in files_output:
    for input in files_input:
        input_fixed = input.replace(".instance", "")
        if input_fixed == output:
            arg2 = "challenge_instances_cgshop25/" + input
            arg3 = "outputs/test_instances/" + output
            command = [program, arg1, arg2, arg3]
            result = subprocess.run(command, capture_output=True, text=True)

            print("Verify run for", output)
            print("stdout:", result.stdout)
            print("stderr:", result.stderr)
            print("\n")
            count += 1

print("\nVerify run for", count, "instances")