#include <stdio.h>
#include <windows.h>

#include "io.h"

#define STRSIZ 1024
void PrintError(DWORD dwErr, LPCWSTR op, LPCWSTR obj)
{
	if (dwErr != 0) {
		LPWSTR lpBuffer = new WCHAR[STRSIZ];
		if (lpBuffer == NULL) {
			fwprintf_s(stderr, L"Memory allocation error\n");
			return;
		}
		lpBuffer[0] = L'\0';
		if (op) {
			wcsncat_s(lpBuffer, STRSIZ, op, _TRUNCATE);
			wcsncat_s(lpBuffer, STRSIZ, L": ", _TRUNCATE);
		}
		if (obj) {
			wcsncat_s(lpBuffer, STRSIZ, obj, _TRUNCATE);
			wcsncat_s(lpBuffer, STRSIZ, L": ", _TRUNCATE);
		}
		DWORD len = (DWORD)wcslen(lpBuffer);
		if (FormatMessage(
					FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
					NULL,
					dwErr,
					MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),
					lpBuffer + len,
					STRSIZ - len,
					NULL))
			fwprintf_s(stderr, L"%s", lpBuffer);
		delete[] lpBuffer;
	}
}

static DWORD WaitForIO(const HANDLE h, OVERLAPPED *ovl, DWORD *IOBytes, const DWORD *ExpectBytes)
{
	DWORD dwBytes;
	DWORD dwErr = WaitForSingleObject(ovl->hEvent, 1000);
	if (dwErr == WAIT_FAILED) {
		return GetLastError();
	} else if (dwErr == WAIT_ABANDONED) {
		return ERROR_CANCELLED;
	}
	if (!GetOverlappedResult(h, ovl, &dwBytes, FALSE)) {
		return GetLastError();
	}
	if (IOBytes != NULL)
		*IOBytes = dwBytes;
	if ((ExpectBytes != NULL) && (*ExpectBytes != dwBytes)) {
		return ERROR_INVALID_DATA;
	}
	return ERROR_SUCCESS;
}

static DWORD GetDeviceGeometryEx(PDISK_DATA pDiskData)
{
	DWORD dwErr = ERROR_GEN_FAILURE;
	OVERLAPPED ovl = {};

	if ((pDiskData == NULL) || (pDiskData->hDevice == INVALID_HANDLE_VALUE))
		return ERROR_INVALID_PARAMETER;

	pDiskData->pGeometryExBuf = NULL;
	pDiskData->pDiskPartitionInfo = NULL;
	pDiskData->pDiskDetectionInfo = NULL;

	DWORD nGeometryExSize = sizeof(DISK_GEOMETRY_EX) + sizeof(DISK_PARTITION_INFO) +
		sizeof(DISK_DETECTION_INFO);
	PBYTE pGeometryExBuf = new BYTE[nGeometryExSize];
	if (pGeometryExBuf == NULL) {
		dwErr = GetLastError();
		PrintError(dwErr, L"Allocate buffer", NULL);
		goto cleanup;
	}
	SecureZeroMemory(pGeometryExBuf, nGeometryExSize);

	ovl.hEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
	if (ovl.hEvent == NULL) {
		dwErr = GetLastError();
		PrintError(dwErr, L"CreateEvent()", NULL);
		goto cleanup;
	}

	DeviceIoControl(pDiskData->hDevice, IOCTL_DISK_GET_DRIVE_GEOMETRY_EX,
		NULL, 0,
		pGeometryExBuf, nGeometryExSize,
		NULL, &ovl);
	dwErr = GetLastError();
	if ((dwErr != ERROR_SUCCESS) && (dwErr != ERROR_IO_PENDING))
		goto cleanup;
	if ((dwErr = WaitForIO(pDiskData->hDevice, &ovl, NULL, NULL)) != ERROR_SUCCESS) {
		if (dwErr == ERROR_INVALID_DATA)
			dwErr = ERROR_NOT_SUPPORTED;
		goto cleanup;
	}
	dwErr = ERROR_SUCCESS;
	pDiskData->pGeometryExBuf = pGeometryExBuf;
	pDiskData->pDiskPartitionInfo = DiskGeometryGetPartition(pDiskData->pGeometryEx);
	pDiskData->pDiskDetectionInfo = DiskGeometryGetDetect(pDiskData->pGeometryEx);

cleanup:
	if ((dwErr != ERROR_SUCCESS) && (pGeometryExBuf != NULL)) delete[] pGeometryExBuf;
	if (ovl.hEvent != NULL) CloseHandle(ovl.hEvent);

	return dwErr;
}

