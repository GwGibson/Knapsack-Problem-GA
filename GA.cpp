#include <numeric>   // std::iota
#include <algorithm> // std::generate
#include <fstream>   // File I/O - export_results
#include <iterator>  // File I/O - export_results

#include "GA.h"

using std::invalid_argument; // If GA parameters are invalid or if cf has no configurations

// Constructor that takes in a seed to allow for deterministic results
GA::GA(std::size_t seed) :
    eng_(seed)
{}

// Sets the cost function that the GA will use. The number of genes in each chromosome is automatically synced
// to the incoming cost function. Will throw if the cost function has not been configured.
void GA::set_cf(const BinaryCostFunction* cf) {
    chromosome_size_ = cf->num_vars();
    if (chromosome_size_ == 0) {
        throw invalid_argument("The cost function has no configurations.");
    }
    cf_ = cf;
}

// Sets the GA operator parameters and adjusts the containers accordingly - also randomly initializes the population
// The mutation rate determines the number of genes that will be mutated each generation.
// Will throw if the cost function has not been set of the parameters are invalid.
void GA::set_parameters(std::size_t pop_size, std::size_t num_elite, std::size_t t_size, double mutation_rate) {
    if (!cf_) {
        throw invalid_argument("The cost function has not been set.");
    }
    // Assuming the user will not use negative parameters as the interface indicates unsigned ints.
    if (pop_size == 0 || num_elite >= pop_size || t_size == 0 || mutation_rate > 1. || mutation_rate < 0.) {
        throw invalid_argument("One or more of the GA parameters is/are invalid.");
    }
    // Set parameters
    population_size_ = pop_size;
    num_elite_ = num_elite;
    tournament_size_ = t_size;
    num_mutations_ = static_cast<std::size_t>(mutation_rate * population_size_ * cf_->num_vars());

    // Adjust containers
    costs_.clear();
    ranks_.clear();
    best_costs_.clear();
    costs_.resize(population_size_);
    ranks_.resize(population_size_);
    std::iota(std::begin(ranks_), std::end(ranks_), 0); // Fills rank from 0 (best) -> population_size - 1 (worst)
    population_.clear();
    for (std::size_t chromosome_no = 0; chromosome_no < population_size_; ++chromosome_no) {
        population_.insert(std::make_pair<const std::size_t&, Chromosome>(chromosome_no, Chromosome(chromosome_size_)));
    }
    rand_init_();
}

// Creates a new generation of chromosomes by crossing and mutating existing chromosomes.
void GA::new_population(std::size_t num_generations) noexcept {
    best_costs_.reserve(num_generations);
    std::size_t generation = 0;
    // Advance the GA through the generations using the selection, cross and mutation operators.
    while (++generation <= num_generations) {
        store_best_cost_();
        Population new_population = population_;
        // Do not alter elite chromosomes
        for (std::size_t rank = num_elite_; rank < population_size_; ++rank) {
#if 0
            // This allows for direct clones which is potentially undesirable
            // Replace the chromosome at the given rank by a new child chromosome
            population_.at(chromosome_at_rank_(rank)) = cross_(select_(), select_());
#else
            // No clones
            const auto parent1 = select_();
            auto parent2 = select_();
            while (parent1 == parent2) {
                parent2 = select_();
            }
            // Replace the chromosome at the given rank by a new child chromosome
            new_population.at(chromosome_at_rank_(rank)) = cross_(parent1, parent2);
 #endif
        }
        population_.swap(new_population); // Swap the old population for the new one
        mutate_();
        calculate_costs_(num_elite_);
        repair_();
    }
}

std::size_t GA::get_solution_generation() const noexcept {
    return std::distance(std::cbegin(best_costs_), std::find(std::cbegin(best_costs_), std::cend(best_costs_),
                                             get_best_cost()));
}

void GA::prep_outfile(const std::string& filename, std::size_t num_gens, std::size_t num_runs) const {
    std::ofstream outfile {filename, std::ios::trunc}; // overwrites existing file
    outfile << population_size_ << '\n';
    outfile << num_elite_ << '\n';
    outfile << tournament_size_ << '\n';
    outfile << static_cast<double>(num_mutations_)/population_size_/cf_->num_vars() << '\n';
    outfile << num_gens << '\n';
    outfile << num_runs << '\n';
    outfile.close();
}

// Appends the best cost in each generation to the input file.
// num_gens +1 representing the best solutions from each generation including the initial generations
void GA::export_results(const std::string& filename) const {
    std::ofstream outfile {filename, std::ios::app}; // appends to existing file
    outfile << '\n';
    outfile << get_solution_generation()+1 << '\n';
    std::copy(std::cbegin(best_costs_), std::cend(best_costs_), std::ostream_iterator<std::size_t>(outfile, "\n"));
    outfile.close();
}

// Displays the chromosome and gene values. Orders them by rank (highest fitness/cost displayed first)
std::ostream& GA::print(std::ostream& os) const {
    for (const auto& chromosome_no : ranks_) {
        os << "Chromosome " << chromosome_no << " - Cost(" << costs_[chromosome_no] << "): ";
        const auto& genes = population_.at(chromosome_no);
        for (const auto& gene : genes) {
            os << ((gene == Gene::Out) ? "Out " : "In ");
        }
        os << '\n';
    }
    return os;
}

// Initializes all the chromosomes with random genes (either Out (0) or In (1))
void GA::rand_init_() noexcept {
    for (auto& member : population_) { // member.second will be a chromosome
        std::generate(std::begin(member.second), std::end(member.second),
                      [this]() { return (rng_(1)) ? Gene::In : Gene::Out; });
    }
    calculate_costs_();
    repair_(); // Ensure starting chromosomes are feasible
}

