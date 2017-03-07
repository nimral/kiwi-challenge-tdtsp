#ifndef DP_HEURISTIC_HPP_
#define DP_HEURISTIC_HPP_

#include<bitset>
#include<unordered_map>
#include<memory>

// http://stackoverflow.com/a/7222201/4786205
template <class T>
inline void hash_combine(std::size_t & seed, const T & v)
{
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

namespace std
{
    template<typename S, typename T> struct hash<pair<S, T>>
    {
        inline size_t operator()(const pair<S, T> & v) const
        {
            size_t seed = 0;
            ::hash_combine(seed, v.first);
            ::hash_combine(seed, v.second);
            return seed;
        }
    };
}


typedef std::pair<cid_t, std::bitset<MAX_N>> umkey_t;


struct PartialTour {
    std::vector<cid_t> tour;
    std::bitset<MAX_N> S;
    cost_t cost;

    PartialTour() {}
    PartialTour(cid_t k) :
        cost(0)
    {
        tour.push_back(k);
        // S.set(k); we do not want to have start in S
    }

    cid_t k() const
    {
        return tour.back();
    }

    PartialTour prolonged(cid_t idx, cost_t cost) const
    {
        PartialTour pt(*this);
        pt.tour.push_back(idx);
        pt.S.set(idx);
        pt.cost += cost;
        return pt;
    }
};
    

void keep_H_best(std::vector<PartialTour> & new_partials, unsigned int H)
{
    if (new_partials.size() > H) {
        std::nth_element(
            new_partials.begin(),
            new_partials.begin() + H - 1,
            new_partials.end(),
            [](const PartialTour & a, const PartialTour & b) {
                return a.cost < b.cost;
            }
        );
        new_partials.resize(H);
    }
}

// step 3
void keep_H_best(std::vector<PartialTour> & new_partials,
                 unsigned int H,
                 std::unordered_map<umkey_t, unsigned int> & k_S2idx)
{
    if (new_partials.size() > H) {
        keep_H_best(new_partials, H);
        k_S2idx.clear();
        for (unsigned int i = 0; i < new_partials.size(); i++) {
            const PartialTour & pt = new_partials[i];
            k_S2idx[std::make_pair(pt.k(), pt.S)] = i;
        }
    }
}


output_t dp_heuristic(int n,
                      const cid_t & start,
                      const costs_table_t & costs,
                      unsigned int H)
{
    std::vector<PartialTour> partials(1, PartialTour(start));
    std::vector<PartialTour> new_partials;

    unsigned int max_partials_size = H + H/2;

    for (cid_t t = 0; t < n; t++) {
        // step 2
        std::unordered_map<umkey_t, unsigned int> k_S2idx;
        // TODO "If a partial arrival time is less than the cost value of the
        // worst retained partial tour in current stage so far, store S, k, and
        // cost values of the new partial tour (?) until the Set and Cost
        // arrays are filled or all possible partial tours are obtained."
        cost_t Hth_worst_cost_upper_bound = 0;
        for (const auto & pt : partials) {
            for (cid_t to = 0; to < n; to++) {
                if (t == n-1 && to != start) {
                    // in the last layer only arcs leading to the beginning of
                    // the tour are relevant
                    continue;
                }
                cost_t cost = costs[t][pt.k()][to];
                if (cost >= 0 && !pt.S[to]) {

                    // a little optimization -- skip pts definitely worse than
                    // than the Hth best pt so far
                    if (new_partials.size() < H) {
                        if (Hth_worst_cost_upper_bound < pt.cost + cost) {
                            Hth_worst_cost_upper_bound = pt.cost + cost;
                        }
                    } else if (Hth_worst_cost_upper_bound < pt.cost + cost) {
                        continue;
                    }

                    PartialTour new_pt = pt.prolonged(to, cost);

                    // keep only the best partial tour for each (k, S) pair
                    // (alternative solution to sorting)
                    auto key = std::make_pair(new_pt.k(), new_pt.S);
                    auto it = k_S2idx.find(key);
                    if (it != k_S2idx.end()) {
                        if (new_partials[it->second].cost >= new_pt.cost) {
                            new_partials[it->second] = new_pt;
                        }
                    } else {
                        new_partials.emplace_back(new_pt);
                        k_S2idx[key] = new_partials.size() - 1;
                    }
                    if (new_partials.size() >= max_partials_size) {
                        // step 3
                        keep_H_best(new_partials, H, k_S2idx);
                    }
                }
            }
        }
        // step 3
        keep_H_best(new_partials, H);
        partials.clear();
        std::swap(partials, new_partials);
    }

    std::vector<IOArc> output_arcs;
    if (partials.empty()) {
        return output_arcs;
    }

    // there can be only one partial tour for S = {0 .. n-1}, k = start
    const PartialTour & best_pt = partials[0];

    output_arcs.reserve(n);
    for (cid_t t = 0; t < n; t++) {
        cid_t from = best_pt.tour[t];
        cid_t to = best_pt.tour[t+1];
        output_arcs.emplace_back(
            IOArc(t, from, to, costs[t][from][to])
        );
    }

    return output_arcs;
}

#endif
// vim: set tabstop=4 expandtab shiftwidth=4 softtabstop=4 shiftround
