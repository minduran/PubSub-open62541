/*
 * VarNode.h
 *
 *  Created on: Nov 9, 2020
 *      Author: minh
 */

#ifndef BRANCHFIELD_H_
#define BRANCHFIELD_H_

#include <open62541/server_pubsub.h>
#include "Publisher.h"
#include "Defines_and_Structures.h"

class BranchField {

	static Publisher *pub;

public:
	static UA_UInt32 count;
	UA_NodeId var;
	UA_NodeId dataSetField;
	UA_UInt16 key[4];
	const UA_DataType *variableType;
	BranchField();
	BranchField(UA_UInt16 *b, const UA_DataType *variableType, UA_NodeId &dataSet, UA_Boolean isString);
	virtual ~BranchField();

	static void init(Publisher *pub);

	void writeValue(char *value);
	void writeStringChar(char value);

	void print();
};

#endif /* BRANCHFIELD_H_ */
