
#ifndef MINIMACORE_SELECTION_OPERATORS_H
#define MINIMACORE_SELECTION_OPERATORS_H

#include "base_individual.h"

#include <algorithm>
#include <random>
#include <set>

#ifdef __has_include
#if __has_include(<execution>)
#include <execution>
#if defined(__cpp_lib_execution) && __cpp_lib_execution >= 201603L
#define HAS_EXECUTION_POLICIES 1
#endif
#endif
#endif

namespace minimacore::genetic_algorithm {

  template <floating_point_type Fp_T> population_t<Fp_T> &sort(population_t<Fp_T> &population) {
    std::sort(
#ifdef HAS_EXECUTION_POLICIES
        std::execution::par_unseq,
#endif
        population.begin(), population.end(),
        [](const individual_ptr<Fp_T> &a, const individual_ptr<Fp_T> &b) { return *a < *b; });
    return population;
  }

  template <floating_point_type Fp_T> class base_selection_for_reproduction {
  public:
    virtual reproduction_selection_t<Fp_T> operator()(population_t<Fp_T> &population) const = 0;
    base_selection_for_reproduction() = default;
    base_selection_for_reproduction(const base_selection_for_reproduction &) = delete;
    base_selection_for_reproduction(base_selection_for_reproduction &&) = delete;
    base_selection_for_reproduction &operator=(const base_selection_for_reproduction &) = delete;
    base_selection_for_reproduction &operator=(base_selection_for_reproduction &&) = delete;
    virtual ~base_selection_for_reproduction() = default;
  };

  template <floating_point_type Fp_T>
  class truncation_selection_for_reproduction : public base_selection_for_reproduction<Fp_T> {
    size_t _selection_size = 0;

  public:
    reproduction_selection_t<Fp_T> operator()(population_t<Fp_T> &population) const override {
      reproduction_selection_t<Fp_T> result;
      result.reserve(_selection_size);
      sort(population);
      for (size_t selected = 0; selected < _selection_size; selected++) {
        result.push_back(population[selected]);
      }
      return result;
    }

    explicit truncation_selection_for_reproduction(size_t selection_size) : _selection_size(selection_size) {}
  };

  template <floating_point_type Fp_T>
  class tournament_selection_for_reproduction : public base_selection_for_reproduction<Fp_T> {
  public:
    reproduction_selection_t<Fp_T> operator()(population_t<Fp_T> &population) const override {
      std::uniform_int_distribution<size_t> dist(0, population.size() - 1);
      reproduction_selection_t<Fp_T> result;
      result.reserve(_selection_size);
      const size_t tournament_size = _tournament_size < population.size() ? _tournament_size : population.size();
      const size_t selection_size = _selection_size < population.size() ? _selection_size : population.size();
      std::set<individual_ptr<Fp_T>> selected_individuals;
      std::random_device rd;
      std::mt19937_64 gen(static_cast<std::mt19937_64::result_type>(rd()));
      while (selection_size > result.size()) {
        reproduction_selection_t<Fp_T> tournament_selection;
        while (tournament_selection.size() < tournament_size) {
          auto &selected_for_tournament = population[dist(gen)];
          if (std::find(tournament_selection.begin(), tournament_selection.end(), selected_for_tournament) ==
              tournament_selection.end()) {
            tournament_selection.push_back(selected_for_tournament);
          }
        }

        auto winner =
            std::min_element(tournament_selection.begin(), tournament_selection.end(),
                             [](const individual_ptr<Fp_T> &a, const individual_ptr<Fp_T> &b) { return *a < *b; });

        if (auto pair = selected_individuals.insert(*winner); pair.second) {
          result.push_back(*pair.first);
        }
      }
      return result;
    }

    explicit tournament_selection_for_reproduction(size_t tournament_size, size_t selection_size)
        : _tournament_size(tournament_size), _selection_size(selection_size) {}

  private:
    size_t _tournament_size{0};
    size_t _selection_size{0};
  };

  template <floating_point_type Fp_T> using ranked_selection_t = vector<population_t<Fp_T>>;

  class ranked_selection {
  public:
    enum class select_by : std::uint8_t {
      RANKS = 0,
      INDIVIDUALS,
    };

    template <floating_point_type Fp_T>
    static bool is_dominant(const individual_ptr<Fp_T> &individual, const reproduction_selection_t<Fp_T> &subgroup) {
#ifdef HAS_EXECUTION_POLICIES
      return std::all_of(std::execution::par_unseq, subgroup.begin(), subgroup.end(), [&](const auto &comparison) {
        if (individual != comparison) {
          bool is_dominant{false};
          for (size_t i = 0; i < individual->get_object_fitnesses().size(); i++)
            is_dominant = is_dominant || individual->objective_fitness(i) < comparison->objective_fitness(i);
          return is_dominant;
        }
        return true;
      });
#else
      return std::all_of(subgroup.begin(), subgroup.end(), [&](const auto &comparison) {
        if (individual != comparison) {
          bool is_dominant{false};
          for (size_t i = 0; i < individual->get_object_fitnesses().size(); i++) {
            is_dominant = is_dominant || individual->objective_fitness(i) < comparison->objective_fitness(i);
          }
          return is_dominant;
        }
        return true;
      });
#endif
    }

