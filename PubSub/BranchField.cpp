/*
 * VarNode.cpp
 *
 *  Created on: Nov 9, 2020
 *      Author: minh
 */

#include "BranchField.h"
#include <iostream>
#include <iomanip>

using namespace std;

UA_UInt32 BranchField::count = 0;
Publisher *BranchField::pub;

BranchField::BranchField() : variableType() {
	// TODO Auto-generated constructor stub
}

BranchField::BranchField(UA_UInt16 *b, const UA_DataType *variableType, UA_NodeId &dataSet, UA_Boolean isString)
: key{b[B_CONN], b[B_WG], b[B_DTS], b[B_VAR]}, variableType(variableType) {
	if(variableType == VAR_TYPE_DATETIME) {
		dataSetField = pub->addDataSetField(dataSet, UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERSTATUS_CURRENTTIME));
	}
	else {
		var = pub->addVariable(variableType);
		dataSetField = pub->addDataSetField(dataSet, var);
	}

	if (!isString)
		UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
//						"FD %u %u %u %u added.    Total Fields: %u", key[B_VAR], key[B_DTS], key[B_WG], key[B_CONN], count);
						"FD %u %u %u %u added", key[B_VAR], key[B_DTS], key[B_WG], key[B_CONN]);

}

BranchField::~BranchField() {

}

void BranchField::init(Publisher *pub) {
	BranchField::pub = pub;
}

void BranchField::writeValue(char *val) {
	if(variableType == VAR_TYPE_DOUBLE) {
		UA_Double value = (UA_Double) atof(val);
		UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "FD %u %u %u %u: Double set to %f", key[B_VAR], key[B_DTS], key[B_WG], key[B_CONN], value);
		pub->writeVariable(&value, var, variableType);
	}
	else if(variableType == VAR_TYPE_FLOAT) {
		UA_Float value = (UA_Float) atof(val);
		UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "FD %u %u %u %u: Float set to %f", key[B_VAR], key[B_DTS], key[B_WG], key[B_CONN], value);
		pub->writeVariable(&value, var, variableType);
	}
	else if(variableType == VAR_TYPE_BOOLEAN) {
		static UA_Boolean value = false;
		if (strcmp(val, "true") == 0 || strcmp(val, "1") == 0) {
			UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "FD %u %u %u %u: Boolean set to true", key[B_VAR], key[B_DTS], key[B_WG], key[B_CONN]);
			value = true;
		}
		else if (strcmp(val, "false") == 0 || strcmp(val, "0") == 0) {
			UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "FD %u %u %u %u: Boolean set to false", key[B_VAR], key[B_DTS], key[B_WG], key[B_CONN]);
			value = false;
		}
		else {
			UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "%s is an invalid boolean input", val);
		}
		pub->writeVariable(&value, var, variableType);
	}
	else if(variableType == VAR_TYPE_BYTE) {
		UA_Byte value = (UA_Byte) atoi(val);
		UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "FD %u %u %u %u: Byte set to %u", key[B_VAR], key[B_DTS], key[B_WG], key[B_CONN], value);
		pub->writeVariable(&value, var, variableType);
	}
	else if(variableType == VAR_TYPE_SBYTE) {
		UA_SByte value = (UA_SByte) atoi(val);
		UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "FD %u %u %u %u: Signed Byte set to %i", key[B_VAR], key[B_DTS], key[B_WG], key[B_CONN], value);
		pub->writeVariable(&value, var, variableType);
	}
	else if(variableType == VAR_TYPE_INT32) {
		UA_Int32 value = (UA_Int32) atoi(val);
		UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "FD %u %u %u %u: Int 32 set to %i", key[B_VAR], key[B_DTS], key[B_WG], key[B_CONN], value);
		pub->writeVariable(&value, var, variableType);
	}
	else if(variableType == VAR_TYPE_UINT32) {
		UA_UInt32 value = (UA_UInt32) atoi(val);
		UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "FD %u %u %u %u: Unsigned Int 32 set to %u", key[B_VAR], key[B_DTS], key[B_WG], key[B_CONN], value);
		pub->writeVariable(&value, var, variableType);
	}
	else if(variableType == VAR_TYPE_INT16) {
		UA_Int16 value = (UA_Int16) atoi(val);
		UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "FD %u %u %u %u: Int 16 set to %i", key[B_VAR], key[B_DTS], key[B_WG], key[B_CONN], value);
		pub->writeVariable(&value, var, variableType);
	}
	else if(variableType == VAR_TYPE_UINT16) {
		UA_UInt16 value = (UA_UInt16) atoi(val);
		UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "FD %u %u %u %u: Unsigned Int 16 set to %u", key[B_VAR], key[B_DTS], key[B_WG], key[B_CONN], value);
		pub->writeVariable(&value, var, variableType);
	}
	else if(variableType == VAR_TYPE_INT64) {
		UA_Int64 value = (UA_Int64) atol(val);
		UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "FD %u %u %u %u: Int 64 set to %li", key[B_VAR], key[B_DTS], key[B_WG], key[B_CONN], value);
		pub->writeVariable(&value, var, variableType);
	}
	else if(variableType == VAR_TYPE_UINT64) {
		UA_UInt64 value = (UA_UInt64) atoi(val);
		UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "FD %u %u %u %u: Unsigned Int 64 set to %lu", key[B_VAR], key[B_DTS], key[B_WG], key[B_CONN], value);
		pub->writeVariable(&value, var, variableType);
	}

}

