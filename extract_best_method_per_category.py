from collections import Counter


def extract_best_method_per_category(file_path):
    method_counter = Counter()
    with open(file_path, 'r') as file:
        for line in file:
            if line.startswith("Smallest method:"):
                methods = line.strip().split(":")[1].strip()
                method_counter[methods] += 1
    most_common = method_counter.most_common(1)
    if most_common:
        method, count = most_common[0]
        category = file_path.split("instances_")[1].split(".")[0]
        print(f"Best method for category '{category}' is '{method}' with {count} occurrences.")
    else:
        print("No methods found in the file.")


# Main
extract_best_method_per_category("test_categories/instances_A.txt")
extract_best_method_per_category("test_categories/instances_B.txt")
extract_best_method_per_category("test_categories/instances_C.txt")
extract_best_method_per_category("test_categories/instances_D.txt")
extract_best_method_per_category("test_categories/instances_E.txt")