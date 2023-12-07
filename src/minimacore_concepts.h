
#ifndef MINIMACORE_MINIMACORE_CONCEPTS_H
#define MINIMACORE_MINIMACORE_CONCEPTS_H

#include <concepts>

template<typename T>
concept floating_point_type = std::is_floating_point_v<T>;

template<typename T>
concept cloneable = requires(T* t){ t->clone(); };

template<typename T>
concept optimization_object = requires{ cloneable<T>; };

#endif //MINIMACORE_MINIMACORE_CONCEPTS_H
