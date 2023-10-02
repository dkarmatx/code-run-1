#include <algorithm>
#include <cstdint>
#include <iostream>
#include <vector>
#include <iterator>

namespace common {
    template <typename T>
    concept ReservableEmplacableContainer = requires (T c) {
        typename T::value_type;
        c.reserve(size_t{0});
        c.emplace_back();
    };

    template <typename Value>
    auto getFromStdin() -> Value {
        Value var;
        std::cin >> var;
        if (!std::cin.good()) {
            throw std::runtime_error("Failed to read value from stdin");
        }
        return var;
    }

    template <ReservableEmplacableContainer Collection, typename ... Args>
    auto getFromStdin(const size_t n, Args ... ns) -> Collection {
        Collection collection;
        collection.reserve(n);
        for (size_t i = 0; i < n; ++i) {
            collection.emplace_back(
                getFromStdin<typename Collection::value_type>(ns...));
        }
        return collection;
    }

    template <std::unsigned_integral I>
    auto safeUMull(const I x, const I y) -> I {
        const I result = x*y;
        if (x != 0 && result/x != y) {
            throw std::overflow_error("overflowed unsigned multiplication");
        }
        return result;
    }

    template <typename Type>
    class Matrix {
    public:
        Matrix(const size_t height, const size_t width, const Type& default_value = Type{})
            : __data(safeUMull(height, width), default_value)
            , __h(height)
            , __w(width)
        {
        }

        Matrix() = delete;
        Matrix(const Matrix&) = delete;
        Matrix(Matrix&&) = delete;

    private:
        auto checkHeightWidth(const size_t h, const size_t w) const -> void {
            if (h >= getHeight() || w >= getWidth()) {
                throw std::out_of_range("invalid indecies to access matrix");
            }
        }

        auto unchekedAt(const size_t i, const size_t j) -> Type& {
            return __data[i * getWidth() + j];
        }

        auto unchekedAt(const size_t i, const size_t j) const -> const Type& {
            return __data[i * getWidth() + j];
        }

    public:
        auto operator()(const size_t i, const size_t j) -> Type& {
            checkHeightWidth(i, j);
            return unchekedAt(i, j);
        }

        auto operator()(const size_t i, const size_t j) const -> const Type& {
            checkHeightWidth(i, j);
            return unchekedAt(i, j);
        }

        auto getHeight() const -> size_t {
            return __h;
        }

        auto getWidth() const -> size_t {
            return __w;
        }

        template <typename Func>
            requires requires (Func f, Type& val) {
                { f(0, 0, val) } -> std::same_as<void>;
            }
        auto forEach(Func f) {
            const size_t h = getHeight();
            const size_t w = getWidth();
            for (size_t i = 0; i < h; ++i) {
                for (size_t j = 0; j < w; ++j) {
                    f(i, j, unchekedAt(i, j));
                }
            }
        }

    private:
        std::vector<Type> __data;
        const size_t __h;
        const size_t __w;
    };
}  // namespace common

namespace solution {
    auto solve(
        const std::vector<std::uint64_t>& s1,
        const std::vector<std::uint64_t>& s2
    ) -> std::vector<std::uint64_t> {
        common::Matrix<size_t> map(s1.size()+1, s2.size()+1, 0ull);

        map.forEach(
            [&](const size_t i, const size_t j, size_t& value) {
                if (i != 0 && j != 0) {
                    value = (s1[i-1] == s2[j-1])
                        ? map(i-1, j-1) + 1
                        : std::max(map(i-1, j),
                                map(i, j-1));
                } else {
                    value = 0;
                }
            });

        std::vector<std::uint64_t> sub;
        sub.reserve(std::min(s1.size(), s2.size()));

        size_t i = map.getHeight()-1;
        size_t j = map.getWidth()-1;
        for (;;) {
            if (const size_t current_len = map(i, j); current_len != 0) {
                while (map(i-1, j) == current_len) {
                    i -= 1;
                }
                while (map(i, j-1) == current_len) {
                    j -= 1;
                }
                sub.emplace_back(s1[i-1]);
                i -= 1;
                j -= 1;
            } else {
                break;
            }
        }
        std::reverse(sub.begin(), sub.end());
        return sub;
    }

}  // namespace solution

auto main() -> int {
    const auto get_sequence = []() {
        const auto len = common::getFromStdin<size_t>();
        return common::getFromStdin<std::vector<std::uint64_t>>(len);
    };

    const auto s1 = get_sequence();
    const auto s2 = get_sequence();
    const auto nop_s = solution::solve(s1, s2);

    std::copy(
        nop_s.begin(),
        nop_s.end(),
        std::ostream_iterator<std::uint64_t>(std::cout, " "));
    std::cout << std::endl;
}