void BranchField::writeStringChar(char val) {
	UA_Byte value = (UA_Byte) val;
//	cout << (char) val << " ";
	pub->writeVariable(&value, var, variableType);
}

void BranchField::print() {
	cout << "Field " << key[B_VAR] << "    Type ";

	if (this->variableType != VAR_TYPE_DATETIME) {
		UA_Variant val;
		this->readValue(&val);

		if (this->variableType == VAR_TYPE_BYTE) {
			cout << "Byte";
			cout << "\t\t Value: " << *(UA_UInt16*)val.data;
		}
		else if (this->variableType == VAR_TYPE_SBYTE) {
			cout << "Signed Byte";
			cout << "\t Value: " << *(UA_Int16*) val.data;
		}
		else if (this->variableType == VAR_TYPE_BOOLEAN) {
			cout << "Boolean";
			cout << "\t\t Value: " <<  (*(UA_Boolean*)val.data == 1 ? "true" : "false");
		}
		else if (this->variableType == VAR_TYPE_DOUBLE) {
			cout << "Double";
			cout << fixed;
			cout << "\t\t Value: " << setprecision(6) << *(UA_Double*)val.data;
			cout << resetiosflags(ios::fixed);
		}
		else if (this->variableType == VAR_TYPE_FLOAT) {
			cout << "Float";
			cout << fixed;
			cout << "\t\t Value: " << setprecision(6) << *(UA_Float*)val.data;
			cout << resetiosflags(ios::fixed);
		}
		else if (this->variableType == VAR_TYPE_INT16) {
			cout << "Int 16";
			cout << "\t\t Value: " << *(UA_Int16*) val.data;
		}
		else if (this->variableType == VAR_TYPE_INT32) {
			cout << "Int 32";
			cout << "\t\t Value: " << *(UA_Int32*) val.data;
		}
		else if (this->variableType == VAR_TYPE_INT64) {
			cout << "Int 64";
			cout << "\t\t Value: " << *(UA_Int64*) val.data;
		}
		else if (this->variableType == VAR_TYPE_UINT16) {
			cout << "Unsigned Int 16";
			cout << "\t Value: " << *(UA_UInt16*) val.data;
		}
		else if (this->variableType == VAR_TYPE_UINT32) {
			cout << "Unsigned Int 32";
			cout << "\t Value: " << *(UA_UInt32*) val.data;
		}
		else if (this->variableType == VAR_TYPE_UINT64) {
			cout << "Unsigned Int 64";
			cout << "\t Value: " << *(UA_UInt64*) val.data;
		}

		UA_Variant_clear(&val);
	}

	else {
		cout << "DateTime";
	}

	cout << endl;

}

void BranchField::readValue(UA_Variant *value) {
	UA_Variant_init(value);
	if(UA_Server_readValue(pub->server, var, value) != UA_STATUSCODE_GOOD) {
		UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Failed to read publish value.");
	}
}
