/*
 * Connection.cpp
 *
 *  Created on: Nov 9, 2020
 *      Author: minh
 */

#include "BranchConnection.h"

#include <iostream>

using namespace std;

UA_UInt16 BranchConnection::count = 0;
Publisher *BranchConnection::pub;

BranchConnection::BranchConnection() : port(0), wgKeyInc(0) {

}

BranchConnection::BranchConnection(UA_UInt16 port, UA_UInt16 chKey) : port(port), wgKeyInc(0)  {
	key[0] = chKey;
	pub->setChannelPort(port);
	this->connection = pub->addPubSubConnection();
	this->count++;
	cout << "\tCH "<< chKey <<" added on Port: " << port << "  Total CH: " << this->count <<  endl;
}

BranchConnection::~BranchConnection() {
//	cout << "BranchConnection Destructor called\n";
//	this->removeAllWritergroup();
}


void BranchConnection::init(Publisher *pub) {
	BranchConnection::pub = pub;
}

bool BranchConnection::addWritergroup() {
	wgKeyInc++;
	if(!writergroupExists(wgKeyInc)) {
		BranchWriterGroup::count++;
		BranchWriterGroup wgBranch(key[B_CONN], wgKeyInc);
		wgBranch.writerGroup = pub->addWriterGroup(100, this->connection);
		memset(&wgBranch.wgConfig, 0, sizeof(UA_WriterGroupConfig));
		UA_Server_getWriterGroupConfig(pub->server, wgBranch.writerGroup, &wgBranch.wgConfig);
		wg.insert(pair<UA_UInt16, BranchWriterGroup>(wgKeyInc, wgBranch));
		return true;
	}
	cout << "WriterGroup " << wgKeyInc << " already exists" << endl;
	return false;
}

bool BranchConnection::removeWritergroup(UA_UInt16 wgKey) {
	map<UA_UInt16, BranchWriterGroup>::iterator it;
	if(getWritergroup(wgKey, &it)) {
		BranchWriterGroup::count--;
		it->second.removeAllDataset();
		pub->removeWriterGroup(it->second.writerGroup);
		wg.erase(wgKey);
		return true;
	}
	cout << "WriterGroup " << wgKey << " does not exist";
	return false;
}

void BranchConnection::removeAllWritergroup() {
	for(UA_UInt16 i = 1; i <= this->wgKeyInc; i++) {
		if(writergroupExists(i))
			this->removeWritergroup(i);
	}
	this->wgKeyInc = 0;
	this->wg.clear();
}

bool BranchConnection::writergroupExists(UA_UInt16 wgKey) {
	return wg.find(wgKey) != wg.end();
}

bool BranchConnection::getWritergroup(UA_UInt16 wgKey,
		map<UA_UInt16, BranchWriterGroup>::iterator *it) {
	return (*it = wg.find(wgKey)) != wg.end();
}

void BranchConnection::print() {
	cout << "\nChannel " << key[B_CONN] << " on Port " << port << endl;
}

void BranchConnection::showWritergroups() {
	map<UA_UInt16, BranchWriterGroup>::iterator it = wg.begin();
	while (it != wg.end()) {
		cout << "\t";
		it->second.print();
//		cout << endl;
		it->second.showDatasets();
//		cout << endl;
		it++;
	}
}

void BranchConnection::disableAllWg() {
	map<UA_UInt16, BranchWriterGroup>::iterator it = wg.begin();
	while(it != wg.end()) {
		it->second.disable();
		it++;
	}
}

void BranchConnection::enableAllWg() {
	map<UA_UInt16, BranchWriterGroup>::iterator it = wg.begin();
	while(it != wg.end()) {
		it->second.enable();
		it++;
	}
}

void BranchConnection::setIntervalOnAllWg(UA_UInt16 interval) {
	map<UA_UInt16, BranchWriterGroup>::iterator it = wg.begin();
	while(it != wg.end()) {
		it->second.update(interval);
		it++;
	}

}
