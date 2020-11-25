/*
 * common_structures.h
 *
 *  Created on: Nov 5, 2020
 *      Author: minh
 */

#ifndef DEFINES_AND_STRUCTURES_H_
#define DEFINES_AND_STRUCTURES_H_



#include <open62541/types.h>
#include <open62541/types_generated.h>

typedef struct {
	UA_String transportProfile;
	UA_NetworkAddressUrlDataType networkAddressUrl;
	UA_UInt16 serverPort;
} InputData;

typedef struct {
	UA_UInt16 connKey;
	UA_UInt16 wgKey;
	UA_UInt16 dtsKey;
	UA_UInt16 varKey;
} BranchID;

#define DEFAULT_PUBLISH_INTERVAL	100

#define B_CONN					0
#define B_WG					1
#define B_DTS					2
#define B_VAR					3

#define STRING_SIZE				242


#define KEY_NOT_PROVIDED		0
#define KEY_FROM_INPUT			1
#define KEY_FROM_SELECTED		2

#define NO_MATCHING_CMD			3


#define VARIABLE_NUMBER			11
#define WRITERGROUP_NUMBER		12

#define VAR_TYPE_DOUBLE			&UA_TYPES[UA_TYPES_DOUBLE]
#define VAR_TYPE_FLOAT			&UA_TYPES[UA_TYPES_FLOAT]
#define VAR_TYPE_BOOLEAN		&UA_TYPES[UA_TYPES_BOOLEAN]
#define VAR_TYPE_BYTE			&UA_TYPES[UA_TYPES_BYTE]
#define VAR_TYPE_SBYTE			&UA_TYPES[UA_TYPES_SBYTE]
#define VAR_TYPE_INT32			&UA_TYPES[UA_TYPES_INT32]
#define VAR_TYPE_UINT32			&UA_TYPES[UA_TYPES_UINT32]
#define VAR_TYPE_INT16			&UA_TYPES[UA_TYPES_INT16]
#define VAR_TYPE_UINT16			&UA_TYPES[UA_TYPES_UINT16]
#define VAR_TYPE_INT64			&UA_TYPES[UA_TYPES_INT64]
#define VAR_TYPE_UINT64			&UA_TYPES[UA_TYPES_UINT64]
#define VAR_TYPE_STRING			&UA_TYPES[UA_TYPES_STRING]
#define VAR_TYPE_DATETIME		&UA_TYPES[UA_TYPES_DATETIME]

//#define ID_DOUBLE(x)			UA_NODEID_NUMERIC(1, 1000 + (UA_UInt32)(x))
//#define ID_FLOAT(x)			UA_NODEID_NUMERIC(1, 2000 + (UA_UInt32)(x))
//#define ID_BOOLEAN(x)			UA_NODEID_NUMERIC(1, 3000 + (UA_UInt32)(x))
//#define ID_BYTE(x)			UA_NODEID_NUMERIC(1, 4000 + (UA_UInt32)(x))
//#define ID_SBYTE(x)			UA_NODEID_NUMERIC(1, 4500 + (UA_UInt32)(x))
//#define ID_INT32(x)			UA_NODEID_NUMERIC(1, 5000 + (UA_UInt32)(x))
//#define ID_UINT32(x)			UA_NODEID_NUMERIC(1, 6000 + (UA_UInt32)(x))
//#define ID_INT16(x)			UA_NODEID_NUMERIC(1, 7000 + (UA_UInt32)(x))
//#define ID_UINT16(x)			UA_NODEID_NUMERIC(1, 7500 + (UA_UInt32)(x))
//#define ID_INT64(x)			UA_NODEID_NUMERIC(1, 8000 + (UA_UInt32)(x))
//#define ID_UINT64(x)			UA_NODEID_NUMERIC(1, 9000 + (UA_UInt32)(x))

#define ID_DOUBLE				UA_NODEID_NUMERIC(1, 1000)
#define ID_FLOAT				UA_NODEID_NUMERIC(1, 2000)
#define ID_BOOLEAN				UA_NODEID_NUMERIC(1, 3000)
#define ID_BYTE					UA_NODEID_NUMERIC(1, 4000)
#define ID_SBYTE				UA_NODEID_NUMERIC(1, 4500)
#define ID_INT32				UA_NODEID_NUMERIC(1, 5000)
#define ID_UINT32				UA_NODEID_NUMERIC(1, 6000)
#define ID_INT16				UA_NODEID_NUMERIC(1, 7000)
#define ID_UINT16				UA_NODEID_NUMERIC(1, 7500)
#define ID_INT64				UA_NODEID_NUMERIC(1, 8000)
#define ID_UINT64				UA_NODEID_NUMERIC(1, 9000)

#define INDEX_DOUBLE			0
#define INDEX_FLOAT				1
#define INDEX_BOOLEAN			2
#define INDEX_BYTE				3
#define INDEX_SBYTE				4
#define INDEX_INT32				5
#define INDEX_UINT32			6
#define INDEX_INT16				7
#define INDEX_UINT16			8
#define INDEX_INT64				9
#define INDEX_UINT64			10
#define INDEX_DATE				11

#endif /* DEFINES_AND_STRUCTURES_H_ */
