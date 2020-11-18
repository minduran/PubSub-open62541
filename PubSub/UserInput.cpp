/*
 * UserInputThread.cpp
 *
 *  Created on: Nov 5, 2020
 *      Author: minh
 */
#include <open62541/server_pubsub.h>
#include "UserInput.h"
#include <limits>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <fstream>


using namespace std;

//UA_Boolean *UserInput::restart;
Publisher *UserInput::pub;
UA_NodeId UserInput::connId;
UA_NodeId UserInput::wg[WRITERGROUP_NUMBER];
UA_WriterGroupConfig UserInput::wgConfig[WRITERGROUP_NUMBER];
UA_NodeId UserInput::varNode[VARIABLE_NUMBER];
UA_Byte UserInput::cbLevel = 0;
UA_UInt16 UserInput::b[4];
TreeTrunk UserInput::tree;

UA_UInt32 UserInput::cmdLength = 0;

BranchConnection UserInput::current_conn;
BranchWriterGroup UserInput::current_wg;
BranchDataSet UserInput::current_dts;
BranchField UserInput::current_field;

const char *UserInput::delim = " \t";



UserInput::UserInput() {
	// TODO Auto-generated constructor stub

}

UserInput::~UserInput() {
	// TODO Auto-generated destructor stub
}


void UserInput::handlingUserInput(Publisher *pub, UA_Boolean *restart) {

//	UserInput::restart = restart;
	UserInput::pub = pub;
	for(UA_Byte i = 0; i < 4; i++) {
		b[i] = 0;
	}

	TreeTrunk::init(pub);

	this_thread::sleep_for(chrono::milliseconds(100));

	string input;

	while(pub->isRunning()) {
		cout << "\n*****************************************************************************";
		if(b[B_CONN] > 0)
			cout << "\nSelected:\tCH: " << b[B_CONN] << " ";
		if(b[B_WG] > 0)
			cout << "WG: " << b[B_WG] << " ";
		if(b[B_DTS] > 0)
			cout << "DS: " << b[B_DTS] << " ";
		if(b[B_VAR] > 0)
			cout << "FD: " << b[B_VAR] << " ";
		cout << endl;
		cout << "User Input: ";
		getline(cin, input);

		if(input.compare("exit") == 0) {
			tree.deleteAllConnections();
			pub->stopRunning();
			break;
		}
		else if(input.compare("restart") == 0) {
			cout << "\nRestarting..." << endl;
			*restart = true;
			tree.deleteAllConnections();
			pub->stopRunning();
			break;
		}


		char *cmdline = strdup(input.c_str());
		decode(cmdline);
		free(cmdline);
	}
}