static DWORD GetDeviceAlignment(PDISK_DATA pDiskData)
{
	DWORD dwErr = ERROR_GEN_FAILURE;
	OVERLAPPED ovl = {};
	STORAGE_PROPERTY_QUERY PropertyQuery = {};

	if ((pDiskData == NULL) || (pDiskData->hDevice == INVALID_HANDLE_VALUE))
		return ERROR_INVALID_PARAMETER;

	pDiskData->pAlignment = NULL;

	PSTORAGE_ACCESS_ALIGNMENT_DESCRIPTOR pAlignment = new STORAGE_ACCESS_ALIGNMENT_DESCRIPTOR;
	if (pAlignment == NULL) {
		dwErr = GetLastError();
		PrintError(dwErr, L"Allocate buffer", NULL);
		goto cleanup;
	}
	SecureZeroMemory(pAlignment, sizeof(STORAGE_ACCESS_ALIGNMENT_DESCRIPTOR));
	pAlignment->Size = sizeof(STORAGE_ACCESS_ALIGNMENT_DESCRIPTOR);

	PropertyQuery.PropertyId = StorageAccessAlignmentProperty;
	PropertyQuery.QueryType = PropertyStandardQuery;

	ovl.hEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
	if (ovl.hEvent == NULL) {
		dwErr = GetLastError();
		PrintError(dwErr, L"CreateEvent()", NULL);
		goto cleanup;
	}

	DeviceIoControl(pDiskData->hDevice, IOCTL_STORAGE_QUERY_PROPERTY,
		&PropertyQuery, sizeof(STORAGE_PROPERTY_QUERY),
		pAlignment, pAlignment->Size,
		NULL, &ovl);
	dwErr = GetLastError();
	if ((dwErr != ERROR_SUCCESS) && (dwErr != ERROR_IO_PENDING))
		goto cleanup;
	// PropertyExistsQuery query do not work, so check for returned data length
	if ((dwErr = WaitForIO(pDiskData->hDevice, &ovl, NULL, &pAlignment->Size)) != ERROR_SUCCESS) {
		if (dwErr == ERROR_INVALID_DATA)
			dwErr = ERROR_NOT_SUPPORTED;
		goto cleanup;
	}
	dwErr = ERROR_SUCCESS;
	pDiskData->pAlignment = pAlignment;

cleanup:
	if ((dwErr != ERROR_SUCCESS) && (pAlignment != NULL)) delete pAlignment;
	if (ovl.hEvent != NULL) CloseHandle(ovl.hEvent);

	return dwErr;
}