// Calculates and stores the costs of each chromosome in the current generation.
void GA::calculate_costs_(std::size_t num_elite) noexcept {
    // This should be parallelized but it is not worthwhile for the simple cost function used in this project
    for (std::size_t rank = num_elite; rank < population_size_; ++rank) {
        const std::size_t chromosome_no = chromosome_at_rank_(rank);
        set_cost_(chromosome_no, cf_->eval(population_.at(chromosome_no)));
     }
    // Sort the rankings according to the new costs. Highest cost chromosome is rank 0 etc.
    std::sort(std::begin(ranks_), std::end(ranks_),
              [this](std::size_t i, std::size_t j){ return cost_(i) > cost_(j);} );
}

// Tournament selection with replacement - the same chromosome may be selected multiple times.
// Creates a pool of chromosomes based on the tournament_size_ parameter. The chromosome
// with the highest fitness is selected as the champion.
std::size_t GA::select_() noexcept {
    std::vector<std::size_t> participants(tournament_size_);
    // Choose random chromosomes for the tournament pool
    std::generate(std::begin(participants), std::end(participants),
                  [this] () { return rng_(population_size_-1); });
    // Return the one with the highest fitness/cost
    return *max_element(std::cbegin(participants), std::cend(participants),
                        [this] (std::size_t i, std::size_t j) { return cost_(i) < cost_(j); });
}

#if 0
// Crosses 2 chromosomes and returns a child chromosome that is a combination of the parent chromosomes.
// If the 2 parent chromosomes have the same gene and a given position, the child will have the same gene. If
// the parent genes at a given position are different, the child will get a random gene value (0 or 1).
GA::Chromosome GA::cross_(std::size_t p1_chromosome_no, std::size_t p2_chromosome_no) noexcept {
    const auto& p1 = population_.at(p1_chromosome_no);
    const auto& p2 = population_.at(p2_chromosome_no);
    Chromosome child;
    // Spawn the child chromosome by crossing the two parent chromosomes
    std::transform(p1.cbegin(), p1.cend(), p2.cbegin(), std::back_inserter(child),
                   [this] (const auto& p1_gene, const auto& p2_gene) {
                       return (p1_gene == p2_gene) ? p1_gene : rng_(1) ? Gene::In : Gene::Out;
                   });
    return child;
}
#else
// Crosses 2 chromosomes and returns a child chromosome that is a combination of the parent chromosomes.
// The child gets half of its genes from parent 1 and the other half from parent two. If the number
// of genes is odd, the extra gene comes from the most fit parent.
GA::Chromosome GA::cross_(std::size_t p1_chromosome_no, std::size_t p2_chromosome_no) noexcept {
    // Ensure p1_chromosome_no is the number of the most fit chromosome
    if (cost_(p2_chromosome_no) > cost_(p1_chromosome_no)) {
        std::swap(p1_chromosome_no, p2_chromosome_no);
    }
    // Get parent chromosomes
    const auto& p1 = population_.at(p1_chromosome_no);
    const auto& p2 = population_.at(p2_chromosome_no);
    Chromosome child;
    // Handle odd chromosome lengths
    const auto mid = (chromosome_size_ + 2 - 1) / 2;
    // Merge parent chromosomes around the midpoint into a child
    std::copy(std::cbegin(p1), std::cbegin(p1) + mid, std::back_inserter(child));
    std::copy(std::cbegin(p2) + mid, std::cend(p2), std::back_inserter(child));
    return child;
}
#endif

// Performs the given number of mutations at random throughout the population excluding elite chromosomes. The
// mutation flips the targeted gene from In to Out or Out to In.
// This is an infinite loop if num_elite >= population_size
void GA::mutate_() noexcept {
    std::size_t mutation_no = 0;
    while (mutation_no <= num_mutations_) {
        if (std::size_t rank_to_mutate = rng_(population_size_-1); rank_to_mutate >= num_elite_) {
            // Choose a random, non-elite chromosomes
            auto& member_chromosome = population_.at(chromosome_at_rank_(rank_to_mutate));
            // Choose a random gene in that chromosomes
            auto& current_gene = member_chromosome[rng_(chromosome_size_-1)];
            // Flip the chosen gene
            current_gene = (current_gene == Gene::Out) ? Gene::In : Gene::Out;
            ++mutation_no;
        }
    }
}

// This function assume a chromosome with a cost of 0 is not feasible. This may not always be the case but
// it will suffice for the project.
// Repairs unfeasible chromosomes by removing items from one end until the chromosome is feasible and
// then adding back as many items as possible from the other end.
// This can potentially result in many more calls to the cost function.
void GA::repair_() noexcept {
    bool repair_flag = false;
    for (auto& [chromosome_no, chromosome] : population_) {
        // If the chromosome is not feasible, we repair it
        if (costs_[chromosome_no] == 0) {
            auto begin = std::begin(chromosome);
            const auto end = std::end(chromosome);
            // remove genes until the chromosome is feasible
            do {
                *begin = Gene::Out;
                begin = std::find(begin, end, Gene::In);
            } while (cf_->eval(chromosome) == 0 && begin != end);
            // add genes from the other end until it is no longer feasible
            auto rbegin = std::rbegin(chromosome);
            const auto rend = std::rend(chromosome);
            do {
                rbegin = std::find(rbegin, rend, Gene::Out);
                *rbegin = Gene::In;
            } while (cf_->eval(chromosome) != 0 && rbegin != rend);
            // This if statement is needed in the odd case that a completely
            // empty backpack has a non-zero cost (not relevant for project).
            if (rbegin != rend) {
                *rbegin = Gene::Out; // Revert the last change that made the chromosome unfeasible
            }
            repair_flag = true;
        }
    }
    // Only recalculate all costs in the event a chromosome has been repaired.
    if (repair_flag) {
        calculate_costs_();
    }
}