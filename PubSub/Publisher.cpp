/*
 * OpenPublisherDemo.cpp
 *
 *  Created on: Nov 1, 2020
 *      Author: minh
 */

#include "Publisher.h"

#include <iostream>

using namespace std;

UA_Boolean Publisher::running = true;

Publisher::Publisher(InputData *data) : data(data) {
		this->initServerConfig(data->serverPort);
}

Publisher::~Publisher() {
}


void Publisher::initServerConfig(UA_UInt16 serverPort) {
	server = UA_Server_new();
	UA_ServerConfig *config = UA_Server_getConfig(server);
	UA_ServerConfig_setMinimal(config, serverPort, NULL);

	 /* Details about the connection configuration and handling are located in
	  * the pubsub connection tutorial */
	config->pubsubTransportLayers =
		(UA_PubSubTransportLayer *) UA_calloc(2, sizeof(UA_PubSubTransportLayer));
	if(!config->pubsubTransportLayers) {
		UA_Server_delete(server);
		this->isServerConfigured = false;
	}
	    config->pubsubTransportLayers[0] = UA_PubSubTransportLayerUDPMP();
	    config->pubsubTransportLayersSize++;

	#ifdef UA_ENABLE_PUBSUB_ETH_UADP
	    config->pubsubTransportLayers[1] = UA_PubSubTransportLayerEthernet();
	    config->pubsubTransportLayersSize++;
	#endif

	this->isServerConfigured = true;
}

UA_NodeId Publisher::addPubSubConnection(UA_Int32 publisherId) {
	/* Details about the connection configuration and handling are located
	 * in the pubsub connection tutorial */
	UA_PubSubConnectionConfig connectionConfig;
	memset(&connectionConfig, 0, sizeof(connectionConfig));
	char name[] = {"UADP Connection 1"};
	connectionConfig.name = UA_STRING(name);
	connectionConfig.transportProfileUri = data->transportProfile;
	connectionConfig.enabled = UA_TRUE;
	UA_Variant_setScalar(&connectionConfig.address, &data->networkAddressUrl,
							 &UA_TYPES[UA_TYPES_NETWORKADDRESSURLDATATYPE]);
	/* Changed to static publisherId from random generation to identify
	 * the publisher on Subscriber side */
	connectionConfig.publisherId.numeric = publisherId;

	UA_NodeId connection;
	if (UA_Server_addPubSubConnection(server, &connectionConfig, &connection)
			== UA_STATUSCODE_GOOD) {
	    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,"\ton " UA_PRINTF_STRING_FORMAT, UA_PRINTF_STRING_DATA(this->data->networkAddressUrl.url));
	} else {
	    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,"ADD PUBSUB CHANNEL ON " UA_PRINTF_STRING_FORMAT " FAILED!", UA_PRINTF_STRING_DATA(this->data->networkAddressUrl.url));
	}
	return connection;
}

