
#ifndef MINIMACORE_BASE_INDIVIDUAL_H
#define MINIMACORE_BASE_INDIVIDUAL_H

#include <Eigen/Dense>
#include <memory>
#include <vector>
#include <minimacore_concepts.h>

namespace minimacore::genetic_algorithm {

using std::shared_ptr;
using std::unique_ptr;
using std::vector;

template<floating_point_type F>
class base_individual {
protected:
  Eigen::VectorX<F> _fitness_values;
  
  explicit base_individual(long objective_count)
      : _fitness_values(objective_count)
  {}

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
  
  void set_fitness_value(size_t index, F value)
  {
    _fitness_values(index) = value;
  }
  
  const Eigen::VectorX<F>& get_fitness_values() const
  {
    return _fitness_values;
  }
  
  F fitness_value(size_t index) const
  {
    return _fitness_values(index);
  }
};

template<floating_point_type F> using individual_ptr = shared_ptr<base_individual<F>>;
template<floating_point_type F> using population_t = vector<individual_ptr<F>>;
template<floating_point_type F> using reproduction_selection_t = vector<individual_ptr<F>>;
template<floating_point_type F> using replacement_selection_t = vector<base_individual<F>*>;

template<floating_point_type F>
class individual : public base_individual<F> {
public:
  explicit individual(Eigen::VectorX<F> genome, long objective_count)
      : base_individual<F>(objective_count),
        _genome(genome)
  {}

private:
  Eigen::VectorX<F> _genome;
  
};

}
#endif //MINIMACORE_BASE_INDIVIDUAL_H
