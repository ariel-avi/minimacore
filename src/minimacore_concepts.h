
#ifndef MINIMACORE_MINIMACORE_CONCEPTS_H
#define MINIMACORE_MINIMACORE_CONCEPTS_H

#include <concepts>

template<typename T>
concept floating_point_type = std::is_floating_point_v<T>;

#endif //MINIMACORE_MINIMACORE_CONCEPTS_H
