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


// Class storing city code to index mapping
class Cities {

    std::map<std::string, int> code2idx_map;
    
public:
    int code2idx(std::string & code)
    {
        auto it = code2idx_map.find(code);
        if (it == code2idx_map.end()) {
            code2idx_map[code] = code2idx_map.size() - 1;
        }
        return code2idx_map[code];
    }

    int size()
    {
        return code2idx_map.size();
    }
};


// For saving input before we know the number of cities
struct InputArc {
    int from, to, day, price;
};


int main()
{
    int n;
    int m;

    Cities cities;

    std::string start_str;
    std::cin >> start_str;
    int start = cities.code2idx(start_str);

    std::vector<InputArc> input_arcs;
    std::string line;
    std::getline(std::cin, line); // read the rest of the first line

    // save all the lines to input_arcs
    while (std::getline(std::cin, line)) {
        std::stringstream sts(line);

        std::string from_str, to_str;
        InputArc a;
        sts >> from_str >> to_str >> a.day >> a.price;
        a.from = cities.code2idx(from_str);
        a.to = cities.code2idx(to_str);

        input_arcs.emplace_back(a);
    }

    n = cities.size();

    // costs[day][from][to] -> int 
    std::vector<std::vector<std::vector<int>>> costs(
        n, std::vector<std::vector<int>>(n, std::vector<int>(n, -1)));

    // arcs[day][city id] -> vector of city ids TODO useful with dense graph?
    std::vector<std::vector<std::vector<short>>> arcs(
        n, std::vector<std::vector<short>>(n, std::vector<short>()));

    for (auto const & a : input_arcs) {
        int & cost = costs[a.day][a.from][a.to];
        if (cost < 0) {
            arcs[a.day][a.from].push_back(a.to);
        }
        if (cost < 0 || cost > a.price) {
            cost = a.price;
        }
    }

}

// vim: set tabstop=4 expandtab shiftwidth=4 softtabstop=4 shiftround
