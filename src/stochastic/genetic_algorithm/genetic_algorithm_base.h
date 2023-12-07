
#ifndef MINIMACORE_GENETIC_ALGORITHM_BASE_H
#define MINIMACORE_GENETIC_ALGORITHM_BASE_H

#include <memory>
#include <vector>
#include <execution>
#include <algorithm>
#include <random>
#include <set>
#include <Eigen/Dense>
#include "selection_operators.h"

#include <minimacore_concepts.h>

namespace minimacore::genetic_algorithm {

template<floating_point_type F>
class base_crossover {

public:
  virtual individual_ptr<F> operator()(const individual<F>& a, const individual<F>& b) const = 0;
  
  virtual ~base_crossover() = default;
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
