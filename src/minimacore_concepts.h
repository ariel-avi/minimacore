
#ifndef MINIMACORE_MINIMACORE_CONCEPTS_H
#define MINIMACORE_MINIMACORE_CONCEPTS_H

#include <concepts>

template <typename Fp_T>
concept floating_point_type = std::is_floating_point_v<Fp_T>;

template <typename Enum_T>
concept enumerator = std::is_enum_v<Enum_T>;

#endif // MINIMACORE_MINIMACORE_CONCEPTS_H