static DWORD GetDeviceDescriptor(PDISK_DATA pDiskData)
{
	DWORD dwErr = ERROR_GEN_FAILURE;
	OVERLAPPED ovl = {};
	STORAGE_PROPERTY_QUERY PropertyQuery = {};

	if ((pDiskData == NULL) || (pDiskData->hDevice == INVALID_HANDLE_VALUE))
		return ERROR_INVALID_PARAMETER;

	pDiskData->pDescriptor = NULL;

	STORAGE_DESCRIPTOR_HEADER DescriptorHdr = {};
	DWORD DescriptorHdrSize = sizeof(DescriptorHdr);
	PBYTE pDescriptorBuf = NULL;

	PropertyQuery.PropertyId = StorageDeviceProperty;
	PropertyQuery.QueryType = PropertyStandardQuery;

	ovl.hEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
	if (ovl.hEvent == NULL) {
		dwErr = GetLastError();
		PrintError(dwErr, L"CreateEvent()", NULL);
		goto cleanup;
	}

	DeviceIoControl(pDiskData->hDevice, IOCTL_STORAGE_QUERY_PROPERTY,
		&PropertyQuery, sizeof(STORAGE_PROPERTY_QUERY),
		&DescriptorHdr, DescriptorHdrSize,
		NULL, &ovl);
	dwErr = GetLastError();
	if ((dwErr != ERROR_SUCCESS) && (dwErr != ERROR_IO_PENDING))
		goto cleanup;
	// PropertyExistsQuery query do not work, so check for returned data length
	if ((dwErr = WaitForIO(pDiskData->hDevice, &ovl, NULL, &DescriptorHdrSize)) != ERROR_SUCCESS) {
		if (dwErr == ERROR_INVALID_DATA)
			dwErr = ERROR_NOT_SUPPORTED;
		goto cleanup;
	}

	pDescriptorBuf = new BYTE[DescriptorHdr.Size];
	if (pDescriptorBuf == NULL) {
		dwErr = GetLastError();
		PrintError(dwErr, L"Allocate buffer", NULL);
		goto cleanup;
	}
	SecureZeroMemory(pDescriptorBuf, DescriptorHdr.Size);

	DeviceIoControl(pDiskData->hDevice, IOCTL_STORAGE_QUERY_PROPERTY,
		&PropertyQuery, sizeof(STORAGE_PROPERTY_QUERY),
		pDescriptorBuf, DescriptorHdr.Size,
		NULL, &ovl);
	dwErr = GetLastError();
	if ((dwErr != ERROR_SUCCESS) && (dwErr != ERROR_IO_PENDING))
		goto cleanup;
	// PropertyExistsQuery query do not work, so check for returned data length
	if ((dwErr = WaitForIO(pDiskData->hDevice, &ovl, NULL, &DescriptorHdr.Size)) != ERROR_SUCCESS) {
		if (dwErr == ERROR_INVALID_DATA)
			dwErr = ERROR_NOT_SUPPORTED;
		goto cleanup;
	}
	dwErr = ERROR_SUCCESS;
	pDiskData->pDescriptorBuf = pDescriptorBuf;

cleanup:
	if ((dwErr != ERROR_SUCCESS) && (pDescriptorBuf != NULL)) delete[] pDescriptorBuf;
	if (ovl.hEvent != NULL) CloseHandle(ovl.hEvent);

	return dwErr;
}

