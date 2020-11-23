# PubSub-open62541
This project contains an implementation of a **basic console app** allowing user to dynamically manipulate on runtime Connections, WriterGroups, DataSets and DataFields by inputs in the console. This work may help understand the concept of the Publisher Implementation of open62541.

The class Publisher implements functions which are mainly derived from add functions found in [tutorial_pubsub_publisher.c](https://github.com/open62541/open62541/blob/master/examples/pubsub/tutorial_pubsub_publish.c)

## Features
  - Manipulating publisher via console input:
    - Establishing/Removing Connections on a specific udp port 
    - Adding/Removing WriterGroups to established connection
      - enabling/disabling publishing
      - updating interval of publishing
    - Adding/Removing DataSets to WriterGroups
      - a DataSet can emulate a String by holding a set of Fields of Type Byte
    - Adding/Removing DataFields to DataSets
      - one Field can hold Data of either type Byte, SByte, Float, Double, Int16, Int32, Int64, UInt16, UInt32, UInt64 or DateTime
      - setting value of Data
