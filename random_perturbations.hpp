#ifndef RANDOM_PERTURBATIONS_HPP_
#define RANDOM_PERTURBATIONS_HPP_

#include "common.hpp"
#include <vector>
#include <algorithm>
#include <random>
#include <atomic>
#include <array>

constexpr std::size_t MAX_PERTURBATIONS{5};
 
std::atomic<bool> TERMINATE{false};

void random_perturbations(const std::size_t n,
                          std::vector<cid_t> & V,
                          const costs_table_t & costs,
                          cost_t & output)
{
    cost_t best_cost{};
    for (cid_t t = 0; t < n; ++t) {
        best_cost += costs[t][V[t]][V[t+1]];
    }

    std::random_device rd;
    std::mt19937 g(rd());
    std::uniform_int_distribution<> dis_days(1, n-1);
    std::uniform_int_distribution<> dis_k(1, MAX_PERTURBATIONS);
    std::array<std::size_t, 2 * MAX_PERTURBATIONS> inds;

    while (!TERMINATE.load()) {
        std::size_t k = 2 * dis_k(g);
        std::size_t rollback_k{k};
        for (std::size_t i = 0; i < k; ++i) {
            inds[i] = dis_days(g);
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
            auto c11 = costs[p1-1][V[p1-1]][V[p1]];
            auto c12 = costs[p1][V[p1]][V[p1+1]];
            auto c21 = costs[p2-1][V[p2-1]][V[p2]];
            auto c22 = costs[p2][V[p2]][V[p2+1]];
            if (p1 - p2 == 1) {
                cost -= c21 + c11 + c12;
            } else {
                cost -= c21 + c22 + c11 + c12;
            }
            std::swap(V[p1], V[p2]);
            c11 = costs[p1-1][V[p1-1]][V[p1]];
            c12 = costs[p1][V[p1]][V[p1+1]];
            c21 = costs[p2-1][V[p2-1]][V[p2]];
            c22 = costs[p2][V[p2]][V[p2+1]];
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
    }

    output = best_cost;
}

#endif
