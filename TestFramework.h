#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <map>
#include <filesystem>
#include <chrono>
#include <algorithm>
#include <numeric>
#include <sstream>

#undef assert

namespace detail {
    static inline const std::string __RED__ = "\033[31m";
    static inline const std::string __GREEN__ = "\033[32m";
    static inline const std::string __YELLOW__ = "\033[33m";
    static inline const std::string __BLUE__ = "\033[34m";
    static inline const std::string __CYAN__ = "\033[36m";
    static inline const std::string __RESET__ = "\033[0m";
}


class Benchmark {    
private:
    std::function<void(Benchmark&)> m_benchmark;
    std::vector<std::chrono::time_point<std::chrono::high_resolution_clock>> m_startTimes;
    std::vector<double> m_durationTimes;
public:
    Benchmark() = default;
    ~Benchmark() = default;

    Benchmark(const Benchmark&) = default;
    Benchmark& operator=(const Benchmark&) = default;

    Benchmark(Benchmark&&) = default;
    Benchmark& operator=(Benchmark&&) = default;

    Benchmark(std::function<void(Benchmark&)> benchmark, size_t iterations) : m_benchmark(benchmark) {
        m_startTimes.resize(iterations);
        m_durationTimes.resize(iterations);
    }

    std::function<void(Benchmark&)>& getBenchmark() {
        return m_benchmark;
    }

    void printStatistics() {
        if (m_durationTimes.empty()) {
            std::cout << "No benchmark data available.\n";
            return;
        }

        std::sort(m_durationTimes.begin(), m_durationTimes.end());

        double avg = std::accumulate(m_durationTimes.begin(), m_durationTimes.end(), 0.0) / m_durationTimes.size();
        double median = (m_durationTimes.size() % 2 == 0) ?
            (m_durationTimes[m_durationTimes.size() / 2 - 1] + m_durationTimes[m_durationTimes.size() / 2]) / 2.0 :
            m_durationTimes[m_durationTimes.size() / 2];
        double min = m_durationTimes.front();
        double max = m_durationTimes.back();

        using namespace detail;
        std::cout << "avg: " << __CYAN__ << avg << __RESET__ << "us, median: " << __CYAN__ << median << __RESET__ << "us, min: " << __CYAN__ << min << __RESET__ << "us, max: " << __CYAN__ << max << __RESET__ << "us\n";
    }

    void setBenchmark(std::function<void(Benchmark&)> benchmark, size_t iterations) {
        m_benchmark = benchmark;
        m_startTimes.resize(iterations);
        m_durationTimes.resize(iterations);
    }
    void startTiming(size_t i) {
        m_startTimes[i] = std::chrono::high_resolution_clock::now();
    }

    void endTiming(size_t i) {
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::micro> duration = end - m_startTimes[i];
        m_durationTimes[i] = duration.count();
    }
};

class TestFramework {   

    using BenchmarkMap = std::map<std::string, std::vector<Benchmark>>;
    using BenchmarkIterator = std::vector<Benchmark>::iterator;

private:
    static inline std::map<std::string, std::vector<std::function<void()>>> m_tests;
    static inline std::map<std::string, std::shared_ptr<void>> m_sharedFixtures;
    static inline BenchmarkMap m_benchmarks;
    static inline std::string m_currentTest;
    static inline size_t m_passedTests;
    static inline size_t m_failedTests;

public:

    static void addTest(const std::string& filename, const std::string& name, std::function<void()> test) {
        m_tests[filename].push_back([name, filename, test]() {
            using namespace detail;
            m_currentTest = name;
            std::cout << "  " << name << "... ";
            try {
                test();
                std::cout << __GREEN__ << "PASSED\n" << __RESET__;
                m_passedTests++;
            } catch (const std::exception& e) {
                std::cout << __RED__ << "FAILED" << __RESET__ 
                << " with exception: " << __YELLOW__ << e.what() 
                << __RESET__ << "\n";
                m_failedTests++;
            }
        });
    }

