#include <iostream>
#include <cstdint>
#include <iterator>
#include <vector>
#include <sstream>

namespace common {
    class StringBuilder {
    public:
        template<class T>
        auto operator<< (const T& arg) -> StringBuilder& {
            __stream << arg;
            return *this;
        }

        operator std::string() const {
            return __stream.str();
        }

    private:
        std::stringstream __stream;
    };

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
            throw std::runtime_error(StringBuilder() << "Failed to read value from stdin");
        }
        return var;
    };

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
}  // namespace common

namespace solution {
    using MapValue = std::int64_t;
    using MapWithPoints = std::vector<std::vector<MapValue>>;
    using Path = std::vector<char>;
    using PointsSum = std::uint64_t;

    auto solve(MapWithPoints map) -> std::tuple<PointsSum, Path> {
        const auto height = map.size();
        const auto width = (height == 0) ? 0
                                         : map[0].size();

        // Мы будем использовать метод мемоизации в нашем решении.
        //
        // Рассчитаем матрицу, каждая клетка которой будет содержать
        // максимально возможную сумму очков с которой можно прийти в
        // эту клетку.
        //
        // Так как двигаться мы можем только в направлении вниз и вправо,
        // то что бы найти такие суммы, мы должны выбрать максимальное значение
        // из верхней или левой клетки и прибавить к текущему, при учете того,
        // что значения для этих клеток уже просчитаны
        for (size_t j = 0; j < height; ++j) {
            for (size_t i = 0; i < width; ++i) {
                map[j][i] += std::max(
                    i >= 1 ? map[j][i-1] : 0,
                    j >= 1 ? map[j-1][i] : 0);
            }
        }

        // Теперь когда у нас есть карта с максимальными значениями маршрута до
        // каждой клетки, то что-бы найти самый эффективный путь, мы должны
        // обойти матрицу начиная с конечной точки до начальной точки, выбирая
        // те клетки, значения суммы в которых максимально.
        size_t i = width-1;
        size_t j = height-1;
        PointsSum points_sum = map[j][i];
        Path path;
        while (j != 0 || i != 0) {
            MapValue up_points = (j != 0) ? map[j-1][i] : -1;
            MapValue left_points = (i != 0) ? map[j][i-1] : -1;
            
            auto& direction = path.emplace_back();
            if (up_points > left_points) {
                direction = 'D';
                --j;
            } else {
                direction = 'R';
                --i;
            }
        }

        // Надо учитывать что тут маршрут находится в перевернутом представлении
        return std::make_tuple(points_sum, path);
    }
}  // namespace solution

// Условия задачи:
// 
// В левом верхнем углу прямоугольной таблицы размером N×MN×M находится черепашка.
// В каждой клетке таблицы записано некоторое число. Черепашка может перемещаться
// вправо или вниз, при этом маршрут черепашки заканчивается в правом нижнем углу таблицы. 
// 
// Подсчитаем сумму чисел, записанных в клетках, через которую проползла черепашка
// (включая начальную и конечную клетку). Найдите наибольшее возможное значение этой
// суммы и маршрут, на котором достигается эта сумма.
// 
// Пример ввода:
//  > 5 5
//  > 9 9 9 9 9
//  > 3 0 0 0 0
//  > 9 9 9 9 9
//  > 6 6 6 6 8
//  > 9 9 9 9 9
//
// Пример вывода:
//  > 74
//  > D D R R R R D D

auto main() -> int {
    const auto height = common::getFromStdin<std::uint64_t>();
    const auto width = common::getFromStdin<std::uint64_t>();

    auto [points_sum, path] = solution::solve(
        common::getFromStdin<solution::MapWithPoints>(height, width));

    std::cout << points_sum << std::endl;
    std::copy(path.rbegin(), path.rend(), std::ostream_iterator<char>(std::cout, " "));
    std::cout << std::endl;
}