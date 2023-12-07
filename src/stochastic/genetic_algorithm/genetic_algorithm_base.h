
#ifndef MINIMACORE_GENETIC_ALGORITHM_BASE_H
#define MINIMACORE_GENETIC_ALGORITHM_BASE_H

#include <memory>
#include <vector>
#include <execution>
#include <algorithm>
#include <random>
#include <set>
#include <Eigen/Dense>
#include <minimacore_concepts.h>

namespace minimacore::genetic_algorithm {

using std::shared_ptr;
using std::unique_ptr;
using std::vector;

template<floating_point_type F>
class base_individual {
protected:
  vector<F> _fitness_values;

public:
  F overall_fitness() const
  {
    F value = 0.;
    return std::accumulate(_fitness_values.begin(), _fitness_values.end(), value);
  }
  
  bool operator<(const base_individual& other) const
  {
    return overall_fitness() < other.overall_fitness();
  }
  
  bool operator==(const base_individual& other) const
  {
    return overall_fitness() == other.overall_fitness();
  }
  
  F set_fitness_value(size_t index, F value)
  {
    _fitness_values[index] = value;
  }
  
  F append_fitness_value(F value)
  {
    _fitness_values.push_back(value);
  }
  
  const vector<F>& get_fitness_values() const
  {
    return _fitness_values;
  }
};

template<floating_point_type F>
using individual_ptr = unique_ptr<base_individual<F>>;


template<floating_point_type F>
class base_selection_for_reproduction {
public:
  virtual vector<base_individual<F>*> operator()(vector<individual_ptr<F>>& population) const = 0;
};

template<floating_point_type F>
class truncation_selection_for_reproduction : public base_selection_for_reproduction<F> {
  size_t _selection_size = 0;
public:
  vector<base_individual<F>*> operator()(vector<individual_ptr<F>>& population) const override
  {
    vector<base_individual<F>*> result;
    result.reserve(_selection_size);
    std::sort(std::execution::par_unseq, population.begin(), population.end(),
              [](const individual_ptr<F>& a, const individual_ptr<F>& b) {
                return *a < *b;
              });
    for (size_t selected = 0; selected < _selection_size; selected++) result.emplace_back(population[selected].get());
    return result;
  }
  
  explicit truncation_selection_for_reproduction(size_t selection_size) : _selection_size(selection_size)
  {}
};

template<floating_point_type F>
class tournament_selection_for_reproduction : public base_selection_for_reproduction<F> {
public:
  vector<base_individual<F>*> operator()(vector<individual_ptr<F>>& population) const override
  {
    std::uniform_int_distribution<size_t> dist(0, population.size() - 1);
    vector<base_individual<F>*> result;
    result.reserve(_selection_size);
    const size_t tournament_size = _tournament_size < population.size() ? _tournament_size : population.size();
    const size_t selection_size = _selection_size < population.size() ? _selection_size : population.size();
    std::set<base_individual<F>*> selected_individuals;
    std::random_device rd;
    std::mt19937_64 gen(rd());
    while (result.size() < selection_size) {
      vector<base_individual<F>*> tournament_selection;
      while (tournament_selection.size() < tournament_size) {
        auto& selected_for_tournament = population[dist(gen)];
        if (std::find(tournament_selection.begin(), tournament_selection.end(), selected_for_tournament.get()) ==
            tournament_selection.end())
          tournament_selection.push_back(selected_for_tournament.get());
      }
      
      auto winner = std::min_element(tournament_selection.begin(),
                                     tournament_selection.end(),
                                     [](const base_individual<F>* a,
                                        const base_individual<F>* b) { return *a < *b; });
      
      if (auto pair = selected_individuals.insert(*winner); pair.second) result.push_back(*pair.first);
      
    }
    return result;
  }
  
  explicit tournament_selection_for_reproduction(size_t tournament_size, size_t selection_size)
      : _tournament_size(tournament_size), _selection_size(selection_size)
  {}

private:
  size_t _tournament_size = 0;
  size_t _selection_size = 0;
};

template<floating_point_type F>
class ranked_selection_for_reproduction : public base_selection_for_reproduction<F> {

};

template<floating_point_type F>
class base_selection_for_replacement {

};

template<floating_point_type F>
class generational_selection_for_replacement : public base_selection_for_replacement<F> {

};

template<floating_point_type F>
class truncation_selection_for_replacement : public base_selection_for_replacement<F> {

};

template<floating_point_type F>
class ranked_selection_for_replacement : public base_selection_for_replacement<F> {

};

template<floating_point_type F>
class individual : public base_individual<F> {
public:
  explicit individual(Eigen::VectorX<F> genome) : _genome(genome)
  {}

private:
  Eigen::VectorX<F> _genome;
  
};

template<floating_point_type F>
class base_crossover {
protected:
  std::mt19937_64 _generator;

public:
  virtual individual_ptr<F> operator()(const individual<F>& a, const individual<F>& b) const = 0;
  
  explicit base_crossover(std::random_device random_device = std::random_device{"mt19937"})
      : _generator(random_device())
  {}
};

template<floating_point_type F>
class uniform_linear_crossover : public base_crossover<F> {
public:
  individual_ptr<F> operator()(const individual<F>& a, const individual<F>& b) const override
  {
    std::uniform_real_distribution<F> distribution(-1., 1.);
    F factor = _alpha * distribution(this->_generator);
    auto midpoint = (a._genome + b._genome) / 2.;
    auto difference = b._genome - a._genome;
    return make_unique<individual<F>>(midpoint + factor * difference);
  };

private:
  F _alpha;
};

template<floating_point_type F>
class uniform_voluminal_crossover : public base_crossover<F> {
public:
  individual_ptr<F> operator()(const individual<F>& a, const individual<F>& b) const override
  {
    std::uniform_real_distribution<F> distribution(-1., 1.);
    Eigen::VectorX<F> midpoint = (a._genome + b._genome) / 2.;
    for (long i = 0; i < midpoint.size(); i++) {
      F factor = _alpha * distribution(this->_generator);
      midpoint(i) += (b._genome(i) - a._genome(i)) * factor;
    }
    return make_unique<individual<F>>(midpoint);
  };

private:
  F _alpha;
};

template<floating_point_type F, typename result>
class base_objective {
public:
  virtual F operator()(const result& evaluation_result) = 0;
};

template<floating_point_type F, typename result>
class base_evaluation {
public:
  virtual result operator()(const individual<F>* evaluated_individual) = 0;

private:
  vector<base_objective<F, result>> _objectives;
};


}
#endif //MINIMACORE_GENETIC_ALGORITHM_BASE_H