UA_NodeId Publisher::addWriterGroup(UA_Duration publishInterval, UA_NodeId &connection, UA_Int16 writerGroupId) {
	/* Now we create a new WriterGroupConfig and add the group to the existing
	 * PubSubConnection. */
	UA_WriterGroupConfig writerGroupConfig;
	memset(&writerGroupConfig, 0, sizeof(UA_WriterGroupConfig));
	char name[] = "Demo WriterGroup";
	writerGroupConfig.name = UA_STRING(name);
	writerGroupConfig.enabled = UA_FALSE;
	writerGroupConfig.encodingMimeType = UA_PUBSUB_ENCODING_UADP;
	writerGroupConfig.messageSettings.encoding = UA_EXTENSIONOBJECT_DECODED;
	writerGroupConfig.messageSettings.content.decoded.type = &UA_TYPES[UA_TYPES_UADPWRITERGROUPMESSAGEDATATYPE];
	writerGroupConfig.publishingInterval = publishInterval;
	writerGroupConfig.writerGroupId = writerGroupId;

	/* The configuration flags for the messages are encapsulated inside the
	 * message- and transport settings extension objects. These extension
	 * objects are defined by the standard. e.g.
	 * UadpWriterGroupMessageDataType */
	UA_UadpWriterGroupMessageDataType *writerGroupMessage  = UA_UadpWriterGroupMessageDataType_new();
	/* Change message settings of writerGroup to send PublisherId,
	 * WriterGroupId in GroupHeader and DataSetWriterId in PayloadHeader
	 * of NetworkMessage */
	writerGroupMessage->networkMessageContentMask = (UA_UadpNetworkMessageContentMask)(UA_UADPNETWORKMESSAGECONTENTMASK_PUBLISHERID |
													(UA_UadpNetworkMessageContentMask)UA_UADPNETWORKMESSAGECONTENTMASK_GROUPHEADER |
													(UA_UadpNetworkMessageContentMask)UA_UADPNETWORKMESSAGECONTENTMASK_WRITERGROUPID |
													(UA_UadpNetworkMessageContentMask)UA_UADPNETWORKMESSAGECONTENTMASK_PAYLOADHEADER);
	writerGroupConfig.messageSettings.content.decoded.data = writerGroupMessage;

	UA_NodeId writerGroup;
	UA_Server_addWriterGroup(server, connection, &writerGroupConfig, &writerGroup);
	UA_Server_setWriterGroupOperational(server, writerGroup);

	UA_UadpWriterGroupMessageDataType_delete(writerGroupMessage);

	return writerGroup;
}

UA_NodeId Publisher::addDataSetWriter(const UA_NodeId &dataSet, const UA_NodeId &writerGroup, UA_Int16 dataSetWriterId) {
	/* We need now a DataSetWriter within the WriterGroup. This means we must
	 * create a new DataSetWriterConfig and add call the addWriterGroup function. */
	UA_DataSetWriterConfig dataSetWriterConfig;
	memset(&dataSetWriterConfig, 0, sizeof(UA_DataSetWriterConfig));
	char name[] = {"Demo DataSetWriter"};
	dataSetWriterConfig.name = UA_STRING(name);
	dataSetWriterConfig.keyFrameCount = 10;

	UA_NodeId dataSetWriter;
	dataSetWriterConfig.dataSetWriterId = dataSetWriterId;
	UA_Server_addDataSetWriter(server, writerGroup, dataSet,
	                               &dataSetWriterConfig, &dataSetWriter);
	return dataSetWriter;

}


UA_NodeId Publisher::addDataSet() {
	/* The PublishedDataSetConfig contains all necessary public
	 * informations for the creation of a new PublishedDataSet */
	UA_PublishedDataSetConfig dataSetConfig;
	memset(&dataSetConfig, 0, sizeof(UA_PublishedDataSetConfig));
	dataSetConfig.publishedDataSetType = UA_PUBSUB_DATASET_PUBLISHEDITEMS;
	char name[] = {"Demo PDS"};
	dataSetConfig.name = UA_STRING(name);

	UA_NodeId dataSet;
	UA_Server_addPublishedDataSet(server, &dataSetConfig, &dataSet);
	return dataSet;
}

UA_NodeId Publisher::addDataSetField(const UA_NodeId &dataSet, const UA_NodeId &varNode, char *name) {

	UA_DataSetFieldConfig dataSetFieldConfig;
	memset(&dataSetFieldConfig, 0, sizeof(UA_DataSetFieldConfig));
	dataSetFieldConfig.dataSetFieldType = UA_PUBSUB_DATASETFIELD_VARIABLE;
	dataSetFieldConfig.field.variable.promotedField = UA_FALSE;
	dataSetFieldConfig.field.variable.publishParameters.attributeId = UA_ATTRIBUTEID_VALUE;
    dataSetFieldConfig.field.variable.fieldNameAlias = UA_STRING(name);
	dataSetFieldConfig.field.variable.publishParameters.publishedVariable =	varNode;

	UA_NodeId dataSetField;
	UA_Server_addDataSetField(server, dataSet,
							  &dataSetFieldConfig, &dataSetField);
	return dataSetField;

}

