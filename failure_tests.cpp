#include "TestFramework.h"
#include <vector>
#include <string>

// Functions that return wrong values for testing
int badAdd(int a, int b) { return a + b + 1; }  // Off by one
double badPi() { return 3.0; }  // Wrong value
int* getNullPtr() { return nullptr; }
int* getNonNullPtr() { static int x = 42; return &x; }

// Basic assertion failures
__TEST__(equality_failure) {
    __ASSERT_EQ__(5, badAdd(2, 3));  // Expected: 5, Actual: 6
}

__TEST__(inequality_failure) {
    __ASSERT_NE__(6, badAdd(2, 3));  // Expected 6 != 6
}

__TEST__(less_than_failure) {
    __ASSERT_LT__(10, 5);  // 10 >= 5
}

__TEST__(greater_than_failure) {
    __ASSERT_GT__(3, 8);  // 3 <= 8
}

__TEST__(less_equal_failure) {
    __ASSERT_LE__(15, 10);  // 15 > 10
}

__TEST__(greater_equal_failure) {
    __ASSERT_GE__(5, 12);  // 5 < 12
}

__TEST__(near_failure) {
    __ASSERT_NEAR__(3.14159, badPi(), 0.1);  // Expected: 3.14159, Actual: 3, Diff: 0.14159 > 0.1
}

__TEST__(boolean_failures) {
    __ASSERT_TRUE__(false);   // Expected true
    __ASSERT_FALSE__(true);   // Expected false
}

__TEST__(pointer_failures) {
    __ASSERT_NULL__(getNonNullPtr());     // Pointer is not null
    __ASSERT_NOT_NULL__(getNullPtr());    // Pointer is null
}

__TEST__(exception_failures) {
    // This should throw but doesn't
    __ASSERT_THROWS__(std::runtime_error, []() { 
        // No exception thrown
    });
    
    // This shouldn't throw but does
    __ASSERT_NO_THROW__([]() { 
        throw std::runtime_error("Oops!"); 
    });
}

// Parameterized test failures
__TEST_PARAM__(bad_math_operations,
    std::make_tuple(2, 3, 5),    // Will fail: badAdd returns 6
    std::make_tuple(1, 1, 2),    // Will fail: badAdd returns 3
    std::make_tuple(0, 0, 0)) {  // Will fail: badAdd returns 1
    auto [a, b, expected] = param;
    __ASSERT_EQ__(expected, badAdd(a, b));
}

// Fixture with failing tests
class BadMathFixture {
public:
    std::vector<int> numbers;
    
    BadMathFixture() {
        numbers = {1, 2, 3, 4, 5};
    }
    
    int wrongSum() const {
        return 100;  // Always returns wrong value
    }
};

__TEST_FIXTURE__(BadMathFixture, wrong_sum) {
    __ASSERT_EQ__(15, fixture.wrongSum());  // Expected: 15, Actual: 100
}

__TEST_FIXTURE__(BadMathFixture, wrong_size) {
    __ASSERT_EQ__(10, fixture.numbers.size());  // Expected: 10, Actual: 5
}

// Parameterized fixture failures
__TEST_FIXTURE_PARAM__(BadMathFixture, bad_operations,
    10, 20, 30) {
    __ASSERT_EQ__(param, fixture.wrongSum());  // All will fail
}

// Shared fixture failures
class BadDatabaseFixture {
public:
    std::vector<std::string> records;
    
    BadDatabaseFixture() {
        records = {"user1", "user2"};  // Only 2 records, not 3
    }
    
    size_t wrongCount() const { 
        return 999;  // Always wrong
    }
};

__TEST_SHARED_FIXTURE__(BadDatabaseFixture, badDb, wrong_initial_count) {
    __ASSERT_EQ__(3, fixture.records.size());  // Expected: 3, Actual: 2
}

__TEST_SHARED_FIXTURE__(BadDatabaseFixture, badDb, wrong_method_count) {
    __ASSERT_EQ__(2, fixture.wrongCount());  // Expected: 2, Actual: 999
}

// Parameterized shared fixture failures
__TEST_SHARED_FIXTURE_PARAM__(BadDatabaseFixture, badDb, wrong_comparisons,
    1, 2, 3) {
    __ASSERT_EQ__(param, fixture.wrongCount());  // All will fail
}

// String comparison failures
__TEST__(string_failures) {
    std::string expected = "Hello World";
    std::string actual = "Hello Universe";
    __ASSERT_EQ__(expected, actual);  // Expected: Hello World, Actual: Hello Universe
}

// Custom message failures
__TEST__(custom_message_failure) {
    __ASSERT_EQ__(42, 24, "The answer should be 42!");
}