
#ifndef MINIMACORE_RUNNER_H
#define MINIMACORE_RUNNER_H

#include "selection_operators.h"
#include "genetic_operators.h"
#include "base_evaluation.h"
#include "base_individual_generator.h"
#include "termination_condition.h"
#include "setup.h"
#include <logger.h>
#include <functional>
#include <minimacore_concepts.h>

namespace minimacore::genetic_algorithm {

using std::function;

template<floating_point_type F>
class runner {
  using evaluation_t = unique_ptr<base_evaluation<F>>;

public:
  enum exit_code {
    successful_exit = 0,
    failed_exit
  };

  enum class state {
    waiting = 0,
    running,
    pausing,
    paused,
    stopping,
    stopped,
    done
  };

  exit_code run()
  {
    if (_state != state::waiting) return exit_code::failed_exit;
    _state = state::running;
    _log << logger::wrapped_uts_timestamp() << "Starting genetic algorithm...\n";
    initialize_individual_zero();
    initialize_population();
    _statistics.register_statistic(_population);
    _setup.add_termination(std::make_unique<generation_termination<F>>(_setup.generations()));
    while (
            std::none_of(
                    std::execution::par_unseq,
                    _setup.termination_conditions().begin(),
                    _setup.termination_conditions().end(),
                    [this](auto& condition) { return (*condition)(_statistics); }
            )
            ) {
      switch (_state) {
      case state::running: {
        auto reproduction_set = _setup.selection_for_reproduction()(_population);
        _setup.selection_for_replacement()(_population);
        fill_population(reproduction_set);
        _statistics.register_statistic(_population);
        update_best_individual();
        _log << logger::wrapped_uts_timestamp() << "Generation " << _statistics.current_generation() << " complete\n";
        break;
      }
      case state::pausing: {
        _state = state::paused;
        break;
      }
      case state::paused: {
        continue;
      }
      case state::stopping: {
        _state = state::stopped;
        break;
      }
      case state::stopped: {
        _log << logger::wrapped_uts_timestamp() << "Genetic successfully stopped, exit code: " << successful_exit
             << '\n';
        return successful_exit;
      }
      default:
        break;
      }
    }
    _state = state::done;
    _log << logger::wrapped_uts_timestamp() << "Genetic algorithm complete, exit code: " << successful_exit << '\n';
    return successful_exit;
  }

  void pause()
  {
    switch (_state) {
    case state::running:
      _state = state::pausing;
      _log << logger::wrapped_uts_timestamp() << "Pause requested, sending signal...\n";
      break;
    default:
      break;
    }
  }

  void resume()
  {
    switch (_state) {
    case state::pausing:
    case state::paused:
      _log << logger::wrapped_uts_timestamp() << "Resuming genetic algorithm...\n";
      _state = state::running;
      break;
    default:
      break;
    }
  }

  void stop()
  {
    switch (_state) {
    case state::running:
    case state::pausing:
    case state::paused:
      _state = state::stopping;
      break;
    default:
      break;
    }
  }

  const shared_ptr<base_individual<F>>& get_best_individual() const
  {
    return _best_individual;
  }

  const shared_ptr<base_individual<F>>& get_individual_zero() const
  {
    return _individual_zero;
  }

  void add_log_stream(std::ostream& stream)
  {
    _log.add_stream(stream);
  }

  void export_statistics(const string& filename, char sep)
  {
    std::ofstream ofs(filename, std::ios::out);
    if (!ofs.is_open() || ofs.bad()) return;
    _statistics.write(ofs, sep);
  }

  explicit runner(setup<F> setup)
          :_statistics(setup.generations()),
           _setup(std::move(setup)) { }

private:
  [[nodiscard]] size_t objective_count() const
  {
    return std::accumulate(
            _setup.evaluations().begin(),
            _setup.evaluations().end(),
            0,
            [](size_t i, const evaluation_t& eval) { return i + eval->objective_count(); }
    );
  }

  void evaluate(const individual_ptr<F>& individual)
  {
    while (!individual->is_valid()) {
      size_t dummy = 0;
      for (auto& evaluation : _setup.evaluations()) dummy = (*evaluation)(*individual, dummy);
    }
  }

  void initialize_individual_zero()
  {
    _log << logger::wrapped_uts_timestamp() << "Initializing individual zero\n";
    _individual_zero = std::make_shared<base_individual<F>>(
            _setup.get_genome_generator().initial_genome(),
            objective_count()
    );
    evaluate(_individual_zero);
    _log << logger::wrapped_uts_timestamp() << "Individual zero fitness: " << _individual_zero->overall_fitness()
         << '\n';
  }

  void initialize_population()
  {
    _log << logger::wrapped_uts_timestamp() << "Initializing population, size = " << _setup.population_size() << '\n';
    while (_population.size() < _setup.population_size()) {
      auto& individual = _population.emplace_back(
              std::make_shared<base_individual<F>>(
                      _setup.get_genome_generator().initial_genome(),
                      objective_count()
              )
      );
      _setup.get_genome_generator()(individual);
      evaluate(individual);
    }
    update_best_individual();
  }

  void fill_population(population_t<F>& reproduction_set)
  {
    std::random_device device{};
    std::mt19937_64 generator{device()};
    std::uniform_real_distribution<F> distribution(0., 1.);
    while (_population.size() < _setup.population_size()) {
      F chance = distribution(generator);
      individual_ptr<F> individual;
      if (_setup.get_mutation().should_mutate()) {
        individual = std::make_shared<base_individual<F>>(
                _setup.get_mutation()(*random_pick(reproduction_set)),
                objective_count());
      }
      else {
        individual = std::make_shared<base_individual<F>>(
                _setup.crossover()(*random_pick(reproduction_set), *random_pick(reproduction_set)),
                objective_count());
      }
      evaluate(individual);
      _population.push_back(individual);
    }
  }

  void update_best_individual()
  {
    _best_individual = std::ranges::min(
            _population, [](auto& a, auto& b) {
              return a->overall_fitness() < b->overall_fitness();
            }
    );
  }

  population_t<F> _population;
  individual_ptr<F> _best_individual{nullptr};
  individual_ptr<F> _individual_zero{nullptr};
  evolution_statistics<F> _statistics;
  setup<F> _setup;
  logger _log;
  std::atomic<state> _state = state::waiting;
};

}
#endif //MINIMACORE_RUNNER_H
