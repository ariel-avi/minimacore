
#ifndef MINIMACORE_TERMINATION_CONDITION_H
#define MINIMACORE_TERMINATION_CONDITION_H

#include "evolution_statistics.h"

namespace minimacore::genetic_algorithm {

template<floating_point_type F>
class termination_condition_base {
public:
  virtual bool operator()(const evolution_statistics<F>& statistics) const = 0;
};

template<floating_point_type F>
using termination_condition_ptr = unique_ptr<termination_condition_base<F>>;

template<floating_point_type F>
class generation_termination : public termination_condition_base<F> {
public:
  bool operator()(const evolution_statistics<F>& statistics) const override
  {
    return statistics.current_generation() >= _maximum_generations;
  }

private:
  size_t _maximum_generations{0};
};

template<floating_point_type F>
class average_fitness_termination : public termination_condition_base<F> {
public:
  bool operator()(const evolution_statistics<F>& statistics) const override
  {
    return statistics[statistics_requests_factory<F>::average_fitness_stat].bottom() < _max_avg_fitness;
  }

private:
  F _max_avg_fitness{0.};
};

template<floating_point_type F>
class best_fitness_termination : public termination_condition_base<F> {
public:
  bool operator()(const evolution_statistics<F>& statistics) const override
  {
    return statistics[statistics_requests_factory<F>::best_fitness_stat].bottom() < _max_best_fitness;
  }

private:
  F _max_best_fitness{0.};
};

template<floating_point_type F>
class selection_pressure_termination : public termination_condition_base<F> {
public:
  bool operator()(const evolution_statistics<F>& statistics) const override
  {
    return statistics[statistics_requests_factory<F>::selection_pressure_stat].bottom() > _min_selection_pressure;
  }

private:
  F _min_selection_pressure{0.};
};


}
#endif //MINIMACORE_TERMINATION_CONDITION_H
