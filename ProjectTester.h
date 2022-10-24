#ifndef PROJECT_PROJECTTESTER_H
#define PROJECT_PROJECTTESTER_H

#include "GA.h"
#include "Knapsack.h"
#include "BruteForce.h"

#include <string>
#include <optional>

class ProjectTester {
public:
    using outfile = const std::optional<const std::string>&;;

    // Constructor creates the cost functions/knapsacks from the project handout
    ProjectTester();

    // Solves the project backpacks by bruteforce. Prints out the best cost and backpack configuration for each
    // of the 5 backpacks.
    void print_solutions();

    // Sets the ga parameters. These can be switched to test different combinations.
    void set_ga_parameters(std::size_t pop_size, std::size_t num_elite, std::size_t t_size, double mutation_rate) noexcept;

    // Prepares (creates/clears) a file to write the GA results to.
    static void create_outfile(const std::string& filename);

    // Solves all the project knapsacks. If a filename is specified, results will be written to 5 different files
    // with the knapsack number prepended to the input filename to avoid messing with extensions
    void solve_all(std::size_t num_gens, std::size_t num_runs, outfile filename = std::nullopt);

    // This sucks - can be used to solve each cost function individually - specifying a filename will write the results
    // to file. The results are the number of generation it takes to find the best cost solution followed
    // by the best cost solution in each generation (num_gens + 1) entries.
    void solve_cf1(std::size_t num_gens, outfile filename = std::nullopt) {return solve_(num_gens, filename, &cf1); }
    void solve_cf2(std::size_t num_gens, outfile filename = std::nullopt) {return solve_(num_gens, filename, &cf2); }
    void solve_cf3(std::size_t num_gens, outfile filename = std::nullopt) {return solve_(num_gens, filename, &cf3); }
    void solve_cf4(std::size_t num_gens, outfile filename = std::nullopt) {return solve_(num_gens, filename, &cf4); }
    void solve_cf5(std::size_t num_gens, outfile filename = std::nullopt) {return solve_(num_gens, filename, &cf5); }

    // Solves a custom user defined cf using a GA
    void solve_custom(const BinaryCostFunction* cf, std::size_t num_gens, outfile filename = std::nullopt);

    // Solves a random knapsack problem - to show how brute force becomes intractable for larger problems whereas GA
    // time should only increase roughly linearly. Also shows how the GA can potentially miss the optimum solution for
    // more complex problem landscapes.
    void solve_random(std::size_t num_gens, std::size_t max_weight, std::size_t num_items, outfile filename = std::nullopt, bool bf_solve = false);

    // Solves the input cf by brute force
    void bf_solve(const BinaryCostFunction* cf);

    // This also sucks.
    void greedy_solve_cf1() {return greedy_solve(cf1); }
    void greedy_solve_cf2() {return greedy_solve(cf2); }
    void greedy_solve_cf3() {return greedy_solve(cf3); }
    void greedy_solve_cf4() {return greedy_solve(cf4); }
    void greedy_solve_cf5() {return greedy_solve(cf5); }

    // Solves the input CF using the greedy approach - specific to knapsack problems.
    // Returns the cost of the best solution and the solution chromosome
    static void greedy_solve(const Knapsack& knapsack);

private:
    // Backpack #1 from handout
    Knapsack cf1{10, 165, 10};
    // Backpack #2 from handout
    Knapsack cf2{5, 26, 5};
    // Backpack #3 from handout
    Knapsack cf3{6, 190, 6};
    // Backpack #4 from handout
    Knapsack cf4{8, 104, 8};
    // Backpack #5 from handout
    Knapsack cf5{15, 750, 15};

    GA ga;
    BruteForce bf;

    std::size_t pop_size_ = 0;
    std::size_t num_elite_ = 0;
    std::size_t t_size_ = 0;
    double mutation_rate_ = 0.;

    void solve_(std::size_t num_gens, outfile filename, const BinaryCostFunction* cf);
};


#endif //PROJECT_PROJECTTESTER_H
