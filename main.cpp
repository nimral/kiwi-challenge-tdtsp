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

    output_t output_arcs = dp_heuristic(n, start, costs);

    print_output(output_arcs, cities);
}
// vim: set tabstop=4 expandtab shiftwidth=4 softtabstop=4 shiftround
