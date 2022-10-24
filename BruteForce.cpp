#include <algorithm>    // std::next_permutation
#include <ostream>

#include "BruteForce.h"

using std::invalid_argument; // If cost function has not been set

void BruteForce::set_cf(const BinaryCostFunction* cf) noexcept {
    cf_ = cf;
    best_ = std::make_pair(0, Chromosome(cf->num_vars()));
}

void BruteForce::solve() {
    if (!cf_) {
        throw invalid_argument("The cost function has not been set.");
    }
    const std::size_t num_genes = cf_->num_vars();
    std::vector<Gene> permutation(num_genes);
    auto& [best_cost, best_chromosome] = best_;
    // Compute the costs of every permutation
    for (std::size_t index = num_genes; index > 0; --index) {
        permutation[index-1] = Gene::In;
        do {
            const std::size_t cost = cf_->eval(permutation);
            if (cost > best_cost) {
                best_cost = cost;
                best_chromosome = permutation;
            }
        } while (std::next_permutation(std::begin(permutation), std::end(permutation)));
    }
}

std::ostream& BruteForce::print(std::ostream& os) const {
    const auto& [best_cost, best_chromosome] = best_;
    os << "Brute force best cost: " << best_cost << '\n';
    os << "Brute force best chromosome: ";
    for (const auto& gene : best_chromosome) {
        os << ((gene == Gene::Out) ? "Out " : "In ");
    }
    os << '\n';
    return os;
}
