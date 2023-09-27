#include <algorithm>
#include <cstdint>
#include <limits>
#include <iostream>
#include <iterator>
#include <vector>
#include <sstream>

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
}  // namespace common

namespace solution {
    class Solver {
    public:
        struct Result {
            std::vector<std::uint64_t> days_with_used_tickets;
            std::uint64_t expenses;
            std::uint64_t tickets_remains;
            std::uint64_t tickets_used;
        };

        Solver() = default;
        auto operator()(const std::vector<std::uint64_t>& prices, const std::uint64_t min_price_to_gain_a_ticket) -> Result;

    private:
        // В условии задано, что цена не может превышать 300, так что берем большое число с потолка,
        // но которое не приводило бы к переполнению uint64 при kUndefinedPrice*2 (то есть при суммировании)
        constexpr static std::uint64_t kUndefinedPrice = std::numeric_limits<std::uint32_t>::max();

        struct Expenses {
            std::uint64_t sum;
            std::vector<std::uint64_t> days_with_used_tickets;

            // В сравнении используем имеет смысл сравнивать только значения сумм расходов
            auto operator<=>(const Expenses& rhs) const -> std::strong_ordering {
                return sum <=> rhs.sum;
            }
        };

        auto isGainingTicket(const std::uint64_t price) const -> bool;
        auto makeDefaultExpenses() const -> Expenses;
        auto makeDefaultExpensesOptions() const -> std::vector<Expenses>;
        auto calculateMostOptimalExpenses(const size_t tickets_count, const size_t day) const -> Expenses;

        auto initBasicProperties(const std::vector<std::uint64_t>& prices, const std::uint64_t min_price_to_gain_a_ticket) -> void;
        auto initCalculatedProperties() -> void;

        // Изначальные свойства
        std::uint64_t __min_price_to_gain_a_ticket = 0ull;
        std::uint64_t __days_cout = 0ull;
        std::vector<std::uint64_t> __prices;

        // Вычисляемые свойства
        std::uint64_t __max_tickets_count = 0ull;
        // Свойства изменяемые во время решения
        std::vector<Expenses> __previous_day_expenses_options;
    };

    auto Solver::initBasicProperties(
        const std::vector<std::uint64_t>& prices,
        const std::uint64_t min_price_to_gain_a_ticket) -> void
    {
        __min_price_to_gain_a_ticket = min_price_to_gain_a_ticket;
        __days_cout = prices.size();
        __prices = prices;
    }

    auto Solver::isGainingTicket(const std::uint64_t price) const -> bool {
        return price >= __min_price_to_gain_a_ticket;
    }

    auto Solver::makeDefaultExpenses() const -> Expenses {
        return Expenses{
            .sum = kUndefinedPrice,
            .days_with_used_tickets = {}
        };
    }

    auto Solver::makeDefaultExpensesOptions() const -> std::vector<Expenses> {
        return std::vector<Expenses>(
            __max_tickets_count+1,
            makeDefaultExpenses());
    }

    auto Solver::initCalculatedProperties() -> void {
        // Считаем максимальное количество купонов которые мы можем получить
        __max_tickets_count = std::count_if(
            __prices.begin(),
            __prices.end(),
            [this](const auto p){
                return this->isGainingTicket(p);
            });

        // По сути выставляем условия для нулевого дня, так как далее мы будем высчитывать все
        // для последующих дней
        __previous_day_expenses_options = makeDefaultExpensesOptions();
        const auto first_price = __prices[0];
        const auto initial_tickets_count = (isGainingTicket(first_price)) ? 1ull : 0ull;
        __previous_day_expenses_options[initial_tickets_count].sum = first_price;
    }

    auto Solver::calculateMostOptimalExpenses(
        const size_t tickets_count,
        const size_t day) const -> Expenses
    {
        const std::uint64_t current_day_price = __prices[day];

        // Если мы тратим купон, то получается мы не прибавляем к расходам текущую цену,
        // но записываем что в этот день мы использовали купон
        const auto get_expenses_if_ticket_was_used = [&](){
            if (tickets_count != __max_tickets_count) {
                auto expenses = __previous_day_expenses_options[tickets_count+1];
                expenses.days_with_used_tickets.emplace_back(day+1);
                return expenses;
            }
            return makeDefaultExpenses();
        };

        const auto get_expenses_if_ticket_was_gained = [&](){
            if (isGainingTicket(current_day_price) && tickets_count > 0) {
                auto expenses = __previous_day_expenses_options[tickets_count-1];
                expenses.sum += current_day_price;
                return expenses;
            }
            return makeDefaultExpenses();
        };

        // Здесь не делаем никаких проверок, что приводит к тому что если мы получаем купон, то
        // записывается фантомный вариант словно купон мы и не получили, но он абсолютно не влияет
        // на результат вычислений, так как этот функтор в таком случае всегда будет проигрывать
        // варианту когда мы тратим купон.
        const auto get_expenses_if_ticket_unused = [&](){
            auto expenses = __previous_day_expenses_options[tickets_count];
            expenses.sum += current_day_price;
            return expenses;
        };

        return std::min({
            get_expenses_if_ticket_unused(),
            get_expenses_if_ticket_was_gained(),
            get_expenses_if_ticket_was_used()});
    }

