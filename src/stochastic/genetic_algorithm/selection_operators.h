
#ifndef MINIMACORE_SELECTION_OPERATORS_H
#define MINIMACORE_SELECTION_OPERATORS_H

#include "base_individual.h"
#include <algorithm>
#include <execution>
#include <random>

namespace minimacore::genetic_algorithm {


template<floating_point_type F>
population_t<F>& sort(population_t<F>& population)
{
  std::sort(std::execution::par_unseq, population.begin(), population.end(),
            [](const individual_ptr<F>& a, const individual_ptr<F>& b) {
              return *a < *b;
            });
  return population;
}


template<floating_point_type F>
class base_selection_for_reproduction {
public:
  virtual reproduction_selection_t<F> operator()(population_t<F>& population) const = 0;
  
  virtual ~base_selection_for_reproduction() = default;
};

template<floating_point_type F>
class truncation_selection_for_reproduction : public base_selection_for_reproduction<F> {
  size_t _selection_size = 0;
public:
  reproduction_selection_t<F> operator()(population_t<F>& population) const override
  {
    reproduction_selection_t<F> result;
    result.reserve(_selection_size);
    sort(population);
    for (size_t selected = 0; selected < _selection_size; selected++) result.push_back(population[selected]);
    return result;
  }
  
  explicit truncation_selection_for_reproduction(size_t selection_size) : _selection_size(selection_size)
  {}
};

template<floating_point_type F>
class tournament_selection_for_reproduction : public base_selection_for_reproduction<F> {
public:
  reproduction_selection_t<F> operator()(population_t<F>& population) const override
  {
    std::uniform_int_distribution<size_t> dist(0, population.size() - 1);
    reproduction_selection_t<F> result;
    result.reserve(_selection_size);
    const size_t tournament_size = _tournament_size < population.size() ? _tournament_size : population.size();
    const size_t selection_size = _selection_size < population.size() ? _selection_size : population.size();
    std::set<individual_ptr<F>> selected_individuals;
    std::random_device rd;
    std::mt19937_64 gen(rd());
    while (result.size() < selection_size) {
      reproduction_selection_t<F> tournament_selection;
      while (tournament_selection.size() < tournament_size) {
        auto& selected_for_tournament = population[dist(gen)];
        if (std::find(tournament_selection.begin(), tournament_selection.end(), selected_for_tournament) ==
            tournament_selection.end())
          tournament_selection.push_back(selected_for_tournament);
      }
      
      auto winner = std::min_element(tournament_selection.begin(),
                                     tournament_selection.end(),
                                     [](const individual_ptr<F>& a,
                                        const individual_ptr<F>& b) { return *a < *b; });
      
      if (auto pair = selected_individuals.insert(*winner); pair.second) result.push_back(*pair.first);
      
    }
    return result;
  }
  
  explicit tournament_selection_for_reproduction(size_t tournament_size, size_t selection_size)
      : _tournament_size(tournament_size), _selection_size(selection_size)
  {}

private:
  size_t _tournament_size{0};
  size_t _selection_size{0};
};

template<floating_point_type F>
using ranked_selection_t = vector<population_t<F>>;

class ranked_selection {
public:
  enum select_by : int {
    select_by_ranks = 0,
    select_by_individuals,
  };
  
  template<floating_point_type F>
  static bool is_dominant(const individual_ptr<F>& individual, const reproduction_selection_t<F>& subgroup)
  {
    return !std::any_of(std::execution::par_unseq,
                        subgroup.begin(),
                        subgroup.end(),
                        [&](const auto& comparison) {
                          if (individual != comparison) {
                            bool is_dominant{true};
                            for (size_t i = 0; i < individual->get_object_fitnesses().size(); i++)
                              is_dominant = is_dominant &&
                                  individual->objective_fitness(i) > comparison->objective_fitness(i);
                            return is_dominant;
                          }
                          return false;
                        });
  }
  
  template<floating_point_type F>
  static ranked_selection_t<F> rank_population(const population_t<F>& population)
  {
    ranked_selection_t<F> ranks;
    size_t initial_population_size = population.size();
    population_t<F> cpy(population);
    while (!cpy.empty()) {
      auto& current_rank = ranks.emplace_back();
      for (auto& individual : cpy) if (is_dominant(individual, cpy)) current_rank.push_back(individual);
      
      std::erase_if(cpy,
                    [&current_rank](const auto& individual) {
                      return std::find(current_rank.begin(), current_rank.end(), individual) != current_rank.end();
                    });
    }
    return ranks;
  }

protected:
  ranked_selection(size_t selection_size, select_by select_by) : _selection_size(selection_size), _select_by(select_by)
  {}
  
