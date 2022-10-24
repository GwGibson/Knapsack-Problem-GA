#include "ProjectTester.h"
#include "Timer.h"

#include <array>    // solve all
#include <iostream> // display results
#include <fstream>  // create output file

using config = std::pair<std::size_t, std::size_t>;

ProjectTester::ProjectTester() {
    cf1.add_configs(config(23, 92), config(31, 57), config(29, 49),
                    config(44, 68), config(53, 60), config(38,43),
                    config(63, 67), config(85, 84), config(89, 87),
                    config(82, 72));

    cf2.add_configs(config(12, 24), config(7, 13), config(11, 23),
                    config(8, 15), config(9, 16));

    cf3.add_configs(config(56, 50), config(59, 50), config(80, 64),
                    config(64, 46), config(75, 50), config(17, 5));

    cf4.add_configs(config(25, 350), config(35, 400), config(45, 450),
                    config(5, 20), config(25, 70), config(3, 8),
                    config(2, 5), config(2, 5));

    cf5.add_configs(config(70, 135), config(73, 139), config(77, 149),
                    config(80, 150), config(82, 156), config(87, 163),
                    config(90, 173), config(94, 184), config(98, 192),
                    config(106, 201), config(110, 210), config(113, 214),
                    config(115, 221), config(118, 229), config(120, 240));
}

void ProjectTester::print_solutions() {
    std::array cfs{cf1, cf2, cf3, cf4, cf5};
    std::size_t backpack = 0;
    for (const auto& cf : cfs) {
        std::cout << "Solving backpack " << ++backpack << " by brute force.\n";
        this->bf_solve(&cf);
        std::cout << '\n';
    }
}

void ProjectTester::set_ga_parameters(std::size_t pop_size, std::size_t num_elite, std::size_t t_size, double mutation_rate) noexcept {
    pop_size_ = pop_size;
    num_elite_ = num_elite;
    t_size_ = t_size;
    mutation_rate_ = mutation_rate;
}

void ProjectTester::create_outfile(const std::string& filename) {
    std::ofstream outfile {filename, std::ios::trunc};
    outfile.close();
}

void ProjectTester::solve_all(std::size_t num_gens, std::size_t num_runs, const std::optional<const std::string>& filename) {
    Knapsack cf6(150, 7500, 150);
    cf6.random_configs();
    std::array cfs{cf1, cf2, cf3, cf4, cf5};
    std::size_t backpack = 0;
    for (const auto& cf : cfs) {
        std::cout << "Evolving Knapsack " << ++backpack << '\n';
        if (filename) {
            std::string name = std::to_string(backpack) + 'r' + *filename;
            ga.set_cf(&cf);
            ga.set_parameters(pop_size_, num_elite_, t_size_, mutation_rate_);
            ga.prep_outfile(name, num_gens, num_runs);
            for (std::size_t i = 0; i < num_runs; ++i) {
                solve_(num_gens, name, &cf);
            }
            bf_solve(&cf);
            greedy_solve(cf);
        } else {
            solve_(num_gens, filename, &cf);
        }
        std::cout << '\n';
    }
}

void ProjectTester::solve_custom(const BinaryCostFunction* cf, std::size_t num_gens, const std::optional<const std::string> &filename) {
    solve_(num_gens, filename, cf);
}

// GA_only toggle prevents the problem from being solved by brute force so that larger backpacks can be used.
void ProjectTester::solve_random(std::size_t num_gens, std::size_t max_weight, std::size_t num_items, const std::optional<const std::string>& filename, bool bf_solve) {
    Knapsack cf(num_items, max_weight, num_items);
    cf.random_configs();
    std::cout << "Evolving a random backpack with a max weight of " << max_weight << " and a max capacity of " << num_items << " items.\n";
    solve_(num_gens, filename, &cf);
    greedy_solve(cf);
    if (bf_solve) {
        std::cout << "\nSolving by brute force.\n";
        this->bf_solve(&cf);
    }
}

void ProjectTester::bf_solve(const BinaryCostFunction* cf) {
    Timer timer;
    bf.set_cf(cf);
    bf.solve();
    bf.print(std::cout);
    timer.time("BF time: ");
}

void ProjectTester::greedy_solve(const Knapsack &knapsack) {
    Timer timer;
    const auto& [best_cost, best_chromosome] = knapsack.greedy_solve();
    std::cout << "Greedy best cost: " << best_cost << '\n';
    std::cout << "Greedy best chromosome: ";
    for (const auto& gene : best_chromosome) {
        std::cout << ((gene == BinaryCostFunction::Gene::Out) ? "Out " : "In ");
    }
    std::cout << '\n';
    timer.time("Greedy time: ");
}

void ProjectTester::solve_(std::size_t num_gens, const std::optional<const std::string>& filename, const BinaryCostFunction* cf) {
    ga.set_cf(cf);
    ga.set_parameters(pop_size_, num_elite_, t_size_, mutation_rate_);
    Timer timer;
    ga.new_population(num_gens); // evolve the population
    std::cout << "GA best solution: " << ga.get_best_cost() << '\n';
    const auto& best_chromosome = ga.get_best_chromosome();
    std::cout << "GA best chromosome: ";
    for (const auto& gene : best_chromosome) { std::cout << ((gene == BinaryCostFunction::Gene::In) ? "In" : "Out") << ' '; }
    std::cout << '\n';
    timer.time("GA time: ");
    if (filename) {
        ga.export_results(*filename);
    }
}










