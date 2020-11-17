/*
 * DataSetBox.cpp
 *
 *  Created on: Nov 9, 2020
 *      Author: minh
 */

#include "BranchDataSet.h"

#include <iostream>

UA_UInt32 BranchDataSet::count = 0;
Publisher *BranchDataSet::pub;

BranchDataSet::BranchDataSet() : isString(false), varKeyInc(0) {

}

BranchDataSet::BranchDataSet(UA_UInt16 connKey, UA_UInt16 wgKey, UA_UInt16 key)
: isString(false), key{connKey, wgKey, key}, varKeyInc(0) {
//	cout << "\tDS " << this->key[B_DTS]  << " added to WG " << this->key[B_WG] << " of CH " << this->key[B_CONN] <<  "   Total DS: "<< this->count <<endl;
	UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
//				"DS %u %u %u added.    Total DataSets: %u", this->key[B_DTS],  this->key[B_WG],  this->key[B_CONN], count);
				"DS %u %u %u added", this->key[B_DTS],  this->key[B_WG],  this->key[B_CONN]);
}

BranchDataSet::BranchDataSet(UA_UInt16 connKey, UA_UInt16 wgKey, UA_UInt16 key, UA_Boolean isString)
: isString(isString), key{connKey, wgKey, key}, varKeyInc(0) {
	UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
					"String DS %u %u %u added.    Total DataSets: %u",  this->key[B_DTS],  this->key[B_WG],  this->key[B_CONN], count);
//	cout << "\tDS " << this->key[B_DTS]  << " added to WG " << this->key[B_WG] << " of CH " << this->key[B_CONN] <<  "   Total DS: "<< this->count <<endl;
}

BranchDataSet::~BranchDataSet() {
//	cout << "ConnectionBox Destructor called\n";
}

void BranchDataSet::init(Publisher *pub) {
	BranchDataSet::pub = pub;
}

bool BranchDataSet::addData(const UA_DataType *variableType) {
	varKeyInc++;
	if(!fieldExists(varKeyInc)) {
		BranchField::count++;
		key[B_VAR] = varKeyInc;
		BranchField fld(key, variableType, dataSet, isString);
		fields.insert(pair<UA_UInt16, BranchField>(varKeyInc, fld));
		return true;
	}
	UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "FD %u %u %u %u already exists", varKeyInc, key[B_DTS], key[B_WG], key[B_CONN]);
	return false;
}

bool BranchDataSet::removeData(UA_UInt16 varKey) {
	map<UA_UInt16, BranchField>::iterator it;
	if(getField(varKey, &it)) {
		BranchField::count--;
		pub->removeDataSetField(it->second.dataSetField);
		fields.erase(varKey);
		UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Field %d %d %d %d removed", varKey, key[B_DTS], key[B_WG], key[B_CONN]);
		return true;
	}
	UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Could not remove FD %u %u %u %u", varKey, key[B_DTS], key[B_WG], key[B_CONN]);
	return false;
}

bool BranchDataSet::fieldExists(UA_UInt16 varKey) {
	return fields.find(varKey) != fields.end();
}

bool BranchDataSet::getField(UA_UInt16 varKey, map<UA_UInt16, BranchField>::iterator *it) {
	if ((*it = fields.find(varKey)) != fields.end()) {
		return true;
	}
	UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "FD %u %u %u %u does not exist", varKey, key[B_DTS], key[B_WG], key[B_CONN]);
	return false;
}

void BranchDataSet::removeAllData() {
	for(UA_UInt16 i = 1; i <= this->varKeyInc; i++) {
		if(fieldExists(i))
			this->removeData(i);
	}
	this->varKeyInc = 0;
	this->fields.clear();
}


void BranchDataSet::print() {
	cout << "DataSet " << key[B_DTS];
	if (isString)
		cout << "\t(String)";
	cout << endl;
}

void BranchDataSet::writeString(char *string) {
	if (isString) {
		UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Writing Content: %s", string);
		if(fields.size() > 0) {
			map<UA_UInt16, BranchField>::iterator it = fields.begin();
			UA_UInt16 i = 0;
//			cout << "Writing into DataSet " << key[B_DTS] << ": ";
			while(it != fields.end() && i < strlen(string)) {
				it->second.writeStringChar(string[i++]);
				it++;
			}
			cout << endl;
			while(i < strlen(string)) {
				this->addData(VAR_TYPE_BYTE);
				this->fields.at(varKeyInc).writeStringChar(string[i++]);
			}

			while(varKeyInc > strlen(string)) {
				if(this->removeData(varKeyInc))
					varKeyInc--;
			}
		}
		else {
			for(UA_UInt16 i = 0; i < strlen(string); i++) {
				this->addData(VAR_TYPE_BYTE);
				this->fields.at(varKeyInc).writeStringChar(string[i]);
			}
		}
	}
	else {
		UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "DS %u %u %u is not a String DataSet", key[B_DTS], key[B_WG], key[B_CONN]);
	}
}

void BranchDataSet::showFields() {
	map<UA_UInt16, BranchField>::iterator it = fields.begin();
	while (it != fields.end()) {
		cout << "\t\t\t";
		it->second.print();
		it++;
	}
	cout << endl;
}
