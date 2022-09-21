//
// Created by Mike Smith on 2021/9/7.
//

#pragma once

#include <cstdint>
#include <cstddef>
#include <tuple>
#include <type_traits>

#include <glm/glm.hpp>

namespace gl_render {

    template<typename... T>
    struct always_false : std::false_type {};

    template<typename... T>
    constexpr auto always_false_v = always_false<T...>::value;

    template<typename... T>
    struct always_true : std::true_type {};

    template<typename... T>
    constexpr auto always_true_v = always_true<T...>::value;

// clang-format off
    template<typename T>
    requires std::is_enum_v<T>
    [[nodiscard]] constexpr auto to_underlying(T e) noexcept {
        return static_cast<std::underlying_type_t<T>>(e);
    }
// clang-format on

    using uint = unsigned int;

    template<typename T>
    using is_integral = std::disjunction<
            std::is_same<std::remove_cvref_t<T>, int>,
            std::is_same<std::remove_cvref_t<T>, uint>>;

    template<typename T>
    constexpr auto is_integral_v = is_integral<T>::value;

    template<typename T>
    using is_boolean = std::is_same<std::remove_cvref_t<T>, bool>;

    template<typename T>
    constexpr auto is_boolean_v = is_boolean<T>::value;

    template<typename T>
    using is_floating_point = std::is_same<std::remove_cvref_t<T>, float>;

    template<typename T>
    constexpr auto is_floating_point_v = is_floating_point<T>::value;

    template<typename T>
    using is_signed = std::disjunction<
            is_floating_point<T>,
            std::is_same<std::remove_cvref_t<T>, int>>;

    template<typename T>
    constexpr auto is_signed_v = is_signed<T>::value;

    template<typename T>
    using is_unsigned = std::is_same<std::remove_cvref_t<T>, uint>;

    template<typename T>
    constexpr auto is_unsigned_v = is_unsigned<T>::value;

    template<typename T>
    using is_scalar = std::disjunction<
            is_integral<T>,
            is_boolean<T>,
            is_floating_point<T>>;

    template<typename T>
    constexpr auto is_scalar_v = is_scalar<T>::value;

    template<typename T, size_t N>
    using Vector = glm::vec<N, T, glm::defaultp>;

    template<size_t N>
    using Matrix = glm::mat<N, N, float, glm::defaultp>;

    namespace detail {

        template<typename T, size_t N = 0u>
        struct is_vector_impl : std::false_type {};

        template<typename T, size_t N>
        struct is_vector_impl<Vector<T, N>, N> : std::true_type {};

        template<typename T, size_t N>
        struct is_vector_impl<Vector<T, N>, 0u> : std::true_type {};

        template<typename T, size_t N = 0u>
        struct is_matrix_impl : std::false_type {};

        template<size_t N>
        struct is_matrix_impl<Matrix<N>, N> : std::true_type {};

        template<size_t N>
        struct is_matrix_impl<Matrix<N>, 0u> : std::true_type {};

        template<typename T>
        struct vector_element_impl {
            using type = T;
        };

        template<typename T, size_t N>
        struct vector_element_impl<Vector<T, N>> {
            using type = T;
        };

        template<typename T>
        struct vector_dimension_impl {
            static constexpr auto value = static_cast<size_t>(1u);
        };

        template<typename T, size_t N>
        struct vector_dimension_impl<Vector<T, N>> {
            static constexpr auto value = N;
        };

        template<typename T>
        struct matrix_dimension_impl {
            static constexpr auto value = static_cast<size_t>(1u);
        };

        template<size_t N>
        struct matrix_dimension_impl<Matrix<N>> {
            static constexpr auto value = N;
        };

        template<typename U, typename V>
        struct is_vector_same_dimension_impl : std::false_type {};

        template<typename U, typename V, size_t N>
        struct is_vector_same_dimension_impl<Vector<U, N>, Vector<V, N>> : std::true_type {};

        template<typename... T>
        struct is_vector_all_same_dimension_impl : std::true_type {};

        template<typename First, typename... Other>
        struct is_vector_all_same_dimension_impl<First, Other...> : std::conjunction<is_vector_same_dimension_impl<First, Other>...> {};

    }// namespace detail

    template<typename... T>
    using is_vector_same_dimension = detail::is_vector_all_same_dimension_impl<std::remove_cvref_t<T>...>;

    template<typename... T>
    constexpr auto is_vector_same_dimension_v = is_vector_same_dimension<T...>::value;

    template<typename T>
    using vector_dimension = detail::vector_dimension_impl<std::remove_cvref_t<T>>;

    template<typename T>
    constexpr auto vector_dimension_v = vector_dimension<T>::value;

    template<typename T>
    using matrix_dimension = detail::matrix_dimension_impl<std::remove_cvref_t<T>>;

    template<typename T>
    constexpr auto matrix_dimension_v = matrix_dimension<T>::value;

    template<typename T>
    using vector_element = detail::vector_element_impl<std::remove_cvref_t<T>>;

    template<typename T>
    using vector_element_t = typename vector_element<T>::type;

