cmake_minimum_required(VERSION 3.17)
project(PubSub)

set(CMAKE_CXX_STANDARD 14)

add_executable(pubsub_by_cmake main.cpp Publisher.cpp InputArguments.cpp UserInput.cpp TreeTrunk.cpp BranchConnection.cpp BranchWriterGroup.cpp BranchDataSet.cpp BranchField.cpp)
target_link_libraries(pubsub_by_cmake LINK_PUBLIC open62541 pthread)

include_directories(${PROJECT_SOURCE_DIR}/includes)
link_libraries(${PROJECT_SOURCE_DIR}/library)
add_executable(subscribe_standalone Subscribe_standalone.cpp)
target_link_libraries(subscribe_standalone LINK_PUBLIC open62541.a)