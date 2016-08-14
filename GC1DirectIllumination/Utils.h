#pragma once

#include <array>

const float c_pi = 3.14159265359f;

//=================================================================================
template <typename... T>
auto make_array(T&&... values) ->
        std::array<
            typename std::decay<
                typename std::common_type<T...>::type>::type,
            sizeof...(T)> {
    return std::array<
        typename std::decay<
            typename std::common_type<T...>::type>::type,
        sizeof...(T)>{std::forward<T>(values)...};
}