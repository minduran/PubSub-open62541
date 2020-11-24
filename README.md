# PubSub-open62541
This project contains an implementation of a **basic console app** which allows user to dynamically manipulate Connections, WriterGroups, DataSets and DataFields on runtime in a rudimental manner by only entering inputs in the console. This work may help understand the concept of the Publisher Implementation of [open62541](https://github.com/open62541/open62541) and its structure. This project uses the UDP Multicast Publisher example.

The [Class Publisher](https://github.com/minduran/PubSub-open62541/blob/main/PubSub/Publisher.h) implements some functions which are mainly derived from add functions found in [tutorial_pubsub_publisher.c](https://github.com/open62541/open62541/blob/master/examples/pubsub/tutorial_pubsub_publish.c). There are also more functions implemented for convenience.

## Features
  - Manipulating publisher via console input:
    - Establishing/Removing Connections on a specific UDP port 
    - Adding/Removing WriterGroups to established connection
      - enabling/disabling publishing
      - updating interval of publishing
    - Adding/Removing DataSets to WriterGroups
      - a DataSet can emulate a String by holding a set of Fields of type Byte. Each field represents a character.
    - Adding/Removing DataFields to DataSets
      - one Field can hold Data of either type Byte, SByte, Float, Double, Int16, Int32, Int64, UInt16, UInt32, UInt64 or DateTime
      - setting value of Data
  - Loading input commands from text file

### Side Notes
- The structure of the Publisher Implementation of open62541 resembles a tree with Connections, WriterGroups, DataSets and DataField as branch forks.
- The [PubSub](https://github.com/minduran/PubSub-open62541/tree/main/PubSub) folder is a C++ project in Eclipse. However, with the provided [CMakeLists](https://github.com/minduran/PubSub-open62541/tree/main/PubSub/CMakeLists.txt) file it should possible to open as project in CLion aswell.
- The [PubSub console app](https://github.com/minduran/PubSub-open62541/tree/main/PubSub/Debug/PubSub) is devoloped on Ubuntu 20.04.1 LTS with Eclipse IDE 2020-09, GNU Make 4.2.1, g++ 9.3.0, gcc 9.3.0
- The [Subscribe_standalone](https://github.com/minduran/PubSub-open62541/tree/main/PubSub/cmake-build-debug/subscribe_standalone) can decode and print the published data into console.
