
#ifndef MINIMACORE_EVOLUTION_STATISTICS_H
#define MINIMACORE_EVOLUTION_STATISTICS_H

#include "base_individual.h"
#include <minimacore_concepts.h>

namespace minimacore::genetic_algorithm {

template<floating_point_type F>
class statistic_request_base {
public:
  [[nodiscard]] virtual const char* name() const = 0;
  
  [[nodiscard]] virtual F operator()(const population_t<F>& population) const = 0;
  
  virtual ~statistic_request_base() = default;
};

template<floating_point_type F>
class best_fitness_request : public statistic_request_base<F> {
public:
  [[nodiscard]] const char* name() const override
  {
    return "best_fitness";
  }
  
  [[nodiscard]] F operator()(const population_t<F>& population) const override
  {
    auto element = std::ranges::min(population,
                                    [](auto& a, auto& b) { return a->overall_fitness() < b->overall_fitness(); });
    return element->overall_fitness();
  };
};

template<floating_point_type F>
class average_fitness_request : public statistic_request_base<F> {
public:
  [[nodiscard]] const char* name() const override
  {
    return "average_fitness";
  }
  
  [[nodiscard]] F operator()(const population_t<F>& population) const override
  {
    return std::accumulate(population.begin(), population.end(), 0.,
                           [](F current, auto& individual) { return current + individual->overall_fitness(); }) /
           (F) population.size();
  };
};

template<floating_point_type F>
class selection_pressure_request : public statistic_request_base<F> {
public:
  [[nodiscard]] const char* name() const override
  {
    return "selection_pressure";
  }
  
  [[nodiscard]] F operator()(const population_t<F>& population) const override
  {
    return best_fitness_request<F>{}(population) / average_fitness_request<F>{}(population);
  };
};

template<floating_point_type F>
using statistic_request_ptr = unique_ptr<statistic_request_base<F>>;

template<floating_point_type F>
class statistics_requests_factory {
public:
  enum stat_requests {
    best_fitness_stat = 0,
    average_fitness_stat,
    selection_pressure_stat,
  };
  
  virtual statistic_request_ptr<F> make(int request)
  {
    switch (request) {
      case best_fitness_stat: {
        return std::make_unique<best_fitness_request<F>>();
      }
      case average_fitness_stat: {
        return std::make_unique<average_fitness_request<F>>();
      }
      case selection_pressure_stat: {
        return std::make_unique<selection_pressure_request<F>>();
      }
      default:
        break;
    }
    return nullptr;
  }
};

template<floating_point_type F>
class evolution_statistics {
public:
  
  [[nodiscard]] size_t current_generation() const
  {
    return _generation;
  }
  
  void register_statistic(const population_t<F>& population)
  {
    for (size_t i = 0; i < _requests.size(); i++)
      if (auto req = _requests_factory->make(_requests[i]))
        _statistics(_generation, i) = (*req)(population);
    ++_generation;
  }
  
  auto operator[](size_t statistic)
  {
    return _statistics.block(0, statistic, _generation, 1);
  }
  
  F current_value(int statistic) const
  {
    return _statistics(_generation - 1, statistic);
  }
  
  size_t operator++()
  {
    return _generation >= _statistics.rows() ? _generation : ++_generation;
  }
  
  explicit evolution_statistics(Eigen::Index maximum_generations,
                                vector<int> requests = vector<int>{
                                    std::initializer_list<int>{statistics_requests_factory<F>::best_fitness_stat,
                                                               statistics_requests_factory<F>::average_fitness_stat,
                                                               statistics_requests_factory<F>::best_fitness_stat}})
      : _statistics{maximum_generations, Eigen::Index(requests.size())},
        _requests{std::move(requests)}
  {}

protected:
  size_t _generation{0};
  Eigen::MatrixX<F> _statistics;
  vector<int> _requests;

private:
  std::unique_ptr<statistics_requests_factory<F>> _requests_factory{std::make_unique<statistics_requests_factory<F>>()};
};

}
#endif //MINIMACORE_EVOLUTION_STATISTICS_H