#include <vector>
#include <limits>
#include <thread>
#include <chrono>
#include "common.hpp"
#include "dp_heuristic.hpp"
#include "random_perturbations.hpp"

int main()
{
    auto start_time = std::chrono::steady_clock::now();
    auto end_time = start_time + std::chrono::milliseconds(1000 * 30 - 200);
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

    std::vector<std::vector<cid_t>> tours(costs.size());
#pragma omp parallel for
    for (std::size_t i = 0; i < costs.size(); ++i) {
        dp_heuristic(n, start, costs[i], H, tours[i]);
    }

    std::vector<std::thread> threads;
    std::vector<cost_t> outputs(costs.size(), std::numeric_limits<cost_t>::max());
    for (std::size_t i = 0; i < costs.size(); ++i) {
        const std::size_t ii = i;
        if (tours[ii].empty()) {
            outputs[ii] = std::numeric_limits<cost_t>::max();
        } else {
            threads.emplace_back(
                std::thread(random_perturbations, n,
                            std::ref(tours[ii]),
                            std::ref(costs[ii]),
                            std::ref(outputs[ii])));
        }
    }
    std::this_thread::sleep_until(end_time);
    TERMINATE.store(true);
    for (auto & thread : threads) {
        thread.join();
    }

    cost_t best_cost = std::numeric_limits<cost_t>::max();
    std::size_t best_idx = 0;
    for (std::size_t i = 0; i < outputs.size(); ++i) {
        const cost_t cost = outputs[i];
        if (cost < best_cost) {
            best_cost = cost;
            best_idx = i;
        }
    }

    output_t output_arcs(n);
    for (cid_t t = 0; t < n; ++t) {
        cid_t from = tours[best_idx][t];
        cid_t to = tours[best_idx][t+1];
        output_arcs[t] = IOArc(from, to, t, costs[best_idx][t][from][to]);
    }

    print_output(output_arcs, best_cost, cities, n, best_idx == 1);
}
// vim: set tabstop=4 expandtab shiftwidth=4 softtabstop=4 shiftround
