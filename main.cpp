#include <vector>
#include <limits>
#include "common.hpp"
#include "dp_heuristic.hpp"

int main()
{
    std::ios::sync_with_stdio(false);
    
    id_t n;
    cid_t start;
    Cities cities;
    std::vector<costs_table_t> costs(2);
    init_from_input(start, cities, costs);
    n = cities.size();

    unsigned int H;
    if (n <= 30) {
        H = 60000;  // u-pl0, n = 30, H = 60000: 18 s
    } else {
        H = 80000000 / (n * n);
    }

    std::vector<output_t> output_arcs(2);
#pragma omp parallel for
    for (std::size_t i = 0; i < costs.size(); ++i) {
        dp_heuristic(n, start, costs[i], H, output_arcs[i]);
    }

    cost_t best_cost = std::numeric_limits<long>::max();
    std::size_t best_idx = 0;
    for (std::size_t i = 0; i < output_arcs.size(); ++i) {
        const cost_t cost = final_cost(output_arcs[i]);
        if (cost <= best_cost) {
            best_cost = cost;
            best_idx = i;
        }
    }

    print_output(output_arcs[best_idx], best_cost, cities, n, best_idx == 1);
}
// vim: set tabstop=4 expandtab shiftwidth=4 softtabstop=4 shiftround
