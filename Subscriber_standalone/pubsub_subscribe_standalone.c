/* This work is licensed under a Creative Commons CCZero 1.0 Universal License.
 * See http://creativecommons.org/publicdomain/zero/1.0/ for more information.
 */

/**
 * IMPORTANT ANNOUNCEMENT
 * The PubSub subscriber API is currently not finished. This examples can be used to receive
 * and print the values, which are published by the tutorial_pubsub_publish example.
 * The following code uses internal API which will be later replaced by the higher-level
 * PubSub subscriber API.
 */

#include <open62541/plugin/log_stdout.h>
#include <open62541/plugin/pubsub.h>
#include <open62541/plugin/pubsub_udp.h>
#include <open62541/server.h>
#include <open62541/server_config_default.h>

#include "ua_pubsub_networkmessage.h"

#include <signal.h>

#ifdef UA_ENABLE_PUBSUB_ETH_UADP
#include <open62541/plugin/pubsub_ethernet.h>
#endif

UA_Boolean running = true;
static void stopHandler(int sign) {
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_SERVER,
                "received ctrl-c");
    running = false;
}

static UA_StatusCode
subscriberListen(UA_PubSubChannel *psc) {
    UA_ByteString buffer;
    UA_StatusCode retval = UA_ByteString_allocBuffer(&buffer, 512);
    if(retval != UA_STATUSCODE_GOOD) {
        UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_SERVER,
                     "Message buffer allocation failed!");
        return retval;
    }

    /* Receive the message. Blocks for 100ms */
    retval = psc->receive(psc, &buffer, NULL, 100);
    if(retval != UA_STATUSCODE_GOOD || buffer.length == 0) {
        /* Workaround!! Reset buffer length. Receive can set the length to zero.
         * Then the buffer is not deleted because no memory allocation is
         * assumed.
         * TODO: Return an error code in 'receive' instead of setting the buf
         * length to zero. */
        buffer.length = 512;
        UA_ByteString_clear(&buffer);
        return UA_STATUSCODE_GOOD;
    }

    /* Decode the message */
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                "Message length: %lu", (unsigned long) buffer.length);
    UA_NetworkMessage networkMessage;
    memset(&networkMessage, 0, sizeof(UA_NetworkMessage));
    size_t currentPosition = 0;
    UA_NetworkMessage_decodeBinary(&buffer, &currentPosition, &networkMessage);
    UA_ByteString_clear(&buffer);

    /* Is this the correct message type? */
    if(networkMessage.networkMessageType != UA_NETWORKMESSAGE_DATASET)
        goto cleanup;

    /* At least one DataSetMessage in the NetworkMessage? */
    if(networkMessage.payloadHeaderEnabled &&
       networkMessage.payloadHeader.dataSetPayloadHeader.count < 1)
        goto cleanup;

    /* Is this a KeyFrame-DataSetMessage? */
    for(size_t j = 0; j < networkMessage.payloadHeader.dataSetPayloadHeader.count; j++) {
        UA_DataSetMessage *dsm = &networkMessage.payload.dataSetPayload.dataSetMessages[j];
        if(dsm->header.dataSetMessageType != UA_DATASETMESSAGE_DATAKEYFRAME)
            continue;

        /* Loop over the fields and print well-known content types */
        for(int i = 0; i < dsm->data.keyFrameData.fieldCount; i++) {
            const UA_DataType *currentType = dsm->data.keyFrameData.dataSetFields[i].value.type;
            if(currentType == &UA_TYPES[UA_TYPES_BYTE]) {
            	UA_Byte value = *(UA_Byte *)dsm->data.keyFrameData.dataSetFields[i].value.data;

            	if (i+1 < dsm->data.keyFrameData.fieldCount && &UA_TYPES[UA_TYPES_BYTE]  == dsm->data.keyFrameData.dataSetFields[i+1].value.type) {
					char *msg = malloc(sizeof(char) * (dsm->data.keyFrameData.fieldCount + 1));
					msg[0] = value;
					UA_Byte it = 1;
					i++;
					while(dsm->data.keyFrameData.dataSetFields[i].value.type == &UA_TYPES[UA_TYPES_BYTE] && i <dsm->data.keyFrameData.fieldCount) {
						value = *(UA_Byte *)dsm->data.keyFrameData.dataSetFields[i].value.data;
						msg[it++] = value;
						i++;
					}
					msg[it] = '\0';
					UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
								"Message content: [String] %d \tReceived data: %s", dsm->data.keyFrameData.fieldCount, msg);
					free(msg);
            	}

            	else {
                UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                            "Message content: [Byte] \tReceived data: %d", value);
            	}

            } else if (currentType == &UA_TYPES[UA_TYPES_SBYTE]) {
			    UA_SByte value = *(UA_SByte *)dsm->data.keyFrameData.dataSetFields[i].value.data;
			    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
						   "Message content: [SByte] \tReceived data: %d", value);
            } else if (currentType == &UA_TYPES[UA_TYPES_INT16]) {
			    UA_Int16 value = *(UA_Int16 *)dsm->data.keyFrameData.dataSetFields[i].value.data;
			    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
						   "Message content: [Int16] \tReceived data: %d", value);
			} else if (currentType == &UA_TYPES[UA_TYPES_INT32]) {
			    UA_Int32 value = *(UA_Int32 *)dsm->data.keyFrameData.dataSetFields[i].value.data;
			    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
						   "Message content: [Int32] \tReceived data: %d", value);
			} else if (currentType == &UA_TYPES[UA_TYPES_INT64]) {
			    UA_Int64 value = *(UA_Int64 *)dsm->data.keyFrameData.dataSetFields[i].value.data;
			    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
						   "Message content: [Int64] \tReceived data: %ld", value);
            } else if (currentType == &UA_TYPES[UA_TYPES_UINT16]) {
                UA_UInt16 value = *(UA_UInt16 *)dsm->data.keyFrameData.dataSetFields[i].value.data;
                UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                            "Message content: [UInt16] \tReceived data: %u", value);
            } else if (currentType == &UA_TYPES[UA_TYPES_UINT32]) {
                UA_UInt32 value = *(UA_UInt32 *)dsm->data.keyFrameData.dataSetFields[i].value.data;
                UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                            "Message content: [UInt32] \tReceived data: %u", value);
            } else if (currentType == &UA_TYPES[UA_TYPES_UINT64]) {
                UA_UInt64 value = *(UA_UInt64 *)dsm->data.keyFrameData.dataSetFields[i].value.data;
                UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                            "Message content: [UInt64] \tReceived data: %lu", value);
            } else if (currentType == &UA_TYPES[UA_TYPES_FLOAT]) {
                UA_Float value = *(UA_Float *)dsm->data.keyFrameData.dataSetFields[i].value.data;
                UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                            "Message content: [Float] \tReceived data: %f", value);
            } else if (currentType == &UA_TYPES[UA_TYPES_DOUBLE]) {
                UA_Double value = *(UA_Double *)dsm->data.keyFrameData.dataSetFields[i].value.data;
                UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                            "Message content: [Double] \tReceived data: %f", value);
            } else if (currentType == &UA_TYPES[UA_TYPES_BOOLEAN]) {
                            UA_Boolean value = *(UA_Boolean *)dsm->data.keyFrameData.dataSetFields[i].value.data;
                            UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                                        "Message content: [Boolean] \tReceived data: %s", value ? "true" : "false");
            } else if (currentType == &UA_TYPES[UA_TYPES_DATETIME]) {
                UA_DateTime value = *(UA_DateTime *)dsm->data.keyFrameData.dataSetFields[i].value.data;
                UA_DateTimeStruct receivedTime = UA_DateTime_toStruct(value);
                UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                            "Message content: [DateTime] \t"
                            "Received data: %02i-%02i-%02i %02i:%02i:%02i.%03i",
                            receivedTime.year, receivedTime.month, receivedTime.day,
                            receivedTime.hour, receivedTime.min, receivedTime.sec, receivedTime.milliSec);
            } else if (currentType == &UA_TYPES[UA_TYPES_STRING]) {
//            	char *msg = malloc(sizeof(char) * dsm->data.keyFrameData.fieldCount);
//            	char msg[1024];
//            	while(currentType == &UA_TYPES[UA_TYPES_STRING]) {
//            		i++;
//            		msg[i] =*(UA_Byte *)dsm->data.keyFrameData.dataSetFields[i].value.data);
//            	}
                UA_Byte value = *(UA_Byte *)dsm->data.keyFrameData.dataSetFields[i].value.data;

                UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                            "Message content: [String] \t"
                            "Received data: %d",
                            value);

