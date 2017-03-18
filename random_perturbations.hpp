#ifndef RANDOM_PERTURBATIONS_HPP_
#define RANDOM_PERTURBATIONS_HPP_

#include "common.hpp"
#include <vector>
#include <algorithm>
#include <random>
#include <atomic>
#include <array>
#include <limits>

// XXX reevaluate
constexpr std::size_t MAX_PERTURBATIONS{5};
constexpr std::size_t MAX_ITERS{std::numeric_limits<std::size_t>::max()};
 
std::atomic<bool> TERMINATE{false};

void random_perturbations(std::vector<cid_t> & V,
                          const std::size_t joining_point,
                          const std::size_t offset,
                          const costs_table_t & costs,
                          cost_t & output,
                          const std::size_t max_iters=MAX_ITERS)
{
    cost_t best_cost{};
    for (cid_t t = 0; t < V.size()-1; ++t) {
        best_cost += costs[offset+t][V[t]][V[t+1]];
    }

    std::random_device rd;
    std::mt19937 g(rd());
    std::uniform_int_distribution<> dis_days(
        1,
        (joining_point == 0 || joining_point == V.size()-1) ?
        V.size()-2 : V.size()-3);
    std::uniform_int_distribution<> dis_k(1, MAX_PERTURBATIONS);
    std::array<std::size_t, 2 * MAX_PERTURBATIONS> inds;

    std::size_t iters{};
    while (iters < max_iters && !TERMINATE.load()) {
        std::size_t k = 2 * dis_k(g);
        std::size_t rollback_k{k};
        for (std::size_t i = 0; i < k; ++i) {
            inds[i] = dis_days(g);
            if (inds[i] == joining_point) {
                inds[i]++;
            }
        }
        cost_t cost = best_cost;
        for (std::size_t i = 0; i < k; i += 2) {
            std::size_t & p1 = inds[i];
            std::size_t & p2 = inds[i+1];
            if (p1 == p2) {
                continue;
            }
            if (p2 - p1 == 1) {
                std::swap(p1, p2);
            }
            auto c11 = costs[offset+p1-1][V[p1-1]][V[p1]];
            auto c12 = costs[offset+p1][V[p1]][V[p1+1]];
            auto c21 = costs[offset+p2-1][V[p2-1]][V[p2]];
            auto c22 = costs[offset+p2][V[p2]][V[p2+1]];
            if (p1 - p2 == 1) {
                cost -= c21 + c11 + c12;
            } else {
                cost -= c21 + c22 + c11 + c12;
            }
            std::swap(V[p1], V[p2]);
            c11 = costs[offset+p1-1][V[p1-1]][V[p1]];
            c12 = costs[offset+p1][V[p1]][V[p1+1]];
            c21 = costs[offset+p2-1][V[p2-1]][V[p2]];
            c22 = costs[offset+p2][V[p2]][V[p2+1]];
            if (c11 == NO_ARC || c12 == NO_ARC || c21 == NO_ARC || c22 == NO_ARC) {
                rollback_k = i + 2;
                goto rollback;
            }
            if (p1 - p2 == 1) {
                cost += c21 + c11 + c12;
            } else {
                cost += c21 + c22 + c11 + c12;
            }
        }
        if (cost <= best_cost) {
            best_cost = cost;
        } else {
rollback:
            for (std::size_t i = rollback_k / 2; i > 0; --i) {
                std::swap(V[inds[2*(i-1)]], V[inds[2*i-1]]);
            }
        }
        iters++;
    }

    output = best_cost;
}

#endif