    template<typename ParamFunction, typename ParamVector>
    static void addParameterizedTest(const std::string& filename, const std::string& name,
        ParamFunction&& test, const ParamVector& params) {
        size_t index = 0;
        for (const auto& param : params) {
            m_tests[filename].push_back([name, filename, test, param, index]() {
                using namespace detail;
                m_currentTest = name + "[" + std::to_string(index) + "]";
                std::cout << "  " << m_currentTest << "... ";
                try {
                    test(param);
                    std::cout << __GREEN__ << "PASSED\n" << __RESET__;
                    m_passedTests++;
                } catch (const std::exception& e) {
                    std::cout << __RED__ << "FAILED" << __RESET__ 
                    << " with exception: " << __YELLOW__ << e.what() 
                    << __RESET__ << "\n";
                    m_failedTests++;
                }
            });
            index++;
        }
    }

    template<typename FixtureType>
    static void addFixtureTest(const std::string& filename, const std::string& name,
        std::function<void(FixtureType&)> test) {
        m_tests[filename].push_back([name, filename, test]() {
            using namespace detail;
            m_currentTest = name;
            std::cout << "  " << name << "... ";
            try {
                {
                    FixtureType fixture;
                    test(fixture);
                }
                std::cout << __GREEN__ << "PASSED\n" << __RESET__;
                m_passedTests++;
            }
            catch (const std::exception& e) {
                std::cout << __RED__ << "FAILED" << __RESET__
                    << " with exception: " << __YELLOW__ << e.what()
                    << __RESET__ << "\n";
                m_failedTests++;
            }
            });
    }

    template<typename FixtureType, typename ParamFunction, typename ParamVector>
    static void addParameterizedFixtureTest(const std::string& filename, const std::string& name,
        ParamFunction&& test, const ParamVector& params) {
        size_t index = 0;
        for (const auto& param : params) {
            m_tests[filename].push_back([name, filename, test, param, index]() {
                using namespace detail;
                m_currentTest = name + "[" + std::to_string(index) + "]";
                std::cout << "  " << m_currentTest << "... ";
                try {
                    {
                        FixtureType fixture;
                        test(fixture, param);
                    }
                    std::cout << __GREEN__ << "PASSED\n" << __RESET__;
                    m_passedTests++;
                }
                catch (const std::exception& e) {
                    std::cout << __RED__ << "FAILED" << __RESET__ 
                    << " with exception: " << __YELLOW__ << e.what() 
                    << __RESET__ << "\n";
                    m_failedTests++;
                }
            });
            index++;
        }
    }

    template<typename FixtureType>
    static void addSharedFixtureTest(const std::string& filename, const std::string& fixtureName,
        const std::string& testName, std::function<void(FixtureType&)> test) {        
        m_tests[filename].push_back([filename, fixtureName, testName, test]() {
            using namespace detail;
            m_currentTest = testName;
            std::cout << "  " << testName << "... ";
            try {
                auto key = filename + ":" + fixtureName;

                auto& fixturePtr = m_sharedFixtures[key];
                
                if (!fixturePtr) {
                    fixturePtr = std::make_shared<FixtureType>();
                }
                
                test(*std::static_pointer_cast<FixtureType>(fixturePtr));
                
                std::cout << __GREEN__ << "PASSED\n" << __RESET__;
                m_passedTests++;
            }
            catch (const std::exception& e) {
                std::cout << __RED__ << "FAILED" << __RESET__ 
                << " with exception: " << __YELLOW__ << e.what() 
                << __RESET__ << "\n";
                m_failedTests++;
            }
            });
    }

