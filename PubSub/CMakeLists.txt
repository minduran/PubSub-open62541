cmake_minimum_required(VERSION 3.17)
project(PubSub)

set(CMAKE_CXX_STANDARD 14)

add_executable(pubsub_by_cmake main.cpp Publisher.cpp InputArguments.cpp UserInput.cpp TreeTrunk.cpp BranchConnection.cpp BranchWriterGroup.cpp BranchDataSet.cpp BranchField.cpp)
target_link_libraries(pubsub_by_cmake LINK_PUBLIC open62541 pthread)