DWORD DoIO(PDISK_DATA pDiskData, DWORD uSectors, UINT64 uAt, PBYTE *lpBuffer, DWORD *uIOBytes)
{
	DWORD dwErr = ERROR_SUCCESS;

	if (uIOBytes)
		*uIOBytes = 0;
	if ((pDiskData->szName == NULL) || (wcslen(pDiskData->szName) == 0))
		return ERROR_INVALID_PARAMETER;

	if (pDiskData->ObjType == OBJ_UNKNOWN) {
		// defaults
		if (pDiskData->PSS == 0)
			pDiskData->PSS = DEFAULT_PHYSICAL_SECTOR;
		if (pDiskData->LSS == 0)
			pDiskData->LSS = DEFAULT_LOGICAL_SECTOR;

		// free and cleanup old data
		if (pDiskData->pGeometryExBuf != NULL) delete[] pDiskData->pGeometryExBuf, pDiskData->pGeometryExBuf = NULL;
		pDiskData->pDiskPartitionInfo = NULL;
		pDiskData->pDiskDetectionInfo = NULL;
		if (pDiskData->pAlignment != NULL) delete pDiskData->pAlignment, pDiskData->pAlignment = NULL;
		if (pDiskData->pDescriptorBuf != NULL) delete pDiskData->pDescriptorBuf, pDiskData->pDescriptorBuf = NULL;
		pDiskData->nLastAllocLBA = 0;

		if (pDiskData->hDevice != INVALID_HANDLE_VALUE)	CloseHandle(pDiskData->hDevice);
		pDiskData->hDevice = CreateFile(pDiskData->szName, GENERIC_READ,
				FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
		if (pDiskData->hDevice == INVALID_HANDLE_VALUE) {
			PrintError(dwErr = GetLastError(), L"Open device/file", pDiskData->szName);
			return dwErr;
		}

		BY_HANDLE_FILE_INFORMATION HandleInformation = {};
		if (GetFileInformationByHandle(pDiskData->hDevice, &HandleInformation)) {
			// file
			pDiskData->ObjType = OBJ_FILE;

			pDiskData->pGeometryExBuf = new BYTE[sizeof(DISK_GEOMETRY_EX)]; // expected as array
			if (pDiskData->pGeometryExBuf == NULL) {
				dwErr = GetLastError();
				PrintError(dwErr, L"Allocate buffer", NULL);

				CloseHandle(pDiskData->hDevice), pDiskData->hDevice = INVALID_HANDLE_VALUE;

				return dwErr;
			}
			pDiskData->pGeometryEx->DiskSize.QuadPart = (LONGLONG)HandleInformation.nFileSizeHigh << 32 | HandleInformation.nFileSizeLow;
			pDiskData->pGeometryEx->Geometry.BytesPerSector = pDiskData->LSS;
			fwprintf_s(stderr, L"Geometry and alignment cannot be determined when reading a dump file\n"
							   L"Will try dump file size as disk size (%" FI64 L"d bytes)\n"
							   L"and default sector sizes of logical:%" FI32 L"u, physical:%" FI32 L"u\n",
							   pDiskData->pGeometryEx->DiskSize.QuadPart, pDiskData->LSS, pDiskData->PSS);
		} else {
			// device
			pDiskData->ObjType = OBJ_DISK;

			if ((dwErr = GetDeviceAlignment(pDiskData)) == ERROR_SUCCESS) {
				pDiskData->LSS = (UINT32)pDiskData->pAlignment->BytesPerLogicalSector;
				pDiskData->PSS = (UINT32)pDiskData->pAlignment->BytesPerPhysicalSector;
			} else {
				PrintError(dwErr, L"Query device alignment", pDiskData->szName);
			}
			if ((dwErr = GetDeviceGeometryEx(pDiskData)) == ERROR_SUCCESS) {
				if (pDiskData->pAlignment == NULL) {
					fputws(L"Falling back to device geometry query providing logical sector size only\n", stderr);
					/*fputws(L"Will try to use default physical sector size\n", stderr);*/
					pDiskData->LSS = pDiskData->pGeometryEx->Geometry.BytesPerSector;
				}
			} else {
				PrintError(dwErr, L"Query device geometry", pDiskData->szName);
				fwprintf_s(stderr, L"Will not use CHS related information\n");
				if (pDiskData->pAlignment == NULL)
					fputws(L"Both disk query methods failed, will try to use default sector sizes\n", stderr);
			}
			GetDeviceDescriptor(pDiskData); // optional feature, don't care if it fails
		}
	}

	if (uSectors > 0) {
		OVERLAPPED ovl = {};
		uAt *= pDiskData->LSS;
		ovl.Offset = uAt & 0xFFFFFFFF; // start at
		ovl.OffsetHigh = uAt >> 32;
		UINT64 uSize = uSectors * pDiskData->LSS;

		if ((lpBuffer == NULL) || (uSize > 0xFFFFFFFF))
			return ERROR_INVALID_PARAMETER;
		if ((pDiskData->ObjType == OBJ_FILE) && (uAt >= (UINT64)pDiskData->pGeometryEx->DiskSize.QuadPart)) {
			fwprintf_s(stderr, L"Reached end of dump file while reading %" FI64 L"u bytes at offset %" FI64 L"u bytes\n", uSize, uAt);
			if (uIOBytes != NULL) *uIOBytes = 0;
			return ERROR_HANDLE_EOF;
		}
		if (*lpBuffer != NULL)
			delete[] *lpBuffer;
		*lpBuffer = new BYTE[(DWORD)uSize];
		if (*lpBuffer == NULL) {
			dwErr = GetLastError();
			PrintError(dwErr, L"Allocate Buffer", NULL);
			return dwErr;
		}
		ovl.hEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
		if (ovl.hEvent == NULL) {
			delete[] *lpBuffer, *lpBuffer = NULL;
			dwErr = GetLastError();
			PrintError(dwErr, L"CreateEvent()", NULL);
			return dwErr;
		}
		ReadFile(pDiskData->hDevice, *lpBuffer, (DWORD)uSize, NULL, &ovl);
		dwErr = GetLastError();
		if ((dwErr != ERROR_SUCCESS) && (dwErr != ERROR_IO_PENDING)) {
			delete[] *lpBuffer, *lpBuffer = NULL;
			CloseHandle(ovl.hEvent);
			PrintError(dwErr, L"Read file", pDiskData->szName);
			return dwErr;
		}
		dwErr = WaitForIO(pDiskData->hDevice, &ovl, uIOBytes, NULL);
		if (dwErr != ERROR_SUCCESS) {
			delete[] *lpBuffer, *lpBuffer = NULL; // free the buffer on failure
			PrintError(dwErr, L"Read file", pDiskData->szName);
		}
		CloseHandle(ovl.hEvent);
	}

	return dwErr;
}
