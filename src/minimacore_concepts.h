
#ifndef MINIMACORE_MINIMACORE_CONCEPTS_H
#define MINIMACORE_MINIMACORE_CONCEPTS_H

#include <concepts>

template<typename T>
concept floating_point_type = std::is_floating_point_v<T>;

template<typename E>
concept enumerator = std::is_enum_v<E>;

#endif //MINIMACORE_MINIMACORE_CONCEPTS_H
