#include <ostream>

#include "Knapsack.h"

// Constructor that requires the total weight/value pair configurations, the capacity of the backpack and
// the maximum number of items it can hold.
// Also sets the rng tools used for the random backpacks
Knapsack::Knapsack(std::size_t num_configurations, std::size_t max_weight, std::size_t num_items) :
        BinaryCostFunction(num_configurations),
        max_weight_(max_weight),
        num_items_(num_items),
        eng_(std::random_device()()),
        dist_(1, std::max(static_cast<std::size_t>(1), static_cast<std::size_t>(max_weight*3 / num_items)))
{}

// Fills the cost function with random configurations based on its weight capacity and the number of items it can store.
void Knapsack::random_configs() noexcept {
    std::size_t item_no = 0;
    while (++item_no <= num_items_) { // So the total number of combinations is known (2^num_items_).
        this->add_config(dist_(eng_), dist_(eng_));
    }
}

// Adds a configuration variable to the cost function
void Knapsack::add_config(std::size_t weight, std::size_t price) noexcept {
    configurations_.emplace_back(std::make_pair(weight, price));
}

// Evaluates the input chromosome and returns its fitness/cost.
std::size_t Knapsack::eval(const Chromosome& chromosome) const noexcept {
    std::size_t current_weight, cost, num_items, index;
    current_weight = cost = num_items = index = 0;

    // need python zip or something to avoid indexing here
    // (iterate over chromosome and configuration simultaneously)
    for (const auto& gene : chromosome) {
        if (gene == Gene::In) {
            const auto& [weight, price] = configurations_[index];
            current_weight += weight;
            cost += price;
            ++num_items;
        }
        ++index;
    }
    // Could do this check in the loop and end early if it is fulfilled but this is cleaner
    // Performance difference is contingent on problem characteristics and mostly irrelevant
    if (current_weight > max_weight_ || num_items > num_items_) {
        cost = 0;
    }
    return cost;
}

// Greedy approach implementation
std::pair<std::size_t, Knapsack::Chromosome> Knapsack::greedy_solve() const {
    auto backpack = std::vector<Gene>(configurations_.size()); // Initialize empty backpack (chromosome in GA parlance)
    auto configs_copy = configurations_;
    std::sort(std::begin(configs_copy), std::end(configs_copy),
              [](std::pair<std::size_t, std::size_t> i, std::pair<std::size_t, std::size_t> j) {
                return (i.second / i.first) > (j.second / j.first);
              });
    // Add items to backpack according to the greedy approach algorithm
    std::size_t current_weight, num_items, cost;
    current_weight = num_items = cost = 0;
    for (const auto& config: configs_copy) {
        const auto& [weight, price] = config;
        // Test if item will fit
        if (current_weight + weight <= max_weight_ && num_items + 1 <= num_items_) {
            backpack[std::distance(std::cbegin(configurations_),
                                   std::find(std::cbegin(configurations_), std::cend(configurations_),
                                             config))] = Gene::In;
            cost += price;
            current_weight += weight;
            ++num_items;
        }
    }
    return std::make_pair(cost, backpack);
}

// Displays the configurations in the cost function
std::ostream& Knapsack::print(std::ostream& os) const {
    for (const auto& config : configurations_) {
        const auto& [weight, price] = config;
        os << "(Weight, Price): (" << weight << ", " << price << ")\n";
    }
    return os;
}

