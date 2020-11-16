/*
 * UserInputThread.h
 *
 *  Created on: Nov 5, 2020
 *      Author: minh
 */

#ifndef USERINPUT_H_
#define USERINPUT_H_

#include "Publisher.h"
#include "TreeTrunk.h"
#include <string>

#define HINT_SELECTION		"date double float bool sbyte byte int uint int16 uint16 int64 uint64"
#define HINT_DISABLE		"disable  <selection> or all"
#define HINT_ENABLE			"enable   <selection> or all"
#define HINT_INTERVAL		"interval <milliseconds> <selection> or all"
#define HINT_SET			"set      <value> <selection>"
#define HINT_GENERAL		"\nNote: It is possible to write multiple commands at once in one line. Commands will be executed from left to right.\n      Also multiple selections are possible."

class UserInput {
	static const char *delim;

	static Publisher *pub;
	static UA_NodeId connId;
	static UA_NodeId wg[];
	static UA_WriterGroupConfig wgConfig[];
	static UA_NodeId varNode[];

//	static UA_Boolean *restart;

	static UA_UInt16 b[];

	static UA_Byte cbLevel;

	static TreeTrunk tree;

	static BranchConnection current_conn;
	static BranchWriterGroup current_wg;
	static BranchDataSet current_dts;
	static BranchField current_field;

	static UA_UInt32 cmdLength;


	static void getOldExample();
	static void printHelpForExamples();
	static void printHelp();

	static void clearInput();
	static int disable(char *ptr);
	static int enable(char *ptr);
	static int set(char *ptr, char* val);
	static int setInterval(char *ptr, char *val);
	static void decode(char *cmdLine);

	static int addDataSetString(char **ptr, char *string);
	static int addDataSet(char **ptr);
	static int addField(const UA_DataType *variableType, char **ptr);
	static int addBranch(char **ptr);
	static int removeBranch(char **ptr);
	static int writeVariable(char **ptr);
	static int updateInterval(char **ptr, UA_UInt16 interval);
	static int disablePublish(char **ptr);
	static int enablePublish(char **ptr);

	static int getStringFromCmd(char *buffer, char *ptr);
	static int writeString(char **pointer, char *ptr);

public:


	UserInput();
	virtual ~UserInput();

	static void handlingUserInput(Publisher *pub, UA_Boolean *restart);

	static void getInput(std::string *s, const char *msg = "Enter something:");
	static void getInputString(std::string *s);
	static UA_Byte getInputByte(const char *msg = "Enter number:");
	static UA_Int16 getInputInt16(const char *msg = "Enter number:");
	static UA_Int32 getInputInt32(const char *msg = "Enter number:");
	static UA_Int64 getInputInt64(const char *msg = "Enter number:");
	static UA_Double getInputDouble(const char *msg = "Enter number:");
};


#endif /* USERINPUT_H_ */
