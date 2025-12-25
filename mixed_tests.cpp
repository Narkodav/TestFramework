#include "TestFramework.h"
#include <vector>
#include <string>
#include <tuple>
#include <stdexcept>

// --- Basic mock tests ---
__TEST__(mock_pass_test1) {
    __ASSERT_EQ__(10, 10);
}

__TEST__(mock_pass_test2) {
    __ASSERT_TRUE__(true);
}

__TEST__(mock_fail_test1) {
    __ASSERT_EQ__(5, 6);  // intentional fail
}

__TEST__(mock_fail_test2) {
    __ASSERT_FALSE__(true); // intentional fail
}

// --- Parameterized tests ---
__TEST_PARAM__(mock_param_add,
    std::make_tuple(1, 2, 3),
    std::make_tuple(2, 3, 5),
    std::make_tuple(3, 3, 7)) { // last one will fail
    auto [a, b, expected] = param;
    __ASSERT_EQ__(expected, a + b);
}

__TEST_PARAM__(mock_param_mult,
    std::make_tuple(2, 2, 4),
    std::make_tuple(3, 3, 9),
    std::make_tuple(4, 5, 20)) { // all pass
    auto [a, b, expected] = param;
    __ASSERT_EQ__(expected, a * b);
}

// --- Fixture ---
class MockFixture {
public:
    int value;
    MockFixture() : value(10) {}
};

__TEST_FIXTURE__(MockFixture, fixture_pass) {
    __ASSERT_EQ__(10, fixture.value);
}

__TEST_FIXTURE__(MockFixture, fixture_fail) {
    __ASSERT_EQ__(5, fixture.value); // fail
}

// --- Parameterized fixture ---
__TEST_FIXTURE_PARAM__(MockFixture, fixture_param,
    1, 2, 10) { // last param will fail
    fixture.value = param;
    __ASSERT_LT__(fixture.value, 5);
}

// --- Shared fixture ---
class SharedMock {
public:
    int counter = 0;
    void increment() { ++counter; }
};

__TEST_SHARED_FIXTURE__(SharedMock, shared, shared_pass1) {
    fixture.increment();
    __ASSERT_EQ__(1, fixture.counter);
}

__TEST_SHARED_FIXTURE__(SharedMock, shared, shared_fail) {
    fixture.increment();
    __ASSERT_EQ__(1, fixture.counter); // fail, counter is now 2
}

__TEST_SHARED_FIXTURE__(SharedMock, shared, shared_pass2) {
    fixture.increment();
    __ASSERT_GE__(fixture.counter, 3);
}

// --- Parameterized shared fixture ---
__TEST_SHARED_FIXTURE_PARAM__(SharedMock, shared, shared_param,
    1, 2, 3) {
    fixture.counter += param;
    __ASSERT_GE__(fixture.counter, param);
}

std::tuple<int, int, int> mock_param_generated(size_t i) {
    if (i == 0) return std::make_tuple(1, 2, 3);
    if (i == 1) return std::make_tuple(2, 3, 5);
    if (i == 2) return std::make_tuple(3, 3, 7); // fail
    if (i == 3) return std::make_tuple(4, 5, 10); // fail
}

int mock_param_generated_shared(size_t i) {
    return i + 1;
}

__TEST_GENERATED_PARAM__(generated_param_test, mock_param_generated, 4) { // all pass
    auto [a, b, expected] = param;
    __ASSERT_EQ__(expected, a + b);
}

__TEST_FIXTURE_GENERATED_PARAM__(SharedMock, fixture_generated_param_test, mock_param_generated, 4) { // all pass
    fixture.counter = 1;
    auto [a, b, expected] = param;
    __ASSERT_EQ__(expected, a + b);
    __ASSERT_EQ__(fixture.counter, 1);
}

__TEST_SHARED_FIXTURE_GENERATED_PARAM__(SharedMock, shared, shared_generated_param_test, mock_param_generated_shared, 4) { // all pass
    fixture.counter += param;
    __ASSERT_GE__(fixture.counter, param);
}