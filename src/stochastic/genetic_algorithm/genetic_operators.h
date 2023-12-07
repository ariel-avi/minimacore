
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
  explicit base_crossover(size_t seed = 0) : _seed(seed)
  {}
  
  virtual genome_t<F> operator()(const base_individual<F>& a, const base_individual<F>& b) const = 0;
  
  virtual ~base_crossover() = default;

protected:
  [[nodiscard]] size_t seed() const
  {
    return _seed;
  }
  
  [[nodiscard]] std::mt19937_64 make_generator() const
  {
    std::random_device device;
    return std::mt19937_64(seed() ? seed() : device());
  }

private:
  size_t _seed{0};
};

template<floating_point_type F>
class uniform_linear_crossover : public base_crossover<F> {
public:
  genome_t<F> operator()(const base_individual<F>& a, const base_individual<F>& b) const override
  {
    std::uniform_real_distribution<F> distribution(-1., 1.);
    auto gen = this->make_generator();
    F factor = _alpha * distribution(gen);
    auto midpoint = (a.genome() + b.genome()) / 2.;
    auto difference = b.genome() - midpoint;
    return midpoint + factor * difference;
  };
  
  explicit uniform_linear_crossover(F alpha, size_t seed = 0) : base_crossover<F>(seed), _alpha(alpha)
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
    auto gen = this->make_generator();
    genome_t<F> result = (a.genome() + b.genome()) / 2.;
    for (long i = 0; i < result.size(); i++) {
      F factor = _alpha * distribution(gen);
      result(i) = result(i) + (b.genome()(i) - result(i)) * factor;
    }
    return result;
  };
  
  explicit uniform_voluminal_crossover(F alpha, size_t seed = 0) : base_crossover<F>(seed), _alpha(alpha)
  {}

private:
  F _alpha;
};

template<floating_point_type F>
class base_mutation {
public:
  virtual genome_t<F> operator()(const base_individual<F>& individual) const = 0;
  
  [[nodiscard]] bool should_mutate() const
  {
    auto gen = make_generator();
    std::uniform_real_distribution<F> distribution(0., 1.);
    F chance = distribution(gen);
    return chance <= _rate;
  }
  
  [[nodiscard]] F get_rate() const
  {
    return _rate;
  }
  
  explicit base_mutation(F rate, size_t seed = 0) : _seed(seed), _rate(rate)
  {}
  
  virtual ~base_mutation() = default;

protected:
  [[nodiscard]] size_t seed() const
  {
    return _seed;
  }
  
  [[nodiscard]] std::mt19937_64 make_generator() const
  {
    std::random_device device;
    return std::mt19937_64(seed() ? seed() : device());
  }

private:
  size_t _seed{0};
  F _rate;
};

template<floating_point_type F>
class gaussian_mutation : public base_mutation<F> {
public:
  genome_t<F> operator()(const base_individual<F>& individual) const override
  {
    std::normal_distribution<F> distribution(0., _std_dev);
    genome_t<F> cpy(individual.genome());
    auto gen = this->make_generator();
    for (long i = 0; i < cpy.size(); i++) cpy(i) += distribution(gen);
    return cpy;
  }
  
  gaussian_mutation(F rate, F std_dev, size_t seed = 0) : base_mutation<F>(rate, seed), _std_dev(std_dev)
  {}

private:
  F _std_dev;
};

template<floating_point_type F>
class uniform_mutation : public base_mutation<F> {
public:
  genome_t<F> operator()(const base_individual<F>& individual) const override
  {
    std::uniform_real_distribution<F> distribution(-1., 1.);
    genome_t<F> cpy(individual.genome());
    auto gen = this->make_generator();
    for (long i = 0; i < cpy.size(); i++) cpy(i) += distribution(gen) * _factor;
    return cpy;
  }
  
  uniform_mutation(F rate, F factor, size_t seed = 0) : base_mutation<F>(rate, seed), _factor(factor)
  {}

private:
  F _factor;
};

}


#endif //MINIMACORE_GENETIC_OPERATORS_H
