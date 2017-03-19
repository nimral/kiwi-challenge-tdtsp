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
    auto end_time = start_time + std::chrono::milliseconds(30 * 1000 - 200);
    std::ios::sync_with_stdio(false);

    id_t n;
    cid_t start;
    Cities cities;
    costs_table_t costs;
    init_from_input(start, cities, costs);
    n = cities.size();

    // XXX reevaluate
    unsigned int H;
    if (n <= 20) {
        H = 180000;  // u-pl0, n = 30, H = 60000: 18 s
    } else if (n <= 30) {
        H = 100000;
    } else if (n <= 50) {
        H = 50000;
    } else if (n <= 70) {
        H = 28000;
    } else if (n <= 100) {
        H = 15000;
    } else if (n <= 200) {
        H = 4200;
    } else if (n <= 300) {
        H = 1900;
    } else {
        H = 1000;
    }

    std::vector<std::vector<cid_t>> tours(CPU_COUNT);
#pragma omp parallel for
    for (std::size_t i = 0; i < CPU_COUNT; ++i) {
        dp_heuristic(n, start, costs, H, DIRECTIONS[i], tours[i]);
    }

    std::vector<std::thread> threads;
    std::vector<cost_t> outputs(CPU_COUNT, std::numeric_limits<cost_t>::max());
    for (std::size_t i = 0; i < CPU_COUNT; ++i) {
        const std::size_t ii = i;
        if (tours[ii].empty()) {
            outputs[ii] = std::numeric_limits<cost_t>::max();
        } else {
            threads.emplace_back(
                std::thread(random_perturbations, n,
                            std::ref(tours[ii]),
                            std::ref(costs),
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
#ifdef DEBUG
#include <set>
        std::set<cid_t> S;
        for (cid_t t = 0; t < n; ++t) {
            cid_t from = tours[i][t];
            cid_t to = tours[i][t+1];
            S.insert(from);
            if (costs[t][from][to] == NO_ARC) {
                std::cerr << "NONEXISTENT FLIGHT!" << std::endl;
                return 1;
            }
        }
        if (S.size() != n) {
            std::cerr << "NOT A PROPER CYCLE!" << std::endl;
            return 1;
        }
#endif
    }

    if (best_cost != std::numeric_limits<cost_t>::max()) {
        output_t output_arcs(n);
        for (cid_t t = 0; t < n; ++t) {
            cid_t from = tours[best_idx][t];
            cid_t to = tours[best_idx][t+1];
            output_arcs[t] = IOArc(from, to, t, costs[t][from][to]);
        }
        print_output(output_arcs, best_cost, cities, n);
    }

    return 0;
}
// vim: set tabstop=4 expandtab shiftwidth=4 softtabstop=4 shiftround
