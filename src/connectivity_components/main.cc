#include <algorithm>
#include <cstdint>
#include <iostream>
#include <vector>
#include <set>
#include <stack>
#include <iterator>

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
}  // namespace common

namespace solution {
    using NodeID = std::uint64_t;
    using Edge = std::tuple<NodeID, NodeID>;

    auto solve(const std::size_t nodes_count, const std::vector<Edge>& edges) -> std::vector<std::set<NodeID>> {
        std::vector<std::vector<NodeID>> nodes_connections(nodes_count+1);
        std::vector<bool> nodes_visited(nodes_count+1, false);
 
        for (const auto& [edge_one, edge_two] : edges) {
            nodes_connections[edge_one].emplace_back(edge_two);
            nodes_connections[edge_two].emplace_back(edge_one);
        }

        std::vector<std::set<NodeID>> connectivity_components;
        for (size_t node_id = 1; node_id < nodes_count+1; ++node_id) {
            std::set<NodeID> connectivity_component;
            std::stack<NodeID> nodes_to_traverse;
            nodes_to_traverse.emplace(node_id);

            while (!nodes_to_traverse.empty()) {
                auto current_node = nodes_to_traverse.top();
                nodes_to_traverse.pop();

                if (!nodes_visited[current_node]) {
                    for (auto next_node : nodes_connections[current_node]) {
                        nodes_to_traverse.emplace(next_node);
                    }
                    connectivity_component.emplace(current_node);
                    nodes_visited[current_node] = true;
                }
            }

            if (!connectivity_component.empty()) {
                connectivity_components.emplace_back(std::move(connectivity_component));
            }
        }

        return connectivity_components;
    }
}  // namespace solution

// https://coderun.yandex.ru/problem/connectivity-components
auto main() -> int {
    const auto nodes_count = common::getFromStdin<std::size_t>();
    const auto edges_count = common::getFromStdin<std::size_t>();

    std::vector<solution::Edge> edges;
    edges.reserve(edges_count);

    for (size_t edge = 0; edge < edges_count; ++edge) {
        edges.emplace_back(
            std::make_tuple(
                common::getFromStdin<solution::NodeID>(),
                common::getFromStdin<solution::NodeID>()));
    }

    const auto connectivity_components = solution::solve(nodes_count, edges);
    std::cout << connectivity_components.size() << std::endl;
    for (const auto& component: connectivity_components) {
        std::cout << component.size() << std::endl;
        std::copy(
            component.begin(),
            component.end(),
            std::ostream_iterator<std::int64_t>(std::cout, " "));
        std::cout << std::endl;
    }
}
