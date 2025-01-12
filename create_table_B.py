import pandas as pd

def create_compare_randomization_dicts(category):
    path = "instances_combinations_values/instances_" + category + ".txt"
    dictionary_method_net_sum = {}
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
            if dict_key not in dictionary_method_net_sum:
                dictionary_method_net_sum[dict_key] = 0
                dictionary_method_count[dict_key] = 0
            if (before_random != "-1"):
                dictionary_method_net_sum[dict_key] += float(after_random) - float(before_random)
                dictionary_method_count[dict_key] += 1

    return (dictionary_method_net_sum, dictionary_method_count)


def create_compare_randomization_dict(category):
    dicts = create_compare_randomization_dicts(category)
    dictionary_method_net_sum = dicts[0]
    dictionary_method_count = dicts[1]
    dictionary_method_average = {}
    for key in dictionary_method_net_sum:
        if dictionary_method_count[key] == 0:
            dictionary_method_average[key] = "N/A"
        else:
            dictionary_method_average[key] = dictionary_method_net_sum[key] / dictionary_method_count[key]

    return dict(sorted(dictionary_method_average.items()))


def create_table_two():
    dictionary_compare_randomization_A = create_compare_randomization_dict("A")
    dictionary_compare_randomization_B = create_compare_randomization_dict("B")
    dictionary_compare_randomization_C = create_compare_randomization_dict("C")
    dictionary_compare_randomization_D = create_compare_randomization_dict("D")
    dictionary_compare_randomization_E = create_compare_randomization_dict("E")

    # Create the excel file
    all_dictionaries = {
        "A": dictionary_compare_randomization_A,
        "B": dictionary_compare_randomization_B,
        "C": dictionary_compare_randomization_C,
        "D": dictionary_compare_randomization_D,
        "E": dictionary_compare_randomization_E
    }

    df = pd.DataFrame(all_dictionaries).T
    output_file = "instances_combinations_values/compare_randomization.xlsx"
    df.to_excel(output_file, sheet_name="Averages")


# Main
create_table_two()