
#ifndef MINIMACORE_BASE_INDIVIDUAL_H
#define MINIMACORE_BASE_INDIVIDUAL_H

#include <Eigen/Dense>
#include <memory>
#include <minimacore_concepts.h>
#include <random>
#include <vector>

namespace minimacore::genetic_algorithm {

  using std::shared_ptr;
  using std::unique_ptr;
  using std::vector;

  template <floating_point_type Fp_T> using genome_t = Eigen::VectorX<Fp_T>;

  template <floating_point_type Fp_T> class base_individual {

  public:
    Fp_T overall_fitness() const {
      return _fitness_values.sum();
    }

    auto operator<=>(const base_individual &other) const {
      if (overall_fitness() < other.overall_fitness()) {
        return -1;
      }
      if (overall_fitness() > other.overall_fitness()) {
        return 1;
      }
      return 0;
    }

    void set_objective_fitness(size_t index, Fp_T value) {
      _fitness_values(index) = value;
    }

    const Eigen::VectorX<Fp_T> &get_object_fitnesses() const {
      return _fitness_values;
    }

    Fp_T objective_fitness(size_t index) const {
      return _fitness_values(index);
    }

    const genome_t<Fp_T> &genome() const {
      return _genome;
    }

    genome_t<Fp_T> &genome() {
      return _genome;
    }

    [[nodiscard]] bool is_valid() const {
      return _fitness_values.allFinite();
    }

    explicit base_individual(genome_t<Fp_T> genome, long objective_count)
        : _genome(genome), _fitness_values(objective_count) {
      _fitness_values.setConstant(NAN);
    }

  private:
    genome_t<Fp_T> _genome;
    Eigen::VectorX<Fp_T> _fitness_values;
  };

  template <floating_point_type Fp_T> using individual_ptr = shared_ptr<base_individual<Fp_T>>;
  template <floating_point_type Fp_T> using population_t = vector<individual_ptr<Fp_T>>;
  template <floating_point_type Fp_T> using reproduction_selection_t = vector<individual_ptr<Fp_T>>;

  template <floating_point_type Fp_T> static const individual_ptr<Fp_T> &random_pick(const population_t<Fp_T> &selection_set) {
    std::random_device device{};
    std::mt19937_64 generator{device()};
    std::uniform_int_distribution<size_t> distribution{0, selection_set.size() - 1};
    return selection_set[distribution(generator)];
  }

} // namespace minimacore::genetic_algorithm
#endif // MINIMACORE_BASE_INDIVIDUAL_H
