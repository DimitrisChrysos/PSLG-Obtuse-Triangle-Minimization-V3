#include "evaluate_instance.hpp"
#include <cmath>
#include <fstream>
#include <iostream>

namespace evaluate_instance {
    double sum_of_rates_of_convergence = 0;
}

double evaluate_instance::calculate_average_rate_of_convergence(int total_number_of_steiner_points) {
    if (total_number_of_steiner_points == 1 || total_number_of_steiner_points == 0)
        return 0;
    return  sum_of_rates_of_convergence / ((double) (total_number_of_steiner_points - 1));
}

void evaluate_instance::calculate_rate_of_convergence(CDT& cdt_with_new_steiner, int old_obtuse_count, int old_steiner_count) {

    if (old_steiner_count == 0) return;
    if (old_steiner_count == cdt_with_new_steiner.steiner_x.size()) return;
    int obtuse_count_with_new_steiner = count_obtuse_triangles(cdt_with_new_steiner);
    if (obtuse_count_with_new_steiner == 0) return;

    double Numerator = log(((double) obtuse_count_with_new_steiner) / ((double) old_obtuse_count));
    double Denominator = log(((double) (old_steiner_count + 1)) / ((double) old_steiner_count));
    sum_of_rates_of_convergence += std::fabs(Numerator / Denominator);
}

double evaluate_instance::calculate_energy(int obtuse_count, int steiner_count) {
    return ALPHA * obtuse_count + BETA * steiner_count;
}

bool evaluate_instance::file_exists(const std::string& file_name) {
    std::ifstream file(file_name);
    return file.good();
}

void evaluate_instance::for_test_instances_dot_py(int argc, char *argv[], CDT& cdt) {

    // Get the name of the file to append
    std::string file_name_unedited = argv[2];
    size_t lastSlashPos = file_name_unedited.find_last_of('/');
    std::string filename = file_name_unedited.substr(lastSlashPos + 1);
    size_t dotPos = filename.find(".instance.json");
    std::string file_name = filename.substr(0, dotPos);
    file_name += "_";
    file_name += argv[5];
    for (int i = 6; i < argc; i++) {
        file_name += "_";
        file_name += argv[i];
    }
    file_name += ".txt";

    // Check if the program was called from the test_instances.py file
    if (file_exists(file_name) && argc > 5 && strcmp(argv[5], "-preselected_params") != 0) {
        
        double value;
        if (count_obtuse_triangles(cdt) == 0) {
            value = calculate_average_rate_of_convergence(cdt.steiner_x.size());
        }
        else {
            value = calculate_energy(count_obtuse_triangles(cdt), cdt.steiner_x.size());
        }
        std::cout << "value: " << value << std::endl;

        std::ofstream file;
        file.open(file_name, std::ios::app);
        if (file.is_open()) {
            file << value << " " << cdt.value_pre_random;
            file.close();
        } else {
            std::cerr << "Failed to open the file." << std::endl;
        }
    }
}