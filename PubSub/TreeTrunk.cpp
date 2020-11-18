/*
 * BranchManager.cpp
 *
 *  Created on: Nov 9, 2020
 *      Author: minh
 */

#include "TreeTrunk.h"

#include <iostream>

using namespace std;

Publisher *TreeTrunk::pub;

TreeTrunk::TreeTrunk() : connKeyInc(0){
	// TODO Auto-generated constructor stub

}

TreeTrunk::~TreeTrunk() {
	// TODO Auto-generated destructor stub
}

void TreeTrunk::init(Publisher *pub) {
	TreeTrunk::pub = pub;
	BranchConnection::init(pub);
	BranchWriterGroup::init(pub);
	BranchDataSet::init(pub);
	BranchField::init(pub);
}

bool TreeTrunk::createConnection(UA_UInt16 port) {
	this->connKeyInc++;
	if(!channelExists(this->connKeyInc)) {
		BranchConnection conn(port, connKeyInc);
		conn.key[B_CONN] = connKeyInc;
		this->conns.insert(pair<UA_UInt16, BranchConnection>(connKeyInc, conn));
		return true;
	}
	UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "CH %u already exists", connKeyInc);
	return false;
}

bool TreeTrunk::deleteConnection(UA_UInt16 chKey) {
	map<UA_UInt16, BranchConnection>::iterator it;
	if(getChannel(chKey, &it)) {
		pub->removePubSubConnection(it->second.connection);
		it->second.count--;
		it->second.removeAllWritergroup();
		conns.erase(chKey);
		UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "CH %u removed", chKey);
		return true;
	}
	UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Could not remove CH %u", chKey);
	return false;
}

void TreeTrunk::deleteAllConnections(){
	for(UA_UInt16 i = 1; i <= this->connKeyInc; i++) {
		if(channelExists(i))
			this->deleteConnection(i);
	}
	this->connKeyInc = 0;
	this->conns.clear();
}

bool TreeTrunk::getChannel(UA_UInt16 connKey, map<UA_UInt16, BranchConnection>::iterator *it) {
	if ((*it = conns.find(connKey)) != conns.end())
		return true;
	UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "CH %u does not exist", connKey);
	return false;
}

bool TreeTrunk::channelExists(UA_UInt16 connKey) {
	return conns.find(connKey) != conns.end();
}

bool TreeTrunk::getConnection(UA_UInt16 chKey,
		BranchConnection *connection) {

	map<UA_UInt16, BranchConnection>::iterator it;
	if( this->getChannel(chKey, &it)) {

		*connection = it->second;
		return true;
	}
	return false;
}

bool TreeTrunk::getWriterGroup(UA_UInt16 *b,
		BranchWriterGroup *writerGroup) {

	BranchConnection conn;
	if(this->getConnection(b[B_CONN], &conn)) {
		map<UA_UInt16, BranchWriterGroup>::iterator it;
		if(conn.getWritergroup(b[B_WG], &it) ){
			*writerGroup = it->second;
			return true;
		}
	}
	return false;
}

bool TreeTrunk::getDataSet(UA_UInt16 *b, BranchDataSet *dataSet) {

	BranchWriterGroup wg;
	if(this->getWriterGroup(b, &wg)) {
		map<UA_UInt16, BranchDataSet>::iterator it;
		if(wg.getDataset(b[B_DTS], &it)) {
			*dataSet = it->second;
			return true;
		}
	}
	return false;
}

bool TreeTrunk::getField(UA_UInt16 *b, BranchField *field) {
	BranchDataSet dts;
	if(this->getDataSet(b, &dts)) {
		map<UA_UInt16, BranchField>::iterator it;
		if(dts.getField(b[B_VAR], &it)) {
			*field = it->second;
			return true;
		}
	}
	return false;
}

void TreeTrunk::showChannels() {
	map<UA_UInt16, BranchConnection>::iterator it = conns.begin();
	while (it != conns.end()) {
		it->second.print();
//		cout << endl;
		it->second.showWritergroups();
		it++;
	}
	cout << endl;
}

void TreeTrunk::enableWriterGroupOfChannel(UA_UInt16 chKey) {
	map<UA_UInt16, BranchConnection>::iterator it;
	if(this->getChannel(chKey, &it)) {
		it->second.enableAllWg();
	}
}

void TreeTrunk::disableWriterGroupOfChannel(UA_UInt16 chKey) {
	map<UA_UInt16, BranchConnection>::iterator it;
	if(this->getChannel(chKey, &it)) {
		it->second.disableAllWg();
	}
}

void TreeTrunk::disableWriterGroupOfPort(UA_UInt16 port) {
	map<UA_UInt16, BranchConnection>::iterator it = conns.begin();
	UA_Boolean found = false;
	while(it != conns.end()) {
		if(it->second.port == port) {
			it->second.disableAllWg();
			found = true;
		}
		it++;
	}
	if(!found)
		UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "No Channel on port %u found", port);
}

void TreeTrunk::enableWriterGroupOfPort(UA_UInt16 port) {
	map<UA_UInt16, BranchConnection>::iterator it = conns.begin();
	UA_Boolean found = false;
	while(it != conns.end()) {
		if(it->second.port == port) {
			it->second.enableAllWg();
			found = true;
		}
		it++;
	}
	if(!found)
		UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "No Channel on port %u found", port);
}

void TreeTrunk::disableAllWg() {
	map<UA_UInt16, BranchConnection>::iterator it = conns.begin();
	while(it != conns.end()) {
		it->second.disableAllWg();
		it++;
	}
}

void TreeTrunk::enableAllWg() {
	map<UA_UInt16, BranchConnection>::iterator it = conns.begin();
	while(it != conns.end()) {
		it->second.enableAllWg();
		it++;
	}
}

void TreeTrunk::updateAllWg(UA_UInt16 interval) {
	map<UA_UInt16, BranchConnection>::iterator it = conns.begin();
	while(it != conns.end()) {
		it->second.setIntervalOnAllWg(interval);
		it++;
	}
}

void TreeTrunk::updateWriterGroupOfChannel(UA_UInt16 chKey, UA_UInt16 interval) {
	map<UA_UInt16, BranchConnection>::iterator it;
	if(this->getChannel(chKey, &it)) {
		cout << "Update Intervals of WriterGroups on Channel " << chKey << endl;
		it->second.setIntervalOnAllWg(interval);
	}
	else {
		UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Could not update interval of WGs of CH %u", chKey);
	}
}

void TreeTrunk::updateWriterGroupOfPort(UA_UInt16 port, UA_UInt16 interval) {
	map<UA_UInt16, BranchConnection>::iterator it = conns.begin();
	UA_Boolean found = false;
	while(it != conns.end()) {
		if(it->second.port == port) {
			cout << "Update Intervals of WriterGroups on Channel port " << port << endl;
			it->second.setIntervalOnAllWg(interval);
			found = true;
		}
		it++;
	}
	if (!found)
		UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "No Channel on port %u found", port);
}
