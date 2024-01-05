
#ifndef MINIMACORE_SETUP_H
#define MINIMACORE_SETUP_H

#include "selection_operators.h"
#include "genetic_operators.h"
#include "base_evaluation.h"
#include "base_individual_generator.h"
#include "termination_condition.h"
#include <functional>

namespace minimacore::genetic_algorithm {

using std::function;

template<floating_point_type F>
class setup {
  using selection_for_replacement_ptr = unique_ptr<base_selection_for_replacement<F>>;
  using selection_for_reproduction_ptr = unique_ptr<base_selection_for_reproduction<F>>;
  using mutation_ptr = unique_ptr<base_mutation<F>>;
  using crossover_ptr = unique_ptr<base_crossover<F>>;
  using genome_generator_ptr = unique_ptr<genome_generator<F>>;
  using evaluation_t = unique_ptr<base_evaluation<F>>;
  using evaluations_t = vector<evaluation_t>;
  using termination_conditions_t = vector<termination_condition_ptr<F>>;

public:

  [[nodiscard]] size_t max_contiguous_failure_on_initialization() const
  {
    return _max_contiguous_failure_on_initialization;
  }

  void set_max_contiguous_failure_on_initialization(size_t n)
  {
    _max_contiguous_failure_on_initialization = n;
  }

  [[nodiscard]] size_t get_thread_count() const
  {
    return _thread_count;
  }

  void set_thread_count(size_t n)
  {
    _thread_count = n;
  }

  [[nodiscard]] size_t population_size() const
  {
    return _population_size;
  }

  [[nodiscard]] size_t generations() const
  {
    return _generations;
  }

  const base_selection_for_reproduction<F>& selection_for_reproduction() const
  {
    return *_selection_for_reproduction;
  }

  const base_selection_for_replacement<F>& selection_for_replacement() const
  {
    return *_selection_for_replacement;
  }

  const base_crossover<F>& crossover() const
  {
    return *_crossover;
  }

  const base_mutation<F>& get_mutation() const
  {
    return *_mutation;
  }

  const genome_generator<F>& get_genome_generator() const
  {
    return *_genome_generator;
  }

  const termination_conditions_t& termination_conditions() const
  {
    return _termination_conditions;
  }

  const evaluations_t& evaluations() const
  {
    return _evaluations;
  }

  setup<F>& set_population_size(size_t population_size)
  {
    _population_size = population_size;
    return *this;
  }

  setup<F>& set_generations(size_t generations)
  {
    _generations = generations;
    return *this;
  }

  setup<F>& set_selection_for_reproduction(selection_for_reproduction_ptr&& selection)
  {
    _selection_for_reproduction = std::move(selection);
    return *this;
  }

  setup<F>& set_selection_for_replacement(selection_for_replacement_ptr&& selection)
  {
    _selection_for_replacement = std::move(selection);
    return *this;
  }

  setup<F>& set_crossover(crossover_ptr&& crossover)
  {
    _crossover = std::move(crossover);
    return *this;
  }

  setup<F>& set_mutation(mutation_ptr&& mutation)
  {
    _mutation = std::move(mutation);
    return *this;
  }

  setup<F>& set_genome_generator(genome_generator_ptr&& genome_generator)
  {
    _genome_generator = std::move(genome_generator);
    return *this;
  }

  setup<F>& add_termination(termination_condition_ptr<F>&& condition)
  {
    _termination_conditions.emplace_back(std::move(condition));
    return *this;
  }

  setup<F>& add_evaluation(evaluation_t&& evaluation)
  {
    _evaluations.emplace_back(std::move(evaluation));
    return *this;
  }

  setup<F>& add_callback(function<void()>&& f)
  {
    _iteration_callbacks.emplace_back(std::move(f));
    return *this;
  }

  void run_iteration_callbacks() const
  {
    std::for_each(
            std::execution::par_unseq,
            _iteration_callbacks.begin(),
            _iteration_callbacks.end(),
            [](auto& f) { f(); }
    );
  }

  setup(setup&& other) noexcept
          :_population_size(other._population_size),
           _generations(other._generations),
           _selection_for_replacement(std::move(other._selection_for_replacement)),
           _selection_for_reproduction(std::move(other._selection_for_reproduction)),
           _crossover(std::move(other._crossover)),
           _mutation(std::move(other._mutation)),
           _genome_generator(std::move(other._genome_generator)),
           _termination_conditions(std::move(other._termination_conditions)),
           _evaluations(std::move(other._evaluations)),
           _thread_count(other._thread_count),
           _max_contiguous_failure_on_initialization(other._max_contiguous_failure_on_initialization),
           _iteration_callbacks(std::move(other._iteration_callbacks)) { }

  setup(const setup& other) = delete;

  setup& operator=(setup&& other) noexcept
  {
    _population_size = other._population_size;
    _generations = other._generations;
    _selection_for_replacement = std::move(other._selection_for_replacement);
    _selection_for_reproduction = std::move(other._selection_for_reproduction);
    _crossover = std::move(other._crossover);
    _mutation = std::move(other._mutation);
    _genome_generator = std::move(other._genome_generator);
    _termination_conditions = std::move(other._termination_conditions);
    _evaluations = std::move(other._evaluations);
    _thread_count(other._thread_count);
    _max_contiguous_failure_on_initialization(other._max_contiguous_failure_on_initialization);
    _iteration_callbacks(std::move(other._iteration_callbacks));
  }

  setup& operator=(const setup& other) = delete;

  setup() = default;

  ~setup() = default;

private:
  size_t _population_size{0};
  size_t _generations{0};
  selection_for_replacement_ptr _selection_for_replacement;
  selection_for_reproduction_ptr _selection_for_reproduction;
  crossover_ptr _crossover;
  mutation_ptr _mutation;
  genome_generator_ptr _genome_generator;
  termination_conditions_t _termination_conditions;
  evaluations_t _evaluations;
  /*
   * Maximum contiguous failure on initialization. The variable sets the maximum number of individuals that can fail on
   * initialization before the algorithm is automatically stopped. This mechanism is intended to prevent impossible
   * initialization boundaries when initializing individuals.
   */
  size_t _max_contiguous_failure_on_initialization = 300;
  size_t _thread_count = std::thread::hardware_concurrency();
  vector<function<void()>> _iteration_callbacks;
};

}

#endif //MINIMACORE_SETUP_H
