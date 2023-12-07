
#ifndef MINIMACORE_EVOLUTION_STATISTICS_H
#define MINIMACORE_EVOLUTION_STATISTICS_H

#include "base_individual.h"
#include <minimacore_concepts.h>

namespace minimacore::genetic_algorithm {

template<floating_point_type F>
class statistic_request_base {
public:
  virtual F operator()(const population_t<F>& population) const = 0;
};

template<floating_point_type F>
class best_fitness_request : public statistic_request_base<F> {
public:
  F operator()(const population_t<F>& population) const override
  {
    auto element = std::ranges::min(population,
                                    [](auto& a, auto& b) { return a->overall_fitness() < b->overall_fitness(); });
    return element->overall_fitness();
  };
};

template<floating_point_type F>
class average_fitness_request : public statistic_request_base<F> {
public:
  F operator()(const population_t<F>& population) const override
  {
    return std::accumulate(population.begin(), population.end(), 0.,
                           [](F current, auto& individual) { return current + individual->overall_fitness(); }) /
           (F) population.size();
  };
};

template<floating_point_type F>
class selection_pressure_request : public statistic_request_base<F> {
public:
  F operator()(const population_t<F>& population) const override
  {
    return best_fitness_request<F>{}(population) / average_fitness_request<F>{}(population);
  };
};

template<floating_point_type F>
using statistic_requests_t = vector<unique_ptr<statistic_request_base<F>>>;

template<floating_point_type F>
class evolution_statistics {
public:
  [[nodiscard]] size_t current_generation() const
  {
    return _generation;
  }
  
  void register_statistic(const population_t<F>& population)
  {
    for (size_t i = 0; i < _statistic_requests.size(); i++)
      _statistics(_generation, i) = _statistic_requests[i](population);
    
  }
  
  size_t operator++()
  {
    return _generation++;
  }

protected:
  Eigen::MatrixX<F> _statistics;
  size_t _generation;
  statistic_requests_t<F> _statistic_requests;
};

}
#endif //MINIMACORE_EVOLUTION_STATISTICS_H
