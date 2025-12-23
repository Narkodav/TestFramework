#include "TestFramework.h"
#include <vector>
#include <string>
#include <memory>
#include <tuple>

// Simple functions to test
int add(int a, int b) { return a + b; }
int multiply(int a, int b) { return a * b; }
double divide(double a, double b) { 
    if (b == 0) throw std::runtime_error("Division by zero");
    return a / b; 
}

// Basic tests
__TEST__(basic_addition) {
    __ASSERT_EQ__(5, add(2, 3));
    __ASSERT_EQ__(0, add(-1, 1));
}

__TEST__(basic_assertions) {
    __ASSERT_TRUE__(true);
    __ASSERT_FALSE__(false);
    __ASSERT_LT__(1, 2);
    __ASSERT_GT__(5, 3);
    __ASSERT_LE__(2, 2);
    __ASSERT_GE__(3, 3);
}

__TEST__(pointer_tests) {
    int* ptr = nullptr;
    __ASSERT_NULL__(ptr);
    
    int value = 42;
    ptr = &value;
    __ASSERT_NOT_NULL__(ptr);
}

__TEST__(exception_tests) {
    __ASSERT_THROWS__(std::runtime_error, []() { divide(1.0, 0.0); });
    __ASSERT_NO_THROW__([]() { divide(10.0, 2.0); });
}

__TEST__(floating_point_test) {
    __ASSERT_NEAR__(3.14159, 22.0/7.0, 0.01);
}

// Parameterized tests - auto type deduction
__TEST_PARAM__(addition_params,
    std::make_tuple(2, 3, 5),
    std::make_tuple(-1, 1, 0),
    std::make_tuple(0, 0, 0),
    std::make_tuple(10, -5, 5)) {
    auto [a, b, expected] = param;
    __ASSERT_EQ__(expected, add(a, b));
}

__TEST_PARAM__(multiplication_params,
    std::make_pair(2, 4),
    std::make_pair(3, 9),
    std::make_pair(-2, 4),
    std::make_pair(0, 0)) {
    auto [input, expected] = param;
    __ASSERT_EQ__(expected, multiply(input, input));
}

// Test fixture
class MathFixture {
public:
    std::vector<int> numbers;
    
    MathFixture() {
        numbers = {1, 2, 3, 4, 5};
    }
    
    ~MathFixture() {
        numbers.clear();
    }
    
    int sum() const {
        int total = 0;
        for (int n : numbers) total += n;
        return total;
    }
};

__TEST_FIXTURE__(MathFixture, sum_test) {
    __ASSERT_EQ__(15, fixture.sum());
}

__TEST_FIXTURE__(MathFixture, size_test) {
    __ASSERT_EQ__(5, fixture.numbers.size());
    fixture.numbers.push_back(6);
    __ASSERT_EQ__(6, fixture.numbers.size());
}

// Parameterized fixture tests - auto type deduction
__TEST_FIXTURE_PARAM__(MathFixture, vector_operations,
    10, 20, 30, 40) {
    fixture.numbers.push_back(param);
    __ASSERT_GT__(fixture.numbers.size(), 5);
    __ASSERT_EQ__(param, fixture.numbers.back());
}

// Shared fixture
class DatabaseFixture {
public:
    std::vector<std::string> records;
    
    DatabaseFixture() {
        records = {"user1", "user2", "admin"};
    }
    
    void addRecord(const std::string& record) {
        records.push_back(record);
    }
    
    size_t count() const { return records.size(); }
};

__TEST_SHARED_FIXTURE__(DatabaseFixture, db, initial_state) {
    __ASSERT_EQ__(3, fixture.count());
}

__TEST_SHARED_FIXTURE__(DatabaseFixture, db, add_record) {
    size_t initial = fixture.count();
    fixture.addRecord("newuser");
    __ASSERT_EQ__(initial + 1, fixture.count());
}

__TEST_SHARED_FIXTURE__(DatabaseFixture, db, persistent_state) {
    // This test sees changes from previous shared fixture tests
    __ASSERT_GE__(fixture.count(), 3);
}

// Parameterized shared fixture - auto type deduction
__TEST_SHARED_FIXTURE_PARAM__(DatabaseFixture, db, batch_add,
    "test1", "test2", "test3") {
    size_t before = fixture.count();
    fixture.addRecord(param);
    __ASSERT_EQ__(before + 1, fixture.count());
}

// Benchmarks
__BENCHMARK__(vector_push_back, 1000) {
    std::vector<int> vec;
    __BENCHMARK_START__();
    vec.push_back(42);
    __BENCHMARK_END__();
}

__BENCHMARK__(vector_reserve_push, 1000) {
    std::vector<int> vec;
    vec.reserve(1000);
    __BENCHMARK_START__();
    vec.push_back(42);
    __BENCHMARK_END__();
}

__BENCHMARK__(string_concatenation, 500) {
    std::string str;
    __BENCHMARK_RAII__();
    str += "Hello World";
}

__BENCHMARK__(scoped_benchmark, 100) {
    __BENCHMARK_SCOPE__() {
        std::vector<int> data(1000, 42);
        std::sort(data.begin(), data.end());
    };
}
