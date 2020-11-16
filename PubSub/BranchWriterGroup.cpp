/*
 * Writergroup.cpp
 *
 *  Created on: Nov 9, 2020
 *      Author: minh
 */

#include "BranchWriterGroup.h"

#include <iostream>

using namespace std;

UA_UInt16 BranchWriterGroup::count = 0;
Publisher *BranchWriterGroup::pub;

BranchWriterGroup::BranchWriterGroup() : enabled(true), dtsKeyInc(0){

}

BranchWriterGroup::BranchWriterGroup(UA_UInt16 connKey, UA_UInt16 key) : enabled(true), key{connKey, key}, dtsKeyInc(0) {

	cout << "\tWG " << this->key[B_WG]  << " added to CH " << this->key[B_CONN] << "  Total WG: " << BranchWriterGroup::count << endl;

}

BranchWriterGroup::~BranchWriterGroup() {
//	cout << "BranchWriterGroup Destructor called\n";
//	this->removeAllDataset();
}

void BranchWriterGroup::init(Publisher *pub) {
	BranchWriterGroup::pub = pub;
}

bool BranchWriterGroup::addDataset() {
	dtsKeyInc++;
	if(!datasetExists(dtsKeyInc)) {
		BranchDataSet::count++;
		BranchDataSet dtsBranch(key[B_CONN], key[B_WG], dtsKeyInc);
		dtsBranch.dataSet = pub->addDataSet();
		dtsBranch.dataSetWriter = pub->addDataSetWriter(dtsBranch.dataSet, writerGroup);
		dts.insert(pair<UA_UInt16, BranchDataSet>(dtsKeyInc, dtsBranch));
		return true;
	}
	cout << "DataSet " << dtsKeyInc << " already exists" << endl;
	return false;
}

bool BranchWriterGroup::addDatasetString() {
	dtsKeyInc++;
	if(!datasetExists(dtsKeyInc)) {
		BranchDataSet::count++;
		BranchDataSet dtsBranch(key[B_CONN], key[B_WG], dtsKeyInc, true);
		dtsBranch.dataSet = pub->addDataSet();
		dtsBranch.dataSetWriter = pub->addDataSetWriter(dtsBranch.dataSet, writerGroup);
		dts.insert(pair<UA_UInt16, BranchDataSet>(dtsKeyInc, dtsBranch));
		return true;
	}
	cout << "DataSet " << dtsKeyInc << " already exists" << endl;
	return false;
}

bool BranchWriterGroup::removeDataset(UA_UInt16 dtsKey) {
	map<UA_UInt16, BranchDataSet>::iterator it;
	if(getDataset(dtsKey, &it)) {
		BranchDataSet::count--;
		it->second.removeAllData();
		pub->removeDataSet(it->second.dataSet);
		pub->removeDataSetWriter(it->second.dataSetWriter);
		dts.erase(dtsKey);
		return true;
	}
	else {
		cout << "DataSet " << dtsKey << " does not exist";
		return false;
	}
}

bool BranchWriterGroup::datasetExists(UA_UInt16 dtsKey) {
	return dts.find(dtsKey) != dts.end();
}

void BranchWriterGroup::removeAllDataset() {
	for(UA_UInt16 i = 1; i <= this->dtsKeyInc; i++) {
		if(datasetExists(i))
			this->removeDataset(i);
	}
	this->dtsKeyInc = 0;
	this->dts.clear();
}


void BranchWriterGroup::print() {
	cout << "WriterGroup " << key[B_WG] << "    interval " << wgConfig.publishingInterval << " ms    " << (enabled ? "enabled" : "disabled") << endl;
}

void BranchWriterGroup::disable() {
	enabled = false;
	UA_Server_setWriterGroupDisabled(pub->server, writerGroup);
}

void BranchWriterGroup::enable() {
	enabled = true;
	UA_Server_setWriterGroupOperational(pub->server, writerGroup);
}

void BranchWriterGroup::update(UA_UInt32 interval) {
	wgConfig.publishingInterval = interval;
	UA_Server_updateWriterGroupConfig(pub->server, writerGroup, &wgConfig);
}

void BranchWriterGroup::showDatasets() {
	map<UA_UInt16, BranchDataSet>::iterator it = dts.begin();
	while (it != dts.end()) {
		cout << "\t\t";
		it->second.print();
//		cout << endl;
		it->second.showFields();
		it++;
	}
	cout << endl;
}

bool BranchWriterGroup::getDataset(UA_UInt16 dtsKey,
		map<UA_UInt16, BranchDataSet>::iterator *it) {
	return (*it = dts.find(dtsKey)) != dts.end();
}
