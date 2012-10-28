cmake_minimum_required(VERSION 2.6)
PROJECT(boost-asio-examples)

# Usage: 
#   cmake .. -DCMAKE_INCLUDE_PATH=/home/ott/exp/include -DCMAKE_LIBRARY_PATH=/home/ott/exp/lib
#       -DBOOST_INCLUDEDIR=/home/ott/exp/include -DBOOST_LIBRARYDIR=/home/ott/exp/lib
#

SET(Boost_USE_STATIC_LIBS OFF)
SET(Boost_USE_MULTITHREAD ON)
FIND_PACKAGE(Boost 1.42.0 REQUIRED COMPONENTS system thread regex)
IF(Boost_FOUND)
  INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
  LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})
ENDIF(Boost_FOUND)

SET(USED_LIBS ${Boost_SYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY} ${Boost_REGEX_LIBRARY})

ADD_EXECUTABLE(test-otpc test-otpc.cpp test-otpc-conn.cpp)
TARGET_LINK_LIBRARIES(test-otpc ${USED_LIBS})

ADD_EXECUTABLE(test-mcmt test-mcmt.cpp)
TARGET_LINK_LIBRARIES(test-mcmt ${USED_LIBS})

#ADD_EXECUTABLE(test-otpc-tp test-otpc-tp.cpp test-otpc-conn.cpp)
#TARGET_LINK_LIBRARIES(test-otpc-tp ${USED_LIBS})

