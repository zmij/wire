/*
 * output_benchmarks.cpp
 *
 *  Created on: 6 нояб. 2016 г.
 *      Author: sergey.fedorov
 */

#include <benchmark/benchmark_api.h>
#include <wire/json/json_ostream.hpp>
#include "test_data_structure.hpp"
#include <sstream>

namespace wire {
namespace json {
namespace bench {

void
BM_StructOutput(::benchmark::State& state)
{
    test::test_structure ts{"foo", 100500, 3.14};
    while (state.KeepRunning()) {
        ::std::ostringstream os;
        json::json_ostream jos{os, true};
        jos << ts;
    }
}

void
BM_VectorOutput(::benchmark::State& state)
{
    ::std::vector<int> vec{ -1, 42, 200500 };
    while (state.KeepRunning()) {
        ::std::ostringstream os;
        json::json_ostream jos{os, true};
        jos << vec;
    }
}

void
BM_StringOutput(::benchmark::State& state)
{
    ::std::string str{"Hello benchmark"};
    while (state.KeepRunning()) {
        ::std::ostringstream os;
        json::json_ostream jos{os, true};
        jos << str;
    }
}

BENCHMARK(BM_StructOutput);
BENCHMARK(BM_VectorOutput);
BENCHMARK(BM_StringOutput);

}  /* namespace bench */
}  /* namespace json */
}  /* namespace wire */

BENCHMARK_MAIN()
