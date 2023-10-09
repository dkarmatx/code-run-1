#include <cstdint>
#include <iostream>
#include <iterator>
#include <vector>
#include <stack>
#include <utility>

namespace common {
    template <typename Value>
    auto getFromStdin() -> Value {
        Value var;
        std::cin >> var;
        if (!std::cin.good()) {
            throw std::runtime_error("Failed to read value from stdin");
        }
        return var;
    }

    template <
        typename Handler,
        typename ... HandlerCustomArgs
    >
    auto traverse(
        std::tuple<HandlerCustomArgs...>&& initial_params,
        Handler handler
    ) -> void {
        using TraveseStackItem = std::tuple<HandlerCustomArgs...>;
        std::stack<TraveseStackItem> traversing_stack;
        traversing_stack.emplace(std::move(initial_params));

        auto pusher = [&traversing_stack](HandlerCustomArgs ... args){
            traversing_stack.emplace(std::forward_as_tuple(args...));
        };

        while (!traversing_stack.empty()) {
            const auto params = std::tuple_cat(
                std::make_tuple(pusher),
                traversing_stack.top());
            traversing_stack.pop();
            std::apply(handler, params);
        }
    }
}  // namespace common

namespace solution {
    using NodeID = std::uint64_t;
    using Edge = std::tuple<NodeID, NodeID>;

    enum class Group : std::uint8_t {
        kUnvisited = 0,
        kFirst = 1,
        kSecond = 2
    };

    constexpr auto getDifferentGroup(const Group g) -> Group {
        switch (g) {
            case Group::kUnvisited:
            case Group::kSecond:
                return Group::kFirst;
            case Group::kFirst:
                return Group::kSecond;
        }
    }

    auto solve(const std::size_t nodes_count, const std::vector<Edge>& edges) -> bool {
        std::vector<std::vector<NodeID>> nodes_connections(nodes_count);
        std::vector<Group> nodes_group(nodes_count, Group::kUnvisited);
 
        for (const auto& [node_one, node_two] : edges) {
            nodes_connections[node_one].emplace_back(node_two);
            nodes_connections[node_two].emplace_back(node_one);
        }

        bool is_groupped = true;
        for (NodeID node = 0; node < nodes_count; ++node) {
            common::traverse(
                std::make_tuple(
                    node,
                    Group::kUnvisited
                ),
                [&](
                    auto traverse_pusher,
                    const NodeID current_node,
                    const Group previous_group
                ){
                    if (auto& group = nodes_group[current_node]; group == Group::kUnvisited) {
                        group = getDifferentGroup(previous_group);
                        for (const auto next_node : nodes_connections[current_node]) {
                            traverse_pusher(next_node, group);
                        }
                    } else if (group != getDifferentGroup(previous_group) && previous_group != Group::kUnvisited) {
                        is_groupped = false;
                    }
                });
        }
        return is_groupped;
    }
}  // namespace solution

// https://coderun.yandex.ru/problem/cheating
auto main() -> int {
    const auto nodes_count = common::getFromStdin<std::size_t>();
    const auto edges_count = common::getFromStdin<std::size_t>();

    std::vector<solution::Edge> edges;
    edges.reserve(edges_count);

    for (size_t edge = 0; edge < edges_count; ++edge) {
        edges.emplace_back(
            std::make_tuple(
                common::getFromStdin<solution::NodeID>() - 1,
                common::getFromStdin<solution::NodeID>() - 1));
    }

    std::cout << (solution::solve(nodes_count, edges) ? "YES" : "NO") << std::endl;
}