void Publisher::addVariableNode(const UA_NodeId &variableNodeId, const UA_DataType *variableType) {
	UA_VariableAttributes publisherAttr = UA_VariableAttributes_default;
	char name[] = {"Variable Node"};
	char text[] = {"en-US"};
	publisherAttr.displayName = UA_LOCALIZEDTEXT(text, name);
	publisherAttr.dataType = variableType->typeId;
	publisherAttr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;

	void* value;
	UA_Byte byteValue = 0;
	UA_SByte sbyteValue = 0;
	UA_Int16 int16Value = 0;
	UA_Int32 int32Value = 0;
	UA_Int64 int64Value = 0;
	UA_UInt16 uint16Value = 0;
	UA_UInt32 uint32Value = 0;
	UA_UInt64 uint64Value = 0;
	UA_Double doubleValue = 0;
	UA_Float floatValue = 0;
	UA_Boolean booleanValue = false;

	if (variableType == &UA_TYPES[UA_TYPES_BYTE]) {
		value = &byteValue;
	}
	else if (variableType == &UA_TYPES[UA_TYPES_SBYTE]) {
		value = &sbyteValue;
	}
	else if (variableType == &UA_TYPES[UA_TYPES_INT16]) {
		value = &int16Value;
	}
	else if (variableType == &UA_TYPES[UA_TYPES_INT32]) {
		value = &int32Value;
	}
	else if (variableType == &UA_TYPES[UA_TYPES_INT64]) {
		value = &int64Value;
	}
	else if (variableType == &UA_TYPES[UA_TYPES_UINT16]) {
		value = &uint16Value;
	}
	else if (variableType == &UA_TYPES[UA_TYPES_UINT32]) {
		value = &uint32Value;
	}
	else if (variableType == &UA_TYPES[UA_TYPES_UINT64]) {
		value = &uint64Value;
	}
	else if (variableType == &UA_TYPES[UA_TYPES_FLOAT]) {
		value = &floatValue;
	}
	else if (variableType == &UA_TYPES[UA_TYPES_DOUBLE]) {
		value = &doubleValue;
	}
	else if (variableType == &UA_TYPES[UA_TYPES_BOOLEAN]) {
		value = &booleanValue;
	}
	else {
		value = &int32Value;
	}

	UA_Variant_setScalar(&publisherAttr.value, value, variableType);
	UA_Server_addVariableNode(server, variableNodeId,
								  UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
								  UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
								  UA_QUALIFIEDNAME(1, name),
								  UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
								  publisherAttr, NULL, NULL);
}


