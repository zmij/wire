/*
 * cereal_output_benchmarks.cpp
 *
 *  Created on: 6 нояб. 2016 г.
 *      Author: sergey.fedorov
 */

#include <benchmark/benchmark.h>
#include "test_data_structure_cereal_io.hpp"
#include <cereal/cereal.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/vector.hpp>

namespace wire {
namespace json {
namespace bench {

void
BM_CerealStructOutput(::benchmark::State& state)
{
    test::test_structure ts{"foo", 100500, 3.14};
    while (state.KeepRunning()) {
        ::std::ostringstream os;
        cereal::JSONOutputArchive ar{os};
        ar(ts);
    }
}

void
BM_CerealVectorOutput(::benchmark::State& state)
{
    ::std::vector<int> vec{ -1, 42, 200500 };
    while (state.KeepRunning()) {
        ::std::ostringstream os;
        cereal::JSONOutputArchive ar{os};
        ar(vec);
    }
}

void
BM_CerealStringOutput(::benchmark::State& state)
{
    ::std::string str{"Hello benchmark"};
    while (state.KeepRunning()) {
        ::std::ostringstream os;
        cereal::JSONOutputArchive ar{os};
        ar(str);
    }
}

void
BM_CerealStructVectorOutput(::benchmark::State& state)
{
    static ::std::vector<test::test_structure> vec(10000, test::test_structure{"foo", 100500, 3.14});
//    vec.reserve(1000);
//    for (int i = 0; i < 1000; ++i) {
//        vec.emplace_back(test::test_structure{"foo", 100500, 3.14});
//    }
    while (state.KeepRunning()) {
        ::std::ostringstream os;
        cereal::JSONOutputArchive ar{os};
        ar(vec);
    }
}


BENCHMARK(BM_CerealStructOutput);
BENCHMARK(BM_CerealVectorOutput);
BENCHMARK(BM_CerealStringOutput);
BENCHMARK(BM_CerealStructVectorOutput);

}  /* namespace bench */
}  /* namespace json */
}  /* namespace wire */
