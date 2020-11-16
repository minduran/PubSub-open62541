/*
 * Connection.h
 *
 *  Created on: Nov 9, 2020
 *      Author: minh
 */

#ifndef BRANCHCONNECTION_H_
#define BRANCHCONNECTION_H_

#include <open62541/server_pubsub.h>
#include "Publisher.h"
#include <map>
#include "Defines_and_Structures.h"
#include "BranchWriterGroup.h"

using namespace std;

class BranchConnection {

	static Publisher *pub;

public:
	static UA_UInt16 count;
	UA_NodeId connection;
	UA_UInt16 port;
	UA_UInt16 key[1];
	UA_UInt16 wgKeyInc;
	map<UA_UInt16 ,BranchWriterGroup> wg;

	BranchConnection();
	BranchConnection(UA_UInt16 port, UA_UInt16 chKey);
	virtual ~BranchConnection();

	static void init(Publisher *pub);

	bool addWritergroup();
	bool removeWritergroup(UA_UInt16 wgKey);
	bool writergroupExists(UA_UInt16 wgKey);
	bool getWritergroup(UA_UInt16 wgKey, map<UA_UInt16, BranchWriterGroup>::iterator *it);
	void removeAllWritergroup();

	void showWritergroups();

	void print();

	void disableAllWg();
	void enableAllWg();
	void setIntervalOnAllWg(UA_UInt16 interval);
};

#endif /* BRANCHCONNECTION_H_ */