UA_NodeId Publisher::addVariable(const UA_DataType *variableType) {
	UA_VariableAttributes publisherAttr = UA_VariableAttributes_default;
	char name[] = {"Variable Node"};
	char text[] = {"en-US"};
	publisherAttr.displayName = UA_LOCALIZEDTEXT(text, name);
	publisherAttr.dataType = variableType->typeId;
	publisherAttr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;

	void* value;
	UA_Byte byteValue = 0;
	UA_SByte sbyteValue = 0;
	UA_Int16 int16Value = 0;
	UA_Int32 int32Value = 0;
	UA_Int64 int64Value = 0;
	UA_UInt16 uint16Value = 0;
	UA_UInt32 uint32Value = 0;
	UA_UInt64 uint64Value = 0;
	UA_Double doubleValue = 0;
	UA_Float floatValue = 0;
	UA_Boolean booleanValue = false;

	if (variableType == &UA_TYPES[UA_TYPES_BYTE] || variableType == &UA_TYPES[UA_TYPES_STRING]) {
		value = &byteValue;
	}
	else if (variableType == &UA_TYPES[UA_TYPES_SBYTE]) {
		value = &sbyteValue;
	}
	else if (variableType == &UA_TYPES[UA_TYPES_INT16]) {
		value = &int16Value;
	}
	else if (variableType == &UA_TYPES[UA_TYPES_INT32]) {
		value = &int32Value;
	}
	else if (variableType == &UA_TYPES[UA_TYPES_INT64]) {
		value = &int64Value;
	}
	else if (variableType == &UA_TYPES[UA_TYPES_UINT16]) {
		value = &uint16Value;
	}
	else if (variableType == &UA_TYPES[UA_TYPES_UINT32]) {
		value = &uint32Value;
	}
	else if (variableType == &UA_TYPES[UA_TYPES_UINT64]) {
		value = &uint64Value;
	}
	else if (variableType == &UA_TYPES[UA_TYPES_FLOAT]) {
		value = &floatValue;
	}
	else if (variableType == &UA_TYPES[UA_TYPES_DOUBLE]) {
		value = &doubleValue;
	}
	else if (variableType == &UA_TYPES[UA_TYPES_BOOLEAN]) {
		value = &booleanValue;
	}
	else {
		value = &int32Value;
	}

	UA_Variant_setScalar(&publisherAttr.value, value, variableType);

	UA_NodeId varNodeId;
	UA_Server_addVariableNode(server, UA_NODEID_NULL,
								  UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
								  UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
								  UA_QUALIFIEDNAME(1, name),
								  UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
								  publisherAttr, NULL, &varNodeId);
	return varNodeId;
}

void Publisher::addVarWithOwnDatasetByPointer(const UA_DataType *variableType,
		const UA_NodeId &writerGroup, UA_NodeId *varNodeId) {

	*varNodeId = this->addVariable(variableType);
	UA_NodeId dataSet = this->addDataSet();
	this->addDataSetField(dataSet, *varNodeId);
	this->addDataSetWriter(dataSet, writerGroup /*, dataSetWriterId default 62541*/);
}

void Publisher::addVarWithOwnDatasetByReference(const UA_DataType *variableType,
		const UA_NodeId &writerGroup, const UA_NodeId &varNodeId) {

	UA_NodeId dataSet = this->addDataSet();
	this->addDataSetField(dataSet, varNodeId);
	this->addDataSetWriter(dataSet, writerGroup /*, dataSetWriterId default 62541*/);
}

UA_NodeId Publisher::addVarWithOwnDataset(const UA_DataType *variableType,
		const UA_NodeId &writerGroup) {
	UA_NodeId varNodeId;
	varNodeId = this->addVariable(variableType);
	UA_NodeId dataSet = this->addDataSet();
	this->addDataSetField(dataSet, varNodeId);
	this->addDataSetWriter(dataSet, writerGroup /*, dataSetWriterId default 62541*/);
	return varNodeId;
}

UA_NodeId Publisher::addVarToDataset(const UA_NodeId &datasetId, const UA_DataType *variableType) {
	UA_NodeId varNode = this->addVariable(variableType);
	this->addDataSetField(datasetId, varNode);
	return varNode;
}


void Publisher::writeVariable(void *value, const UA_NodeId &varNodeId, const UA_DataType *variableType) {

	UA_Variant myVar;
	UA_Variant_init(&myVar);
	UA_Variant_setScalar(&myVar, value, variableType);
	UA_Server_writeValue(server, varNodeId, myVar);
}


void Publisher::setChannelPort(UA_UInt16 port) {
	char p[4];
	sprintf(p, "%d", port);
	UA_String s = this->data->networkAddressUrl.url;
	UA_Byte l = s.length;
	UA_Byte k = 5;
	for (int i = 2; i < 6; i++) {
		s.data[l-i] = p[k-i];
	}
}

