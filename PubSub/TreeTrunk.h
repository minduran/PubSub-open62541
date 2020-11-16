/*
 * BranchManager.h
 *
 *  Created on: Nov 9, 2020
 *      Author: minh
 */

#ifndef TREETRUNK_H_
#define TREETRUNK_H_

#include <open62541/server_pubsub.h>
#include "Publisher.h"
#include <map>
#include "BranchConnection.h"
#include "BranchWriterGroup.h"
#include "BranchDataSet.h"
#include "BranchField.h"
#include "Defines_and_Structures.h"

using namespace std;

class TreeTrunk {
	static Publisher *pub;
public:

	UA_UInt16 connKeyInc;
	map<UA_UInt16, BranchConnection> conns;
	TreeTrunk();
	virtual ~TreeTrunk();

	static void init(Publisher *pub);

	bool createConnection(UA_UInt16 port);
	bool deleteConnection(UA_UInt16 chKey);
	void deleteAllConnections();

	bool getChannel(UA_UInt16 connKey, map<UA_UInt16, BranchConnection>::iterator *it);

	bool channelExists(UA_UInt16 connKey);

	bool getConnection(UA_UInt16 chKey, BranchConnection *connection);
	bool getWriterGroup(UA_UInt16 *b, BranchWriterGroup *writerGroup);
	bool getDataSet(UA_UInt16 *b, BranchDataSet *dataSet);
	bool getField(UA_UInt16 *b, BranchField *field);

	void showChannels();

	void enableWriterGroupOfChannel(UA_UInt16 chKey);
	void disableWriterGroupOfChannel(UA_UInt16 chKey);
	void disableWriterGroupOfPort(UA_UInt16 port);
	void enableWriterGroupOfPort(UA_UInt16 port);
	void disableAllWg();
	void enableAllWg();
	void updateAllWg(UA_UInt16 interval);
	void updateWriterGroupOfChannel(UA_UInt16 chKey, UA_UInt16 interval);
	void updateWriterGroupOfPort(UA_UInt16 port, UA_UInt16 interval);

};

#endif /* TREETRUNK_H_ */
