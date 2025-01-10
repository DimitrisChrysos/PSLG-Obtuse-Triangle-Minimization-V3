import subprocess
import os


countA = 0
countB = 0
countC = 0
countD = 0
countE = 0
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
            countA += 1
        elif last_word == "1":
            countB += 1
        elif last_word == "2":
            countC += 1
        elif last_word == "3":
            countD += 1
        elif last_word == "4":
            countE += 1


print(f"Count A: {countA}")
print(f"Count B: {countB}")
print(f"Count C: {countC}")
print(f"Count D: {countD}")
print(f"Count E: {countE}")