    template <floating_point_type Fp_T>
    static ranked_selection_t<Fp_T> rank_population(const population_t<Fp_T> &population) {
      ranked_selection_t<Fp_T> ranks;
      population_t<Fp_T> cpy(population);
      while (!cpy.empty()) {
        auto &current_rank = ranks.emplace_back();
        for (auto &individual : cpy)
          if (is_dominant(individual, cpy))
            current_rank.push_back(individual);
        std::erase_if(cpy, [&current_rank](const auto &individual) {
          return std::find(current_rank.begin(), current_rank.end(), individual) != current_rank.end();
        });
      }
      return ranks;
    }

  protected:
    ranked_selection(const size_t selection_size, select_by select_by)
        : _selection_size(selection_size), _select_by(select_by) {}

    size_t _selection_size{0};
    select_by _select_by{0};
  };

  template <floating_point_type Fp_T>
  class ranked_selection_for_reproduction : public base_selection_for_reproduction<Fp_T>, public ranked_selection {
  public:
    reproduction_selection_t<Fp_T> operator()(population_t<Fp_T> &population) const override {
      auto ranks = rank_population(population);
      reproduction_selection_t<Fp_T> result;
      switch (_select_by) {
      case select_by::RANKS: {
        size_t selected_amount{0};
        while (selected_amount < ranks.size() && selected_amount < _selection_size) {
          for (auto &individual : ranks[selected_amount++]) {
            result.push_back(individual);
          }
        }
        return result;
        break;
      }
      case select_by::INDIVIDUALS:
      default:
        break;
      }
      return result;
      size_t selected_amount{0};
      int count{0};
      while (selected_amount < _selection_size) {
        reproduction_selection_t<Fp_T> subgroup;
#ifdef HAS_EXECUTION_POLICIES
        std::for_each(std::execution::par_unseq, population.begin(), population.end(),
                      [&result, &subgroup](auto &individual) {
                        if (std::find_if(std::execution::par_unseq, result.begin(), result.end(),
                                         [&individual](auto &subgroup_individual) {
                                           return individual == subgroup_individual;
                                         }) == result.end())
                          subgroup.push_back(individual);
                      });
#else
        std::for_each(population.begin(), population.end(), [&result, &subgroup](auto &individual) {
          if (std::find_if(result.begin(), result.end(), [&individual](auto &subgroup_individual) {
                return individual == subgroup_individual;
              }) == result.end()) {
            subgroup.push_back(individual);
          }
        });
#endif
        for (auto &subgroup_individual : subgroup) {
          if (is_dominant(subgroup_individual, subgroup)) {
            result.push_back(subgroup_individual);
            if (_select_by == select_by::INDIVIDUALS && ++selected_amount < _selection_size)
              return result;
          }
        }
        selected_amount += _select_by == select_by::RANKS;
      }
      return result;
    }

    ranked_selection_for_reproduction(size_t selection_size, select_by by)
        : base_selection_for_reproduction<Fp_T>(), ranked_selection(selection_size, by) {}
  };

  template <floating_point_type Fp_T> class base_selection_for_replacement {
  public:
    virtual population_t<Fp_T> &operator()(population_t<Fp_T> &population) const = 0;
    base_selection_for_replacement() = default;
    base_selection_for_replacement(const base_selection_for_replacement &) = delete;
    base_selection_for_replacement(base_selection_for_replacement &&) = delete;
    base_selection_for_replacement &operator=(const base_selection_for_replacement &) = delete;
    base_selection_for_replacement &operator=(base_selection_for_replacement &&) = delete;
    virtual ~base_selection_for_replacement() = default;
  };

  template <floating_point_type Fp_T>
  class generational_selection_for_replacement : public base_selection_for_replacement<Fp_T> {
  public:
    population_t<Fp_T> &operator()(population_t<Fp_T> &population) const override {
      population.clear();
      return population;
    }

    generational_selection_for_replacement() = default;
  };

  template <floating_point_type Fp_T>
  class truncation_selection_for_replacement : public base_selection_for_replacement<Fp_T> {
  public:
    population_t<Fp_T> &operator()(population_t<Fp_T> &population) const override {
      sort(population);
      size_t elements_to_remove =
          _selection_size < population.size() ? population.size() - _selection_size : population.size();
      auto first = population.begin() + elements_to_remove;
      population.erase(first, population.end());
      return population;
    }

    explicit truncation_selection_for_replacement(size_t selection_size) : _selection_size(selection_size) {}

  private:
    size_t _selection_size{0};
  };

  template <floating_point_type Fp_T>
  class ranked_selection_for_replacement : public base_selection_for_replacement<Fp_T>, public ranked_selection {
  public:
    population_t<Fp_T> &operator()(population_t<Fp_T> &population) const override {
      const size_t initial_population_size = population.size();
      ranked_selection_t<Fp_T> ranks = rank_population(population);
      population.clear();
      switch (_select_by) {
      case select_by::RANKS:
        for (size_t i{0}; ranks.size() - i > _selection_size;) {
          std::ranges::for_each(ranks[i++],
                                [&population](const auto &individual) { population.push_back(individual); });
        }
        break;
      case select_by::INDIVIDUALS: {
        auto ranks_it = ranks.begin();
        auto current_rank_it = ranks_it->begin();
        while (population.size() < initial_population_size - _selection_size) {
          population.push_back(*current_rank_it);
          if (++current_rank_it == ranks_it->end()) {
            if (ranks_it++ == ranks.end()) {
              break;
            }
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
        : ranked_selection(selection_size, select_by) {}
  };

} // namespace minimacore::genetic_algorithm
#endif // MINIMACORE_SELECTION_OPERATORS_H
