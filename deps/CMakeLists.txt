include(FetchContent)

if (NOT TARGET doctest::doctest_with_main)
  FetchContent_Declare(doctest
    URL https://github.com/doctest/doctest/archive/refs/tags/v2.4.12.tar.gz
    EXCLUDE_FROM_ALL
    SYSTEM
    OVERRIDE_FIND_PACKAGE
  )
  FetchContent_MakeAvailable(doctest)
endif()


if (NOT TARGET benchmark::benchmark_main)
  FetchContent_Declare(benchmark
    URL https://github.com/google/benchmark/archive/refs/tags/v1.8.4.tar.gz
    EXCLUDE_FROM_ALL
    SYSTEM
    OVERRIDE_FIND_PACKAGE
  )
  set(BENCHMARK_ENABLE_GTEST_TESTS FALSE)
  FetchContent_MakeAvailable(benchmark)
endif()