    template<typename T, size_t N = 0u>
    using is_vector = detail::is_vector_impl<std::remove_cvref_t<T>, N>;

    template<typename T>
    using is_vector2 = is_vector<T, 2u>;

    template<typename T>
    using is_vector3 = is_vector<T, 3u>;

    template<typename T>
    using is_vector4 = is_vector<T, 4u>;

    template<typename T>
    using is_bool_vector = std::conjunction<is_vector<T>, std::is_same<vector_element_t<T>, bool>>;

    template<typename T>
    constexpr auto is_bool_vector_v = is_bool_vector<T>::value;

    template<typename T>
    using is_float_vector = std::conjunction<is_vector<T>, std::is_same<vector_element_t<T>, float>>;

    template<typename T>
    constexpr auto is_float_vector_v = is_float_vector<T>::value;

    template<typename T>
    using is_int_vector = std::conjunction<is_vector<T>, std::is_same<vector_element_t<T>, int>>;

    template<typename T>
    constexpr auto is_int_vector_v = is_int_vector<T>::value;

    template<typename T>
    using is_uint_vector = std::conjunction<is_vector<T>, std::is_same<vector_element_t<T>, uint>>;

    template<typename T>
    constexpr auto is_uint_vector_v = is_uint_vector<T>::value;

    template<typename T, size_t N = 0u>
    constexpr auto is_vector_v = is_vector<T, N>::value;

    template<typename T>
    constexpr auto is_vector2_v = is_vector2<T>::value;

    template<typename T>
    constexpr auto is_vector3_v = is_vector3<T>::value;

    template<typename T>
    constexpr auto is_vector4_v = is_vector4<T>::value;

    template<typename T, size_t N = 0u>
    using is_matrix = detail::is_matrix_impl<std::remove_cvref_t<T>, N>;

    template<typename T>
    using is_matrix2 = is_matrix<T, 2u>;

    template<typename T>
    using is_matrix3 = is_matrix<T, 3u>;

    template<typename T>
    using is_matrix4 = is_matrix<T, 4u>;

    template<typename T, size_t N = 0u>
    constexpr auto is_matrix_v = is_matrix<T, N>::value;

    template<typename T>
    constexpr auto is_matrix2_v = is_matrix2<T>::value;

    template<typename T>
    constexpr auto is_matrix3_v = is_matrix3<T>::value;

    template<typename T>
    constexpr auto is_matrix4_v = is_matrix4<T>::value;

    template<typename T>
    using is_basic = std::disjunction<is_scalar<T>, is_vector<T>, is_matrix<T>>;

    template<typename T>
    constexpr auto is_basic_v = is_basic<T>::value;



//    template<typename T>
//    [[nodiscard]] inline constexpr bool is_vector_v() noexcept {
//        return false;
//    }
//#define GL_RENDER_PROCESS_VECTOR(type)                                  \
//    template<>                                                          \
//    [[nodiscard]] inline constexpr bool is_vector_v<type>() noexcept {  \
//        return true;                                                    \
//    }
//    GL_RENDER_PROCESS_VECTOR(float2)
//    GL_RENDER_PROCESS_VECTOR(float3)
//    GL_RENDER_PROCESS_VECTOR(float4)
//    GL_RENDER_PROCESS_VECTOR(uint2)
//    GL_RENDER_PROCESS_VECTOR(uint3)
//    GL_RENDER_PROCESS_VECTOR(uint4)
//#undef GL_RENDER_PROCESS_VECTOR
//
//    template<typename T>
//    [[nodiscard]] inline constexpr bool is_matrix_v() noexcept {
//        return false;
//    }
//#define GL_RENDER_PROCESS_MATRIX(type)                                  \
//    template<>                                                          \
//    [[nodiscard]] inline constexpr bool is_matrix_v<type>() noexcept {  \
//        return true;                                                    \
//    }
//    GL_RENDER_PROCESS_MATRIX(float2x2)
//    GL_RENDER_PROCESS_MATRIX(float3x3)
//    GL_RENDER_PROCESS_MATRIX(float4x4)
//#undef GL_RENDER_PROCESS_MATRIX
//
//    template<typename T>
//    [[nodiscard]] inline constexpr bool is_scalar_v() noexcept {
//        return true;
//    }
//#define GL_RENDER_PROCESS_SCALAR(type)                                  \
//    template<>                                                          \
//    [[nodiscard]] inline constexpr bool is_scalar_v<type>() noexcept {  \
//        return false;                                                   \
//    }
//    GL_RENDER_PROCESS_SCALAR(float2)
//    GL_RENDER_PROCESS_SCALAR(float3)
//    GL_RENDER_PROCESS_SCALAR(float4)
//    GL_RENDER_PROCESS_SCALAR(uint2)
//    GL_RENDER_PROCESS_SCALAR(uint3)
//    GL_RENDER_PROCESS_SCALAR(uint4)
//    GL_RENDER_PROCESS_SCALAR(float2x2)
//    GL_RENDER_PROCESS_SCALAR(float3x3)
//    GL_RENDER_PROCESS_SCALAR(float4x4)
//#undef GL_RENDER_PROCESS_SCALAR

}// namespace lgl_render