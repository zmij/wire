/*
 * dummy_lib.cpp
 *
 *  Created on: Dec 4, 2017
 *      Author: zmij
 */

#include "dummy_lib.hpp"
#include <iostream>

int
the_answer()
{
    return 42;
}

void
void_entry()
{
    ::std::cerr << "dummy lib void_entry\n";
}

int
int_entry(int* a, int* b)
{
    ::std::cerr << "dummy lib int_entry " << *a << " " << *b << "\n";
    return *a + *b;
}
