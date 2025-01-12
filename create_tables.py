def create_table_one():
    with open("instances_combinations_values/instances_A.txt", "r") as file:
        results = []
        for line in file:
            last_two_values = line.strip().split()[-2:]
            results.append(last_two_values)
            after_random = results[0]
            before_random = results[1]
            print(after_random, before_random)




