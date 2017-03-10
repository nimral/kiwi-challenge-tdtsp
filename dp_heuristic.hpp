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
    std::bitset<MAX_N> S;  // set of all the nodes visited by this pt
    cost_t cost;

    // index of this pt in the heap in Keeper
    // (well it feels kind of hackish...)
    unsigned int heap_idx;

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


struct HeapElem {
    cost_t cost;

    // index of corresponding PartialTour in partials
    unsigned long int idx;
};


struct Keeper {
    unsigned int H;
    std::unordered_map<umkey_t, unsigned int> k_S2idx;
    std::vector<HeapElem> heap;  // max heap ordered by HeapElem.cost
    std::vector<PartialTour> partials;

    /* Structure for keeping H best partial tours (pt).
     *  -> Stored in `partials`.
     *
     * We don't care about their ordering, but need to be able to replace the
     * worst one quickly.
     *  -> There is a max heap with (index to `partials`, pt's cost) pairs
     *  (actually HeapElems) ordered by cost.
     *
     * We are only interested in the best pt for each (k, S) pair
     *  -> We maintain the mapping from (k, S) to index in `partials` in
     *  `k_S2idx` so that when a better pt comes, we can place it easily.
     */

    Keeper(unsigned int H) : H(H)
    {
        partials.reserve(H);
        heap.reserve(H+1);  // 1-indexing for simple child access (i*2, i*2+1)
        heap.push_back(HeapElem{0, 0});
    }

    void heap_print()
    {
        for (auto & elem : heap) {
            std::cout << elem.cost << " ";
        }
        std::cout << std::endl;
    }

    bool heap_is_heap()  // na naa nanana
    {
        for (unsigned int i = 2; i < heap.size(); i++) {
            if (heap[i / 2].cost < heap[i].cost) {
                return false;
            }
        }
        return true;
    }

    bool check()
    {
        for (unsigned int i = 0; i < partials.size(); i++) {
            if (heap[partials[i].heap_idx].idx != i) {
                return false;
            }
        }
        return true;
    }

    void heap_update_hidx(unsigned int hidx)
    {
        partials[heap[hidx].idx].heap_idx = hidx;
    }

    // swap elements in heap and don't forget to update pointers to them in the
    // corresponding pts
    void heap_swap(unsigned int a, unsigned int b) {
        auto tmp = heap[a];

        heap[a] = heap[b];
        heap_update_hidx(a);

        heap[b] = tmp;
        heap_update_hidx(b);
    }

    void heap_insert(HeapElem && elem)
    {
        unsigned int hidx = heap.size();
        heap.emplace_back(elem);
        heap_update_hidx(hidx);
        while (hidx > 1 && heap[hidx].cost > heap[hidx / 2].cost) {
            heap_swap(hidx, hidx / 2);
            hidx = hidx / 2;
        }
    }

    // note this is not your regular decrease-key, as we have max heap
    void heap_decrease_key(unsigned int hidx, cost_t cost)
    {
        heap[hidx].cost = cost;
        while (hidx * 2 < heap.size()) {
            int to_swap = -1;
            if (hidx * 2 + 1 < heap.size()) {
                if (heap[hidx * 2].cost > heap[hidx].cost) {
                    if (heap[hidx * 2].cost > heap[hidx * 2 + 1].cost) {
                        to_swap = hidx * 2;
                    } else if (heap[hidx * 2 + 1].cost > heap[hidx].cost) {
                        to_swap = hidx * 2 + 1;
                    }
                } else if (heap[hidx * 2 + 1].cost > heap[hidx].cost) {
                    to_swap = hidx * 2 + 1;
                }
            } else if (heap[hidx * 2].cost > heap[hidx].cost) {
                to_swap = hidx * 2;
            }

            if (to_swap != -1) {
                heap_swap(hidx, to_swap);
                hidx = to_swap;
            } else {
                break;
            }
        }

    }

    // add one pt to the Keeper if it is good enough
    void add(const PartialTour && pt) {
        // is pt with this (k, S) pair stored already?
        auto key = std::make_pair(pt.k(), pt.S);
        auto it = k_S2idx.find(key);
        if (it != k_S2idx.end()) {
            PartialTour & found = partials[it->second];
            if (found.cost > pt.cost) {
                unsigned int hidx = found.heap_idx;
                found = pt;
                found.heap_idx = hidx;
                heap_decrease_key(hidx, pt.cost);
            }
        // no stored pt has this (k, S) pair
        } else {
            if (partials.size() < H) {
                partials.emplace_back(pt);
                heap_insert(HeapElem{pt.cost, partials.size()-1});
                k_S2idx[key] = partials.size() - 1;
            } else if (heap[1].cost > pt.cost) {
                partials[heap[1].idx] = pt;
                heap_update_hidx(1);
                k_S2idx[key] = heap[1].idx;
                heap_decrease_key(1, pt.cost);
            }
        }
    }

    void clear()
    {
        k_S2idx.clear();
        heap.resize(1);
        partials.clear();
    }
};


void dp_heuristic(int n,
                  const cid_t start,
                  const costs_table_t & costs,
                  unsigned int H,
                  std::vector<IOArc> &output_arcs)
{
    Keeper keeper(H);
    keeper.add(PartialTour(start));
    Keeper new_keeper(H);

    for (cid_t t = 0; t < n; t++) {
        for (const auto & pt : keeper.partials) {
            for (cid_t to = 0; to < n; to++) {
                if (t == n-1 && to != start) {
                    // in the last layer only arcs leading to the beginning of
                    // the tour are relevant
                    continue;
                }
                cost_t cost = costs[t][pt.k()][to];
                if (cost >= 0 && !pt.S[to]) {
                    new_keeper.add(pt.prolonged(to, cost));
                }
            }
        }
        keeper.clear();
        std::swap(keeper, new_keeper);
    }

    if (keeper.partials.empty()) {
        return;
    }

    // there can be only one partial tour for S = {0 .. n-1}, k = start
    const PartialTour & best_pt = keeper.partials[0];

    output_arcs.reserve(n);
    for (cid_t t = 0; t < n; t++) {
        cid_t from = best_pt.tour[t];
        cid_t to = best_pt.tour[t+1];
        output_arcs.emplace_back(
            IOArc(from, to, t, costs[t][from][to])
        );
    }
}

#endif
// vim: set tabstop=4 expandtab shiftwidth=4 softtabstop=4 shiftround
