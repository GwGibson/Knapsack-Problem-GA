#ifndef PROJECT_GA_H
#define PROJECT_GA_H

#include <unordered_map>    // Could use a vector to store the population but it might be messier
#include <random>           // Rng generator & distribution
#include <string>           // For export filename

#include "BinaryCostFunction.h"

// This GA solves binary cost functions where gene values are represented by 0 or 1 (Out or In). It is currently
// a generational implementation where all chromosomes are replaced every generation. Elite chromosomes, if specified,
// will not be replaced or altered.
class GA {
public:
    using Gene = BinaryCostFunction::Gene;
    using Chromosome = BinaryCostFunction::Chromosome;
    using Population = std::unordered_map<std::size_t, Chromosome>;

    // Default constructor that uses random seed for the random number generator
    GA() = default;

    // Constructor that takes in a seed to allow for deterministic results
    explicit GA(std::size_t seed);

    // Sets the cost function that the GA will use. The number of genes in each chromosome is automatically synced
    // to the incoming cost function. Will throw if the cost function has not been configured.
    void set_cf(const BinaryCostFunction* cf);

    // Adjusts the parameters that the GA used to find the solution and randomly initializes the population
    void set_parameters(std::size_t pop_size, std::size_t num_elite, std::size_t t_size, double mutation_rate);

    // Turnover the population of chromosomes by using the select, cross & mutate functions.
    // Will throw if any of the GA parameters are invalid or if the CF has not been set.
    void new_population(std::size_t num_generations) noexcept;

    // Returns the cost of the best chromosome
    [[nodiscard]] std::size_t get_best_cost() const noexcept { return best_costs_.back(); }

    // Returns the best chromosome (vector of genes)
    [[nodiscard]] Chromosome get_best_chromosome() const noexcept { return population_.at(ranks_[0]); }

    // Returns the generation in which the best solution was found
    [[nodiscard]] std::size_t get_solution_generation() const noexcept;

    void prep_outfile(const std::string& filename, std::size_t num_gens, std::size_t num_runs) const;

    // Exports the GA parameters and results to file. This can be improved to extract more info from the
    // GA if desired.
    void export_results(const std::string& filename) const;

    // For testing & possibly output (best chromosome)
    std::ostream& print(std::ostream& os) const;
    friend std::ostream &operator<<(std::ostream& os, const GA& ga) { return ga.print(os); }

    GA &operator=(const GA &) = delete;
    GA(const GA &) = delete;
private:
    std::size_t chromosome_size_ = 0;
    const BinaryCostFunction* cf_ = nullptr;

    // GA parameters
    std::size_t population_size_ = 0;
    std::size_t num_elite_ = 0;
    std::size_t tournament_size_ = 0;
    std::size_t num_mutations_ = 0;

    Population                  population_;    // The population of chromosomes
    std::vector<std::size_t>    ranks_;         // Stores the rank of each chromosome - rank[0] is best
    std::vector<std::size_t>    costs_;         // Stores the costs/fitness' of each chromosome
    std::vector<std::size_t>    best_costs_;    // Stores the highest fitness chromosome of each generation

    // Initializes all the genes at random - uniform distribution of In's (1) and Out's (0)
    // and calculates their initial costs
    void rand_init_() noexcept;

    // Calculate the cost or fitness of each chromosome in the population.
    // num_elite is used to specify elite chromosomes.
    void calculate_costs_(std::size_t num_elite = 0) noexcept;

    // Sets the cost of chromosome 'chromosome_no' to cost
    // No error checking since this is only used internally
    void set_cost_(std::size_t chromosome_no, std::size_t cost) { costs_[chromosome_no] = cost; }

    // Return the costs/fitness' of a chromosome.
    [[nodiscard]] std::size_t cost_(std::size_t chromosome_no) const noexcept { return costs_[chromosome_no]; }

    // Returns the chromosome number of the chromosome with the given rank
    [[nodiscard]] std::size_t chromosome_at_rank_(std::size_t rank) const noexcept { return ranks_[rank]; }

    // Stores the cost of the most fit chromosome - for output purposes
    void store_best_cost_() noexcept { best_costs_.push_back(cost_(ranks_[0])); }

    // Tournament selection with replacement. A higher tournament size increases the selection pressure.
    // Tournament size of 1 is uniform selection.
    [[nodiscard]] std::size_t select_() noexcept;

    // Crosses the 2 parent chromosomes and with the given chromosome numbers and returns a child chromosome
    // that is a combination of the 2 parent chromosomes.
    [[nodiscard]] Chromosome cross_(std::size_t p1_chromosome_no, std::size_t p2_chromosome_no) noexcept;

    // Performs num_mutations mutations at random throughout the population. A mutation simply toggles the targeted
    // gene (In to Out, Out to In). Elite chromosomes (if specified) are immune to mutations.
    void mutate_() noexcept;

    // Ensures all chromosomes correspond to feasible solutions.
    // Assumes unfeasible chromosome have a cost of zero!
    void repair_() noexcept;

    // rng engine
    std::mt19937 eng_{std::random_device{}()};

    // Returns a random unsigned integer in the range [0, max] -> should use population_size_-1 & chromosome_size_-1
    // where applicable as max argument
    std::size_t rng_(std::size_t max) { return std::uniform_int_distribution<std::size_t>{0, max}(eng_); }
};

#endif //PROJECT_GA_H