    // С помощью методов динамического программирования будем искать
    auto Solver::operator()(
        const std::vector<std::uint64_t>& prices,
        const std::uint64_t min_price_to_gain_a_ticket) -> Result
    {
        // Из-за присутствия у нас в коде такого понятия как варианты расходов для
        // предыдущего дня, а мы расчитываем это перед основным циклом для первого дня,
        // то нам надо бы иметь здесь такой костыль.
        if (prices.size() == 0) {
            return Result{
                .days_with_used_tickets={},
                .expenses = 0,
                .tickets_remains = 0,
                .tickets_used = 0
            };
        }

        // Инициируем базовые свойства которые нужны для работы всех последующих методов
        initBasicProperties(prices, min_price_to_gain_a_ticket);
        // Иницируем вычисляемые свойства зависящие от предыдущих свойств, к примеру
        // варианты расходов для предыдущего дня: $__previous_day_expenses_options
        initCalculatedProperties();

        // Для первого дня мы уже сделали нужные вычисления в предыдущей функции, так
        // что обходим все последующие дни и расчитываем при помощи мемоизации оптимальные
        // варианты на текущий день
        for (size_t day = 1; day < __days_cout; ++day) {
            std::vector<Expenses> current_day_expenses_options;
            current_day_expenses_options.reserve(__max_tickets_count+1);
            for (size_t t = 0; t < __max_tickets_count+1; ++t) {
                current_day_expenses_options.emplace_back(calculateMostOptimalExpenses(t, day));
            }
            __previous_day_expenses_options = std::move(current_day_expenses_options);
        }

        // В итоговом дне находим такой варинат, у которого самые минимальные расходы. Притом мы используем >=
        // на случай если мы имеем одинаковое количество расходов, но вариант находящийся правее, это вариант
        // с большим количеством купонов.
        size_t minimal_expenses_tickets = 0;
        for (size_t t = 1; t < __max_tickets_count+1; ++t) {
            if (__previous_day_expenses_options[minimal_expenses_tickets] >= __previous_day_expenses_options[t]) {
                minimal_expenses_tickets = t;
            }
        }
        auto& minimal = __previous_day_expenses_options[minimal_expenses_tickets];
        return Result{
            .days_with_used_tickets = minimal.days_with_used_tickets,
            .expenses = minimal.sum,
            .tickets_remains = minimal_expenses_tickets,
            .tickets_used = minimal.days_with_used_tickets.size(),
        };
    }
}  // namespace solution