/******************DECODE COMMAND*************************************************************/
void UserInput::decode(char *cmdLine) {
	cmdLength = strlen(cmdLine);
	char *ptr = strtok(cmdLine, delim);

	while(ptr != NULL)
	{
		cmdLength -= strlen(ptr);

		if(strcmp(ptr, "disable") == 0) {
			ptr = strtok(NULL, delim);
			if(ptr != NULL) {
				while (ptr != NULL && disable(ptr) == 0) {
					ptr = strtok(NULL, delim);
				}
				continue;
			}
			else {
				cout << "\n   Type: " << HINT_DISABLE << endl;
			}
		}
		else if(strcmp(ptr, "enable") == 0) {
			ptr = strtok(NULL, delim);
			if(ptr != NULL) {
				while (ptr != NULL && enable(ptr) == 0) {
					ptr = strtok(NULL, delim);
				}
				continue;
			}
			else {
				cout << "\n   Type: " << HINT_ENABLE << endl;
			}
		}
		else if(strcmp(ptr, "interval") == 0) {
			ptr = strtok(NULL, delim);
			if(ptr != NULL) {
				char *val = ptr;
				ptr = strtok(NULL, delim);
				if(ptr != NULL) {
					while (ptr != NULL && setInterval(ptr, val) == 0) {
						ptr = strtok(NULL, delim);
					}
					continue;
				}
				else {
					cout << "\n   Type: " << HINT_INTERVAL << endl;
				}
			}
			else {
				cout << "\n   Type: " << HINT_INTERVAL << endl;
			}
		}
		else if(strcmp(ptr, "set") == 0) {
			ptr = strtok(NULL, delim);
			if(ptr != NULL) {
				char *val = ptr;
				ptr = strtok(NULL, delim);
				if(ptr != NULL) {
					while (ptr != NULL && set(ptr, val) == 0) {
						ptr = strtok(NULL, delim);
					}
					continue;
				}
				else {
					cout << "\n   Type: " << HINT_SET << endl;
				}
			}
			else {
				cout << "\n   Type: " << HINT_SET << endl;
			}
		}

		else if(strcmp(ptr, "help") == 0) {
			printHelp();
		}
		else if(strcmp(ptr, "example") == 0) {
			ptr = strtok(NULL, delim);
			if(ptr == NULL)
				continue;

			if(strcmp(ptr, "on") == 0) {
				getOldExample();
				cout << "****************************Examples switched on*****************************\n";
				printHelpForExamples();
			}
			if(strcmp(ptr, "off") == 0) {
				pub->removePubSubConnection(connId);
				cout << "****************************Examples switched off****************************";
			}
			if(strcmp(ptr, "hello") == 0) {
				pub->publishHelloWorld(connId);
				cout << "****************************Publishing Hello World!**************************";
			}

		}



/******************Real deal***********************************************************************/
		else if(strcmp(ptr, "clear") == 0) {
// CSI[2J clears screen, CSI[H moves the cursor to top-left corner
//		    cout << "\x1B[2J\x1B[H";
		    cout << "\x1B[2J";
		}
		else if(strcmp(ptr, "show") == 0) {
			tree.showChannels();
		}
		else if (strcmp(ptr, "load") == 0) {
			ptr = strtok(NULL, delim);

			ifstream file;

			const char *def= "pubsub.txt";
			if (ptr == NULL) {
//				cout << "Specify file location" << endl;
				file.open(def);
			} else {
				file.open(ptr);
				ptr = strtok(NULL, delim);
			}

			string lineText;
			if (file.is_open()) {
				UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Open script file '%s'!", ptr == NULL ? def : ptr);

				while (getline(file,lineText))
				{
					char *script = strdup(lineText.c_str());
					UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Read CMD Line: '%s'", script);
//					cout << script << endl;

					if (!(script[0] == '/' && script[1] == '/'))
						decode(script);

					free(script);
				}
				file.close();
				continue;
			}
			UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Could not open script file '%s'!", ptr == NULL ? def : ptr);

		}

		else if(strcmp(ptr, "add") == 0) {
			ptr = strtok(NULL, delim);

			if (ptr == NULL) {
				cout << "Specify subject to add" << endl;
				continue;
			}
			cmdLength -= strlen(ptr);


			while (ptr != NULL && addBranch(&ptr) < 3)
			{
				if(ptr != NULL && atoi(ptr) == 0)
					continue;
				if (ptr == NULL)
					break;


				ptr = strtok(NULL, delim);
			}

			continue;
		}
		else if(strcmp(ptr, "rm") == 0) {
			ptr = strtok(NULL, delim);

			if (ptr == NULL) {
				cout << "Specify subject to remove" << endl;
				continue;
			}

			while (ptr != NULL && removeBranch(&ptr) < 3)
			{
				if(ptr != NULL && atoi(ptr) == 0)
					continue;
				if (ptr == NULL)
					break;
				ptr = strtok(NULL, delim);
			}

			continue;

		}
		else if(strcmp(ptr, "wrt") == 0) {
			ptr = strtok(NULL, delim);
			if(ptr == NULL) {
				cout << "Specify value" << endl;
				continue;
			}

			while (ptr != NULL) {
				if(writeVariable(&ptr) == KEY_FROM_SELECTED)
					break;
				if(ptr == NULL || atoi(ptr) != 0)
					break;
				ptr = strtok(NULL, delim);
			}

		continue;

		}
		else if(strcmp(ptr, "cb") == 0) {
			cout << "CH: " << b[B_CONN] << " WG: " << b[B_WG] << " DS: " << b[B_DTS] << " FD: " << b[B_VAR] << endl;

//			ptr = strtok(NULL, delim);
//			if(ptr != NULL) {
//				BranchConnection connection;
//				tree.getConnection(atoi(ptr), &connection);
//
//				cout  << "ch "<< connection.wgKeyInc << endl;
//				UA_UInt16 b[4];
//				b[B_CONN] = atoi(ptr);
//
//				ptr = strtok(NULL, delim);
//				if(ptr != NULL) {
//
//					b[B_WG] = atoi(ptr);
//					BranchWriterGroup wg;
//					tree.getWriterGroup(b, &wg);
//
//					cout << "wg "<< wg.dtsKeyInc << endl;
//				}
//			}
		}
		else if(strcmp(ptr, "ch") == 0) {
			ptr = strtok(NULL, delim);
			if(ptr == NULL) {
				continue;
			}
			b[B_CONN] = atoi(ptr);
			b[B_WG] = 0;
			b[B_DTS] = 0;
			b[B_VAR] = 0;
			if(!(b[B_CONN] != 0 && tree.getConnection(b[B_CONN], &current_conn))) {
				b[B_CONN] = 0;
			}
		}
		else if(strcmp(ptr, "wg") == 0) {
			ptr = strtok(NULL, delim);
			if(ptr == NULL) {
				continue;
			}
			b[B_WG] = atoi(ptr);
			b[B_DTS] = 0;
			b[B_VAR] = 0;
			if(!(b[B_CONN] != 0 && b[B_WG] != 0 && tree.getWriterGroup(b, &current_wg))) {
				b[B_WG] = 0;
			}
		}
		else if(strcmp(ptr, "ds") == 0) {
			ptr = strtok(NULL, delim);
			if(ptr == NULL) {
				continue;
			}
			b[B_DTS] = atoi(ptr);
			b[B_VAR] = 0;
			if(!(b[B_CONN] != 0 && b[B_WG] != 0 && b[B_DTS] != 0 && tree.getDataSet(b, &current_dts))) {
				b[B_DTS] = 0;
			}
		}
		else if(strcmp(ptr, "fd") == 0) {
			ptr = strtok(NULL, delim);
			if(ptr == NULL) {
				continue;
			}
			b[B_VAR] = atoi(ptr);
			if(!(b[B_CONN] != 0 && b[B_WG] != 0 && b[B_DTS] != 0 && b[B_VAR] != 0 && tree.getField(b, &current_field))) {
				b[B_VAR] = 0;
			}
		}

		else if(strcmp(ptr, "on") == 0) {

			ptr = strtok(NULL, delim);
			if(ptr != NULL && atoi(ptr) == 0) {
				if(strcmp(ptr,"all") == 0) {
					tree.enableAllWg();
					ptr = strtok(NULL, delim);
					continue;
				}
				else if(strcmp(ptr,"ch") == 0) {
					ptr = strtok(NULL, delim);
					if(ptr == NULL) {
						cout << "Provide Channel ID to update interval of its WriterGroups" << endl;
						continue;
					}
					UA_UInt16 chID;

					while(ptr != NULL && atoi(ptr) != 0) {
						chID = atoi(ptr);
						tree.enableWriterGroupOfChannel(chID);
						ptr = strtok(NULL, delim);
					}

					continue;
				}
				else if(strcmp(ptr,"port") == 0) {
					ptr = strtok(NULL, delim);
					if(ptr == NULL) {
						cout << "Provide Channel port to update interval of its WriterGroups" << endl;
						continue;
					}
					UA_UInt16 port;

					while(ptr != NULL && (port = atoi(ptr)) > 999 && port < 10000) {
						tree.enableWriterGroupOfPort(port);
						ptr = strtok(NULL, delim);
					}

					continue;
				}
			}


			do{
				if(/*nextCmdInQueue*/ enablePublish(&ptr) == KEY_FROM_SELECTED)
					break;

				if(ptr == NULL || atoi(ptr) != 0)
					break;
				ptr = strtok(NULL, delim);
			} while(ptr != NULL);
			continue;
		}
		else if(strcmp(ptr, "off") == 0) {

			ptr = strtok(NULL, delim);
			if(ptr != NULL && atoi(ptr) == 0) {
				if(strcmp(ptr,"all") == 0) {
					tree.disableAllWg();
					ptr = strtok(NULL, delim);
					continue;
				}
				else if(strcmp(ptr,"ch") == 0) {
					ptr = strtok(NULL, delim);
					if(ptr == NULL) {
						cout << "Provide Channel ID to update interval of its WriterGroups" << endl;
						continue;
					}
					UA_UInt16 chID;

					while(ptr != NULL && atoi(ptr) != 0) {
						chID = atoi(ptr);
						tree.disableWriterGroupOfChannel(chID);
						ptr = strtok(NULL, delim);
					}

					continue;
				}
				else if(strcmp(ptr,"port") == 0) {
					ptr = strtok(NULL, delim);
					if(ptr == NULL) {
						cout << "Provide Channel port to update interval of its WriterGroups" << endl;
						continue;
					}
					UA_UInt16 port;

					while(ptr != NULL && (port = atoi(ptr)) > 999 && port < 10000) {
						tree.disableWriterGroupOfPort(port);
						ptr = strtok(NULL, delim);
					}
					continue;
				}
			}


			do {
				if(/*nextCmdInQueue*/ disablePublish(&ptr) == KEY_FROM_SELECTED)
					break;

				if(ptr == NULL || atoi(ptr) != 0)
					break;
				ptr = strtok(NULL, delim);
			} while(ptr != NULL);
			continue;
		}
		else if(strcmp(ptr, "intv") == 0) {
			ptr = strtok(NULL, delim);
			if(ptr == NULL) {
				cout << "Provide interval time" << endl;
				continue;
			}
			else if(strcmp(ptr,"all") == 0) {
				ptr = strtok(NULL, delim);
				if(ptr == NULL) {
					cout << "Provide interval time" << endl;
					continue;
				}
				UA_UInt16 interval = atoi(ptr);
				tree.updateAllWg(interval);
				ptr = strtok(NULL, delim);
				continue;
			}

			UA_UInt16 interval = atoi(ptr);
			ptr = strtok(NULL, delim);


			if(ptr != NULL && strcmp(ptr,"ch") == 0) {
				ptr = strtok(NULL, delim);
				if(ptr == NULL) {
					cout << "Provide Channel ID to update interval of its WriterGroups" << endl;
					continue;
				}
				UA_UInt16 chID;

				while(ptr != NULL && (chID = atoi(ptr)) != 0) {
					tree.updateWriterGroupOfChannel(chID, interval);
					ptr = strtok(NULL, delim);
				}

				continue;
			}
			else if(ptr != NULL && strcmp(ptr,"port") == 0) {
				ptr = strtok(NULL, delim);
				if(ptr == NULL) {
					cout << "Provide Channel port to update interval of its WriterGroups" << endl;
					continue;
				}
				UA_UInt16 port;

				while(ptr != NULL && (port = atoi(ptr)) > 999 && port < 10000) {
					tree.updateWriterGroupOfPort(port, interval);
					ptr = strtok(NULL, delim);
				}

				continue;
			}

			do {
				if(updateInterval(&ptr, interval) == KEY_FROM_SELECTED)
					break;
				if(ptr == NULL || atoi(ptr) != 0)
					break;
				ptr = strtok(NULL, delim);
			} while(ptr!= NULL);

			continue;
		} else if (strcmp(ptr, "wrtstr") == 0) {
			ptr = strtok(NULL, delim);

			if(ptr == NULL || ptr[0] != '"') {
				cout << "Provide string to write" << endl;
				continue;
			}

			char buff[STRING_SIZE];

			UA_UInt16 count = getStringFromCmd(buff, ptr);

			while (count-- > 0) {
				ptr = strtok(NULL, delim);

				if(ptr != NULL && strstr(ptr, "\"") != NULL) {
					ptr = strtok(NULL, delim);
					break;
				}
			}

			do {
				if(writeString(&ptr, buff) == KEY_FROM_SELECTED)
					break;
				if(ptr == NULL)
					break;
				ptr = strtok(NULL, delim);
			} while(ptr != NULL && atoi(ptr) != 0);


			continue;

		} else {
			if(ptr == NULL || ptr[0] != '"') {
				if(ptr != NULL) {
				    cout << "'" << ptr <<"' is an invalid command" << endl;
					ptr = strtok(NULL, delim);
				}
				continue;
			}
			cmdLength += strlen(ptr) + 1;

			char buff[STRING_SIZE];

			UA_UInt16 count = getStringFromCmd(buff, ptr);

			while (count-- > 0) {
				ptr = strtok(NULL, delim);

				if(ptr != NULL && strstr(ptr, "\"") != NULL) {
					ptr = strtok(NULL, delim);
					break;
				}
			}

			do {
				if(writeString(&ptr, buff) == KEY_FROM_SELECTED)
					break;
				if(ptr == NULL)
					break;
				ptr = strtok(NULL, delim);
			} while(ptr != NULL && atoi(ptr) != 0);

			continue;
		}


/****************Next Command in queue*************************************************************/
		ptr = strtok(NULL, delim);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////


/****************Add Branch************************************************************************/
UA_UInt16 getN(char *p) {
	char *buff = (char*)malloc(sizeof(char) * strlen(p) -2);
	if (strncmp(p, "-n=", 3) == 0) {
		memcpy(buff, &p[3], strlen(p) -2);
	}
	UA_UInt16 n = atoi(buff);
	free(buff);
	return n;
}



int UserInput::addBranch(char **ptr) {
	UA_Byte status = NO_MATCHING_CMD;

	if (*ptr == NULL) {
		return 0;
	}

	if (strcmp(*ptr, "ch") == 0) {
		*ptr = strtok(NULL, delim);
		if (*ptr == NULL) {
			cout << "Provide port for adding channel" << endl;
			return 0;
		}
		status = 0;
		UA_UInt16 port;

		while(*ptr != NULL && (port = atoi(*ptr)) != 0) {

			if (port < 1000 || port > 9999) {
				cout << port << " is an invalid port" << endl;
				*ptr = strtok(NULL, delim);
				continue;
			}

			if(tree.createConnection(port)) {
				b[B_CONN] = tree.connKeyInc;
				b[B_WG] = 0;
				b[B_DTS] = 0;
				b[B_VAR] = 0;
				current_conn = tree.conns.at(tree.connKeyInc);
	//			cout << "New added Ch " << current_conn->port << endl;
			}
			*ptr = strtok(NULL, delim);
		}

	}
	else if (strcmp(*ptr, "wg") == 0) {
		*ptr = strtok(NULL, delim);

		UA_Int16 n = 1;
		if (*ptr != NULL && strncmp(*ptr, "-n=", 3) == 0) {
			n = getN(*ptr);
			*ptr = strtok(NULL, delim);
		}

		if (*ptr == NULL && b[B_CONN] == 0) {
			cout << "Specify Channel ID to add WriterGroup" << endl;
			return 0;
		}


		UA_Int16 count;
		UA_UInt16 chID;
		do {
			chID = 0;
			status = KEY_NOT_PROVIDED;
			if (*ptr != NULL) {
				 if(atoi(*ptr) != 0) {
					 chID = atoi(*ptr);
					 status = KEY_FROM_INPUT;
				 } else {
					 status = KEY_FROM_SELECTED;
				 }
			}

			if(status != KEY_FROM_INPUT) {
				if (b[B_CONN] == 0) {
					cout << "Provide channel ID for removing" << endl;
					return 0;
				}
				chID = b[B_CONN];
				status = KEY_FROM_SELECTED;
			}


			map<UA_UInt16, BranchConnection>::iterator it;
			if(tree.getChannel(chID, &it)) {
				count = n;
				while(count-- > 0) {
					if(it->second.addWritergroup()) {
						b[B_CONN] = chID;
						b[B_WG] = it->second.wgKeyInc;
						b[B_DTS] = 0;
						b[B_VAR] = 0;


						current_conn = it->second;
						current_wg = it->second.wg.at(b[B_WG]);
		//				cout << "New added WG " << current_wg->key[B_WG] << endl;

					}
					else {
						break;
					}
				}
				continue;
			}
		} while(status != KEY_FROM_SELECTED && *ptr != NULL && (*ptr = strtok(NULL, delim)) !=  NULL && atoi(*ptr) != 0);
	}


	else if (strcmp(*ptr, "ds") == 0) {
		*ptr = strtok(NULL, delim);

		UA_Int16 n = 1;
		if (*ptr != NULL && strncmp(*ptr, "-n=", 3) == 0) {
			n = getN(*ptr);
			*ptr = strtok(NULL, delim);
		}

		if (*ptr == NULL && b[B_WG] == 0) {
			cout << "Specify WriterGroup ID to add DataSet" << endl;
			return 0;
		}


		UA_UInt16 wgID;
		UA_UInt16 chID;
		UA_Int16 count;
		do {
			wgID = 0;
			chID = 0;
			status = KEY_NOT_PROVIDED;

			if(*ptr != NULL) {
				if(atoi(*ptr) != 0) {
					wgID = atoi(*ptr);
					status = KEY_FROM_INPUT;
				}
				else  {
					status = KEY_FROM_SELECTED;
				}
			}

			if (status != KEY_FROM_INPUT) {
				if (b[B_WG] == 0) {
					cout << "Specify WriterGroup ID to add DataSet to" << endl;
					return 0;
				}
				wgID = b[B_WG];
				status = KEY_FROM_SELECTED;
			}

			if (status == KEY_FROM_INPUT) {
				status = KEY_NOT_PROVIDED;
				*ptr = strtok(NULL, delim);
				if(*ptr != NULL) {
					if(atoi(*ptr) != 0) {
						chID = atoi(*ptr);
						status = KEY_FROM_INPUT;
					}
					else  {
						status = KEY_FROM_SELECTED;
					}
				}
			}


			if (status != KEY_FROM_INPUT) {
				if (b[B_CONN] == 0) {
					cout << "Specify Channel ID to add DataSet to" << endl;
					return 0;
				}
				chID = b[B_CONN];
				status = KEY_FROM_SELECTED;
			}

			map<UA_UInt16, BranchConnection>::iterator it;
			if(tree.getChannel(chID, &it)) {
				map<UA_UInt16, BranchWriterGroup>::iterator itwg;
				if(it->second.getWritergroup(wgID, &itwg)) {
					count = n;
					while(count-- > 0) {
						if(itwg->second.addDataset()) {
							b[B_CONN] = chID;
							b[B_WG] = wgID;
							b[B_DTS] = itwg->second.dtsKeyInc;
							b[B_VAR] = 0;

							current_conn = it->second;
							current_wg = itwg->second;
							current_dts = itwg->second.dts.at(b[B_DTS]);
		//					cout << "New added DS " << current_dts->key[B_DTS] << endl;

						}
						else {
							break;
						}
					}
					continue;
				}
			}

		} while(status != KEY_FROM_SELECTED && *ptr != NULL && (*ptr = strtok(NULL, delim)) !=  NULL && atoi(*ptr) != 0);
	}


	else if (strcmp(*ptr, "byte") == 0) {
		return addField(VAR_TYPE_BYTE, ptr);
	}

	else if (strcmp(*ptr, "sbyte") == 0) {
		return addField(VAR_TYPE_SBYTE, ptr);
	}

	else if (strcmp(*ptr, "bool") == 0) {
		return addField(VAR_TYPE_BOOLEAN, ptr);
	}

	else if (strcmp(*ptr, "double") == 0) {
		return addField(VAR_TYPE_DOUBLE, ptr);
	}

	else if (strcmp(*ptr, "float") == 0) {
		return addField(VAR_TYPE_FLOAT, ptr);
	}

	else if (strcmp(*ptr, "int16") == 0) {
		return addField(VAR_TYPE_INT16, ptr);
	}

	else if (strcmp(*ptr, "int32") == 0) {
		return addField(VAR_TYPE_INT32, ptr);
	}

	else if (strcmp(*ptr, "int64") == 0) {
		return addField(VAR_TYPE_INT64, ptr);
	}

	else if (strcmp(*ptr, "uint16") == 0) {
		return addField(VAR_TYPE_UINT16, ptr);
	}

	else if (strcmp(*ptr, "uint32") == 0) {
		return addField(VAR_TYPE_UINT32, ptr);
	}

	else if (strcmp(*ptr, "uint64") == 0) {
		return addField(VAR_TYPE_UINT64, ptr);
	}

	else if (strcmp(*ptr, "date") == 0) {
		return addField(VAR_TYPE_DATETIME, ptr);
	}

	else if (strcmp(*ptr, "string") == 0) {
		*ptr = strtok(NULL, delim);
		if(*ptr == NULL || *ptr[0] != '"') {
			cout << "Provide string to be added" << endl;
			return 0;
		}
		status = 0;
		char buff[STRING_SIZE];

		UA_UInt16 count = getStringFromCmd(buff, *ptr);

		while (count-- > 0) {
			*ptr = strtok(NULL, delim);

			if(*ptr != NULL && strstr(*ptr, "\"") != NULL) {
				*ptr = strtok(NULL, delim);
				break;
			}
		}

		do {
			if((status = addDataSetString(ptr, buff)) == KEY_FROM_SELECTED)
				break;

			if(*ptr == NULL)
				break;
			*ptr = strtok(NULL, delim);
		} while(*ptr != NULL);

	}


	return status;
}

//////////////////////////////////////////////////////////////////////////////////
/////////////////////////Adding String//////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

int UserInput::getStringFromCmd(char *buff, char *ptr) {
	UA_UInt16 i = 0;
	UA_UInt16 count = 1;
//	cout << ptr << " length " << cmdLength << endl;
	UA_Boolean missingClosure;

	while ((missingClosure = (ptr[++i] != '"')) && i < STRING_SIZE && i < cmdLength - 2) {

		if(ptr[i] == 0) {
			buff[i - 1] = ' ';
		}
		else {

//			if(ptr[i] == '\\' && ptr[i + 1] == 't') {
//				buff[i - 1] = '\t';
//				i++;
//			}
//			else if(ptr[i] == '\\' && ptr[i + 1] == 'n') {
//				buff[i - 1] = '\n';
//				i++;
//			}
//			else {
//				buff[i - 1] = ptr[i];
//			}

			buff[i - 1] = ptr[i];
		}


		if ((ptr[i] == ' ' && ptr[i - 1] != ' ') || ptr[i] == 0 || ptr[i] == '\t' || ptr[i] == '\0')
			count++;

	}
	buff[i - 1] = '\0';

	if(missingClosure) {
		UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Sign \" missing or string too long (max %u chars allowed) -> empty String set", STRING_SIZE - 1);
		buff[0]= '\0';
	}

	return count;
}

int UserInput::addDataSetString(char **ptr, char *string) {
	UA_UInt16 wgID = 0;
	UA_UInt16 chID = 0;
	UA_Byte status = KEY_NOT_PROVIDED;


	if(*ptr != NULL) {
		if(atoi(*ptr) != 0) {
			wgID = atoi(*ptr);
			status = KEY_FROM_INPUT;
		}
		else  {
			status = KEY_FROM_SELECTED;
		}
	}

	if (status != KEY_FROM_INPUT) {
		if (b[B_WG] == 0) {
			cout << "Specify WriterGroup ID to add DataSet to" << endl;
			return 0;
		}
		wgID = b[B_WG];
		status = KEY_FROM_SELECTED;
	}

	if (status == KEY_FROM_INPUT) {
		status = KEY_NOT_PROVIDED;
		*ptr = strtok(NULL, delim);
		if(*ptr != NULL) {
			if(atoi(*ptr) != 0) {
				chID = atoi(*ptr);
				status = KEY_FROM_INPUT;
			}
			else  {
				status = KEY_FROM_SELECTED;
			}
		}
	}


	if (status != KEY_FROM_INPUT) {
		if (b[B_CONN] == 0) {
			cout << "Specify Channel ID to add DataSet to" << endl;
			return 0;
		}
		chID = b[B_CONN];
		status = KEY_FROM_SELECTED;
	}

	map<UA_UInt16, BranchConnection>::iterator it;
	if(tree.getChannel(chID, &it)) {
		map<UA_UInt16, BranchWriterGroup>::iterator itwg;
		if(it->second.getWritergroup(wgID, &itwg)) {
			if(itwg->second.addDatasetString()) {
				b[B_CONN] = chID;
				b[B_WG] = wgID;
				b[B_DTS] = itwg->second.dtsKeyInc;

				itwg->second.dts.at(b[B_DTS]).writeString(string);

				return status;
			}
		}
	}

	return status;
}


int UserInput::writeString(char **ptr, char *string) {
	UA_UInt16 dsID = 0;
	UA_UInt16 wgID = 0;
	UA_UInt16 chID = 0;
	UA_Byte status = KEY_NOT_PROVIDED;


	if(*ptr != NULL) {
		if(atoi(*ptr) != 0) {
			dsID = atoi(*ptr);
			status = KEY_FROM_INPUT;
		}
		else  {
			status = KEY_FROM_SELECTED;
		}
	}

	if (status != KEY_FROM_INPUT) {
		if (b[B_DTS] == 0) {
			cout << "Specify DataSet ID to write String" << endl;
			return 0;
		}
		dsID = b[B_DTS];
		status = KEY_FROM_SELECTED;
	}

	if (status == KEY_FROM_INPUT) {
		status = KEY_NOT_PROVIDED;
		*ptr = strtok(NULL, delim);
		if(*ptr != NULL) {
			if(atoi(*ptr) != 0) {
				wgID = atoi(*ptr);
				status = KEY_FROM_INPUT;
			}
			else  {
				status = KEY_FROM_SELECTED;
			}
		}
	}


	if (status != KEY_FROM_INPUT) {
		if (b[B_WG] == 0) {
			cout << "Specify WriterGroup ID to write String" << endl;
			return 0;
		}
		wgID = b[B_WG];
		status = KEY_FROM_SELECTED;
	}

	if (status == KEY_FROM_INPUT) {
		status = KEY_NOT_PROVIDED;
		*ptr = strtok(NULL, delim);
		if(*ptr != NULL) {
			if(atoi(*ptr) != 0) {
				chID = atoi(*ptr);
				status = KEY_FROM_INPUT;
			}
			else  {
				status = KEY_FROM_SELECTED;
			}
		}
	}

	if (status !=  KEY_FROM_INPUT) {
		if (b[B_CONN] == 0) {
			cout << "Specify Channel ID to write String" << endl;
			return 0;
		}
		chID = b[B_CONN];
	}

	map<UA_UInt16, BranchConnection>::iterator it;
	if(tree.getChannel(chID, &it)) {
		map<UA_UInt16, BranchWriterGroup>::iterator itwg;
		if(it->second.getWritergroup(wgID, &itwg)) {
			map<UA_UInt16, BranchDataSet>::iterator itdts;
			if(itwg->second.getDataset(dsID, &itdts)) {
				itdts->second.writeString(string);
				return status;
			}
		}
	}
	return status;
}



int UserInput::addField(const UA_DataType *variableType, char **ptr) {
	*ptr = strtok(NULL, delim);

	UA_Int16 n = 1;
	if (*ptr != NULL && strncmp(*ptr, "-n=", 3) == 0) {
		n = getN(*ptr);
		*ptr = strtok(NULL, delim);
	}

	UA_UInt16 dsID;
	UA_UInt16 wgID;
	UA_UInt16 chID;
	UA_Byte status;
	UA_Int16 count;
	do {
		dsID = 0;
		wgID = 0;
		chID = 0;
		status = KEY_NOT_PROVIDED;

		if(*ptr != NULL) {
			if(atoi(*ptr) != 0) {
				dsID = atoi(*ptr);
				status = KEY_FROM_INPUT;
			}
			else  {
				status = KEY_FROM_SELECTED;
			}
		}

		if (status != KEY_FROM_INPUT) {
			if (b[B_DTS] == 0) {
				cout << "Specify DataSet ID to add DataField to" << endl;
				return 0;
			}
			dsID = b[B_DTS];
			status = KEY_FROM_SELECTED;
		}

		if (status == KEY_FROM_INPUT) {
			status = KEY_NOT_PROVIDED;
			*ptr = strtok(NULL, delim);
			if(*ptr != NULL) {
				if(atoi(*ptr) != 0) {
					wgID = atoi(*ptr);
					status = KEY_FROM_INPUT;
				}
				else  {
					status = KEY_FROM_SELECTED;
				}
			}
		}


		if (status != KEY_FROM_INPUT) {
			if (b[B_WG] == 0) {
				cout << "Specify WriterGroup ID to add DataField to" << endl;
				return 0;
			}
			wgID = b[B_WG];
			status = KEY_FROM_SELECTED;
		}

		if (status == KEY_FROM_INPUT) {
			status = KEY_NOT_PROVIDED;
			*ptr = strtok(NULL, delim);
			if(*ptr != NULL) {
				if(atoi(*ptr) != 0) {
					chID = atoi(*ptr);
					status = KEY_FROM_INPUT;
				}
				else  {
					status = KEY_FROM_SELECTED;
				}
			}
		}

		if (status !=  KEY_FROM_INPUT) {
			if (b[B_CONN] == 0) {
				cout << "Specify Channel ID to add DataField to" << endl;
				return 0;
			}
			chID = b[B_CONN];
		}

		map<UA_UInt16, BranchConnection>::iterator it;
		if(tree.getChannel(chID, &it)) {
			map<UA_UInt16, BranchWriterGroup>::iterator itwg;
			if(it->second.getWritergroup(wgID, &itwg)) {

				map<UA_UInt16, BranchDataSet>::iterator itdts;
				if(itwg->second.getDataset(dsID, &itdts)) {

					if (itdts->second.isString) {
						UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Forbidden to directly manipulate String DataSet");
						continue;
					}

					count = n;
					while(count-- > 0) {
						if(itdts->second.addData(variableType)) {
							b[B_CONN] = chID;
							b[B_WG] = wgID;
							b[B_DTS] = dsID;
							b[B_VAR] = itdts->second.varKeyInc;

						}
						else {
							break;
						}
					}
					continue;
				}
			}
		}

	} while(status != KEY_FROM_SELECTED && *ptr != NULL && (*ptr = strtok(NULL, delim)) != NULL && atoi(*ptr) != 0);

	return status;
}


/****************Remove Branch************************************************************************/
int UserInput::removeBranch(char **ptr) {
	UA_Byte status = NO_MATCHING_CMD;


	if (strcmp(*ptr, "ch") == 0) {
		*ptr = strtok(NULL, delim);
			UA_UInt16 chID;

		do {
			chID = 0;
			status = KEY_NOT_PROVIDED;
			if (*ptr != NULL) {
				 if(atoi(*ptr) != 0) {
					 chID = atoi(*ptr);
					 status = KEY_FROM_INPUT;
				 } else {
					 status = KEY_FROM_SELECTED;
				 }
			}

			if(status != KEY_FROM_INPUT) {
				if (b[B_CONN] == 0) {
					cout << "Provide channel ID for removing" << endl;
					return 0;
				}
				chID = b[B_CONN];
				status = KEY_FROM_SELECTED;
			}


			string confirm;
			cout << "Do you really want to delete Channel " << chID << "? y/n ";
			getInputString(&confirm);
			if (confirm.compare("y") != 0) {
				continue;
			}

			if(tree.deleteConnection(chID)) {
				b[B_CONN] = 0;
				b[B_WG] = 0;
				b[B_DTS] = 0;
				b[B_VAR] = 0;
				continue;
			}

		} while(*ptr != NULL && (*ptr = strtok(NULL, delim)) != NULL && atoi(*ptr) != 0 && status != KEY_FROM_SELECTED);

	}

/**********************Remove WG************************************************************************/
	else if (strcmp(*ptr, "wg") == 0) {
		*ptr = strtok(NULL, delim);

		do {

			UA_UInt16 wgID = 0;
			UA_UInt16 chID = 0;
			status = KEY_NOT_PROVIDED;


			if(*ptr != NULL) {
				if(atoi(*ptr) != 0) {
					wgID = atoi(*ptr);
					status = KEY_FROM_INPUT;
				}
				else  {
					status = KEY_FROM_SELECTED;
				}
			}

			if (status != KEY_FROM_INPUT) {
				if (b[B_WG] == 0) {
					cout << "Specify WriterGroup ID for removing" << endl;
					return 0;
				}
				wgID = b[B_WG];
				status = KEY_FROM_SELECTED;
			}

			if (status == KEY_FROM_INPUT) {
				status = KEY_NOT_PROVIDED;
				*ptr = strtok(NULL, delim);
				if(*ptr != NULL) {
					if(atoi(*ptr) != 0) {
						chID = atoi(*ptr);
						status = KEY_FROM_INPUT;
					}
					else  {
						status = KEY_FROM_SELECTED;
					}
				}
			}


			if (status != KEY_FROM_INPUT) {
				if (b[B_CONN] == 0) {
					cout << "Specify Channel ID to remove WriterGroup from" << endl;
					return 0;
				}
				chID = b[B_CONN];
				status = KEY_FROM_SELECTED;
			}

			map<UA_UInt16, BranchConnection>::iterator it;
			if(tree.getChannel(chID, &it)) {

				string confirm;
				cout << "Remove WriterGroup " << wgID << " from Channel " << chID << "? y/n ";
				getInputString(&confirm);
				if (confirm.compare("y") != 0) {
					continue;
				}

				if(it->second.removeWritergroup(wgID)) {
					b[B_CONN] = chID;
					b[B_WG] = 0;
					b[B_DTS] = 0;
					b[B_VAR] = 0;
					continue;
				}
			}
		} while(*ptr != NULL && (*ptr = strtok(NULL, delim)) != NULL && atoi(*ptr) != 0 && status != KEY_FROM_SELECTED);

	}

/*******************Remove Dataset*************************************************/

	else if (strcmp(*ptr, "ds") == 0) {
		*ptr = strtok(NULL, delim);


		do {
			UA_UInt16 dsID = 0;
			UA_UInt16 wgID = 0;
			UA_UInt16 chID = 0;
			status = KEY_NOT_PROVIDED;

			if(*ptr != NULL) {
				if(atoi(*ptr) != 0) {
					dsID = atoi(*ptr);
					status = KEY_FROM_INPUT;
				}
				else  {
					status = KEY_FROM_SELECTED;
				}
			}

			if (status != KEY_FROM_INPUT) {
				if (b[B_DTS] == 0) {
					cout << "Specify DataSet ID for removing" << endl;
					return 0;
				}
				dsID = b[B_DTS];
				status = KEY_FROM_SELECTED;
			}

			if (status == KEY_FROM_INPUT) {
				status = KEY_NOT_PROVIDED;
				*ptr = strtok(NULL, delim);
				if(*ptr != NULL) {
					if(atoi(*ptr) != 0) {
						wgID = atoi(*ptr);
						status = KEY_FROM_INPUT;
					}
					else  {
						status = KEY_FROM_SELECTED;
					}
				}
			}


			if (status != KEY_FROM_INPUT) {
				if (b[B_WG] == 0) {
					cout << "Specify WriterGroup ID to remove DataSet from" << endl;
					return 0;
				}
				wgID = b[B_WG];
				status = KEY_FROM_SELECTED;
			}

			if (status == KEY_FROM_INPUT) {
				status = KEY_NOT_PROVIDED;
				*ptr = strtok(NULL, delim);
				if(*ptr != NULL) {
					if(atoi(*ptr) != 0) {
						chID = atoi(*ptr);
						status = KEY_FROM_INPUT;
					}
					else  {
						status = KEY_FROM_SELECTED;
					}
				}
			}

			if (status !=  KEY_FROM_INPUT) {
				if (b[B_CONN] == 0) {
					cout << "Specify Channel ID to remove DataSet from" << endl;
					return 0;
				}
				chID = b[B_CONN];
			}

			map<UA_UInt16, BranchConnection>::iterator it;
			if(tree.getChannel(chID, &it)) {
			map<UA_UInt16, BranchWriterGroup>::iterator itwg;
				if(it->second.getWritergroup(wgID, &itwg)) {

					string confirm;
					cout << "Remove DataSet " << dsID << " from WriterGroup " << wgID << " of Channel " << chID << "? y/n ";
					getInputString(&confirm);
					if (confirm.compare("y") != 0) {
						continue;
					}

					if(itwg->second.removeDataset(dsID)) {
						b[B_CONN] = chID;
						b[B_WG] = wgID;
						b[B_DTS] = 0;
						b[B_VAR] = 0;

						continue;
					}
				}
			}

		} while(*ptr != NULL && (*ptr = strtok(NULL, delim)) != NULL && atoi(*ptr) != 0 && status != KEY_FROM_SELECTED);

	}

	/*******************Remove Field*************************************************/
	else if (strcmp(*ptr, "fd") == 0) {
		*ptr = strtok(NULL, delim);

		do {
			UA_UInt16 fdID = 0;
			UA_UInt16 dsID = 0;
			UA_UInt16 wgID = 0;
			UA_UInt16 chID = 0;
			status = KEY_NOT_PROVIDED;


			if(*ptr != NULL) {
				if(atoi(*ptr) != 0) {
					fdID = atoi(*ptr);
					status = KEY_FROM_INPUT;
				}
				else  {
					status = KEY_FROM_SELECTED;
				}
			}

			if (status != KEY_FROM_INPUT) {
				if (b[B_DTS] == 0) {
					cout << "Specify Field ID for removing" << endl;
					return 0;
				}
				fdID = b[B_VAR];
				status = KEY_FROM_SELECTED;
			}

			if (status == KEY_FROM_INPUT) {
				status = KEY_NOT_PROVIDED;
				*ptr = strtok(NULL, delim);
				if(*ptr != NULL) {
					if(atoi(*ptr) != 0) {
						dsID = atoi(*ptr);
						status = KEY_FROM_INPUT;
					}
					else  {
						status = KEY_FROM_SELECTED;
					}
				}
			}
			if (status != KEY_FROM_INPUT) {
				if (b[B_DTS] == 0) {
					cout << "Specify DataSet ID to remove Field from" << endl;
					return 0;
				}
				dsID = b[B_DTS];
				status = KEY_FROM_SELECTED;
			}


			if (status == KEY_FROM_INPUT) {
				status = KEY_NOT_PROVIDED;
				*ptr = strtok(NULL, delim);
				if(*ptr != NULL) {
					if(atoi(*ptr) != 0) {
						wgID = atoi(*ptr);
						status = KEY_FROM_INPUT;
					}
					else  {
						status = KEY_FROM_SELECTED;
					}
				}
			}
			if (status != KEY_FROM_INPUT) {
				if (b[B_WG] == 0) {
					cout << "Specify WriterGroup ID to remove DataSet from" << endl;
					return 0;
				}
				wgID = b[B_WG];
				status = KEY_FROM_SELECTED;
			}

			if (status == KEY_FROM_INPUT) {
				status = KEY_NOT_PROVIDED;
				*ptr = strtok(NULL, delim);
				if(*ptr != NULL) {
					if(atoi(*ptr) != 0) {
						chID = atoi(*ptr);
						status = KEY_FROM_INPUT;
					}
					else  {
						status = KEY_FROM_SELECTED;
					}
				}
			}
			if (status !=  KEY_FROM_INPUT) {
				if (b[B_CONN] == 0) {
					cout << "Specify Channel ID to remove DataSet from" << endl;
					return 0;
				}
				chID = b[B_CONN];
			}

			map<UA_UInt16, BranchConnection>::iterator it;
			if(tree.getChannel(chID, &it)) {
				map<UA_UInt16, BranchWriterGroup>::iterator itwg;
				if(it->second.getWritergroup(wgID, &itwg)) {


					map<UA_UInt16, BranchDataSet>::iterator itdts;

					if(itwg->second.getDataset(dsID, &itdts)) {

						if (itdts->second.isString) {
							cout << "Forbidden to directly manipulate String DataSet" << endl;
							continue;
						}

						string confirm;
						cout << "Remove Field " << fdID  << " from DataSet " << dsID << " in WriterGroup " << wgID << " of Channel " << chID << "? y/n ";
						getInputString(&confirm);
						if (confirm.compare("y") != 0) {
							continue;
						}

						if (itdts->second.removeData(fdID)) {
							b[B_CONN] = chID;
							b[B_WG] = wgID;
							b[B_DTS] = dsID;
							b[B_VAR] = 0;

							continue;
						}
					}
				}
			}

		} while(*ptr != NULL && (*ptr = strtok(NULL, delim)) !=  NULL && atoi(*ptr) != 0 && status != KEY_FROM_SELECTED);

	}

	else if (strcmp(*ptr, "all") == 0) {
		*ptr = strtok(NULL, delim);

		string confirm;
		cout << "Remove all Connections? y/n ";
		getInputString(&confirm);
		if (confirm.compare("y") != 0) {
			return 0;
		}

		b[B_CONN] = 0;
		b[B_WG] = 0;
		b[B_DTS] = 0;
		b[B_VAR] = 0;
		tree.deleteAllConnections();
	}

	return status;
}


/****************Write Variable************************************************************************/

int UserInput::writeVariable(char **ptr) {


	UA_UInt16 fdID = 0;
	UA_UInt16 dsID = 0;
	UA_UInt16 wgID = 0;
	UA_UInt16 chID = 0;
	UA_Byte status = KEY_NOT_PROVIDED;

	char *value = *ptr;


	*ptr = strtok(NULL, delim);
	if(*ptr != NULL) {
		if(atoi(*ptr) != 0) {
			fdID = atoi(*ptr);
			status = KEY_FROM_INPUT;
		}
		else  {
			status = KEY_FROM_SELECTED;
		}
	}

	if (status != KEY_FROM_INPUT) {
		if (b[B_DTS] == 0) {
			cout << "Specify Field ID for removing" << endl;
			return 0;
		}
		fdID = b[B_VAR];
		status = KEY_FROM_SELECTED;
	}

	if (status == KEY_FROM_INPUT) {
		status = KEY_NOT_PROVIDED;
		*ptr = strtok(NULL, delim);
		if(*ptr != NULL) {
			if(atoi(*ptr) != 0) {
				dsID = atoi(*ptr);
				status = KEY_FROM_INPUT;
			}
			else  {
				status = KEY_FROM_SELECTED;
			}
		}
	}
	if (status != KEY_FROM_INPUT) {
		if (b[B_DTS] == 0) {
			cout << "Specify DataSet ID to remove Field from" << endl;
			return 0;
		}
		dsID = b[B_DTS];
		status = KEY_FROM_SELECTED;
	}


	if (status == KEY_FROM_INPUT) {
		status = KEY_NOT_PROVIDED;
		*ptr = strtok(NULL, delim);
		if(*ptr != NULL) {
			if(atoi(*ptr) != 0) {
				wgID = atoi(*ptr);
				status = KEY_FROM_INPUT;
			}
			else  {
				status = KEY_FROM_SELECTED;
			}
		}
	}
	if (status != KEY_FROM_INPUT) {
		if (b[B_WG] == 0) {
			cout << "Specify WriterGroup ID to remove DataSet from" << endl;
			return 0;
		}
		wgID = b[B_WG];
		status = KEY_FROM_SELECTED;
	}

	if (status == KEY_FROM_INPUT) {
		status = KEY_NOT_PROVIDED;
		*ptr = strtok(NULL, delim);
		if(*ptr != NULL) {
			if(atoi(*ptr) != 0) {
				chID = atoi(*ptr);
				status = KEY_FROM_INPUT;
			}
			else  {
				status = KEY_FROM_SELECTED;
			}
		}
	}
	if (status !=  KEY_FROM_INPUT) {
		if (b[B_CONN] == 0) {
			cout << "Specify Channel ID to remove DataSet from" << endl;
			return 0;
		}
		chID = b[B_CONN];
	}

	map<UA_UInt16, BranchConnection>::iterator it;
	if(tree.getChannel(chID, &it)) {
		map<UA_UInt16, BranchWriterGroup>::iterator itwg;
		if(it->second.getWritergroup(wgID, &itwg)) {


			map<UA_UInt16, BranchDataSet>::iterator itdts;

			if(itwg->second.getDataset(dsID, &itdts)) {

				map<UA_UInt16, BranchField>::iterator itfd;
				if (itdts->second.getField(fdID, &itfd)) {
					itfd->second.writeValue(value);
					return status;
				}
			}
		}
	}
	return 0;

}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

int UserInput::updateInterval(char **ptr, UA_UInt16 interval) {

	UA_UInt16 wgID = 0;
	UA_UInt16 chID = 0;
	UA_Byte status = KEY_NOT_PROVIDED;


	if(*ptr != NULL) {
		if(atoi(*ptr) != 0) {
			wgID = atoi(*ptr);
			status = KEY_FROM_INPUT;
		}
		else  {
			status = KEY_FROM_SELECTED;
		}
	}

	if (status != KEY_FROM_INPUT) {
		if (b[B_WG] == 0) {
			cout << "Specify WriterGroup ID for setting interval" << endl;
			return 0;
		}
		wgID = b[B_WG];
		status = KEY_FROM_SELECTED;
	}

	if (status == KEY_FROM_INPUT) {
		status = KEY_NOT_PROVIDED;
		*ptr = strtok(NULL, delim);
		if(*ptr != NULL) {
			if(atoi(*ptr) != 0) {
				chID = atoi(*ptr);
				status = KEY_FROM_INPUT;
			}
			else  {
				status = KEY_FROM_SELECTED;
			}
		}
	}


	if (status != KEY_FROM_INPUT) {
		if (b[B_CONN] == 0) {
			cout << "Specify Channel ID for setting interval" << endl;
			return 0;
		}
		chID = b[B_CONN];
		status = KEY_FROM_SELECTED;
	}

	map<UA_UInt16, BranchConnection>::iterator it;
	if(tree.getChannel(chID, &it)) {


		map<UA_UInt16, BranchWriterGroup>::iterator itwg;
		if(it->second.getWritergroup(wgID, &itwg)) {
			itwg->second.update(interval);
//			b[B_CONN] = chID;
//			b[B_WG] = wgID;
//			b[B_DTS] = 0;
//			b[B_VAR] = 0;


			return status;
		}
	}
	return 0;
}

int UserInput::disablePublish(char **ptr) {

	UA_UInt16 wgID = 0;
	UA_UInt16 chID = 0;
	UA_Byte status = KEY_NOT_PROVIDED;


	if(*ptr != NULL) {
		if(atoi(*ptr) != 0) {
			wgID = atoi(*ptr);
			status = KEY_FROM_INPUT;
		}
		else  {
			status = KEY_FROM_SELECTED;
		}
	}

	if (status != KEY_FROM_INPUT) {
		if (b[B_WG] == 0) {
			cout << "Specify WriterGroup ID to disable" << endl;
			return 0;
		}
		wgID = b[B_WG];
		status = KEY_FROM_SELECTED;
	}

	if (status == KEY_FROM_INPUT) {
		status = KEY_NOT_PROVIDED;
		*ptr = strtok(NULL, delim);
		if(*ptr != NULL) {
			if(atoi(*ptr) != 0) {
				chID = atoi(*ptr);
				status = KEY_FROM_INPUT;
			}
			else  {
				status = KEY_FROM_SELECTED;
			}
		}
	}


	if (status != KEY_FROM_INPUT) {
		if (b[B_CONN] == 0) {
			cout << "Specify Channel ID to disable" << endl;
			return 0;
		}
		chID = b[B_CONN];
		status = KEY_FROM_SELECTED;
	}


	map<UA_UInt16, BranchConnection>::iterator it;
	if(tree.getChannel(chID, &it)) {


		map<UA_UInt16, BranchWriterGroup>::iterator itwg;
		if(it->second.getWritergroup(wgID, &itwg)) {
			itwg->second.disable();

//			b[B_CONN] = chID;
//			b[B_WG] = wgID;
//			b[B_DTS] = 0;
//			b[B_VAR] = 0;

			return status;
		}
	}
	return 0;

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

int UserInput::enablePublish(char **ptr) {

	UA_UInt16 wgID = 0;
	UA_UInt16 chID = 0;
	UA_Byte status = KEY_NOT_PROVIDED;


	if(*ptr != NULL) {
		if(atoi(*ptr) != 0) {
			wgID = atoi(*ptr);
			status = KEY_FROM_INPUT;
		}
		else  {
			status = KEY_FROM_SELECTED;
		}
	}

	if (status != KEY_FROM_INPUT) {
		if (b[B_WG] == 0) {
			cout << "Specify WriterGroup ID enable" << endl;
			return 0;
		}
		wgID = b[B_WG];
		status = KEY_FROM_SELECTED;
	}

	if (status == KEY_FROM_INPUT) {
		status = KEY_NOT_PROVIDED;
		*ptr = strtok(NULL, delim);
		if(*ptr != NULL) {
			if(atoi(*ptr) != 0) {
				chID = atoi(*ptr);
				status = KEY_FROM_INPUT;
			}
			else  {
				status = KEY_FROM_SELECTED;
			}
		}
	}


	if (status != KEY_FROM_INPUT) {
		if (b[B_CONN] == 0) {
			cout << "Specify Channel ID to enable" << endl;
			return 0;
		}
		chID = b[B_CONN];
		status = KEY_FROM_SELECTED;
	}

	map<UA_UInt16, BranchConnection>::iterator it;
	if(tree.getChannel(chID, &it)) {


		map<UA_UInt16, BranchWriterGroup>::iterator itwg;
		if(it->second.getWritergroup(wgID, &itwg)) {
			itwg->second.enable();

//			b[B_CONN] = chID;
//			b[B_WG] = wgID;
//			b[B_DTS] = 0;
//			b[B_VAR] = 0;

			return status;
		}
	}
	return 0;

}



//////////////////WORKING ONLY FOR EXAMPLES/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/******************DISABLE*************************************************************/
int UserInput::disable(char *p) {
	if (strcmp(p, "date") == 0) {
		cout << "Disable publishing DATETIME" << endl;
		UA_Server_setWriterGroupDisabled(pub->server, wg[INDEX_DATE]);
		return 0;
	}
	else if (strcmp(p, "double") == 0) {
		cout << "Disable publishing DOUBLE" << endl;
		UA_Server_setWriterGroupDisabled(pub->server, wg[INDEX_DOUBLE]);
		return 0;
	}
	else if (strcmp(p, "float") == 0) {
		cout << "Disable publishing FLOAT" << endl;
		UA_Server_setWriterGroupDisabled(pub->server, wg[INDEX_FLOAT]);
		return 0;
	}
	else if (strcmp(p, "bool") == 0) {
		cout << "Disable publishing BOOLEAN" << endl;
		UA_Server_setWriterGroupDisabled(pub->server, wg[INDEX_BOOLEAN]);
		return 0;
	}
	else if (strcmp(p, "byte") == 0) {
		cout << "Disable publishing BYTE" << endl;
		UA_Server_setWriterGroupDisabled(pub->server, wg[INDEX_BYTE]);
		return 0;
	}
	else if (strcmp(p, "sbyte") == 0) {
		cout << "Disable publishing SIGNED BYTE" << endl;
		UA_Server_setWriterGroupDisabled(pub->server, wg[INDEX_SBYTE]);
		return 0;
	}
	else if (strcmp(p, "int") == 0 || strcmp(p, "int32") == 0) {
		cout << "Disable publishing INT 32" << endl;
		UA_Server_setWriterGroupDisabled(pub->server, wg[INDEX_INT32]);
		return 0;
	}
	else if (strcmp(p, "uint") == 0 || strcmp(p, "uint32") == 0) {
		cout << "Disable publishing UNSIGNED INT 32" << endl;
		UA_Server_setWriterGroupDisabled(pub->server, wg[INDEX_UINT32]);
		return 0;
	}
	else if (strcmp(p, "int16") == 0) {
		cout << "Disable publishing INT 16" << endl;
		UA_Server_setWriterGroupDisabled(pub->server, wg[INDEX_INT16]);
		return 0;
	}
	else if (strcmp(p, "uint16") == 0) {
		cout << "Disable publishing UNSIGNED INT 16" << endl;
		UA_Server_setWriterGroupDisabled(pub->server, wg[INDEX_UINT16]);
		return 0;
	}
	else if (strcmp(p, "int64") == 0) {
		cout << "Disable publishing INT 64" << endl;
		UA_Server_setWriterGroupDisabled(pub->server, wg[INDEX_INT64]);
		return 0;
	}
	else if (strcmp(p, "uint64") == 0) {
		cout << "Disable publishing UNSIGNED INT 64" << endl;
		UA_Server_setWriterGroupDisabled(pub->server, wg[INDEX_UINT64]);
		return 0;
	}
	else if (strcmp(p, "all") == 0) {
		cout << "Disable all publishing" << endl;
		for(UA_Byte i = 0; i < WRITERGROUP_NUMBER; i++) {
			UA_Server_setWriterGroupDisabled(pub->server, wg[i]);
		}
		return 1;
	}
	return 1;
}


/******************ENABLE*************************************************************/
int UserInput::enable(char *p) {
	if (strcmp(p, "date") == 0) {
		cout << "Enable publishing DATETIME" << endl;
		UA_Server_setWriterGroupOperational(pub->server, wg[INDEX_DATE]);
		return 0;
	}
	else if (strcmp(p, "double") == 0) {
		cout << "Enable publishing DOUBLE" << endl;
		UA_Server_setWriterGroupOperational(pub->server, wg[INDEX_DOUBLE]);
	}
	else if (strcmp(p, "float") == 0) {
		cout << "Enable publishing FLOAT" << endl;
		UA_Server_setWriterGroupOperational(pub->server, wg[INDEX_FLOAT]);
		return 0;
	}
	else if (strcmp(p, "bool") == 0) {
		cout << "Enable publishing BOOLEAN" << endl;
		UA_Server_setWriterGroupOperational(pub->server, wg[INDEX_BOOLEAN]);
		return 0;
	}
	else if (strcmp(p, "byte") == 0) {
		cout << "Enable publishing BYTE" << endl;
		UA_Server_setWriterGroupOperational(pub->server, wg[INDEX_BYTE]);
		return 0;
	}
	else if (strcmp(p, "sbyte") == 0) {
		cout << "Enable publishing SIGNED BYTE" << endl;
		UA_Server_setWriterGroupOperational(pub->server, wg[INDEX_SBYTE]);
		return 0;
	}
	else if (strcmp(p, "int") == 0 || strcmp(p, "int32") == 0) {
		cout << "Enable publishing INT 32" << endl;
		UA_Server_setWriterGroupOperational(pub->server, wg[INDEX_INT32]);
		return 0;
	}
	else if (strcmp(p, "uint") == 0 || strcmp(p, "uint32") == 0) {
		cout << "Enable publishing UNSIGNED INT 32" << endl;
		UA_Server_setWriterGroupOperational(pub->server, wg[INDEX_UINT32]);
		return 0;
	}

	else if (strcmp(p, "int16") == 0) {
		cout << "Enable publishing INT 16" << endl;
		UA_Server_setWriterGroupOperational(pub->server, wg[INDEX_INT16]);
		return 0;
	}
	else if (strcmp(p, "uint16") == 0) {
		cout << "Enable publishing UNSIGNED INT 16" << endl;
		UA_Server_setWriterGroupOperational(pub->server, wg[INDEX_UINT16]);
		return 0;
	}
	else if (strcmp(p, "int64") == 0) {
		cout << "Enable publishing INT 64" << endl;
		UA_Server_setWriterGroupOperational(pub->server, wg[INDEX_INT64]);
		return 0;
	}
	else if (strcmp(p, "uint64") == 0) {
		cout << "Enable publishing UNSIGNED INT 64" << endl;
		UA_Server_setWriterGroupOperational(pub->server, wg[INDEX_UINT64]);
		return 0;
	}

	else if (strcmp(p, "all") == 0) {
		cout << "Enable all publishing" << endl;
		for(UA_Byte i = 0; i < WRITERGROUP_NUMBER; i++) {
			UA_Server_setWriterGroupOperational(pub->server, wg[i]);
		}
		return 1;
	}
	return 1;
}


/******************SET*************************************************************/
int UserInput::set(char *p, char *val) {
	if(strcmp(p, "double") == 0) {
		UA_Double value = (UA_Double) atof(val);
		cout << "DOUBLE set to " << val << endl;
		pub->writeVariable(&value, varNode[INDEX_DOUBLE], VAR_TYPE_DOUBLE);
		return 0;
	}
	else if(strcmp(p, "float") == 0) {
		UA_Float value = (UA_Float) atof(val);
		cout << "FLOAT set to " << val << endl;
		pub->writeVariable(&value, varNode[INDEX_FLOAT], VAR_TYPE_FLOAT);
		return 0;
	}
	else if(strcmp(p, "bool") == 0) {
		static UA_Boolean value = false;
		if (strcmp(val, "true") == 0 || strcmp(val, "1") == 0) {
			cout << "BOOLEAN set to true" << endl;
			value = true;
		}
		else if (strcmp(val, "false") == 0 || strcmp(val, "0") == 0) {
			cout << "BOOLEAN set to false" << endl;
			value = false;
		}
		else {
			cout << "Invalid boolean value!" << endl;
		}
		pub->writeVariable(&value, varNode[INDEX_BOOLEAN], VAR_TYPE_BOOLEAN);
		return 0;
	}
	else if(strcmp(p, "byte") == 0) {
		UA_Byte value = (UA_Byte) atoi(val);
		cout << "BYTE set to " << (UA_Int16) value << endl;
		pub->writeVariable(&value, varNode[INDEX_BYTE], VAR_TYPE_BYTE);
		return 0;
	}
	else if(strcmp(p, "sbyte") == 0) {
		UA_SByte value = (UA_SByte) atoi(val);
		cout << "SIGNED BYTE set to " << (UA_UInt16) value << endl;
		pub->writeVariable(&value, varNode[INDEX_SBYTE], VAR_TYPE_SBYTE);
		return 0;
	}
	else if(strcmp(p, "int") == 0 || strcmp(p, "int32") == 0) {
		UA_Int32 value = (UA_Int32) atoi(val);
		cout << "INT 32 set to " << value << endl;
		pub->writeVariable(&value, varNode[INDEX_INT32], VAR_TYPE_INT32);
		return 0;
	}
	else if(strcmp(p, "uint") == 0 || strcmp(p, "uint32") == 0) {
		UA_UInt32 value = (UA_UInt32) atoi(val);
		cout << "UNSIGNED INT 32 set to " << value << endl;
		pub->writeVariable(&value, varNode[INDEX_UINT32], VAR_TYPE_UINT32);
		return 0;
	}
	else if(strcmp(p, "int16") == 0) {
		UA_Int16 value = (UA_Int16) atoi(val);
		cout << "INT 16 set to " << value << endl;
		pub->writeVariable(&value, varNode[INDEX_INT16], VAR_TYPE_INT16);
		return 0;
	}
	else if(strcmp(p, "uint16") == 0) {
		UA_UInt16 value = (UA_UInt16) atoi(val);
		cout << "UNSIGNED INT 16 set to " << value << endl;
		pub->writeVariable(&value, varNode[INDEX_UINT16], VAR_TYPE_UINT16);
		return 0;
	}
	else if(strcmp(p, "int64") == 0) {
		UA_Int64 value = (UA_Int64) atoi(val);
		cout << "INT 64 set to " << value << endl;
		pub->writeVariable(&value, varNode[INDEX_INT64], VAR_TYPE_INT64);
		return 0;
	}
	else if(strcmp(p, "uint64") == 0) {
		UA_UInt64 value = (UA_UInt64) atoi(val);
		cout << "UNSIGNED INT 64 set to " << value << endl;
		pub->writeVariable(&value, varNode[INDEX_UINT64], VAR_TYPE_UINT64);
		return 0;
	}
	else if(strcmp(p, "all") == 0) {
		cout << "There is no option 'all' for command 'set'" << endl;
		return 0;
	}
	return 1;
}

/******************INTERVAL*************************************************************/
int UserInput::setInterval(char *p, char *val) {
	if (strcmp(p, "date") == 0) {
			wgConfig[INDEX_DATE].publishingInterval = atoi(val);
			cout << "Set publishing interval of DATETIME to " << wgConfig[INDEX_DATE].publishingInterval << "ms" << endl;
			UA_Server_updateWriterGroupConfig(pub->server, wg[INDEX_DATE], &wgConfig[INDEX_DATE]);
			return 0;
		}
	else if(strcmp(p, "double") == 0) {
		wgConfig[INDEX_DOUBLE].publishingInterval = atoi(val);
		cout << "Set publishing interval of DOUBLE to " << wgConfig[INDEX_DOUBLE].publishingInterval << "ms" << endl;
		UA_Server_updateWriterGroupConfig(pub->server, wg[INDEX_DOUBLE], &wgConfig[INDEX_DOUBLE]);
		return 0;
	}
	else if(strcmp(p, "float") == 0) {
		wgConfig[INDEX_FLOAT].publishingInterval = atoi(val);
		cout << "Set publishing interval of FLOAT to " << wgConfig[INDEX_FLOAT].publishingInterval << "ms" << endl;
		UA_Server_updateWriterGroupConfig(pub->server, wg[INDEX_FLOAT], &wgConfig[INDEX_FLOAT]);
		return 0;
	}
	else if(strcmp(p, "bool") == 0) {
		wgConfig[INDEX_BOOLEAN].publishingInterval = atoi(val);
		cout << "Set publishing interval of BOOLEAN to " << wgConfig[INDEX_BOOLEAN].publishingInterval << "ms" << endl;
		UA_Server_updateWriterGroupConfig(pub->server, wg[INDEX_BOOLEAN], &wgConfig[INDEX_BOOLEAN]);
		return 0;
	}
	else if(strcmp(p, "byte") == 0) {
		wgConfig[INDEX_BYTE].publishingInterval = atoi(val);
		cout << "Set publishing interval of BYTE to " << wgConfig[INDEX_BYTE].publishingInterval << "ms" << endl;
		UA_Server_updateWriterGroupConfig(pub->server, wg[INDEX_BYTE], &wgConfig[INDEX_BYTE]);
		return 0;
	}
	else if(strcmp(p, "sbyte") == 0) {
		wgConfig[INDEX_SBYTE].publishingInterval = atoi(val);
		cout << "Set publishing interval of SIGNED BYTE to " << wgConfig[INDEX_SBYTE].publishingInterval << "ms" << endl;
		UA_Server_updateWriterGroupConfig(pub->server, wg[INDEX_BYTE], &wgConfig[INDEX_SBYTE]);
		return 0;
	}
	else if(strcmp(p, "int") == 0 || strcmp(p, "int32") == 0) {
		wgConfig[INDEX_INT32].publishingInterval = atoi(val);
		cout << "Set publishing interval of INT 32 to " << wgConfig[INDEX_INT32].publishingInterval << "ms" << endl;
		UA_Server_updateWriterGroupConfig(pub->server, wg[INDEX_INT32], &wgConfig[INDEX_INT32]);
		return 0;
	}
	else if(strcmp(p, "uint") == 0 || strcmp(p, "uint32") == 0) {
		wgConfig[INDEX_UINT32].publishingInterval = atoi(val);
		cout << "Set publishing interval of UNSIGNED INT 32 to " << wgConfig[INDEX_UINT32].publishingInterval << "ms" << endl;
		UA_Server_updateWriterGroupConfig(pub->server, wg[INDEX_UINT32], &wgConfig[INDEX_UINT32]);
		return 0;
	}
	else if(strcmp(p, "int16") == 0) {
		wgConfig[INDEX_INT16].publishingInterval = atoi(val);
		cout << "Set publishing interval of INT 16 to " << wgConfig[INDEX_INT16].publishingInterval << "ms" << endl;
		UA_Server_updateWriterGroupConfig(pub->server, wg[INDEX_INT16], &wgConfig[INDEX_INT16]);
		return 0;
	}
	else if(strcmp(p, "uint16") == 0) {
		wgConfig[INDEX_UINT16].publishingInterval = atoi(val);
		cout << "Set publishing interval of UNSIGNED INT 16 to " << wgConfig[INDEX_UINT16].publishingInterval << "ms" << endl;
		UA_Server_updateWriterGroupConfig(pub->server, wg[INDEX_UINT16], &wgConfig[INDEX_UINT16]);
		return 0;
	}
	else if(strcmp(p, "int64") == 0) {
		wgConfig[INDEX_INT64].publishingInterval = atoi(val);
		cout << "Set publishing interval of INT 64 to " << wgConfig[INDEX_INT64].publishingInterval << "ms" << endl;
		UA_Server_updateWriterGroupConfig(pub->server, wg[INDEX_INT64], &wgConfig[INDEX_INT64]);
		return 0;
	}
	else if(strcmp(p, "uint64") == 0) {
		wgConfig[INDEX_UINT64].publishingInterval = atoi(val);
		cout << "Set publishing interval of UNSIGNED INT 64 to " << wgConfig[INDEX_UINT64].publishingInterval << "ms" << endl;
		UA_Server_updateWriterGroupConfig(pub->server, wg[INDEX_UINT64], &wgConfig[INDEX_UINT64]);
		return 0;
	}

	else if(strcmp(p, "all") == 0) {
		cout << "Set publishing interval of all to " << atoi(val) << "ms" << endl;
		for(UA_Byte i = 0; i < WRITERGROUP_NUMBER; i++) {
			wgConfig[i].publishingInterval = atoi(val);
			UA_Server_updateWriterGroupConfig(pub->server, wg[i], &wgConfig[i]);
		}
		return 1;
	}
	return 1;
}


void UserInput::clearInput() {
	cin.clear(); //clear bad input flag
	cin.ignore(numeric_limits<streamsize>::max(), '\n'); //discard input
}

void UserInput::getInputString(string *s) {
	//executes loop if the input fails (e.g., no characters were read)
	while (!(cin >> *s)) {
		UserInput::clearInput();
		cout << "Invalid input; please re-enter.\n";
	}
	UserInput::clearInput();
}

void UserInput::getInput(string *s, const char *msg) {
	//executes loop if the input fails (e.g., no characters were read)
	while (cout << endl << msg << " " && !(cin >> *s)) {
		UserInput::clearInput();
		cout << "Invalid input; please re-enter.\n";
	}
	UserInput::clearInput();
}

UA_Byte UserInput::getInputByte(const char *msg) {
	UA_Byte value;
	while (cout << endl << msg << " " && !(cin >> value)) {
		UserInput::clearInput();
		cout << "Invalid input; please re-enter.\n";
	}
	cout << "Accepted Input: " << value << endl;
	UserInput::clearInput();
	return value;
}

UA_Int16 UserInput::getInputInt16(const char *msg) {
	UA_Int16 value;
	while (cout << endl << msg << " " && !(cin >> value)) {
		UserInput::clearInput();
		cout << "Invalid input; please re-enter.\n";
	}
	cout << "Accepted Input: " << value << endl;
	UserInput::clearInput();
	return value;
}

UA_Int32 UserInput::getInputInt32(const char *msg) {
	UA_Int32 value;
	while (cout << endl << msg << " " && !(cin >> value)) {
		UserInput::clearInput();
		cout << "Invalid input; please re-enter.\n";
	}
	cout << "Accepted Input: " << value << endl;
	UserInput::clearInput();
	return value;
}

UA_Int64 UserInput::getInputInt64(const char *msg) {
	UA_Int64 value;
	while (cout << endl << msg << " " && !(cin >> value)) {
		UserInput::clearInput();
		cout << "Invalid input; please re-enter.\n";
	}
	cout << "Accepted Input: " << value << endl;
	UserInput::clearInput();
	return value;
}

UA_Double UserInput::getInputDouble(const char *msg) {
	UA_Double value;
	while (cout << endl << msg << " " && !(cin >> value)) {
		UserInput::clearInput();
		cout << "Invalid input; please re-enter.\n";
	}
	cout << "Accepted Input: " << value << endl;
	UserInput::clearInput();
	return value;
}

void UserInput::getOldExample() {

		pub->setChannelPort(4840);
	//	One PubSub channel
		connId = pub->addPubSubConnection();

	//	One WriterGroup for each variable

	//	Publish DateTime with one of the WriterGroups
		memset(&wg[INDEX_DATE], 0, sizeof(UA_NodeId));
		pub->publishDateTime(1000, wg[INDEX_DATE], connId);

	// 	Adding WriterGroups
		for(UA_Byte i = 0; i < VARIABLE_NUMBER; i++) {
			wg[i] = pub->addWriterGroup(1000, connId);
			memset(&wgConfig[i], 0, sizeof(UA_WriterGroupConfig));
			UA_Server_getWriterGroupConfig(pub->server, wg[i], &wgConfig[i]);
//			UA_Server_setWriterGroupDisabled(pub->server, wg[i]);
		}

	//	Add each basic data type variable in its own DataSet and each DataSet in one of the WriterGroups
		pub->addVarWithOwnDatasetByPointer(VAR_TYPE_DOUBLE, wg[INDEX_DOUBLE], &varNode[INDEX_DOUBLE]);
		pub->addVarWithOwnDatasetByPointer(VAR_TYPE_FLOAT, wg[INDEX_FLOAT], &varNode[INDEX_FLOAT]);
		pub->addVarWithOwnDatasetByPointer(VAR_TYPE_BOOLEAN, wg[INDEX_BOOLEAN], &varNode[INDEX_BOOLEAN]);
		pub->addVarWithOwnDatasetByPointer(VAR_TYPE_BYTE, wg[INDEX_BYTE], &varNode[INDEX_BYTE]);
		pub->addVarWithOwnDatasetByPointer(VAR_TYPE_SBYTE, wg[INDEX_SBYTE], &varNode[INDEX_SBYTE]);
		pub->addVarWithOwnDatasetByPointer(VAR_TYPE_INT32, wg[INDEX_INT32], &varNode[INDEX_INT32]);
		pub->addVarWithOwnDatasetByPointer(VAR_TYPE_UINT32, wg[INDEX_UINT32], &varNode[INDEX_UINT32]);
		pub->addVarWithOwnDatasetByPointer(VAR_TYPE_INT16, wg[INDEX_INT16], &varNode[INDEX_INT16]);
		pub->addVarWithOwnDatasetByPointer(VAR_TYPE_UINT16, wg[INDEX_UINT16], &varNode[INDEX_UINT16]);
		pub->addVarWithOwnDatasetByPointer(VAR_TYPE_INT64, wg[INDEX_INT64], &varNode[INDEX_INT64]);
		pub->addVarWithOwnDatasetByPointer(VAR_TYPE_UINT64, wg[INDEX_UINT64], &varNode[INDEX_UINT64]);
}

void UserInput::printHelpForExamples() {

	cout << "Examples publishing on port 4840" << endl << endl;
	cout << "Selections:" << endl;
	cout << "\t" << HINT_SELECTION << endl;
	cout << "Commands:" << endl;
	cout << "1.\t" << HINT_ENABLE << endl;
	cout << "2.\t" << HINT_DISABLE << endl;
	cout << "3.\t" << HINT_INTERVAL << endl;
	cout << "4.\t" << HINT_SET << endl;
	cout << "5.\thelp     (shows this dialog)" << endl;
	cout << "6. \tclear    (clears terminal)" << endl;
	cout << "4.\trestart  (restarts pubsub)" << endl;
	cout << "8.\texit     (terminates program)" << endl;
	cout << "9.\texample  on/off/hello" << endl;
	cout << HINT_GENERAL << endl;
}

void UserInput::printHelp() {
	cout << "\nCommands:" << endl;
	cout << "1. \thelp     (shows this dialog)" << endl;
	cout << "2. \tclear    (clears terminal)" << endl;
	cout << "3. \trestart  (restarts pubsub)" << endl;
	cout << "4. \texit     (terminates program)" << endl;
	cout << "5. \texample  on/off/hello" << endl;
	cout << endl;
	cout << endl;

	cout << "Selecting Existing Node:" << endl;
	cout << "6. \tch      <ch ID>" << endl;
	cout << "   \twg      <wg ID>" << endl;
	cout << "   \tds      <ds ID>" << endl;
	cout << "   \tfd      <Field ID>" << endl;
	cout << "   Example:    ch 1 wg 2 ds 4 fd 1" << endl;
	cout << endl;
	cout << endl;

	cout << "Adding Node:"<< endl;
	cout << "7. \tadd      ch <port>" << endl;
	cout << "   \tadd      wg -n=<amount> <ch ID>" << endl;
	cout << "   \tadd      ds -n=<amount> <wg ID> <ch ID>" << endl;
	cout << "   \tadd      <selection> -n=<amount> <ds ID> <wg ID> <ch ID>" << endl;
	cout << "   \tadd      ch <port> wg ds <selection>" << endl;
	cout << "   Option:     '-n=<amount>' for adding certain amount of Nodes" << endl;
	cout << "               without this option just adds 1 Node" << endl;
	cout << "   Selection:  byte sbyte bool double float int16 int32 int64 uint16 uint32 uint64 date" << endl;
	cout << "   Example:    add ch 4840 wg ds bool" << endl;
	cout << "               add wg 1 ds 1 1 bool 1 1 1" << endl;
	cout << "               add double 1 1 1" << endl;
	cout << endl;
	cout << endl;

	cout << "Removing Node:"<< endl;
	cout << "8. \trm       ch <ch ID>" << endl;
	cout << "   \trm       wg <wg ID> <ch ID>" << endl;
	cout << "   \trm       ds <ds ID> <wg ID> <ch ID>" << endl;
	cout << "   \trm       fd <selection ID> <ds ID> <wg ID> <ch ID>" << endl;
	cout << "   \trm       all (removes all connections and their nodes)" << endl;
	cout << "   Example:    rm fd 4 1 2 3" << endl;
	cout << endl;
	cout << endl;

	cout << "Configure Publishing:" << endl;
	cout << "9. \ton       <wg ID> <ch ID>" << endl;
	cout << "   \ton       all" << endl;
	cout << "   \ton       ch <ch ID>" << endl;
	cout << "   \ton       port <port>" << endl;
	cout << endl;
	cout << "10.\toff      <wg ID> <ch ID>" << endl;
	cout << "   \toff      all" << endl;
	cout << "   \toff      ch <ch ID>" << endl;
	cout << "   \toff      port <port>" << endl;
	cout << endl;
	cout << "11.\tintv     <milliseconds> <wg ID> <ch ID>" << endl;
	cout << "   \tintv     <milliseconds> ch <ch ID>" << endl;
	cout << "   \tintv     <milliseconds> port <port>" << endl;
	cout << "   \tintv     all <milliseconds>" << endl;
	cout << endl;
	cout << endl;

	cout << "Writting Variables:" << endl;
	cout << "12.\twrt      <value> <fd ID> <ds ID> <wg ID> <ch ID>" << endl;
	cout << endl;
	cout << endl;

	cout << "Adding and Writting String" << endl;
	cout << "13.\tadd      string <wg ID> <ch ID>" << endl;
	cout << endl;
	cout << "14.\twrtstr   <string> <ds ID> <wg ID> <ch ID>" << endl;
	cout << endl;
	cout << "15.\t<string> <ds ID> <wg ID> <ch ID>  (works if string dataset is selected; see selecting commands)" << endl;
	cout << "   Example:    \"hello world!\" 3 2 1" << endl;
	cout << endl;
	cout << endl;

	cout << "Loading Command Script" << endl;
	cout << "16.\tload     <filename>" << endl;


	cout << HINT_GENERAL << endl;
}

