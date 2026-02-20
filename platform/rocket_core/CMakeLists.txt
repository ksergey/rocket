set(TargetName rocket_core)

option(ROCKET_LOGGER_TSC_CLOCK "Use TSC clock instead of clock_gettime" ON)

add_library(${TargetName} STATIC)
target_compile_features(${TargetName}
  PUBLIC cxx_std_23)
set_target_properties(${TargetName}
  PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF)
target_compile_options(${TargetName}
  PUBLIC -Wall -Wextra -Wattributes -Wpedantic -Wstrict-aliasing -Wcast-align -g -fno-rtti)
target_include_directories(${TargetName}
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${TargetName}
  PRIVATE fmt::fmt)

if (ROCKET_PYTHON)
  set_property(TARGET ${TargetName}
    PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()

if (ROCKET_SANITIZER)
  target_compile_options(${TargetName}
    PUBLIC -fsanitize=address,leak,undefined)
  target_link_options(${TargetName}
    PUBLIC -fsanitize=address,leak,undefined)
endif()

if (ROCKET_LOGGER_TSC_CLOCK)
  target_compile_definitions(${TargetName}
    PUBLIC -DROCKET_LOGGER_TSC_CLOCK)
endif()

file(GLOB_RECURSE Sources "${CMAKE_CURRENT_SOURCE_DIR}/rocket/*.cpp")
file(GLOB_RECURSE Headers "${CMAKE_CURRENT_SOURCE_DIR}/rocket/*.h")

RocketAddTestsFromSourceList(Sources
  PREFIX ${TargetName}
  COMPILE_OPTIONS -Wall -Wextra -g
  LIBS doctest::doctest_with_main ${TargetName})

RocketAddBenchmarksFromSourceList(Sources
  PREFIX ${TargetName}
  COMPILE_OPTIONS -Wall -Wextra -g
  LIBS benchmark::benchmark_main ${TargetName})

RocketExcludeTestsAndBenchmarksFromSourceList(Sources)

foreach(file ${Headers})
  target_sources(${TargetName} PUBLIC ${file})
endforeach()

foreach(file ${Sources})
  target_sources(${TargetName} PRIVATE ${file})
endforeach()

add_library(rocket::core ALIAS ${TargetName})
