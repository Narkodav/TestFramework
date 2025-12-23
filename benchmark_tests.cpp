#include "TestFramework.h"
#include <vector>
#include <string>
#include <algorithm>
#include <map>

// Basic benchmarks
__BENCHMARK__(vector_push_back, 1000) {
    std::vector<int> vec;
    __BENCHMARK_START__();
    vec.push_back(42);
    __BENCHMARK_END__();
}

__BENCHMARK__(vector_with_reserve, 1000) {
    std::vector<int> vec;
    vec.reserve(1000);
    __BENCHMARK_START__();
    vec.push_back(42);
    __BENCHMARK_END__();
}

__BENCHMARK__(string_append, 500) {
    std::string str;
    __BENCHMARK_RAII__();
    str += "Hello World";
}

__BENCHMARK__(map_insert, 500) {
    std::map<int, int> m;
    __BENCHMARK_SCOPE__() {
        m[42] = 100;
    };
}

// Benchmark with fixture
class DataFixture {
public:
    std::vector<int> data;
    
    DataFixture() {
        data.reserve(10000);
        for (int i = 0; i < 1000; ++i) {
            data.push_back(i);
        }
    }
};

__BENCHMARK_FIXTURE__(DataFixture, sort_data, 100) {
    auto copy = fixture.data;
    __BENCHMARK_START__();
    std::sort(copy.begin(), copy.end());
    __BENCHMARK_END__();
}

__BENCHMARK_FIXTURE__(DataFixture, reverse_data, 100) {
    auto copy = fixture.data;
    __BENCHMARK_RAII__();
    std::reverse(copy.begin(), copy.end());
}

__BENCHMARK_FIXTURE__(DataFixture, find_element, 200) {
    __BENCHMARK_SCOPE__() {
        auto it = std::find(fixture.data.begin(), fixture.data.end(), 500);
    };
}

// Parameterized benchmarks - different vector sizes
__BENCHMARK_PARAM__(vector_allocation, 500,
    100, 1000, 10000, 100000) {
    __BENCHMARK_START__();
    std::vector<int> vec(param);
    __BENCHMARK_END__();
}

__BENCHMARK_PARAM__(string_operations, 300,
    10, 50, 100, 500) {
    std::string str;
    __BENCHMARK_SCOPE__() {
        for (int i = 0; i < param; ++i) {
            str += "x";
        }
    };
}

__BENCHMARK_PARAM__(map_operations, 200,
    10, 100, 500) {
    std::map<int, int> m;
    __BENCHMARK_RAII__();
    for (int i = 0; i < param; ++i) {
        m[i] = i * 2;
    }
}

// Complex benchmark - sorting different data patterns
__BENCHMARK_PARAM__(sort_patterns, 50,
    std::make_pair("sorted", 1000),
    std::make_pair("reversed", 1000),
    std::make_pair("random", 1000)) {
    auto [pattern, size] = param;
    
    std::vector<int> data(size);
    if (pattern == std::string("sorted")) {
        for (int i = 0; i < size; ++i) data[i] = i;
    } else if (pattern == std::string("reversed")) {
        for (int i = 0; i < size; ++i) data[i] = size - i;
    } else {
        for (int i = 0; i < size; ++i) data[i] = rand() % 1000;
    }
    
    __BENCHMARK_START__();
    std::sort(data.begin(), data.end());
    __BENCHMARK_END__();
}