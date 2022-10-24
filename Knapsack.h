#ifndef PROJECT_KNAPSACK_H
#define PROJECT_KNAPSACK_H

#include "BinaryCostFunction.h"

#include <random> // For random backpacks

// Implements the cost function for a 0-1 knapsack type problem.
class Knapsack : public BinaryCostFunction {
public:
    using Configurations = std::vector<std::pair<std::size_t, std::size_t>>;

    // Constructor that requires the total weight/value pair configurations, the capacity of the backpack and
    // the maximum number of items it can hold.
    // Also sets the rng tools used for the random backpacks
    Knapsack(std::size_t num_configurations, std::size_t max_weight, std::size_t num_items);

    // Fills the cost function with random configurations based on its weight capacity and the number of items it can store.
    void random_configs() noexcept;

    // Adds configurations to the cost function one at a time.
    void add_config(std::size_t weight, std::size_t price) noexcept;

    // Allows configurations to be added all at once as a sequence of <weight, value> pairs.
    template<typename... Tn>
    constexpr void add_configs(Tn... args) {
        (this->add_config(args.first, args.second), ...);
    }

    // Evaluates the input chromosome and returns its fitness/cost.
    [[nodiscard]] std::size_t eval(const Chromosome& chromosome) const noexcept override;

    // Solves the backpack using a greedy approach. Returns the best cost and the solution chromosome as a pair object.
    [[nodiscard]] std::pair<std::size_t, Chromosome> greedy_solve() const;

    std::ostream& print(std::ostream& os) const;
    friend std::ostream &operator<<(std::ostream& os, const Knapsack& cf) { return cf.print(os); }
private:
    const std::size_t   max_weight_; // The weight capacity of the backpack (assumed to be an unsigned integer based on the project handout)
    const std::size_t   num_items_;  // The maximum number of items that can be stored in the backpack
    Configurations      configurations_; // The weight/value pairs corresponding to the genes in a chromosome

    std::mt19937 eng_{std::random_device{}()};      // rng engine for generating random backpacks
    std::uniform_int_distribution<std::size_t> dist_;   // range [1, max(max_weight*2/num_items,1)]
};
#endif //PROJECT_KNAPSACK_H