    template<typename FixtureType, typename ParamFunction, typename ParamVector>
    static void addParameterizedSharedFixtureTest(const std::string& filename, const std::string& fixtureName,
        const std::string& testName, ParamFunction&& test, const ParamVector& params) {
        size_t index = 0;
        for (const auto& param : params) {
            m_tests[filename].push_back([filename, fixtureName, testName, test, param, index]() {
                using namespace detail;
                m_currentTest = testName + "[" + std::to_string(index) + "]";
                std::cout << "  " << m_currentTest << "... ";
                try {
                    auto key = filename + ":" + fixtureName;

                    auto& fixturePtr = m_sharedFixtures[key];

                    if (!fixturePtr) {
                        fixturePtr = std::make_shared<FixtureType>();
                    }

                    test(*std::static_pointer_cast<FixtureType>(fixturePtr), param);
                    
                    std::cout << __GREEN__ << "PASSED\n" << __RESET__;
                    m_passedTests++;
                }
                catch (const std::exception& e) {
                    std::cout << __RED__ << "FAILED" << __RESET__ 
                    << " with exception: " << __YELLOW__ << e.what() 
                    << __RESET__ << "\n";
                    m_failedTests++;
                }
            });
            index++;
        }
    }

    static void addBenchmark(const std::string& filename, const std::string& name,
        std::function<void(Benchmark&, size_t)> benchmarkFunc, size_t iterations = 1000) {
        auto& it = m_benchmarks[filename];
        size_t benchmarkIndex = it.size();

        it.emplace_back([name, benchmarkFunc, iterations, benchmarkIndex](Benchmark& benchmark) {
            using namespace detail;
        m_currentTest = name;
            std::cout << "  " << name << " (" << __CYAN__ << iterations << __RESET__ << " iterations)... ";

            for (size_t i = 0; i < iterations; ++i) {
                benchmarkFunc(benchmark, i);
            }

            benchmark.printStatistics();
            }, iterations);
    }

    template<typename FixtureType>
    static void addFixtureBenchmark(const std::string& filename, const std::string& name,
        std::function<void(FixtureType&, Benchmark&, size_t)> benchmarkFunc, size_t iterations = 1000) {
        auto& it = m_benchmarks[filename];
        
        it.emplace_back([name, benchmarkFunc, iterations](Benchmark& benchmark) {
            using namespace detail;
            m_currentTest = name;
            std::cout << "  " << name << " (" << __CYAN__ << iterations << __RESET__ << " iterations)... ";

            FixtureType fixture;
            for (size_t i = 0; i < iterations; ++i) {
                benchmarkFunc(fixture, benchmark, i);
            }

            benchmark.printStatistics();
            }, iterations);
    }

    template<typename ParamFunction, typename ParamVector>
    static void addParameterizedBenchmark(const std::string& filename, const std::string& name,
        ParamFunction&& benchmarkFunc, const ParamVector& params, size_t iterations = 1000) {
        size_t index = 0;
        for (const auto& param : params) {
            auto& it = m_benchmarks[filename];
            
            it.emplace_back([name, benchmarkFunc, param, iterations, index](Benchmark& benchmark) {
                using namespace detail;
                m_currentTest = name + "[" + std::to_string(index) + "]";
                std::cout << "  " << m_currentTest << " (" << __CYAN__ << iterations << __RESET__ << " iterations)... ";

                for (size_t i = 0; i < iterations; ++i) {
                    benchmarkFunc(param, benchmark, i);
                }

                benchmark.printStatistics();
                }, iterations);
            index++;
        }
    }
    
    static inline void startTiming(Benchmark& benchmark, size_t iteration) {
        benchmark.startTiming(iteration);
    }
    
    static inline void endTiming(Benchmark& benchmark, size_t iteration) {
        benchmark.endTiming(iteration);
    }

    static void runAll() {
        runAllTests();
        runAllBenchmarks();
    }

    static void runAllTests() {
        using namespace detail;
        m_passedTests = 0;
        m_failedTests = 0;
        int totalTests = 0;

        for (const auto& [file, tests] : m_tests) {
            totalTests += tests.size();
        }

        if (totalTests > 0) {
            std::cout << "Running " << __CYAN__ << totalTests << __RESET__ << " tests from " << __CYAN__ << m_tests.size() << __RESET__ << " files...\n\n";
            for (const auto& [filename, tests] : m_tests) {
                std::cout << __YELLOW__ << "[" << filename << "]\n" << __RESET__;
                for (auto& test : tests) {
                    test();
                }
                std::cout << "\n";
            }
            std::cout << "Results: " << __CYAN__ << m_passedTests << __RESET__ << " passed, " << __CYAN__ << m_failedTests << __RESET__ << " failed\n";
        }
    }

