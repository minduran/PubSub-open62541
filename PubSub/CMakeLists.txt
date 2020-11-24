cmake_minimum_required(VERSION 3.17)
project(PubSub)

set(CMAKE_CXX_STANDARD 14)


# Build Console App pubsub using installed open62541 library:
add_executable(pubsub main.cpp Publisher.cpp InputArguments.cpp UserInput.cpp TreeTrunk.cpp BranchConnection.cpp BranchWriterGroup.cpp BranchDataSet.cpp BranchField.cpp)
target_link_libraries(pubsub LINK_PUBLIC open62541 pthread)

# Build Console App pubsub using static library of open62541 provided in directory 'library':
link_libraries(${PROJECT_SOURCE_DIR}/library)
add_executable(pubsub_with_static_library main.cpp Publisher.cpp InputArguments.cpp UserInput.cpp TreeTrunk.cpp BranchConnection.cpp BranchWriterGroup.cpp BranchDataSet.cpp BranchField.cpp)
target_link_libraries(pubsub_with_static_library LINK_PUBLIC open62541.a pthread)

# Build Subscriber:
include_directories(${PROJECT_SOURCE_DIR}/includes)
link_libraries(${PROJECT_SOURCE_DIR}/library)
add_executable(subscribe_standalone Subscribe_standalone.cpp)
target_link_libraries(subscribe_standalone LINK_PUBLIC open62541.a)