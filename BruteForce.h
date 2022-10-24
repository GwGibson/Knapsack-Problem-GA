#ifndef PROJECT_BRUTEFORCE_H
#define PROJECT_BRUTEFORCE_H

#include "BinaryCostFunction.h"

class BruteForce {
public:
    using Gene = BinaryCostFunction::Gene;
    using Chromosome = BinaryCostFunction::Chromosome;

    BruteForce() = default;

    void set_cf(const BinaryCostFunction* cf) noexcept;

    void solve();

    std::ostream& print(std::ostream& os) const;
    friend std::ostream& operator<<(std::ostream& os, const BruteForce& bf) { return bf.print(os); }
private:
    const BinaryCostFunction* cf_ = nullptr;
    std::pair<std::size_t, Chromosome> best_;
};
#endif //PROJECT_BRUTEFORCE_H