    static void runAllBenchmarks() {
        using namespace detail;
        int totalBenchmarks = 0;

        for (const auto& [file, benches] : m_benchmarks) {
            totalBenchmarks += benches.size();
        }

        if (totalBenchmarks > 0) {
            std::cout << "Running " << __CYAN__ << totalBenchmarks << __RESET__ << " benchmarks from " << __CYAN__ << m_benchmarks.size() << __RESET__ << " files...\n\n";

            for (auto& [filename, benches] : m_benchmarks) {
                std::cout << __YELLOW__ << "[" << filename << "]\n" << __RESET__;

                for (auto& bench : benches) {
                    bench.getBenchmark()(bench);
                }
                std::cout << "\n";
            }
        }
    }

    static void cleanup() {
        m_sharedFixtures.clear();
    }

    static void assert(bool condition, const std::string& message = "") {
        if (!condition) {
            throw std::runtime_error(message.empty() ? "Assertion failed" : message);
        }
    }

    template<typename T, typename U>
    static void assertEqual(const T& expected, const U& actual, const std::string& message = "") {
        if (expected != actual) {
            std::ostringstream oss;
            oss << "Expected: " << expected << ", Actual: " << actual;
            throw std::runtime_error(message.empty() ? oss.str() : message);
        }
    }

    template<typename T, typename U>
    static void assertNotEqual(const T& expected, const U& actual, const std::string& message = "") {
        if (expected == actual) {
            std::ostringstream oss;
            oss << "Expected " << expected << " != " << actual;
            throw std::runtime_error(message.empty() ? oss.str() : message);
        }
    }

    template<typename T, typename U>
    static void assertLessThan(const T& left, const U& right, const std::string& message = "") {
        if (left >= right) {
            std::ostringstream oss;
            oss << left << " >= " << right;
            throw std::runtime_error(message.empty() ? oss.str() : message);
        }
    }

    template<typename T, typename U>
    static void assertGreaterThan(const T& left, const U& right, const std::string& message = "") {
        if (left <= right) {
            std::ostringstream oss;
            oss << left << " <= " << right;
            throw std::runtime_error(message.empty() ? oss.str() : message);
        }
    }

    template<typename T>
    static void assertNear(const T& expected, const T& actual, const T& epsilon, const std::string& message = "") {
        if (std::abs(expected - actual) > epsilon) {
            std::ostringstream oss;
            oss << "Expected: " << expected << ", Actual: " << actual << ", Diff: " << std::abs(expected - actual) << " > " << epsilon;
            throw std::runtime_error(message.empty() ? oss.str() : message);
        }
    }

    template<typename T, typename U>
    static void assertLessEqual(const T& left, const U& right, const std::string& message = "") {
        if (left > right) {
            std::ostringstream oss;
            oss << left << " > " << right;
            throw std::runtime_error(message.empty() ? oss.str() : message);
        }
    }

    template<typename T, typename U>
    static void assertGreaterEqual(const T& left, const U& right, const std::string& message = "") {
        if (left < right) {
            std::ostringstream oss;
            oss << left << " < " << right;
            throw std::runtime_error(message.empty() ? oss.str() : message);
        }
    }

    template<typename T>
    static void assertNull(const T* ptr, const std::string& message = "") {
        if (ptr != nullptr) {
            throw std::runtime_error(message.empty() ?
                "Pointer is not null" : message);
        }
    }

    template<typename T>
    static void assertNotNull(const T* ptr, const std::string& message = "") {
        if (ptr == nullptr) {
            throw std::runtime_error(message.empty() ?
                "Pointer is null" : message);
        }
    }

    static void assertTrue(bool condition, const std::string& message = "") {
        if (!condition) {
            throw std::runtime_error(message.empty() ?
                "Expected true" : message);
        }
    }

