#pragma once

#include "construct.h"
#include "type_traits.h"
#include "utility.h"
namespace my {
template <class InputIt, class OutputIt>
inline OutputIt uninitialized_copy(InputIt first, InputIt last,
                                   OutputIt restult) {
    my::construct(&(*result), first);
}
}  // namespace my