  size_t _selection_size{0};
  select_by _select_by{0};
};

template<floating_point_type F>
class ranked_selection_for_reproduction : public base_selection_for_reproduction<F>, public ranked_selection {
public:
  
  reproduction_selection_t<F> operator()(population_t<F>& population) const override
  {
    auto ranks = rank_population(population);
    reproduction_selection_t<F> result;
    switch (_select_by) {
      case ranked_selection::select_by_ranks:
      case ranked_selection::select_by_individuals:
      default:
        break;
    }
    size_t selected_amount{0};
    int count{0};
    while (selected_amount < _selection_size) {
      reproduction_selection_t<F> subgroup;
      std::for_each(std::execution::par_unseq,
                    population.begin(),
                    population.end(),
                    [&result, &subgroup](auto& individual) {
                      if (std::find_if(std::execution::par_unseq,
                                       result.begin(),
                                       result.end(),
                                       [&individual](auto& subgroup_individual) {
                                         return individual == subgroup_individual;
                                       }) == result.end())
                        subgroup.push_back(individual);
                    });
      for (auto& subgroup_individual : subgroup) {
        if (is_dominant(subgroup_individual, subgroup)) {
          result.push_back(subgroup_individual);
          if (_select_by == select_by_individuals && ++selected_amount < _selection_size) return result;
        }
      }
      selected_amount += _select_by == select_by_ranks;
    }
    return result;
  }
  
  ranked_selection_for_reproduction(size_t selection_size, select_by by)
      : base_selection_for_reproduction<F>(), ranked_selection(selection_size, by)
  {}
};

template<floating_point_type F>
class base_selection_for_replacement {
public:
  virtual population_t<F>& operator()(population_t<F>& population) const = 0;
  
  virtual ~base_selection_for_replacement() = default;
};

template<floating_point_type F>
class generational_selection_for_replacement : public base_selection_for_replacement<F> {
public:
  population_t<F>& operator()(population_t<F>& population) const override
  {
    population.clear();
    return population;
  }
  
  generational_selection_for_replacement() = default;
};

template<floating_point_type F>
class truncation_selection_for_replacement : public base_selection_for_replacement<F> {
public:
  population_t<F>& operator()(population_t<F>& population) const override
  {
    sort(population);
    size_t elements_to_remove =
        _selection_size < population.size() ? population.size() - _selection_size : population.size();
    auto first = population.begin() + elements_to_remove;
    population.erase(first, population.end());
    return population;
  }
  
  explicit truncation_selection_for_replacement(size_t selection_size) : _selection_size(selection_size)
  {}

private:
  size_t _selection_size{0};
};

template<floating_point_type F>
class ranked_selection_for_replacement : public base_selection_for_replacement<F>, public ranked_selection {
public:
  population_t<F>& operator()(population_t<F>& population) const override
  {
    size_t initial_population_size = population.size();
    ranked_selection_t<F> ranks = rank_population(population);
    population.clear();
    switch (_select_by) {
      case ranked_selection::select_by_ranks:
        for (size_t i{0}; ranks.size() - i > _selection_size;)
          std::ranges::for_each(ranks[i++],
                                [&population](const auto& individual) {
                                  population.push_back(individual);
                                });
        break;
      case ranked_selection::select_by_individuals: {
        auto ranks_it = ranks.begin();
        auto current_rank_it = ranks_it->begin();
        while (population.size() < initial_population_size - _selection_size) {
          population.push_back(*current_rank_it);
          if (current_rank_it++ == ranks_it->end()) {
            if (ranks_it++ == ranks.end()) break;
            current_rank_it = ranks_it->begin();
          }
        }
        break;
      }
      default:
        break;
    }
    return population;
  }
  
  ranked_selection_for_replacement(size_t selection_size, select_by select_by)
      : ranked_selection(selection_size, select_by)
  {}
};

}
#endif //MINIMACORE_SELECTION_OPERATORS_H