void Publisher::setUrl(char *url) {
	this->data->networkAddressUrl.url = UA_STRING(url);
}

void Publisher::disablePublishing(UA_NodeId *writerGroup) {
	UA_Server_setWriterGroupDisabled(server, *writerGroup);
}

void Publisher::enablePublishing(UA_NodeId *writerGroup) {
	UA_Server_setWriterGroupOperational(server, *writerGroup);
}


void Publisher::stopHandler(int sign) {
	UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "received ctrl-c");
	Publisher::running = false;
}

bool Publisher::isRunning() {
	return Publisher::running;
}

void Publisher::stopRunning() {
	Publisher::running = false;
}

void Publisher::resetRunning() {
	Publisher::running = true;
}

int Publisher::run() {
	signal(SIGINT, stopHandler);
	signal(SIGTERM, stopHandler);

	if(!isServerConfigured)
		return EXIT_FAILURE;

	UA_StatusCode retval = UA_Server_run(server, &Publisher::running);

	UA_Server_delete(server);
	return retval == UA_STATUSCODE_GOOD ? EXIT_SUCCESS : EXIT_FAILURE;
}


void Publisher::publishDateTime(UA_UInt16 interval, UA_NodeId &writerGroup, UA_NodeId &connId) {

	writerGroup = this->addWriterGroup(/*interval:*/1000, connId /*, writerGroupId default 100*/);

	UA_NodeId publishedDataSet = this->addDataSet();
	this->addDataSetField(publishedDataSet, UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERSTATUS_CURRENTTIME));
	this->addDataSetWriter(publishedDataSet, writerGroup /*, dataSetWriterId default 62541*/);
}

UA_NodeId Publisher::addDateTimeToDataSet(const UA_NodeId &dataSet) {
	return this->addDataSetField(dataSet, UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERSTATUS_CURRENTTIME));
}

