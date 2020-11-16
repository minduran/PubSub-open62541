/*
 * ParseArguments.cpp
 *
 *  Created on: Nov 4, 2020
 *      Author: minh
 */


#include "InputArguments.h"
#include <iostream>
#include <unistd.h>
#include <fstream>


bool InputArguments::inputSpecified = false;
char *InputArguments::transport_profile;
char *InputArguments::address_url;

InputArguments::InputArguments() {
	// TODO Auto-generated constructor stub

}

InputArguments::~InputArguments() {
	// TODO Auto-generated destructor stub
	cleanup();
}

void InputArguments::cleanup() {
	free(transport_profile);
	free(address_url);
}


int InputArguments::decode(uint index, char **argument, InputData &data) {

	InputData backup = data;

	if(strcmp(argument[index], "-sp") == 0) {
		data.serverPort = atoi(argument[++index]);
//		cout << "server port from input: " << argument[index] << endl;
		if(data.serverPort > 9999 || data.serverPort < 1000) {
			data = backup;
			cout << endl << "\tWarning: Invalid server port input. Default port set!" << endl;
		}
		InputArguments::inputSpecified = true;
		return 0;// return 0 to skip the second argument <parameter> in queue
	}
	else if (strcmp(argument[index], "-cp") == 0) {
		UA_Int16 port = atoi(argument[++index]);
//		cout << "channel port from input: " << argument[index] << endl;
		if(port > 9999 || port < 1000) {
			cout << endl <<"\tWarning: Invalid channel port input! Default port set!" << endl;
		}
		else {
			UA_Byte l = data.networkAddressUrl.url.length;
			UA_Byte i = 1;
			UA_Byte k = 4;
			if(data.networkAddressUrl.url.data[l-1] == '/' ) {
				i++;
				k++;
			}
			while ( i < k + 1) {
				data.networkAddressUrl.url.data[l-i] = argument[index][k-i];
				i++;
			}
		}
		InputArguments::inputSpecified = true;
		return 0;// return 0 to skip the second argument <parameter> in queue
	}
	else if (strcmp(argument[index], "-h") == 0) {
		cout << endl;
		cout << "Arguments:" << endl;
		cout << "\t-sp <server port>" << endl;
		cout << "\t-cp <channel port>" << endl;
		cout << "\t-f  <config file>" << endl;
		cout << "\t-df loads default config file and then you can override with -cp and -sp" << endl;
		cout << endl;
		exit(EXIT_SUCCESS);
	}
	else if (strcmp(argument[index],"-f") == 0) {
		if (argument[++index] == NULL) {
			cout << "\tYou must specify config file path and name!" << endl;
			exit(EXIT_FAILURE);
		}
		InputArguments::inputSpecified = InputArguments::interpretFromFile(argument[index], data);
		return 0;// return 0 to skip the second argument <parameter> in queue
	}
	else if (strcmp(argument[index],"-df") == 0) {
		InputArguments::interpretFromDefaultConfigFile(argument[0], data);
		return 1;// return 1 to avoid skipping next argument in queue
	}
	else if (strcmp(argument[index], "-n") == 0) {
		cout << endl << "usage: " << argument[0] << " <uri> [device]" << endl << endl;
		exit(EXIT_SUCCESS);
	}
	else if (strncmp(argument[index], "opc.udp://", 10) == 0) {
		data.networkAddressUrl.url = UA_STRING(argument[index]);
		InputArguments::inputSpecified = true;
		return 1;// return 1 to avoid skipping next argument in queue
	}
	else if (strncmp(argument[index], "opc.eth://", 10) == 0) {
		transport_profile = strdup("http://opcfoundation.org/UA-Profile/Transport/pubsub-eth-uadp");
        data.transportProfile =
            UA_STRING(transport_profile);
        if (argument[index + 1] == NULL) {
            printf("\nError: UADP/ETH needs an interface name\n");
            exit(EXIT_FAILURE);
        }
        data.networkAddressUrl.networkInterface = UA_STRING(argument[index + 1]);
        data.networkAddressUrl.url = UA_STRING(argument[index]);
		InputArguments::inputSpecified = true;
        return 1;// return 1 to avoid skipping next argument in queue
    }
	return 1;
}




static void findNumber(string &s) {
	s = s.substr(s.find_first_of("0123456789")); //strip none numbers before
	s = s.substr(0, s.find_first_not_of("0123456789")); //strip none numbers after
}

static void stripWhitespace(string &s) {
	UA_Byte i = s.find_first_not_of(" \n\r\t");
	i = s.find_first_not_of(" \n\r\t");
	if (i > 0)
		s = s.substr(0, i);
}

int InputArguments::decodeFile(string &lineText, InputData &data) {

	InputData backup = data;

	if (lineText.find("opc.udp://") != string::npos) {
		stripWhitespace(lineText);
		address_url = strdup(lineText.c_str());
		data.networkAddressUrl.url = UA_STRING(address_url);
//		free(tmp);
		return 0;
	}
	else if (lineText.find("Server port:") != string::npos) {
		string s = lineText.substr(12);
		findNumber(s);
		data.serverPort = atoi(s.c_str());
		if(data.serverPort > 9999 || data.serverPort < 1000) {
			data = backup;
			cout << endl << "\tWarning: Invalid server port! Default port set!" << endl;
		}
		return 0;
	}
	return 1;
}

void InputArguments::interpretArguments(int argc, char **argv, InputData &data) {

	for (int i = 1; i < argc; i++) {
		if(decode(i, argv, data) == 0) {
			i++; //skips second argument <parameter> of combined argument input: -<identifier> <parameter>
		}
	}

	if(!InputArguments::inputSpecified) {
		const char *filename = "default_config.txt";
		InputArguments::interpretFromDefaultConfigFile(argv[0], data, filename);
	}
}

bool InputArguments::interpretFromFile(const char *filename, InputData &data) {
	ifstream file(filename);
	string lineText;
	if (file.is_open()) {
		cout << "Loaded config file: '" << filename << "'" << endl;
		while (getline(file,lineText))
		{
			decodeFile(lineText, data);
		}
		file.close();
		return true;
	}
	cout << "\tWarning: Unable to open config file!"<< endl;
	return false;
}

bool InputArguments::interpretFromDefaultConfigFile(char *programPath, InputData &data,
		const char *filename) {
    UA_Int16 pathLength = strlen(programPath);
	while (--pathLength > 0 && programPath[pathLength] != '/') {
		programPath[pathLength] = '\0';
	}
	char *filepath = (char*)malloc(strlen(programPath) + strlen(filename));
	strcpy(filepath, programPath);
	strcat(filepath, filename);
	filepath[strlen(filepath)] = '\0';
	if (InputArguments::interpretFromFile(filepath, data)) {
		free(filepath);
		return true;
	}
	ofstream configFile(filepath);
	if(configFile.is_open()){
		configFile << data.networkAddressUrl.url.data << endl;
		configFile << "Server port: " << data.serverPort;
		configFile.close();
		cout << "\tNote: Default config file generated in '"<< filepath << "'"<< endl;
		free(filepath);
		return true;
	}
	free(filepath);
	return false;
}
