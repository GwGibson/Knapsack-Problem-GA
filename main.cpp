#include "ProjectTester.h"

#include <iostream>
#include <fstream>


int main(int argc, char* argv[]) {

    // Testing -> Ideally should not have to recompile program if we
    // want to alter these parameters
    constexpr std::size_t num_gens = 1000;
    constexpr std::size_t population_size = 20;
    constexpr std::size_t num_elite = 1;
    constexpr std::size_t tournament_size = 2;
    constexpr double mutation_rate = 0.2;
    constexpr std::size_t num_simulations = 1;

    // Instantiate the tester and create/clear the output file
    ProjectTester tester;
    tester.set_ga_parameters(population_size, num_elite, tournament_size, mutation_rate);
    std::string outfile = "best_costs";
    //tester.solve_random(150, 7500, 150);
    tester.solve_all(num_gens, num_simulations, outfile);
}