void Publisher::publishOnDifferentChannelDemo() {
    // Adds PubSubChannel on default port "opc.udp://224.0.0.22:4840/":
	UA_NodeId connection_1 = this->addPubSubConnection(/*publisherId default 2234*/);

    UA_NodeId writerGroup_0 = this->addWriterGroup(/*interval:*/1000, connection_1 /*, writerGroupId default 100*/);

    UA_NodeId publishedDataSet = this->addDataSet();
	this->addDataSetField(publishedDataSet, UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERSTATUS_CURRENTTIME));
	this->addDataSetWriter(publishedDataSet, writerGroup_0 /*, dataSetWriterId default 62541*/);


	UA_NodeId writerGroup_1 = this->addWriterGroup(/*interval:*/5000, connection_1 /*, writerGroupId default 100*/);

	this->addVarWithOwnDatasetByReference(&UA_TYPES[UA_TYPES_BYTE], writerGroup_1, UA_NODEID_NUMERIC(1, 1111));
	this->addVarWithOwnDatasetByReference(&UA_TYPES[UA_TYPES_FLOAT], writerGroup_1, UA_NODEID_NUMERIC(1, 2222));
	this->addVarWithOwnDatasetByReference(&UA_TYPES[UA_TYPES_BOOLEAN], writerGroup_1, UA_NODEID_NUMERIC(1, 1234));

	UA_Byte byteVal = 127;
	UA_SByte sbyteVal = -128;
	UA_Boolean boolVal = true;
	UA_Float floatVal = 127.1234;
	UA_Double doubleVal = 0.987989;
	UA_Int16 int16Val = 3651;
	UA_Int32 int32Val = INT32_MIN;
	UA_Int64 int64Val = INT64_MAX;
	UA_UInt16 uInt16Val = 9864;
	UA_UInt32 uInt32Val = 1519849;
	UA_UInt64 uInt64Val = 546546;

	this->writeVariable(&byteVal, UA_NODEID_NUMERIC(1, 1111), &UA_TYPES[UA_TYPES_BYTE]);
	this->writeVariable(&floatVal, UA_NODEID_NUMERIC(1, 2222), &UA_TYPES[UA_TYPES_FLOAT]);
	this->writeVariable(&boolVal, UA_NODEID_NUMERIC(1, 1234), &UA_TYPES[UA_TYPES_BOOLEAN]);


	// Add another PubSubChannel on different port
    this->setChannelPort(4860);
    UA_NodeId connection_2 = this->addPubSubConnection(/*publisherId default 2234*/);

	UA_NodeId writerGroup_2 = this->addWriterGroup(/*interval:*/2000, connection_2 /*, writerGroupId default 100*/);

	this->addVarWithOwnDatasetByReference(&UA_TYPES[UA_TYPES_SBYTE], writerGroup_2, UA_NODEID_NUMERIC(0, 1598));
	this->addVarWithOwnDatasetByReference(&UA_TYPES[UA_TYPES_UINT16], writerGroup_2, UA_NODEID_NUMERIC(1, 3333));
	this->addVarWithOwnDatasetByReference(&UA_TYPES[UA_TYPES_UINT32], writerGroup_2, UA_NODEID_NUMERIC(1, 4444));
	this->addVarWithOwnDatasetByReference(&UA_TYPES[UA_TYPES_UINT64], writerGroup_2, UA_NODEID_NUMERIC(1, 5555));

	this->writeVariable(&sbyteVal, UA_NODEID_NUMERIC(0, 1598), &UA_TYPES[UA_TYPES_SBYTE]);
	this->writeVariable(&uInt16Val, UA_NODEID_NUMERIC(1, 3333), &UA_TYPES[UA_TYPES_UINT16]);
	this->writeVariable(&uInt32Val, UA_NODEID_NUMERIC(1, 4444), &UA_TYPES[UA_TYPES_UINT32]);
	this->writeVariable(&uInt64Val, UA_NODEID_NUMERIC(1, 5555), &UA_TYPES[UA_TYPES_UINT64]);

	// Add another PubSubChannel
    this->setChannelPort(4870);
    UA_NodeId connection_3 = this->addPubSubConnection(/*publisherId default 2234*/);

    UA_NodeId writerGroup_3 = this->addWriterGroup(/*interval:*/1000, connection_3 /*, writerGroupId default 100*/);

    this->addVarWithOwnDatasetByReference(&UA_TYPES[UA_TYPES_INT16], writerGroup_3, UA_NODEID_NUMERIC(1, 6666));
    this->addVarWithOwnDatasetByReference(&UA_TYPES[UA_TYPES_INT32], writerGroup_3, UA_NODEID_NUMERIC(1, 7777));
    this->addVarWithOwnDatasetByReference(&UA_TYPES[UA_TYPES_INT64], writerGroup_3, UA_NODEID_NUMERIC(1, 8888));
    this->addVarWithOwnDatasetByReference(&UA_TYPES[UA_TYPES_DOUBLE], writerGroup_3, UA_NODEID_NUMERIC(1, 9999));

    this->writeVariable(&int16Val, UA_NODEID_NUMERIC(1, 6666), &UA_TYPES[UA_TYPES_INT16]);
    this->writeVariable(&int32Val, UA_NODEID_NUMERIC(1, 7777), &UA_TYPES[UA_TYPES_INT32]);
    this->writeVariable(&int64Val, UA_NODEID_NUMERIC(1, 8888), &UA_TYPES[UA_TYPES_INT64]);
    this->writeVariable(&doubleVal, UA_NODEID_NUMERIC(1, 9999), &UA_TYPES[UA_TYPES_DOUBLE]);
}


