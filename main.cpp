#include "common.hpp"
#include "dp_heuristic.hpp"

int main()
{
    std::ios::sync_with_stdio(false);
    
    id_t n;
    cid_t start;
    Cities cities;
    costs_table_t costs;
    init_from_input(start, cities, costs);
    n = cities.size();

    unsigned int H;
    if (n <= 30) {
        H = 60000;  // u-pl0, n = 30, H = 60000: 18 s
    } else {
        H = 80000000 / (n * n);
    }
    output_t output_arcs = dp_heuristic(n, start, costs, H);

    print_output(output_arcs, cities);
}
// vim: set tabstop=4 expandtab shiftwidth=4 softtabstop=4 shiftround
