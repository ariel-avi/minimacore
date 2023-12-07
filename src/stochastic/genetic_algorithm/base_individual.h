
#ifndef MINIMACORE_BASE_INDIVIDUAL_H
#define MINIMACORE_BASE_INDIVIDUAL_H

#include <Eigen/Dense>
#include <memory>
#include <vector>
#include <random>
#include <minimacore_concepts.h>

namespace minimacore::genetic_algorithm {

using std::shared_ptr;
using std::unique_ptr;
using std::vector;

template<floating_point_type F> using genome_t = Eigen::VectorX<F>;

template<floating_point_type F>
class base_individual {

public:
  F overall_fitness() const
  {
    return _fitness_values.sum();
  }
  
  bool operator<(const base_individual& other) const
  {
    return overall_fitness() < other.overall_fitness();
  }
  
  bool operator==(const base_individual& other) const
  {
    return overall_fitness() == other.overall_fitness();
  }
  
  void set_objective_fitness(size_t index, F value)
  {
    _fitness_values(index) = value;
  }
  
  const Eigen::VectorX<F>& get_object_fitnesses() const
  {
    return _fitness_values;
  }
  
  F objective_fitness(size_t index) const
  {
    return _fitness_values(index);
  }
  
  const genome_t<F>& genome() const
  {
    return _genome;
  }
  
  genome_t<F>& genome()
  {
    return _genome;
  }
  
  [[nodiscard]] bool is_valid() const
  {
    return _fitness_values.allFinite();
  }
  
  explicit base_individual(genome_t<F> genome, long objective_count)
      : _genome(genome), _fitness_values(objective_count)
  {
    _fitness_values.setConstant(NAN);
  }

protected:
  genome_t<F> _genome;
  Eigen::VectorX<F> _fitness_values;
};

template<floating_point_type F> using individual_ptr = shared_ptr<base_individual<F>>;
template<floating_point_type F> using population_t = vector<individual_ptr<F>>;
template<floating_point_type F> using reproduction_selection_t = vector<individual_ptr<F>>;

template<floating_point_type F>
static const individual_ptr<F>& random_pick(const population_t<F>& selection_set)
{
  std::random_device device{};
  std::mt19937_64 generator{device()};
  std::uniform_int_distribution<size_t> distribution{0, selection_set.size() - 1};
  return selection_set[distribution(generator)];
}

}
#endif //MINIMACORE_BASE_INDIVIDUAL_H