void Publisher::publishDataSetsDemo() {

#ifdef INT_DATASET_DEMO
	UA_NodeId datasetDemo = pub.addDataSet();
	UA_NodeId wgInt32 = pub.addWriterGroup(1000, connId);
	pub.addDataSetWriter(datasetDemo, wgInt32);

	UA_NodeId var[VARIABLE_NUMBER];
	for(UA_Byte i = 0; i < VARIABLE_NUMBER; i++) {
		var[i] = pub.addVarToDataset(datasetDemo, VAR_TYPE_INT32);
		UA_Int32 val = 1000 + i*i*10;
		pub.writeVariable(&val, var[i], VAR_TYPE_INT32);
	}

#ifdef MIXED_DATASET_DEMO
	pub.addVarToDataset(datasetDemo, VAR_TYPE_BOOLEAN);
	pub.addVarToDataset(datasetDemo, VAR_TYPE_FLOAT);
	pub.addVarToDataset(datasetDemo, VAR_TYPE_UINT16);
	pub.addDateTimeToDataSet(datasetDemo);
#endif

#ifdef DOUBLE_DATASET_DEMO
	UA_NodeId datasetDouble = pub.addDataSet();
	UA_NodeId varD[VARIABLE_NUMBER];
	for(UA_Byte i = 0; i < VARIABLE_NUMBER; i++) {
		varD[i] = pub.addVarToDataset(datasetDouble, VAR_TYPE_DOUBLE);
		UA_Double val = (UA_Double)(1000 + i*i*i*10);
		pub.writeVariable(&val, varD[i], VAR_TYPE_DOUBLE);
	}
	pub.addDataSetWriter(datasetDouble, wgInt32);
#endif
#endif
}

void Publisher::removePubSubConnection(const UA_NodeId &connection) {
	UA_Server_removePubSubConnection(this->server, connection);
}

void Publisher::removeWriterGroup(const UA_NodeId &writerGroup) {
	UA_Server_removeWriterGroup(this->server, writerGroup);
}

void Publisher::removeDataSet(const UA_NodeId &dataSet) {
	UA_Server_removePublishedDataSet(this->server, dataSet);
}

void Publisher::removeDataSetWriter(const UA_NodeId &dataSetWriter) {
	UA_Server_removeDataSetWriter(this->server, dataSetWriter);
}

void Publisher::removeDataSetField(const UA_NodeId &dataSetField) {
	UA_Server_removeDataSetField(this->server, dataSetField);
}

void Publisher::publishHelloWorld(UA_NodeId &conn) {
	conn = addPubSubConnection();
	UA_NodeId wg = addWriterGroup(1000, conn);
	UA_NodeId dts = addDataSet();
	UA_NodeId var[13];
	for (UA_Byte i = 0; i < 13; i++) {
		var[i] = addVariable(VAR_TYPE_BYTE);
		addDataSetField(dts, var[i]);
	}
	UA_Byte c = 'H';
	writeVariable(&c, var[0], VAR_TYPE_BYTE);
	c = 'e';
	writeVariable(&c, var[1], VAR_TYPE_BYTE);
	c = 'l';
	writeVariable(&c, var[2], VAR_TYPE_BYTE);
	c = 'l';
	writeVariable(&c, var[3], VAR_TYPE_BYTE);
	c = 'o';
	writeVariable(&c, var[4], VAR_TYPE_BYTE);
	c = ' ';
	writeVariable(&c, var[5], VAR_TYPE_BYTE);
	c = 'W';
	writeVariable(&c, var[6], VAR_TYPE_BYTE);
	c = 'o';
	writeVariable(&c, var[7], VAR_TYPE_BYTE);
	c = 'r';
	writeVariable(&c, var[8], VAR_TYPE_BYTE);
	c = 'l';
	writeVariable(&c, var[9], VAR_TYPE_BYTE);
	c = 'd';
	writeVariable(&c, var[10], VAR_TYPE_BYTE);
	c = '!';
	writeVariable(&c, var[11], VAR_TYPE_BYTE);
	c = '\0';
	writeVariable(&c, var[12], VAR_TYPE_BYTE);
	addDataSetWriter(dts, wg);
}
