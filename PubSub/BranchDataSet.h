/*
 * DataSetBox.h
 *
 *  Created on: Nov 9, 2020
 *      Author: minh
 */

#ifndef BRANCHDATASET_H_
#define BRANCHDATASET_H_

#include <open62541/server_pubsub.h>
#include "Publisher.h"
#include <map>
#include "Defines_and_Structures.h"
#include "BranchField.h"

using namespace std;
class BranchDataSet {

	static Publisher *pub;

public:
	static UA_UInt32 count;
	UA_Boolean isString;
	UA_NodeId dataSet;
	UA_NodeId dataSetWriter;
	UA_UInt16 key[3];
	UA_UInt16 varKeyInc;
	map<UA_UInt16, BranchField> fields;

	BranchDataSet();
	BranchDataSet(UA_UInt16 connKey, UA_UInt16 wgKey, UA_UInt16 key);
	BranchDataSet(UA_UInt16 connKey, UA_UInt16 wgKey, UA_UInt16 key, UA_Boolean isString);
	virtual ~BranchDataSet();

	static void init(Publisher *pub);

	bool addData(const UA_DataType *variableType);
	void writeString(char *string);
	bool removeData(UA_UInt16 varKey);
	bool fieldExists(UA_UInt16 varKey);
	bool getField(UA_UInt16 varKey, map<UA_UInt16, BranchField>::iterator *it);
	void removeAllData();

	void print();

	void showFields();
};

#endif /* BRANCHDATASET_H_ */
