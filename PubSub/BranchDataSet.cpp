/*
 * DataSetBox.cpp
 *
 *  Created on: Nov 9, 2020
 *      Author: minh
 */

#include "BranchDataSet.h"

#include <iostream>

UA_UInt16 BranchDataSet::count = 0;
Publisher *BranchDataSet::pub;

BranchDataSet::BranchDataSet() : isString(false), varKeyInc(0) {

}

BranchDataSet::BranchDataSet(UA_UInt16 connKey, UA_UInt16 wgKey, UA_UInt16 key)
: isString(false), key{connKey, wgKey, key}, varKeyInc(0) {
	cout << "\tDS " << this->key[B_DTS]  << " added to WG " << this->key[B_WG] << " of CH " << this->key[B_CONN] <<  "   Total DS: "<< this->count <<endl;
}

BranchDataSet::BranchDataSet(UA_UInt16 connKey, UA_UInt16 wgKey, UA_UInt16 key, UA_Boolean isString)
: isString(isString), key{connKey, wgKey, key}, varKeyInc(0) {
	cout << "\tDS " << this->key[B_DTS]  << " added to WG " << this->key[B_WG] << " of CH " << this->key[B_CONN] <<  "   Total DS: "<< this->count <<endl;
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
//		fld.var = pub->addVariable(variableType);
//		fld.dataSetField = pub->addDataSetField(dataSet, fld.var);
		fields.insert(pair<UA_UInt16, BranchField>(varKeyInc, fld));
		return true;
	}
	cout << "DataSetField " << varKeyInc << " already exists";
	return false;
}

bool BranchDataSet::removeData(UA_UInt16 varKey) {
	map<UA_UInt16, BranchField>::iterator it;
	if(getField(varKey, &it)) {
		BranchField::count--;
		pub->removeDataSetField(it->second.dataSetField);
		fields.erase(varKey);
//		cout << "data removed" << endl;
		return true;
	}
	cout << "DataField " << varKey << " does not exist";
	return false;
}

bool BranchDataSet::fieldExists(UA_UInt16 varKey) {
	return fields.find(varKey) != fields.end();
}

bool BranchDataSet::getField(UA_UInt16 varKey, map<UA_UInt16, BranchField>::iterator *it) {
	return (*it = fields.find(varKey)) != fields.end();
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

		if(fields.size() > 0) {
			map<UA_UInt16, BranchField>::iterator it = fields.begin();
			UA_UInt16 i = 0;
			cout << "Writing into DataSet " << key[B_DTS] << ": ";
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
		cout << "This is not a String DataSet" << endl;
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