//                free(msg);
            }
        }
    }

    cleanup:
    UA_NetworkMessage_clear(&networkMessage);
    return retval;
}

int main(int argc, char **argv) {
    signal(SIGINT, stopHandler);
    signal(SIGTERM, stopHandler);

    UA_PubSubTransportLayer udpLayer = UA_PubSubTransportLayerUDPMP();

    UA_PubSubConnectionConfig connectionConfig;
    memset(&connectionConfig, 0, sizeof(connectionConfig));
    connectionConfig.name = UA_STRING("UADP Connection 1");
    connectionConfig.transportProfileUri =
        UA_STRING("http://opcfoundation.org/UA-Profile/Transport/pubsub-udp-uadp");
    connectionConfig.enabled = UA_TRUE;

    char url[] = {"opc.udp://224.0.0.22:4840/"};
    UA_NetworkAddressUrlDataType networkAddressUrl =
        {UA_STRING_NULL , UA_STRING(url)};

    if (argc > 1) {
		if (strncmp(argv[1], "opc.udp://", 10) == 0) {
			networkAddressUrl.url = UA_STRING(argv[1]);
		}
		else if (strcmp(argv[1], "-cp") == 0) {
			UA_Int16 port = atoi(argv[2]);
	//		cout << "channel port from input: " << argument[index] << endl;
			if(port > 9999 || port < 1000) {
				printf("\n\tWarning: Invalid channel port input! Default port set!\n");
			}
			else {
				UA_Byte l = networkAddressUrl.url.length;
				UA_Byte i = 1;
				UA_Byte k = 4;
				if(networkAddressUrl.url.data[l-1] == '/' ) {
					i++;
					k++;
				}
				while ( i < k + 1) {
					networkAddressUrl.url.data[l-i] = argv[2][k-i];
					i++;
				}
			}
		}
    }
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,"URL: " UA_PRINTF_STRING_FORMAT, UA_PRINTF_STRING_DATA(networkAddressUrl.url));
    UA_Variant_setScalar(&connectionConfig.address, &networkAddressUrl,
                         &UA_TYPES[UA_TYPES_NETWORKADDRESSURLDATATYPE]);


    UA_PubSubChannel *psc =
        udpLayer.createPubSubChannel(&connectionConfig);
    psc->regist(psc, NULL, NULL);

    UA_StatusCode retval = UA_STATUSCODE_GOOD;
    while(running && retval == UA_STATUSCODE_GOOD)
        retval = subscriberListen(psc);

    psc->close(psc);
        
    return 0;
}
