# Tests directory - contains both unit and E2E tests

# Add unit tests
if(BUILD_TESTS)
    add_subdirectory(unit)
endif()

# Add E2E tests
if(BUILD_E2E_TESTS)
    add_subdirectory(e2e)
endif()
