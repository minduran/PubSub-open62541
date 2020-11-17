/*
 * Writergroup.h
 *
 *  Created on: Nov 9, 2020
 *      Author: minh
 */

#ifndef BRANCHWRITERGROUP_H_
#define BRANCHWRITERGROUP_H_

#include <open62541/server_pubsub.h>
#include "Publisher.h"
#include <map>
#include "Defines_and_Structures.h"
#include "BranchDataSet.h"

using namespace std;

class BranchWriterGroup {

	static Publisher *pub;

	bool enabled;

public:
	static UA_UInt32 count;
	UA_NodeId writerGroup;
	UA_WriterGroupConfig wgConfig;
	UA_UInt16 key[2];
	UA_UInt16 dtsKeyInc;
	map<UA_UInt16, BranchDataSet> dts;

	BranchWriterGroup();
	BranchWriterGroup(UA_UInt16 connKey, UA_UInt16 key);
	virtual ~BranchWriterGroup();

	static void init(Publisher *pub);

	bool addDataset();
	bool addDatasetString();
	bool removeDataset(UA_UInt16 dtsKey);
	bool datasetExists(UA_UInt16 dtsKey);
	bool getDataset(UA_UInt16 dtsKey, map<UA_UInt16, BranchDataSet>::iterator *it);
	void removeAllDataset();

	void disable();
	void enable();

	void update(UA_UInt32 interval);

	void print();
	void showDatasets();
};

#endif /* BRANCHWRITERGROUP_H_ */