    static void assertFalse(bool condition, const std::string& message = "") {
        if (condition) {
            throw std::runtime_error(message.empty() ?
                "Expected false" : message);
        }
    }

    template<typename Exception, typename Func>
    static void assertThrows(Func&& func, const std::string& message = "") {
        try {
            func();
            throw std::runtime_error(message.empty() ?
                "Expected exception was not thrown" : message);
        } catch (const Exception&) {
            // Expected exception caught
        }
    }

    template<typename Func>
    static void assertNoThrow(Func&& func, const std::string& message = "") {
        try {
            func();
        } catch (...) {
            throw std::runtime_error(message.empty() ?
                "Unexpected exception thrown" : message);
        }
    }
};

class BenchmarkRAII {
    Benchmark& m_benchmark;
    size_t m_iteration;
public:
    BenchmarkRAII(Benchmark& benchmark, size_t iteration) : m_benchmark(benchmark), m_iteration(iteration) {
        TestFramework::startTiming(benchmark, iteration);
    }
    ~BenchmarkRAII() {
        TestFramework::endTiming(m_benchmark, m_iteration);
    }
};

class BenchmarkScope {
    Benchmark& m_benchmark;
    size_t m_iteration;
public:

    BenchmarkScope(Benchmark& benchmark, size_t iteration) : m_benchmark(benchmark), m_iteration(iteration) {
    }

    template <typename Func>
    BenchmarkScope& operator=(Func&& benchmark) {
        try {
            TestFramework::startTiming(m_benchmark, m_iteration);
            benchmark();
            TestFramework::endTiming(m_benchmark, m_iteration);
        }
        catch (...) {
            TestFramework::endTiming(m_benchmark, m_iteration);
            throw;
        }
        return *this;
    }
};

#define __TEST__(name) \
    static void test_##name(); \
    static bool registered_##name = (TestFramework::addTest(std::filesystem::path(__FILE__).filename().string(), #name, test_##name), true); \
    static void test_##name()

#define __TEST_FIXTURE__(FixtureType, name) \
    static void test_fixture_##name(FixtureType& fixture); \
    static bool registered_fixture_##name = (TestFramework::addFixtureTest<FixtureType>(std::filesystem::path(__FILE__).filename().string(), #name, test_fixture_##name), true); \
    static void test_fixture_##name(FixtureType& fixture)

#define __TEST_SHARED_FIXTURE__(FixtureType, fixtureName, testName) \
    static void test_shared_##fixtureName##_##testName(FixtureType& fixture); \
    static bool registered_shared_##fixtureName##_##testName = (TestFramework::addSharedFixtureTest<FixtureType>(std::filesystem::path(__FILE__).filename().string(), #fixtureName, #testName, test_shared_##fixtureName##_##testName), true); \
    static void test_shared_##fixtureName##_##testName(FixtureType& fixture)

#define __TEST_PARAM__(name, ...) \
    static void test_param_auto_##name(const decltype(std::vector{__VA_ARGS__})::value_type& param); \
    static bool registered_param_auto_##name = (TestFramework::addParameterizedTest(std::filesystem::path(__FILE__).filename().string(), #name, test_param_auto_##name, std::vector{__VA_ARGS__}), true); \
    static void test_param_auto_##name(const decltype(std::vector{__VA_ARGS__})::value_type& param)

#define __TEST_FIXTURE_PARAM__(FixtureType, name, ...) \
    static void test_fixture_param_auto_##name(FixtureType& fixture, const decltype(std::vector{__VA_ARGS__})::value_type& param); \
    static bool registered_fixture_param_auto_##name = (TestFramework::addParameterizedFixtureTest<FixtureType>(std::filesystem::path(__FILE__).filename().string(), #name, test_fixture_param_auto_##name, std::vector{__VA_ARGS__}), true); \
    static void test_fixture_param_auto_##name(FixtureType& fixture, const decltype(std::vector{__VA_ARGS__})::value_type& param)

