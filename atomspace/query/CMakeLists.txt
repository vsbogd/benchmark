# Build file for the atomspace pattern matching benchmarks

IF (HAVE_GUILE)
    ADD_EXECUTABLE (query_benchmark
        query_benchmark.cc
        )

    TARGET_LINK_LIBRARIES (query_benchmark
        ${ATOMSPACE_LIBRARIES}
        ${COGUTIL_LIBRARY}
        )

IF (benchmark_FOUND)
        ADD_EXECUTABLE (query_google_benchmark
            query_google_benchmark.cc
            )

        TARGET_LINK_LIBRARIES (query_google_benchmark
            ${ATOMSPACE_LIBRARIES}
            ${COGUTIL_LIBRARY}
            benchmark::benchmark
            benchmark::benchmark_main
            )
ENDIF(benchmark_FOUND)

    ADD_CUSTOM_TARGET(run_query_benchmark
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND ${CMAKE_CURRENT_BINARY_DIR}/query_benchmark -c query_benchmark.conf
        )
ENDIF (HAVE_GUILE)
