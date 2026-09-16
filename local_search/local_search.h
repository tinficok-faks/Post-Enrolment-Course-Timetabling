#ifndef LOCAL_SEARCH_H
#define LOCAL_SEARCH_H

#include <vector>
#include "filereader.h"

void best_improving_neighbor_transfer(SearchHelper& schedule);
void best_improving_neighbor_swap(SearchHelper& schedule);

void first_improving_neighbor_transfer(SearchHelper& schedule);
void first_improving_neighbor_swap(SearchHelper& schedule);

#endif