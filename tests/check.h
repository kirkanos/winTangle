// Minimal test harness. Deliberately free of external dependencies so the
// geometry tests build everywhere, including on the Mac this is developed on.
#pragma once

#include <cstdio>
#include <functional>
#include <string>
#include <vector>

namespace check {

struct TestCase {
    std::string name;
    std::function<void()> fn;
};

inline std::vector<TestCase>& Registry() {
    static std::vector<TestCase> tests;
    return tests;
}

inline int& Failures() {
    static int failures = 0;
    return failures;
}

inline std::string& CurrentTest() {
    static std::string name;
    return name;
}

struct Registrar {
    Registrar(const char* name, std::function<void()> fn) {
        Registry().push_back({name, std::move(fn)});
    }
};

inline void Fail(const char* file, int line, const std::string& what) {
    ++Failures();
    std::printf("  FAIL %s:%d  %s\n", file, line, what.c_str());
}

inline int RunAll() {
    for (auto& t : Registry()) {
        CurrentTest() = t.name;
        const int before = Failures();
        t.fn();
        std::printf("%s %s\n", Failures() == before ? "ok  " : "FAIL", t.name.c_str());
    }
    std::printf("\n%zu tests, %d failures\n", Registry().size(), Failures());
    return Failures() == 0 ? 0 : 1;
}

}  // namespace check

#define TEST(name)                                                       \
    static void name();                                                  \
    static ::check::Registrar name##_registrar(#name, name);             \
    static void name()

#define CHECK(cond)                                                      \
    do {                                                                 \
        if (!(cond)) ::check::Fail(__FILE__, __LINE__, #cond);           \
    } while (0)

#define CHECK_EQ(a, b)                                                   \
    do {                                                                 \
        const auto& _a = (a);                                            \
        const auto& _b = (b);                                            \
        if (!(_a == _b))                                                 \
            ::check::Fail(__FILE__, __LINE__,                            \
                          std::string(#a) + " != " + std::string(#b));   \
    } while (0)
