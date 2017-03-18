#include <vector>
#include <limits>
#include <thread>
#include <chrono>
#include <algorithm>
#include "common.hpp"
#include "dp_heuristic.hpp"
#include "random_perturbations.hpp"

int main()
{
    auto start_time = std::chrono::steady_clock::now();
    auto end_time = start_time + std::chrono::milliseconds(1000 * 30 - 200);
    std::ios::sync_with_stdio(false);

    cid_t start;
    Cities cities;
    costs_table_t costs;
    init_from_input(start, cities, costs);
    const id_t n = cities.size();

    // XXX reevaluate
    unsigned int H;
    if (n <= 30) {
        H = 1000000;
    } else {
        H = 90000000 / (n * n);
    }

    std::vector<std::thread> threads;
    std::vector<cost_t> final_costs(
        CPU_COUNT, std::numeric_limits<cost_t>::max());
    std::vector<std::vector<cid_t>> tours(CPU_COUNT);
    std::vector<output_t> output_arcs = std::vector<output_t>(
        CPU_COUNT, output_t(n));
    for (std::size_t i = 0; i < CPU_COUNT; ++i) {
        const std::size_t ii = i;
        threads.emplace_back(
            std::thread(dp_heuristic,
                        n,
                        start,
                        std::ref(costs),
                        H,
                        std::ref(DIRECTIONS[ii]),
                        std::ref(tours[ii]),
                        std::ref(output_arcs[ii]),
                        std::ref(final_costs[ii])));
    }

    std::this_thread::sleep_until(end_time);
    TERMINATE.store(true);
    for (auto & thread : threads) {
        thread.join();
    }

    cost_t best_cost = std::numeric_limits<cost_t>::max();
    std::size_t best_idx = 0;
    for (std::size_t i = 0; i < final_costs.size(); ++i) {
        const cost_t cost = final_costs[i];
        if (cost < best_cost) {
            best_cost = cost;
            best_idx = i;
        }
    }

    if (best_cost != std::numeric_limits<cost_t>::max()) {
        print_output(output_arcs[best_idx], best_cost, cities, n);
    }

    return 0;
}
// vim: set tabstop=4 expandtab shiftwidth=4 softtabstop=4 shiftround
