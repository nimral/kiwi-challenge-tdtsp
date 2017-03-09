#ifndef COMMON_HPP_
#define COMMON_HPP_

#include<cstdio>
#include<vector>
#include<map>
#include<set>
#include<iostream>
#include<iomanip>
#include<time.h>
#include<sstream>
#include<fstream>
#include<string>
#include<string.h>
#include<algorithm>


#define MAX_N 300
#define NO_ARC -1

typedef long cost_t;
typedef unsigned short cid_t;  // city id or day number (the same range)
typedef std::vector<std::vector<std::vector<cost_t>>> costs_table_t;


// Class storing city code to index mapping
class Cities {

    std::map<std::string, cid_t> code2idx_map;
    std::map<cid_t, std::string> idx2code_map;
    
public:
    cid_t code2idx(std::string & code)
    {
        auto it = code2idx_map.find(code);
        if (it == code2idx_map.end()) {
            cid_t id = code2idx_map.size();
            code2idx_map[code] = id;
            idx2code_map[id] = code;
        }
        return code2idx_map[code];
    }

    std::string idx2code(cid_t idx) const {
        auto it = idx2code_map.find(idx);
        return it->second;
    }

    unsigned int size()
    {
        return code2idx_map.size();
    }
};


// For saving input before we know the number of cities
struct IOArc {
    cid_t from, to, day;
    cost_t price;
    IOArc() {}
    IOArc(cid_t from, cid_t to, cid_t day, cost_t price) :
        from(from),
        to(to),
        day(day),
        price(price)
        {}
};
typedef std::vector<IOArc> output_t;


void init_from_input(cid_t & start, Cities & cities, std::vector<costs_table_t> & costs)
{
    std::string start_str;
    std::cin >> start_str;
    start = cities.code2idx(start_str);

    std::vector<IOArc> input_arcs;
    std::string line;
    std::getline(std::cin, line); // read the rest of the first line

    // save all the lines to input_arcs
    while (std::getline(std::cin, line)) {
        std::stringstream sts(line);

        std::string from_str, to_str;
        IOArc a;
        sts >> from_str >> to_str >> a.day >> a.price;
        a.from = cities.code2idx(from_str);
        a.to = cities.code2idx(to_str);

        input_arcs.emplace_back(a);
    }

    int n = cities.size();

    // costs[day][from][to] -> cost_t 
    for (std::size_t i = 0; i < costs.size(); ++i) {
        costs[i] = costs_table_t(
            n, std::vector<std::vector<cost_t>>(n, std::vector<cost_t>(n, NO_ARC))
        );
    }

    for (const auto & a : input_arcs) {
        cost_t & costf = costs[0][a.day][a.from][a.to];
        cost_t & costb = costs[1][n-a.day-1][a.to][a.from];
        if (costf == NO_ARC || costf > a.price) {
            // visits to start allowed only in the last day
            if (a.day == n-1 || a.to != start) {
                costf = a.price;
            }
        }
        if (costb == NO_ARC || costb > a.price) {
            // visits to start allowed only in the last day
            if (a.day == 0 || a.from != start) {
                costb = a.price;
            }
        }
    }
}


cost_t final_cost(const output_t & arcs)
{
    cost_t cost = 0;
    for (const auto & arc : arcs) {
        cost += arc.price;
    }
    return cost;
}


void print_output(output_t & output_arcs,
                  const cost_t cost,
                  const Cities & cities,
                  bool just_cost=false)
{
    bool sorted = true;
    for (unsigned int i = 0; i < output_arcs.size(); i++) {
        if (output_arcs[i].day != i) {
            sorted = false;
        }
    }
    std::cout << cost << std::endl;
    if (just_cost) {
        return;
    }
    if (!sorted) {
        std::sort(
            output_arcs.begin(),
            output_arcs.end(),
            [](const IOArc & a, const IOArc & b) { return a.day < b.day; }
        );
    }
    for (const auto & arc : output_arcs) {
        std::cout << cities.idx2code(arc.from) << " "
                  << cities.idx2code(arc.to) << " "
                  << arc.day << " " 
                  << arc.price << std::endl;
    }
}


// iteratively remove arcs which cannot be used to reach start
void prune_costs(cid_t n, cid_t start, costs_table_t & costs)
{
    std::vector<std::vector<char>> can_reach(n+1, std::vector<char>(n, 0));
    can_reach[n][start] = 1;
    bool not_all_can = true;
    for (cid_t t = n; t >= 1 && not_all_can; t--) {
        int pruned = 0;
        for (cid_t from = 0; from < n && not_all_can; from++) {
            for (cid_t to = 0; to < n; to++) {
                if (can_reach[t][to] && costs[t-1][from][to] > NO_ARC) {
                    can_reach[t-1][from] = 1;
                }
            }
            if (!can_reach[t-1][from]) {
                pruned++;
                for (cid_t to = 0; to < n; to++) {
                    costs[t-1][from][to] = NO_ARC;
                }
                not_all_can = true;
            }
        }
    }
}


#endif
// vim: set tabstop=4 expandtab shiftwidth=4 softtabstop=4 shiftround
