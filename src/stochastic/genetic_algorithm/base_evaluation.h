
#ifndef MINIMACORE_BASE_EVALUATION_H
#define MINIMACORE_BASE_EVALUATION_H

#include "base_individual.h"

namespace minimacore::genetic_algorithm {

template<floating_point_type F>
class base_evaluation {
public:
  /**
   * @brief Evaluates the individual and writes the objective functions to the object.
   * @param individual
   * @param objective_index
   * @return The index of the next objective to be filled by the next evaluation
   */
  [[nodiscard]] virtual size_t operator()(base_individual<F>& individual, size_t objective_index) const = 0;
};

}

#endif //MINIMACORE_BASE_EVALUATION_H
