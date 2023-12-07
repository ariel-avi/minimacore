
#ifndef MINIMACORE_BASE_INDIVIDUAL_GENERATOR_H
#define MINIMACORE_BASE_INDIVIDUAL_GENERATOR_H

#include "base_individual.h"

namespace minimacore::genetic_algorithm {


template<floating_point_type F>
class base_chromosome_generator {
public:
  virtual void generate_chromosome(const individual_ptr<F>& individual) const = 0;
  
  virtual ~base_chromosome_generator() = default;
};

template<floating_point_type F>
using chromosome_generator_ptr = unique_ptr<base_chromosome_generator<F>>;

template<floating_point_type F>
class genome_generator {
  vector<chromosome_generator_ptr<F>> _genome_generators;
public:
  individual_ptr<F>& operator()(individual_ptr<F>& individual) const
  {
    for (auto& generator : _genome_generators)
      generator->generate_chromosome(individual);
    return individual;
  }
  
  void append_chromosome_generator(chromosome_generator_ptr<F> chromosome_generator)
  {
    _genome_generators.push_back(std::move(chromosome_generator));
  }
};

}

#endif //MINIMACORE_BASE_INDIVIDUAL_GENERATOR_H
