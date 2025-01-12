import pandas as pd

def create_methods_dicts(category):
    path = "instances_combinations_values/instances_" + category + ".txt"
    dictionary_method_sum = {}
    dictionary_method_count = {}
    with open(path, "r") as file:
        for line in file:

            # get the dict key
            dict_key = line.split(".json-")[1].split(":")[0]

            # get the last two values
            last_two_values = line.strip().split()[-2:]
            after_random = last_two_values[0]
            before_random = last_two_values[1]

            # add the values to the dictionaries
            if dict_key not in dictionary_method_sum:
                dictionary_method_sum[dict_key] = 0
                dictionary_method_count[dict_key] = 0
            dictionary_method_sum[dict_key] += float(after_random)
            dictionary_method_count[dict_key] += 1

    return (dictionary_method_sum, dictionary_method_count)

def create_method_average_dict(category):
    dicts = create_methods_dicts(category)
    dictionary_method_sum = dicts[0]
    dictionary_method_count = dicts[1]
    dictionary_method_average = {}
    for key in dictionary_method_sum:
        dictionary_method_average[key] = dictionary_method_sum[key] / dictionary_method_count[key]

    return dict(sorted(dictionary_method_average.items()))

def create_table_one():
    dictionary_method_average_A = create_method_average_dict("A")
    dictionary_method_average_B = create_method_average_dict("B")
    dictionary_method_average_C = create_method_average_dict("C")
    dictionary_method_average_D = create_method_average_dict("D")
    dictionary_method_average_E = create_method_average_dict("E")

    # Create the excel file
    all_dictionaries = {
        "A": dictionary_method_average_A,
        "B": dictionary_method_average_B,
        "C": dictionary_method_average_C,
        "D": dictionary_method_average_D,
        "E": dictionary_method_average_E
    }

    df = pd.DataFrame(all_dictionaries).T
    output_file = "instances_combinations_values/method_averages.xlsx"
    df.to_excel(output_file, sheet_name="Averages")

create_table_one()