#ifndef LOCAL_SEARCH_H
#define LOCAL_SEARCH_H

#include <vector>
#include "tabu_search.h"

void best_improving_neighbor_transfer(TabuSearch& schedule);
void best_improving_neighbor_swap(TabuSearch& schedule);

void first_improving_neighbor_transfer(TabuSearch& schedule);
void first_improving_neighbor_swap(TabuSearch& schedule);

#endif