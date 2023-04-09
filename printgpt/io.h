#ifndef _IO_H_
#define _IO_H_

#define FI32 L"I32"
#define FI64 L"I64"

#define DEFAULT_LOGICAL_SECTOR 512
#define DEFAULT_PHYSICAL_SECTOR 512

#define ERROR_CUSTOM 0x20000000 // MSDN: "Bit 29 is reserved for application-defined error codes; no system error code has this bit set."

typedef enum _OBJ_TYPE {
	OBJ_UNKNOWN = 0,
	OBJ_DISK,
	OBJ_FILE
} OBJ_TYPE;

typedef struct {
	LPCWSTR szName;
	OBJ_TYPE ObjType;
	UINT32 PSS;
	UINT32 LSS;
	HANDLE hDevice;
	union {
		PBYTE pGeometryExBuf;
		PDISK_GEOMETRY_EX pGeometryEx;
	};
	PDISK_PARTITION_INFO pDiskPartitionInfo;
	PDISK_DETECTION_INFO pDiskDetectionInfo;
	PSTORAGE_ACCESS_ALIGNMENT_DESCRIPTOR pAlignment;
	union {
		PBYTE pDescriptorBuf;
		PSTORAGE_DEVICE_DESCRIPTOR pDescriptor;
	};
	UINT64 nLastAllocLBA;
} DISK_DATA, *PDISK_DATA;

void PrintError(DWORD dwError, LPCWSTR op, LPCWSTR obj);
// DoIO
// Determines if pDiskData->szName file or device, determines device geometry and alignment when posible,
// allocates buffer and reads specified sector count to it.
// uSectors is optional, when specified(>0), uAt must provide first sector position
// and lpBuffer must not be NULL, uIOBytes is optional (may be NULL)
// if lpBuffer points not to NULL (i.e. already allocated buffer) it will be freed
// do not forget init lpBuffer to NULL before first call to DoIO
// if Alignment points to empty structure, default sector size values will be loaded into it
// Geometry is optional
// function behavior depends on value of pDiskData->objType:
//   OBJ_UNKNOWN: determine object type and also sector sizes if object is device
//   OBJ_DISK or OBJ_FILE: does not determine type and read sectors using provided or default(if not provided) sector sizes
DWORD DoIO(PDISK_DATA pDiskData, DWORD uSectors, UINT64 uAt, PBYTE *lpBuffer, DWORD *uIOBytes);

#endif // _IO_H_
