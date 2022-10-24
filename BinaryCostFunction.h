#ifndef PROJECT_BINARYCOSTFUNCTION_H
#define PROJECT_BINARYCOSTFUNCTION_H

#include <vector>

// Abstract base class for a binary cost function. Derived classes
// must implement the eval function which evaluates the input chromosomes
// and returns its fitness or cost.
class BinaryCostFunction {
public:
    enum class Gene { Out=0, In=1 }; // vector<bool> might work here but it is generally to be avoided (dynamic bitset not in STL).
    using Chromosome = std::vector<Gene>;

    constexpr explicit BinaryCostFunction(std::size_t num_vars) noexcept : num_vars_(num_vars) {};

    // Destructor is virtual in the abstract base class
    virtual ~BinaryCostFunction() = default;

    // Evaluates the input chromosome and returns its fitness/cost.
    [[nodiscard]] virtual std::size_t eval(const Chromosome& chromosome) const = 0;

    // Accessor for the number of variables
    [[nodiscard]] std::size_t num_vars() const noexcept { return num_vars_; }
private:
    // The number of variables that the cost function will use to evaluate chromosomes.
    const std::size_t num_vars_;
};
#endif //PROJECT_BINARYCOSTFUNCTION_H