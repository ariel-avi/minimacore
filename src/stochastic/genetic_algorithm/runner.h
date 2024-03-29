
#ifndef MINIMACORE_RUNNER_H
#define MINIMACORE_RUNNER_H

#include "selection_operators.h"
#include "genetic_operators.h"
#include "base_evaluation.h"
#include "base_individual_generator.h"
#include "termination_condition.h"
#include "setup.h"
#include <logger.h>
#include <thread_pool.h>
#include <functional>
#include <minimacore_concepts.h>

namespace minimacore::genetic_algorithm {

using std::function;
typedef std::chrono::high_resolution_clock clock_t;
typedef std::chrono::time_point<clock_t> time_point_t;
typedef std::chrono::duration<double, std::milli> duration_t;

template<floating_point_type F>
class runner {
  using evaluation_t = unique_ptr<base_evaluation<F>>;

public:
  enum exit_flag {
    success = 0,
    failure
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

  exit_flag run()
  {
    if (_state != state::waiting) return exit_flag::failure;
    _start_time = std::chrono::high_resolution_clock::now();
    _state = state::running;
    _log << logger::wrapped_uts_timestamp() << "Starting genetic algorithm...\n";
    initialize_individual_zero();
    if (!initialize_population()) {
      display_final_message(failure);
      return exit_flag::failure;
    }
    _setup.run_iteration_callbacks();
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
        _setup.run_iteration_callbacks();
        _log << logger::wrapped_uts_timestamp() << "Generation " << _statistics.current_generation() << " complete\n";
        break;
      }
      case state::pausing: {
        _state = state::paused;
        continue;
      }
      case state::paused: {
        continue;
      }
      case state::stopping: {
        _state = state::stopped;
        break;
      }
      case state::stopped: {
        display_final_message(success);
        return success;
      }
      default:
        break;
      }
    }
    _state = state::done;
    display_final_message(success);
    return success;
  }

  setup<F>& get_setup()
  {
    return _setup;
  }

  const population_t<F>& get_population() const
  {
    return _population;
  }

  [[nodiscard]] duration_t elapsed_time_ms() const {
    auto duration = clock_t::now() - _start_time;
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration);
  }

  explicit runner(setup<F> s)
          :_statistics(s.generations()),
           _threads(s.get_thread_count()),
           _setup(std::move(s)) { }

private:

  void display_final_message(exit_flag flag)
  {
    _log << logger::wrapped_uts_timestamp() << "Optimization finished.\n"
         << logger::wrapped_uts_timestamp() << "Total evaluations: " << _statistics.evaluation_count() << '\n'
         << logger::wrapped_uts_timestamp() << "Total elapsed time: " << elapsed_time_ms().count() << "ms\n";
  }

  [[nodiscard]] size_t objective_count() const
  {
    return std::accumulate(
            _setup.evaluations().begin(),
            _setup.evaluations().end(),
            0,
            [](size_t i, const evaluation_t& eval) { return i + eval->objective_count(); }
    );
  }

  F evaluate(const individual_ptr<F>& individual)
  {
    size_t counter = 0; // used to count objectives and align fitness values
    for (auto& evaluation : _setup.evaluations()) counter = (*evaluation)(*individual, counter);
    _statistics.increment_evaluation_count(counter);
    return individual->overall_fitness();
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

  bool initialize_individual(const individual_ptr<F>& individual)
  {
    size_t contiguous_failures = 0;
    _setup.get_genome_generator()(individual);
    while (std::isnan(evaluate(individual))
            && contiguous_failures < _setup.max_contiguous_failure_on_initialization()) {
      _setup.get_genome_generator()(individual);
      contiguous_failures++;
      switch (_state) {
        [[unlikely]]
      case state::stopping:
        [[unlikely]]
      case state::stopped:
        return false;
      default:
        break;
      }
    }
    if (contiguous_failures >= _setup.max_contiguous_failure_on_initialization()) {
      _log << logger::wrapped_uts_timestamp()
           << "Failed to initialize population. Maximum contiguous failure reached: "
           << _setup.max_contiguous_failure_on_initialization() << '\n';
      return false;
    }
    return true;
  }

  bool initialize_population()
  {
    _log << logger::wrapped_uts_timestamp() << "Initializing population, size = " << _setup.population_size() << '\n';
    while (_population.size() < _setup.population_size()) {
      _population.emplace_back(
              std::make_shared<base_individual<F>>(
                      _setup.get_genome_generator().initial_genome(),
                      objective_count()
              )
      );
    }

    vector<std::future<bool>> futures;
    futures.reserve(_population.size());
    for (auto& individual : _population)
      futures.emplace_back(_threads.enqueue([this, &individual]() { return initialize_individual(individual); }));

    bool success_flag = true;
    std::ranges::for_each(futures, [this, &success_flag](auto& f) { success_flag &= f.get(); });
    if (success_flag) update_best_individual();
    return success_flag;
  }

  void fill_population(population_t<F>& reproduction_set)
  {
    std::random_device device{};
    std::mt19937_64 generator{device()};
    std::uniform_real_distribution<F> distribution(0., 1.);
    while (_population.size() < _setup.population_size()) {
      size_t count = _setup.population_size() - _population.size();
      vector<std::future<individual_ptr<F>>> futures;
      futures.reserve(count);
      for (size_t i = 0; i < count; i++) {
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

        futures.emplace_back(
                _threads.enqueue(
                        [this, individual]() {
                          // Individuals are discarded if the evaluation fails
                          return std::isnan(evaluate(individual)) ? individual_ptr<F>(nullptr) : individual;
                        }
                ));
      }

      for (auto& fut : futures) if (auto individual = fut.get()) _population.emplace_back(individual);

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
  thread_pool _threads;
  std::atomic<state> _state = state::waiting;
  time_point_t _start_time;
};

}
#endif //MINIMACORE_RUNNER_H
