PROJECT(tspell)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6.0)

SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# depends
FIND_PACKAGE(ICU REQUIRED)

# definitions
INCLUDE_DIRECTORIES(${ICU_INCLUDE_DIR} ${PROJECT_SOURCE_DIR}/lib)

ADD_DEFINITIONS(-Wall -Wextra)

# utilities
ADD_EXECUTABLE(tspell utils/tspell.cc)
TARGET_LINK_LIBRARIES(tspell ${ICU_LIBRARY})

# install
INSTALL(FILES lib/tspell/*.hh DESTINATION include/tspell)
INSTALL(TARGETS tspell DESTINATION bin)

# tests
ADD_EXECUTABLE(stringtrie_test tests/stringtrie_test.cc)
ADD_EXECUTABLE(unitrie_test tests/unitrie_test.cc)
TARGET_LINK_LIBRARIES(unitrie_test ${ICU_LIBRARY})

ENABLE_TESTING()
ADD_TEST(stringtrie_test stringtrie_test)
ADD_TEST(unitrie_test unitrie_test)
