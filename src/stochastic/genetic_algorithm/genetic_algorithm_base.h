
#ifndef MINIMACORE_GENETIC_ALGORITHM_BASE_H
#define MINIMACORE_GENETIC_ALGORITHM_BASE_H

#include "selection_operators.h"
#include "genetic_operators.h"

#include <minimacore_concepts.h>

namespace minimacore::genetic_algorithm {

template<floating_point_type F, typename T>
class setup {
public:

private:
  unique_ptr<individual_converter<F, T>> _factory;
};

}
#endif //MINIMACORE_GENETIC_ALGORITHM_BASE_H
