#include <iostream>
#include <thread>
#include <chrono>

#include "InputArguments.h"
#include "Publisher.h"
#include "UserInput.h"

using namespace std;


int main(int argc, char **argv) {
	UA_Boolean restart;
	int retval;

	do {
		restart = false;

		InputData data;
		char tp[] = {"http://opcfoundation.org/UA-Profile/Transport/pubsub-udp-uadp"};
		data.transportProfile = UA_STRING(tp);
		data.serverPort = 4840;
		char url[] = {"opc.udp://224.0.0.22:4840/"};
		data.networkAddressUrl = {UA_STRING_NULL, UA_STRING(url)};

		InputArguments::interpretArguments(argc, argv, data);

	//    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,"URL: " UA_PRINTF_STRING_FORMAT"\n", UA_PRINTF_STRING_DATA(data.networkAddressUrl.url));

		cout << "\nParameters set:" << endl;
		cout << "\tserver port:\t" << data.serverPort << endl;
		cout << "\taddress url:\t" << data.networkAddressUrl.url.data << "\n";
		cout << "\ttp profile:\t" << data.transportProfile.data << "\n\n";


		Publisher pub(&data);

		using namespace std;
		thread t(UserInput::handlingUserInput, &pub, &restart);
		t.detach();


		retval = pub.run();

		if(restart){
			pub.~Publisher();
			Publisher::resetRunning();
		}
		InputArguments::cleanup();
	} while (restart);

    return retval;
}
