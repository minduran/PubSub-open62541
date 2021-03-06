# PubSub-open62541
This project contains an implementation of a **basic console app** which allows the user to dynamically manipulate Connections, WriterGroups, DataSets and DataFields on runtime in a rudimental manner by only entering commands as inputs in the console. This work offers flexibility to creating and deleting of such nodes and may help understand the concept of the Publisher Implementation of [open62541](https://github.com/open62541/open62541) and its structure. This project uses the UDP Multicast Publisher example.

The [Class Publisher](https://github.com/minduran/PubSub-open62541/blob/main/PubSub/Publisher.h) implements some functions which are mainly derived from add functions found in [tutorial_pubsub_publisher.c](https://github.com/open62541/open62541/blob/master/examples/pubsub/tutorial_pubsub_publish.c) of open62541. There are also more functions implemented for convenience.


## Features
  - Manipulating publisher via console input commands:
    - Establishing/Removing Connections on a specific UDP port 
    - Adding/Removing WriterGroups to established connection
      - enabling/disabling publishing
      - updating interval of publishing
    - Adding/Removing DataSets to WriterGroups
      - a DataSet can emulate a String by holding a set of Fields of type Byte. Each field represents a character.
    - Adding/Removing DataFields to DataSets
      - one Field can hold Data of either type Byte, SByte, Float, Double, Int16, Int32, Int64, UInt16, UInt32, UInt64 or DateTime
      - changing the value of Data
  - Loading input commands from text file
  - Exporting commands from current setup into text file
  
  
## Compiling and Executing the PubSub console app
1. Get the PubSub folder from repository:
```shell
git clone https://github.com/minduran/PubSub-open62541
```
2. Compile: the easiest way to compile the app is achieved by using following command lines and it takes about 1 minute with the [static library of open62541](https://github.com/minduran/PubSub-open62541/blob/main/PubSub/library/libopen62541.a):
```shell
cd PubSub-open62541
g++ -o pubsub_publisher_console  PubSub/BranchConnection.cpp PubSub/BranchDataSet.cpp PubSub/BranchField.cpp PubSub/BranchWriterGroup.cpp PubSub/InputArguments.cpp PubSub/Publisher.cpp PubSub/TreeTrunk.cpp PubSub/UserInput.cpp PubSub/main.cpp  -LPubSub/library/ -lopen62541 -lpthread
```
3. Execute:
```shell
./pubsub_publisher_console    # Note: Default TCP server port is 4840

# To use different TCP server port:
./pubsub_publisher_console -sp <port>
```

#### Note
There is already an executable of the app [PubSub](https://github.com/minduran/PubSub-open62541/tree/main/PubSub/Debug) located in directory *PubSub-open62541/PubSub/Debug*. This might just require permission to execute, for instance with `chmod +x PubSub`


## Using the Publisher console app
Once the console app is started you can enter ***help***. This will list all possible commands. Or take a look in [pubsub.txt](https://github.com/minduran/PubSub-open62541/blob/main/PubSub/Debug/pubsub.txt) which contains some random commands. You can load that text file containing commands with command ***load <file location>/pubsub.txt***.


## Subscribing
There is an executable [subscribe_standalone](https://github.com/minduran/PubSub-open62541/blob/main/PubSub/cmake-build-debug/subscribe_standalone) provided in directory *PubSub_open62541/PubSub/cmake-build-debug*. It might need permission to execute, this can be achieved with: `chmod +x subscribe_standalone`. Alternatively you can compile the subscriber by yourself and execute it with: 
```shell
cd PubSub_open62541

# Compiling:
g++ -o subscribe_standalone PubSub/Subscribe_standalone.cpp  -IPubSub/includes -LPubSub/library -lopen62541

# Executing:
./subcribe_standalone   # Subscribes UDP port 4840 by default

# To subscribe different UDP port:
./subscribe_standalone -cp <port>
```

#### Note
This is an extended implementation of [pubsub_subscribe_standalone.c](https://github.com/open62541/open62541/blob/master/examples/pubsub/pubsub_subscribe_standalone.c) from open62541

## Side Notes
- The structure of the Publisher Implementation of open62541 resembles a tree with Connections, WriterGroups, DataSets and DataField as branch forks.

- The Publisher and Subscriber of open65241 requires access to internet, since the connection profile is available only online.

- The [PubSub console app](https://github.com/minduran/PubSub-open62541/tree/main/PubSub/Debug/PubSub) is devoloped on Ubuntu 20.04.1 LTS with Eclipse IDE 2020-09, GNU Make 4.2.1, g++ 9.3.0, gcc 9.3.0

- The [PubSub](https://github.com/minduran/PubSub-open62541/tree/main/PubSub) folder is a C++ project in Eclipse IDE. In addition, with the provided [CMakeLists](https://github.com/minduran/PubSub-open62541/tree/main/PubSub/CMakeLists.txt) file it should be possible to open it as project in CLion.
  - After opening the project in your Eclipse, most likely, you need to adjust the setting in the properties in order to be able to build the project:
      1.  right click on *Project -> Properties -> C/C++ Build* and check *Generate Makefiles automatically*
      2.  If open62541 library is not installed on your System, right click on *Project -> Properties -> C/C++ Build -> Settings -> Tool Settings -> Cross G++ Linker -> Libraries* and add *Library search path*: click *Workspace...* and select *PubSub/library*
      3.  *clean project* and *build project* should work now

- How to build the open62541 static library for [Publish/Subscribe](https://open62541.org/doc/current/pubsub.html#pubsub), [source](https://open62541.org/doc/current/building.html):
```shell
# Installing necessary tools:
sudo apt-get install git build-essential gcc pkg-config cmake python

# Get open62541 and its submodules from repository:
git clone https://github.com/open62541/open62541.git
cd open62541
git submodule update --init --recursive

# Creating directory "build" inside open62541 and move inside "build":
mkdir build && cd build

# Initializing cmake with following flags:
cmake -DBUILD_SHARED_LIBS=OFF -DUA_BUILD_EXAMPLES=ON DCMAKE_BUILD_TYPE=Debug -DUA_ENABLE_PUBSUB=ON -DUA_ENABLE_PUBSUB_ETH_UADP=ON -DUA_ENABLE_PUBSUB_INFORMATIONMODEL=ON -DUA_NAMESPACE_ZERO=FULL ..

# Building:
make
# Creates library files in open62541/build/bin/
#
# Note: with the flag -DBUILD_SHARED_LIBS=ON it creates dynamic libraries.
# Compiling Publisher with dynamic library is much faster than with static library.
# However, dynamic library does not support Subscriber, thus it will fail trying to compile Subscriber with dynamic library.
```