#define __TEST_SHARED_FIXTURE_PARAM__(FixtureType, fixtureName, testName, ...) \
    static void test_shared_param_auto_##fixtureName##_##testName(FixtureType& fixture, const decltype(std::vector{__VA_ARGS__})::value_type& param); \
    static bool registered_shared_param_auto_##fixtureName##_##testName = (TestFramework::addParameterizedSharedFixtureTest<FixtureType>(std::filesystem::path(__FILE__).filename().string(), #fixtureName, #testName, test_shared_param_auto_##fixtureName##_##testName, std::vector{__VA_ARGS__}), true); \
    static void test_shared_param_auto_##fixtureName##_##testName(FixtureType& fixture, const decltype(std::vector{__VA_ARGS__})::value_type& param)

#define __BENCHMARK__(name, ...) \
    static void benchmark_##name(Benchmark& __benchmark__, size_t __iteration__); \
    static bool registered_bench_##name = (TestFramework::addBenchmark(std::filesystem::path(__FILE__).filename().string(), #name, benchmark_##name, ##__VA_ARGS__), true); \
    static void benchmark_##name(Benchmark& __benchmark__, size_t __iteration__)

#define __BENCHMARK_FIXTURE__(FixtureType, name, ...) \
    static void benchmark_fixture_##name(FixtureType& fixture, Benchmark& __benchmark__, size_t __iteration__); \
    static bool registered_bench_fixture_##name = (TestFramework::addFixtureBenchmark<FixtureType>(std::filesystem::path(__FILE__).filename().string(), #name, benchmark_fixture_##name, ##__VA_ARGS__), true); \
    static void benchmark_fixture_##name(FixtureType& fixture, Benchmark& __benchmark__, size_t __iteration__)

#define __BENCHMARK_PARAM__(name, iterations, ...) \
    static void benchmark_param_##name(const decltype(std::vector{__VA_ARGS__})::value_type& param, Benchmark& __benchmark__, size_t __iteration__); \
    static bool registered_bench_param_##name = (TestFramework::addParameterizedBenchmark(std::filesystem::path(__FILE__).filename().string(), #name, benchmark_param_##name, std::vector{__VA_ARGS__}, iterations), true); \
    static void benchmark_param_##name(const decltype(std::vector{__VA_ARGS__})::value_type& param, Benchmark& __benchmark__, size_t __iteration__)

#define __BENCHMARK_START__() TestFramework::startTiming(__benchmark__, __iteration__)
#define __BENCHMARK_END__() TestFramework::endTiming(__benchmark__, __iteration__)
#define __BENCHMARK_RAII__() BenchmarkRAII __benchmark_RAII__(__benchmark__, __iteration__)
#define __BENCHMARK_SCOPE__() BenchmarkScope __benchmark_scope__(__benchmark__, __iteration__); \
    __benchmark_scope__= [&]() \

#define __ASSERT__(condition) TestFramework::assert(condition, #condition)
#define __ASSERT_EQ__(expected, actual) TestFramework::assertEqual(expected, actual)
#define __ASSERT_NE__(expected, actual) TestFramework::assertNotEqual(expected, actual)
#define __ASSERT_LT__(a, b) TestFramework::assertLessThan(a, b)
#define __ASSERT_GT__(a, b) TestFramework::assertGreaterThan(a, b)
#define __ASSERT_NEAR__(expected, actual, epsilon) TestFramework::assertNear(expected, actual, epsilon)
#define __ASSERT_LE__(a, b) TestFramework::assertLessEqual(a, b)
#define __ASSERT_GE__(a, b) TestFramework::assertGreaterEqual(a, b)
#define __ASSERT_NULL__(ptr) TestFramework::assertNull(ptr)
#define __ASSERT_NOT_NULL__(ptr) TestFramework::assertNotNull(ptr)
#define __ASSERT_TRUE__(condition) TestFramework::assertTrue(condition)
#define __ASSERT_FALSE__(condition) TestFramework::assertFalse(condition)
#define __ASSERT_THROWS__(exception, func) TestFramework::assertThrows<exception>(func)
#define __ASSERT_NO_THROW__(func) TestFramework::assertNoThrow(func)