// Для решения заданой задачи будем использовать метод мемоизации. Нам важно рассмотреть
// все варинты количества купонов на каждый день и их использование, а точнее рассматривать
// самые оптимальные из них.
//
// Рассмотрим такой пример:
//
// [35 40 101 59 63 5] - здесь нам нужно использовать купон, который мы получаем в 3-ий день,
//                       на тот день, который в последующем будет более дорогим, то есть на 5-ый.
//
// По сути каждый раз когда у нас появляется купон, это приводит к тому что на каждом
// последующем дне мы могли бы его использовать, но не обходить же нам все варинты.
//
// Давайте для начала нарисуем на дерево использования купонов, где каждый узел будет
// количеством денег, которые мы тратим в этот день. Соотвественно если расходы равны 0,
// то мы либо использовали купон, либо цена была равна 0
//
// 35
// |
// 40
// |
// 101  -- +1 купон
// |  \
// 0  59
// |  | \
// 63 0  63
// |  |  | \
// 5  5  0  5
// ===========
// A  B  C  D
//
// A=(s:244,k=0), B=(s:240,k=0), C=(s:298,k=0), D=(s:303,k=1)
// s - сумма расходов
// k - кол-во оставшихся купонов
// Итого имеем, что самый оптимальный путь расходов - это B.
//
// Сделаем несколько заявлений, доказывать я их естественно не буду:
// - Именно количество текущих на какой то день купонов влияет на дальнейшую
//   ветвистость последующего пути решений в дереве.
// - Если несколько путей дерева привели нас в день X, к одинаковому кол-ву
//   купонов, то получается последующий путь будет совпадать по вариативности
//   решений по использованию купонов (исходя из предыдущего заявления).
//
// Соотвественно если в точку, которую можно было бы описать как (номер_дня, колво_купонов),
// существует несколько путей решений, то чем же они отличаются? А отличаются они расходами
// на такие решения. Значит нам надо выбирать такой, который имел бы наименьшие расходы.
//
// Если нам важны именно пути до точек (day, ticket), то можно было бы нарисовать карту
// таких точек и вести по ней расчет оптимального пути. Значениями в таких точках можно
// было бы считать оптимальную сумму расходов для данной точки.
//
// Какие способы существуют которые приводили бы нас в определенную точку в такой карте?
//
// 1) Если на прошлом дне мы использовали купон.
// 2) Если на прошлом дне мы получили купон.
// 3) Если на прошлом дне мы не использовали купон.
//
// То есть если представить формулу расчета значения для точки m[d][t], то выглядит она так:
// 1) m[d][t] = m[d-1][t+1]
// 2) m[d][t] = m[d-1][t-1] + current_day_price
// 3) m[d][t] = m[d-1][t] + current_day_price
// Все что нам остается, только выбрать самое минимальное значение из этих трех.
//
// Что-бы начать строить такую карту, мы должны инициировать первый день ручками,
// ведь до первого дня возможно был большой взрыв, и там расходов еще не существовало.
// Сделать это просто: Если цена в первый день превышала сумму для получения купона, то
// изначально мы записываем такую цену в m[0][1], ведь у нас прямо в первый день есть купон,
// а иначе пишем эту цену m[0][0].
//
// Так-же надо учесть, что некоторые точки на карте не имеют значений, ведь в первый день
// к примеру у нас не может быть сразу 2-х купонов. А значит будем заполнять такие точки
// каким то Х, который при выборе одного из трех вариантов мы будем просто игнорировать.
//
// Посмотрим на такую карту, для нашего примера, до третьего дня:
//   d   0   1   2   3   4   5
// t *------------------------
// 0 |  35  75   X 176   ?   ?
// 1 |  75   X 176 235   ?   ?
// 2 |   X   X   X   ?   ?   ?
//
// Как видим из точки m[2][1] мы можем прийти в точку m[3][0] (если тратим купон),
// а так-же в точку m[3][1], если купон мы оставляем на потом.
//
// Но вот что-же делать теперь с точкой m[4][0]. Туда мы можем прийти из ситуации
// когда будем тратить купон, с суммой 235 (m[3][1]), а так-же из точки c cуммой 176
// (m[3][0]), это если мы не потратим купон. Ну просто находим минимальный по сумме вариант:
//
// - m[4][0] = m[3][0] + price = 176 + 63 = 239  // тут купона нету, так-что прибавляем цену
// - m[4][0] = m[3][1] + 0 = 235                 // тут мы тратим купон, поэтому цена не прибавляется
//
// Минимальное значение для точки (4,0) - 235
//
// В точку же (4,1) есть только один пусть, это из точки (3,1): мы не тратим купон и просто платим
// цену: m[4][1] = m[3][1] + price = 235 + 63 = 298
//
// В точку (4,2) - путя нету, купон нам не получить.
// Заполняем карту:
//
//   d   0   1   2   3   4   5
// t *------------------------
// 0 |  35  75   X 176 235   ?
// 1 |  75   X 176 235 298   ?
// 2 |   X   X   X   X   X   ?
//
// Аналогично делаем для последнего дня (цена=5):
//
// (5,0) - можно потратить купон из точки (4,1), или прийти туда из точки (4,0). Находим минимальное.
// (5,1) - туда ведет один путь из (4,1), так что просто прибавляем цену без всяких выборов
// (5,2) - купона нету, так что увы и ах
//
//   d   0   1   2   3   4   5
// t *------------------------
// 0 |  35  75   X 176 235 240
// 1 |  75   X 176 235 298 303
// 2 |   X   X   X   X   X   X
//
// Как видим тут есть наш оптимальный вариант B из дерева решений для точки где у нас 0 купонов в шестой день
// вот и все!
//
// В программе данный метод реализуется через простую мемоизацию. Хранить нам надо только предыдущий день для
// расчета текущего. Так-же в каждой точке карты надо бы сохранять историю использования купонов, что не меняет
// нашего алгоритма. Для иксов в точках без значений, можно просто использовать просто такое то условное большое
// число, что бы оно всегда игнорировалось при выборе минимального значения.
//
// Динамическое программирование ВАН-ЛАВ

// https://coderun.yandex.ru/problem/cafe
auto main() -> int {
    const auto days_count = common::getFromStdin<std::uint64_t>();
    const auto prices = common::getFromStdin<std::vector<uint64_t>>(days_count);
    const std::uint64_t min_price_to_gain_a_ticket = 101;

    auto [
        tickets_history,
        expenses,
        tickets_remains,
        tickets_used
    ] = solution::Solver()(prices, min_price_to_gain_a_ticket);

    std::cout << expenses << std::endl;;
    std::cout << tickets_remains << " " << tickets_used << std::endl;;
    for (auto x : tickets_history) {
        std::cout << x << std::endl;
    }
}
