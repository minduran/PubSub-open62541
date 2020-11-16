/*
 * OpenPublisherDemo.h
 *
 *  Created on: Nov 1, 2020
 *      Author: minh
 */

#ifndef PUBLISHER_H_
#define PUBLISHER_H_

#include <open62541/plugin/log_stdout.h>
#include <open62541/plugin/pubsub_ethernet.h>
#include <open62541/plugin/pubsub_udp.h>
#include <open62541/server.h>
#include <open62541/server_config_default.h>

#include <signal.h>

#include "Defines_and_Structures.h"

/* For using publishDataSetDemo */
#define INT_DATATSET_DEMO
//#define MIXED_DATATSET_DEMO
//#define DOUBLE_DATATSET_DEMO


class Publisher {
private:
	InputData *data;

	UA_Boolean isServerConfigured;
	UA_Duration interval;

	static UA_Boolean running;

public:
	UA_Server *server;

	Publisher(InputData *data);

	virtual ~Publisher();

	void initServerConfig(UA_UInt16 serverPort = 4840);

	void removePubSubConnection(const UA_NodeId &connection);
	void removeWriterGroup(const UA_NodeId &writerGroup);
	/* Warning removing DataSet also removes WriterGroup connected by DataSetWriter */
	void removeDataSet(const UA_NodeId &dataSet);
	void removeDataSetWriter(const UA_NodeId &dataSetWriter);
	void removeDataSetField(const UA_NodeId &dataSetField);

	void setChannelPort(UA_UInt16 port);

	UA_NodeId addPubSubConnection(UA_Int32 publisherId = 2234);
	UA_NodeId addWriterGroup(UA_Duration publishInterval, UA_NodeId &connectionIdent, UA_Int16 writerGroupId = 100);
	UA_NodeId addDataSet();
	UA_NodeId addDataSetWriter(const UA_NodeId &dataSet, const UA_NodeId &writerGroup, UA_Int16  dataSetWriterId = 62541);
	UA_NodeId addDataSetField(const UA_NodeId &dataSet, const UA_NodeId &varNode, char *name = NULL);


	/* addVariable creates a new Variable Node
	 * and returns a UA_NodeId which is used as token to identify the Variable Node */
	UA_NodeId addVariable(const UA_DataType *variableType);


	UA_NodeId addDateTimeToDataSet(const UA_NodeId &dataSet);


	/* addVariableNode creates a Variable Node
	 * @param UA_NodeId variableNodeId is used as token to identify the new Variable Node */
	void addVariableNode(const UA_NodeId &varNodeId, const UA_DataType *variableType);


	void writeVariable(void *value, const UA_NodeId &varNodeId, const UA_DataType *variableType);



	/* addVarToDataset creates Variable Node of specified DataType
	 * and adds created Variable Node to specified DataSet by creating a DataSetField in the DataSet
	 * does not return DataSetField
	 * returns the created Variable Node */
	UA_NodeId addVarToDataset(const UA_NodeId &dataSet, const UA_DataType *variableType);

	/* addVarToOwnDatasetByReference takes a reference of a Variable Node
	 * and add the Variable Node to a new DataSet
	 * NOTE: It does not create a new Variable Node.*/
	void addVarWithOwnDatasetByReference(const UA_DataType *variableType, const UA_NodeId &writerGroup, const UA_NodeId &varNodeId);

	/* addVarToOwnDatasetByReference takes a Pointer of a Variable Node
	 * creates a new Variable Node and adds it to a new DataSet
	 * and writes created Variable Node to the Variable Node of the Pointer*/
	void addVarWithOwnDatasetByPointer(const UA_DataType *variableType, const UA_NodeId &writerGroup, UA_NodeId *varNodeId);

	/* creates a new Variable Node and adds it to a new DataSet
	 * returns the new Variable Node */
	UA_NodeId addVarWithOwnDataset(const UA_DataType *variableType, const UA_NodeId &writerGroup);


	void disablePublishing(UA_NodeId *writerGroup);
	void enablePublishing(UA_NodeId *writerGroup);

	void setUrl(char *url);

	int run();

	static bool isRunning();
	static void stopRunning();
	static void resetRunning();

	static void stopHandler(int sign);

	/*
	 * Demonstration Functions
	 */
	void publishDateTime(UA_UInt16 interval, UA_NodeId &writerGroup, UA_NodeId &connId);
	void publishOnDifferentChannelDemo();
	void publishDataSetsDemo();
	void publishHelloWorld(UA_NodeId &conn);
};

#endif /* PUBLISHER_H_ */
