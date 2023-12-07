
#ifndef MINIMACORE_GENETIC_OPERATORS_H
#define MINIMACORE_GENETIC_OPERATORS_H

#include <execution>
#include <algorithm>
#include <random>
#include "base_individual.h"

namespace minimacore::genetic_algorithm {


template<floating_point_type F>
class base_crossover {

public:
  virtual genome_t<F> operator()(const base_individual<F>& a, const base_individual<F>& b) const = 0;
  
  virtual ~base_crossover() = default;
};

template<floating_point_type F>
class uniform_linear_crossover : public base_crossover<F> {
public:
  genome_t<F> operator()(const base_individual<F>& a, const base_individual<F>& b) const override
  {
    std::uniform_real_distribution<F> distribution(-1., 1.);
    std::random_device device;
    std::mt19937_64 gen(device());
    F factor = _alpha * distribution(gen);
    auto midpoint = (a.get_genome() + b.get_genome()) / 2.;
    auto difference = b.get_genome() - midpoint;
    return midpoint + factor * difference;
  };
  
  explicit uniform_linear_crossover(F alpha) : _alpha(alpha)
  {}

private:
  F _alpha;
};

template<floating_point_type F>
class uniform_voluminal_crossover : public base_crossover<F> {
public:
  genome_t<F> operator()(const base_individual<F>& a, const base_individual<F>& b) const override
  {
    std::uniform_real_distribution<F> distribution(-1., 1.);
    std::random_device device;
    std::mt19937_64 gen(device());
    genome_t<F> result = (a.get_genome() + b.get_genome()) / 2.;
    for (long i = 0; i < result.size(); i++) {
      F factor = _alpha * distribution(gen);
      result(i) = result(i) + (b.get_genome()(i) - result(i)) * factor;
    }
    return result;
  };
  
  explicit uniform_voluminal_crossover(F alpha) : _alpha(alpha)
  {}

private:
  F _alpha;
};

template<floating_point_type F>
class mutation {
public:
  virtual genome_t<F> operator()(const base_individual<F>& individual) const = 0;
  
  [[nodiscard]] F get_rate() const
  {
    return _rate;
  }
  
  explicit mutation(F rate) : _rate(rate)
  {}

private:
  F _rate;
};

template<floating_point_type F>
class gaussian_mutation : public mutation<F> {
public:
  genome_t<F> operator()(const base_individual<F>& individual) const override
  {
    std::normal_distribution<F> distribution(0., _std_dev);
    std::random_device device;
    std::mt19937_64 gen(device());
    genome_t<F> cpy(individual.get_genome());
    for (long i = 0; i < cpy.size(); i++) cpy(i) += distribution(gen);
    return cpy;
  }
  
  gaussian_mutation(F rate, F std_dev) : mutation<F>(rate), _std_dev(std_dev)
  {}

private:
  F _std_dev;
};

template<floating_point_type F>
class uniform_mutation : public mutation<F> {
public:
  genome_t<F> operator()(const base_individual<F>& individual) const override
  {
    std::uniform_real_distribution<F> distribution(-1., 1.);
    std::random_device device;
    std::mt19937_64 gen(device());
    genome_t<F> cpy(individual.get_genome());
    for (long i = 0; i < cpy.size(); i++) cpy(i) += distribution(gen) * _factor;
    return cpy;
  }
  
  uniform_mutation(F rate, F factor) : mutation<F>(rate), _factor(factor)
  {}
  
private:
  F _factor;
};

}


#endif //MINIMACORE_GENETIC_OPERATORS_H
