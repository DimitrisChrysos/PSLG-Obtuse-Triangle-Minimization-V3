import subprocess
import os


A = []
B = []
C = []
D = []
E = []
folder = "challenge_instances_cgshop25"
for file in os.listdir(folder):
    file_path = os.path.join(folder, file)
    if os.path.isfile(file_path) and file_path.endswith(('.json')):
        # print(f"Reading file: {file}")
        program = "./opt_triangulation"
        arg1 = "-i"
        arg2 = "challenge_instances_cgshop25/" + file
        arg3 = "-o"
        command = [program, arg1, arg2, arg3]
        result = subprocess.run(command, capture_output=True, text=True)
        last_word = result.stdout.split()[-1] if result.stdout else ""
        # print("Last word from stdout:", last_word)
        if last_word == "0":
            A.append(file)
        elif last_word == "1":
            B.append(file)
        elif last_word == "2":
            C.append(file)
        elif last_word == "3":
            D.append(file)
        elif last_word == "4":
            E.append(file)


def find_variable_name(value):
    for var_name, var_value in globals().items():
        if var_value is value:
            return var_name

categories = [A, B, C, D, E]
for category in categories:
    print(find_variable_name(category))
    print(f"Count: {len(category)}")
    for file in category:
        print(file)
    print("\n\n")