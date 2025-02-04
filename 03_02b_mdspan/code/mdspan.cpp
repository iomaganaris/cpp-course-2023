#include <experimental/mdspan>

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <ranges>
#include <span>
#include <vector>

#if defined(__cpp_multidimensional_subscript) and (__cpp_multidimensional_subscript >= 202110L)

void extents_snippets() {
    auto ext1 = std::extents<int, std::dynamic_extent, 3, std::dynamic_extent, 4>{ 42, 43 };
    static_assert(decltype(ext1)::rank() == 4);
    static_assert(decltype(ext1)::rank_dynamic() == 2);
    static_assert(decltype(ext1)::static_extent(0) == std::dynamic_extent);
    assert(ext1.extent(0) == 42);
    static_assert(decltype(ext1)::static_extent(1) == 3);

    auto ext2 = std::extents<std::uint8_t, 3, 4>{};
    static_assert(decltype(ext2)::static_extent(0) == 3);
    static_assert(decltype(ext2)::static_extent(1) == 4);

    auto ext3 = std::extents{ 42, 44 };
    static_assert(decltype(ext3)::static_extent(42) == std::dynamic_extent);
    assert(ext3.extent(0) == 42);

    auto ext4 = std::dextents<int, 3>{ 42, 43, 44 };
}

void layout_strides() {
    std::vector<float> v(100);

    // using strided_md_span = std::mdspan<float, std::dextents<std::size_t, 3>, std::layout_stride>;
    // auto s = strided_md_span(v.data(), { std::dextents<std::size_t, 3>(2, 5, 10), std::array<std::size_t, 3>{ 5, 1, 10 } });

    auto s = std::mdspan(v.data(), std::layout_stride::mapping(std::extents(2, 5, 10), std::array{ 5, 1, 10 }));

    s[1, 2, 3] = 42;
    assert((v[2 + 5 + 30] == s[1, 2, 3]));
}

enum class DeviceType {
    CPU,
    CUDA
};

template <class ElementType, DeviceType Device>
struct host_device_protector {
    using offset_policy = host_device_protector;
    using element_type = ElementType;
    using reference = ElementType&;
    using data_handle_type = ElementType*;

    constexpr data_handle_type
    offset(data_handle_type p, size_t i) const noexcept {
        return p + i;
    }

    constexpr reference access(data_handle_type p, size_t i) const noexcept {
#ifdef __CUDA_ARCH__
        static_assert(Device == DeviceType::CUDA);
#else
        static_assert(Device == DeviceType::CPU);
#endif
        return p[i];
    }
};

void test_host_device_protector() {
    float dev_ptr[] = { 1, 2, 3, 4 };
    auto s = std::mdspan<float, std::dextents<int, 2>, std::layout_right, host_device_protector<float, DeviceType::CPU>>{ dev_ptr, std::dextents<int, 2>{ 2, 2 } };
    std::cout << s[1, 2] << std::endl;
}

auto compute_strides(auto size, auto... sizes) {
}

template <class T, std::size_t N>
struct my_mdarray {
    std::vector<T> data_;
    std::array<int, N> sizes_;

    my_mdarray(std::convertible_to<int> auto... sizes) : data_((sizes * ...)), sizes_{ sizes... } {
        std::iota(data_.begin(), data_.end(), 0);
    }

    operator std::mdspan<T, std::dextents<int, N>>() {
        return { data_.data(), std::dextents<int, N>{ sizes_ } };
    }

    std::mdspan<T, std::dextents<int, N>> mdspan() {
        return { data_.data(), std::dextents<int, N>{ sizes_ } };
    }
};

template <class T, class E, class L, class A>
T apply_mdspan(std::mdspan<T, E, L, A> s) {
    return s[1, 2];
}

int apply_int_matrix(std::mdspan<int, std::dextents<int, 2>> s) {
    return s[1, 2];
}

void test_my_md_array() {
    auto a = my_mdarray<int, 2>{ 3, 4 };
    std::cout << a.data_.size() << std::endl;
    std::cout << apply_mdspan(a.mdspan()) << std::endl;
    std::cout << apply_int_matrix(a);
}

int main() {
    test_my_md_array();
    extents_snippets();
    layout_strides();
    test_host_device_protector();

    std::vector<float> v(100);

    using ext_t = std::extents<int, std::dynamic_extent, std::dynamic_extent>;
    auto v_ext = ext_t{ 10, 10 };
    auto v_span_explicit = std::mdspan<float, ext_t>{ v.data(), v_ext };

    auto v_span = std::mdspan(v.data(), std::extents{ 10, 10 });
    for (std::size_t i : std::ranges::iota_view(0, 9))
    {
        for (std::size_t j : std::ranges::iota_view(0, 9)) {
            v_span[i, j] = i * 10 + j;
        }
    }

    std::cout << v[15] << std::endl;
}


#else
int main() {}
#endif
