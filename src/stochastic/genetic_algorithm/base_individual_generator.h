
#ifndef MINIMACORE_BASE_INDIVIDUAL_GENERATOR_H
#define MINIMACORE_BASE_INDIVIDUAL_GENERATOR_H

#include "base_individual.h"

namespace minimacore::genetic_algorithm {


template<floating_point_type F>
class base_genome_generator {
public:
  virtual void operator()(const individual_ptr<F>& genome) const = 0;
  
  virtual ~base_genome_generator() = default;
};

template<floating_point_type F>
using genome_generator_ptr = unique_ptr<base_genome_generator<F>>;

template<floating_point_type F>
class population_generator {
  vector<genome_generator_ptr<F>> _genome_generators;
public:
  population_t<F>& operator()(population_t<F>& population) const
  {
    for (auto& individual : population)
      for (auto& generator : _genome_generators)
        (*generator)(individual);
    return population;
  }
  
  void append_genome_generator(genome_generator_ptr<F> genome_generator) {
    _genome_generators.push_back(std::move(genome_generator));
  }
};

}

#endif //MINIMACORE_BASE_INDIVIDUAL_GENERATOR_H
