#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <locale.h>

#include "io.h"
#include "data.h"
#include "version.h"

#define DEFAULT_DEVICE L"\\\\.\\PhysicalDrive0"

enum {
	ERR_SUCCESS = 0,
	ERR_SYSTEM,
	ERR_MBR,
	ERR_GPT
};

HANDLE hStdOut = INVALID_HANDLE_VALUE;		// will be set in main
WORD DEF = FWH;								// default console foreground color
BYTE optcolor = 1;							// use colored output by default
BOOL optsi = FALSE;							// use binary units by default

// from Hacker's Delight
UINT32 crc32c(PBYTE message, UINT64 size) {
	UINT64 i;
	UINT32 crc;
	static UINT32 table[256];

	/* Set up the table, if necessary. */

	if (table[1] == 0) {
		UINT32 byte, mask;
		INT8 j;
		for (byte = 0; byte <= 255; byte++) {
			crc = byte;
			for (j = 7; j >= 0; j--) {	// Do eight times.
				mask = -(INT32)(crc & 1);
				crc = (crc >> 1) ^ (0xEDB88320 & mask);
			}
			table[byte] = crc;
		}
	}

	/* Through with table setup, now calculate the CRC. */

	i = 0;
	crc = 0xFFFFFFFF;
	while (i < size) {
		crc = (crc >> 8) ^ table[(crc ^ message[i++]) & 0xFF];
	}
	return ~crc;
}

inline void printGUID(GUID guid)
{
	wprintf_s(L"{%08X-%04hX-%04hX-%02X%02X-%02X%02X%02X%02X%02X%02X}", guid.Data1, guid.Data2,
		guid.Data3, guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
		guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
}

void printalign(PDISK_DATA pDiskData, UINT64 offset) // sectors
{
	offset *= pDiskData->LSS;
	UINT32 align = pDiskData->LSS; // initial align
	UINT32 talign = align;
	while((talign *= 2) <= 2097152UL) { // test up to 2MiB
		if ((offset % talign) != 0)
			break;
		align = talign;
	}

	if (optcolor) {
		if (align < pDiskData->PSS)
			SetConsoleTextAttribute(hStdOut, FRD);
		if (align >= 1048576UL)
			SetConsoleTextAttribute(hStdOut, FGR);
		else
			SetConsoleTextAttribute(hStdOut, FYL);
	}

	if (align == 512U)
		wprintf_s(L"512 bytes");
	else if (align < 1048576UL)
		wprintf_s(L"%" FI32 L"u KiB", align / 1024UL); // using binary only units here
	else
		wprintf_s(L"%" FI32 L"u MiB", align / 1048576UL);
	if (optcolor) SetConsoleTextAttribute(hStdOut, DEF);
}

typedef const struct {
	UINT64 mul;
	LPCWSTR unit;
} CSIZE_UNIT, *PCSIZE_UNIT;
#define ARSZ(a) (sizeof(a)/sizeof(a[0]))
void printhsize(UINT64 size, BOOL si)
{
	LPCWSTR suff = NULL;
	DOUBLE fsize = 0.0;
	static CSIZE_UNIT mul[] =   { { 1152921504606846976ULL, L"EiB" }, { 1125899906842624ULL, L"PiB" },
								{ 1099511627776ULL, L"TiB" }, { 1073741824ULL,  L"GiB" },
								{ 1048576ULL, L"MiB" }, { 1024ULL, L"KiB" }, { 0, NULL } };
	static CSIZE_UNIT mulsi[] = { { 1000000000000000000ULL, L"EB" }, { 1000000000000000ULL, L"PB" },
								{ 1000000000000ULL, L"TB" }, { 1000000000ULL,  L"GB" },
								{ 1000000ULL, L"MB" }, { 1000ULL, L"KB" }, { 0, NULL } };

	for(PCSIZE_UNIT i = si?mulsi:mul; i->mul > 0; i++) {
		if (size >= i->mul) {
			fsize = ((DOUBLE)size) / i->mul;
			suff = i->unit;
			break;
		}
	}
	if (suff)
		wprintf_s(L"%.2f %s ( %" FI64 L"u bytes )", fsize, suff, size);
	else
		wprintf_s(L"%" FI64 L"u bytes", size);
}

void printmbrentry(PDISK_DATA pDiskData, PMBR_ENTRY entry, UINT32 at)
{
	wprintf_s(	  L"    Type:              0x%02X", entry->type);
	if (MBR_TYPES[entry->type].type != NULL) {
		wprintf_s(L" ( %s )", MBR_TYPES[entry->type].type);
	} else if (MBR_TYPES[entry->type].origin != NULL) {
		wprintf_s(L" ( %s )", MBR_TYPES[entry->type].origin);
	}
	wprintf_s(	L"\n    Status:            0x%02X%s"
				L"\n    First LBA:         %" FI32 L"u",
				entry->status,
				entry->status & (1<<7)?L" (active)":L"",  // Bit 7 = active
				at + entry->firstlba);
	if (at > 0)
		wprintf_s(L" (%" FI32 L"u + %" FI32 L"u)", at, entry->firstlba);
	UINT32 lastlba = at + entry->firstlba + entry->sectors - 1;
	wprintf_s(	L"\n*   Last  LBA:         %" FI32 L"u", lastlba);
	wprintf_s(	L"\n    Sectors:           %" FI32 L"u", entry->sectors);
	if (entry->type != 0xEE) { // Not protective MBR
		wprintf_s(L"\n*   Align:             ");
		printalign(pDiskData, at + entry->firstlba);
		if (lastlba > pDiskData->nLastAllocLBA) pDiskData->nLastAllocLBA = lastlba;
	}
	wprintf_s(	L"\n*   Size:              ");
	printhsize((UINT64)entry->sectors * pDiskData->LSS, optsi);
	wprintf_s(	L"\n");
}

DWORD printebr(PDISK_DATA pDiskData, PMBR_ENTRY extended, UINT32 at)
{
	PBYTE buf = NULL;
	DWORD dwErr, read;
	UINT32 nat = 0;
	static UINT extat = 0;
	static UINT et = 0;
	if (extended->firstlba != extat) { // reset partition counter for different extended
		et = 0;
		extat = extended->firstlba;
	}
	if ((dwErr = DoIO(pDiskData, 1, at, &buf, &read)) != ERROR_SUCCESS) {
		return dwErr;
	}
	if (read < pDiskData->LSS) {
		delete[] buf;
		return ERROR_CUSTOM|ERROR_INVALID_DATA;
	}
	PMBR ebr = (PMBR)buf;
	if (ebr->bootsignature != MBR_MAGIC){
		delete[] buf;
		wprintf_s(L"  EBR at %" FI32 L"u: invalid boot signature\n", at);
		return ERROR_CUSTOM|ERROR_INVALID_DATA;
	}
	INT i;
	for (i = 0; i < 4; i++) {
		if (ebr->entries[i].type == 0x00) {
			continue;
		} else if (ebr->entries[i].type == extended->type) { // next ebr
			if (i != 1) {
				wprintf_s(L"  EBR at %" FI32 L"u: next EBR record is not second\n", at);
				dwErr = ERROR_CUSTOM|ERROR_BAD_FORMAT;
			}
			if (nat > 0) {
				wprintf_s(L"  EBR at %" FI32 L"u: multiple next EBR records\n", at);
				dwErr = ERROR_CUSTOM|ERROR_BAD_FORMAT;
			}
			wprintf_s(L"  EBR at %" FI32 L"u: next EBR entry\n", at);
			printmbrentry(pDiskData, &ebr->entries[i], extended->firstlba);
			if ((nat == 0) || (i == 1)) // set nat only if not set or record correctly positioned
				nat = extended->firstlba + ebr->entries[i].firstlba;
		} else {
			if (i != 0) {
				wprintf_s(L"  EBR at %" FI32 L"u: partition record is not first\n", at);
				dwErr = ERROR_CUSTOM|ERROR_BAD_FORMAT;
			}
			et++;
			wprintf_s(L"  EBR at %" FI32 L"u: extended partition entry %u:\n", at, et);
			printmbrentry(pDiskData, &ebr->entries[i], at);
		}
	}
	delete[] buf;
	if (nat > 0) {
		DWORD dwnErr = printebr(pDiskData, extended, nat);
		if (dwnErr != ERROR_SUCCESS) // overwrite previous error code only on error
			dwErr = dwnErr;
		return dwErr;
	}
	if (et == 0)
		wprintf_s(L"  No EBR partition records\n");
	return ERROR_SUCCESS;
}

DWORD printmbr(PDISK_DATA pDiskData, PMBR mbr)
{
	DWORD dwErr = ERROR_SUCCESS;
	wprintf_s(L"MBR:\n  Disk signature:      0x%08" FI32 L"X\n", mbr->disksignature);
	UINT i, t = 0, te = 0;
	for (i = 0; i < 4; i++) {
		if (mbr->entries[i].type == 0)
			continue;
		t++;
		wprintf_s(L"  MBR entry %u:\n", i + 1);
		printmbrentry(pDiskData, &mbr->entries[i], 0);
		if ((mbr->entries[i].type == 0x05) || // extended CHS
			(mbr->entries[i].type == 0x0F)) { // extended LBA
			if (te > 0) {
				wprintf_s(L"MBR has multiple extended records\n");
				dwErr = ERROR_CUSTOM|ERROR_BAD_FORMAT;
			}
			te++;
			DWORD dwnErr = printebr(pDiskData, &mbr->entries[i], mbr->entries[i].firstlba);
			if (dwnErr != ERROR_SUCCESS)
				dwErr = dwnErr;
		}
	}
	if (t < 1)
		wprintf_s(L"  No MBR partition records\n");
	return dwErr;
}

inline void printgptheader(PDISK_DATA pDiskData, PGPT_HEADER hdr)
{
	wprintf_s(L"GPT:\n  Disk GUID:           ");
	printGUID(hdr->diskGUID);
	wprintf_s(	L"\n  Table entry count:   %" FI32 L"u"
				L"\n  Entry size:          %" FI32 L"u"
				L"\n  Entries       LBA:   %" FI64 L"u"
				L"\n  Backup header LBA:   %" FI64 L"u"
				L"\n  First usable  LBA:   %" FI64 L"u"
				L"\n  Last usable   LBA:   %" FI64 L"u"
				L"\n* Total sectors:       %" FI64 L"u"
				L"\n* Total disk size:     ",
				hdr->entrycount, hdr->entrysize, hdr->entrieslba, hdr->backuplba,
				hdr->firstusablelba, hdr->lastusablelba, hdr->backuplba + 1);
	printhsize((hdr->backuplba + 1) * pDiskData->LSS, optsi);
	wprintf_s(	L"\n");
	UINT64 lasthdrlba = hdr->entrieslba + (hdr->entrycount * hdr->entrysize + pDiskData->LSS - 1) / pDiskData->LSS;
	if (lasthdrlba > pDiskData->nLastAllocLBA) pDiskData->nLastAllocLBA = lasthdrlba;
}

void printgptattrs(UINT64 attr, PGPT_TYPE type)
{
	wprintf_s(L"0x%016" FI64 L"X", attr);
	if (attr == 0)
		return;
	wprintf_s(L" (");
	UINT8 i, attrnum = 0;
	for (i = 0; i < 64; i++) {
		if (attr & (1ULL<<i)) {
			if (attrnum)
				wprintf_s(L", ");
			if (i < 48) {						// generic attrs
				if (GPT_GEN_ATTR[i])
					wprintf_s(L"%s", GPT_GEN_ATTR[i]);
				else
					wprintf_s(L"bit %d", i);
			} else {							// type specific
				if (type && type->attr[i - 48])
					wprintf_s(L"%s", type->attr[i - 48]);
				else
					wprintf_s(L"bit %d", i);
			}
			attrnum++;
		}
	}
	wprintf_s(L")");
}

BOOL printgptentry(PDISK_DATA pDiskData, PGPT_ENTRY entry, UINT num)
{
	UINT i;
	PGPT_TYPE type = NULL;
	for (i = 0; i < sizeof(GPT_TYPES)/sizeof(GPT_TYPES[0]); i++) {
		if (memcmp(&entry->typeGUID, &GPT_TYPES[i].typeGUID, sizeof(GUID)) == 0) {
			if (i == 0) // Unused entry, skip it
				return FALSE;
			type = GPT_TYPES + i;
		}
	}
	wprintf_s(L"  GPT entry %u:"
		      L"\n    Type:              ", num + 1);
	printGUID(entry->typeGUID);
	if (type) {
		if (type->os)
			wprintf_s(L" ( %s: %s )", type->os, type->type);
		else
			wprintf_s(L" ( %s )", type->type);
	}
	wprintf_s(L"\n    Unique GUID:       ");
	printGUID(entry->uniqueGUID);
	UINT64 sectors = entry->lastlba - entry->firstlba + 1;
	if (entry->lastlba > pDiskData->nLastAllocLBA) pDiskData->nLastAllocLBA = entry->lastlba;
	wprintf_s(L"\n    First LBA:         %" FI64 L"u"
			  L"\n    Last  LBA:         %" FI64 L"u"
			  L"\n*   Sectors:           %" FI64 L"u"
			  L"\n*   Align:             ", entry->firstlba, entry->lastlba, sectors);
	printalign(pDiskData, entry->firstlba);
	wprintf_s(L"\n*   Size:              ");
	printhsize(sectors * pDiskData->LSS, optsi);
	wprintf_s(L"\n    Attributes:        ");
	printgptattrs(entry->attr, type);
	if (wcslen(entry->name) > 0)
		wprintf_s(L"\n    Name:              %s\n", entry->name);
	else
		wprintf_s(L"\n");
	return TRUE;
}

DWORD printgpt(PDISK_DATA pDiskData, PGPT_HEADER gpthdr)
{
	DWORD dwErr = ERROR_SUCCESS, read;
	PBYTE gptentbuf = NULL;
	PBYTE gptbhdrbuf = NULL;
	PBYTE gptbentbuf = NULL;

	printgptheader(pDiskData, gpthdr);
	// header CRC
	UINT32 oldcrc = gpthdr->hdrcrc;
	gpthdr->hdrcrc = 0;
	UINT32 newcrc = crc32c((PBYTE)gpthdr, gpthdr->hdrsize);
	gpthdr->hdrcrc = oldcrc;
	if (oldcrc == newcrc)
		wprintf_s(L"  Header  CRC:         0x%08" FI32 L"X [match]\n", oldcrc);
	else {
		wprintf_s(L"  Header  CRC:         0x%08" FI32 L"X [mismatch, payload CRC is 0x%08X]\n", oldcrc, newcrc);
		dwErr = ERROR_CUSTOM|ERROR_CRC;
	}

	// read entries
	UINT32 entriessize = gpthdr->entrycount * gpthdr->entrysize;
	UINT32 entrycount = gpthdr->entrycount;
	if (entriessize > 524288) {	// just in case...
		entrycount = 524288 / gpthdr->entrysize;
		entriessize = entrycount * gpthdr->entrysize;
		fwprintf_s(stderr, L"Enormous GPT entries array, processing %" FI32 L"u available entries only\n", entrycount);
		dwErr = ERROR_CUSTOM|ERROR_INVALID_DATA;
	}
	if ((dwErr = DoIO(pDiskData, (DWORD)(entriessize + pDiskData->LSS - 1) / pDiskData->LSS, // occupied sectors count
				gpthdr->entrieslba, &gptentbuf, &read)) != ERROR_SUCCESS) {
		fwprintf_s(stderr, L"Error reading GPT entries data\n");
		goto cleanup;
	}

	if (read < entriessize) {
		fwprintf_s(stderr, L"Not enough GPT entries data"
			L" (disk/dump size should be at least %" FI64 L"u bytes)\n", gpthdr->entrieslba * pDiskData->LSS + entriessize);
		dwErr = ERROR_CUSTOM|ERROR_BAD_LENGTH;
		// available entries count
		entrycount = read / gpthdr->entrysize;
		entriessize = 0;
		if (entrycount < 1) { // not even one entry
			goto cleanup;
		}
		fwprintf_s(stderr, L"Processing %" FI32 L"u available GPT entries only\n", entrycount);
	}
	PGPT_ENTRY gptentries = (PGPT_ENTRY)gptentbuf;

	// entries CRC
	if (entrycount == gpthdr->entrycount) { // only check if all entries data available
		oldcrc = gpthdr->entriescrc;
		gpthdr->entriescrc = 0;
		newcrc = crc32c((PBYTE)gptentries, entriessize);
		gpthdr->entriescrc = oldcrc;
		if (oldcrc == newcrc)
			wprintf_s(L"  Entries CRC:         0x%08" FI32 L"X [match]\n", oldcrc);
		else {
			wprintf_s(L"  Entries CRC:         0x%08" FI32 L"X [mismatch, payload CRC is 0x%08" FI32 L"X]\n", oldcrc, newcrc);
			dwErr = ERROR_CUSTOM|ERROR_CRC;
		}
	} else {
		wprintf_s(L"  Entries CRC:         0x%08" FI32 L"X [not verified]\n", gpthdr->entriescrc);
	}

	// backup header
	DWORD ldwErr;
	if ((ldwErr = DoIO(pDiskData, 1,	gpthdr->backuplba, &gptbhdrbuf, &read)) != ERROR_SUCCESS) {
		fwprintf_s(stderr, L"Error reading GPT backup sector\n");
		if ((pDiskData->ObjType != OBJ_FILE) && (dwErr == ERROR_SUCCESS))
			dwErr = ldwErr;
		goto entries;
	}
	if (read < pDiskData->LSS) {
		fwprintf_s(stderr, L"Not enough backup GPT header data"
			L" (disk/dump size should be at least %" FI64 L"u bytes)\n", (gpthdr->backuplba + 1) * pDiskData->LSS );
		if (dwErr == ERROR_SUCCESS) dwErr = ERROR_CUSTOM|ERROR_BAD_LENGTH;
		goto entries;
	}
	PGPT_HEADER gptbhdr = (PGPT_HEADER)gptbhdrbuf;
	if (gptbhdr->magic != GPT_MAGIC) {
		fwprintf_s(stderr, L"Invalid backup GPT header signature\n");
		if (dwErr == ERROR_SUCCESS) dwErr = ERROR_CUSTOM|ERROR_INVALID_DATA; // don't overwrite previous errors
		goto entries;
	}
	if ((gptbhdr->backuplba != gpthdr->currentlba) || (gptbhdr->firstusablelba != gpthdr->firstusablelba) ||
		(gptbhdr->lastusablelba != gpthdr->lastusablelba) || (gptbhdr->hdrsize != gpthdr->hdrsize) ||
		(gptbhdr->revision != gpthdr->revision) || (memcmp(&gptbhdr->diskGUID, &gpthdr->diskGUID, sizeof(GUID)) != 0)) {
		fwprintf_s(stderr, L"Backup GPT header doesn't match primary header\n");
		if (dwErr == ERROR_SUCCESS) dwErr = ERROR_CUSTOM|ERROR_INVALID_DATA; // don't overwrite previous errors
		goto entries;
	}
	// backup header CRC
	oldcrc = gptbhdr->hdrcrc;
	gptbhdr->hdrcrc = 0;
	newcrc = crc32c((PBYTE)gptbhdr, gptbhdr->hdrsize);
	gptbhdr->hdrcrc = oldcrc;
	if (oldcrc != newcrc) {
		wprintf_s(L"  Backup Header CRC:   0x%08" FI32 L"X [mismatch, payload CRC is 0x%08X]\n", oldcrc, newcrc);
		if (dwErr == ERROR_SUCCESS) dwErr = ERROR_CUSTOM|ERROR_CRC;
		goto entries;
	}
	if (entriessize == 0)
		goto entries; // incomplete primary entries data, don't compare to backup entries
	if ((gptbhdr->entrycount != gpthdr->entrycount) || (gptbhdr->entrysize != gpthdr->entrysize)) {
		fwprintf_s(stderr, L"Backup GPT header entries count doesn't match primary header\n");
		if (dwErr == ERROR_SUCCESS) dwErr = ERROR_CUSTOM|ERROR_INVALID_DATA; // don't overwrite previous errors
		goto entries;
	}
	if ((ldwErr = DoIO(pDiskData, (DWORD)(entriessize + pDiskData->LSS - 1) / pDiskData->LSS, // occupied sectors count
				gptbhdr->entrieslba, &gptbentbuf, &read)) != ERROR_SUCCESS) {
		fwprintf_s(stderr, L"Error reading backup GPT entries data\n");
		if ((pDiskData->ObjType != OBJ_FILE) && (dwErr == ERROR_SUCCESS))
			dwErr = ldwErr;
		goto entries;
	}
	if (read < entriessize) {
		fwprintf_s(stderr, L"Not enough backup GPT entries data"
			L" (disk/dump size should be at least %" FI64 L"u bytes)\n", gptbhdr->entrieslba * pDiskData->LSS + entriessize);
		if (dwErr == ERROR_SUCCESS) dwErr = ERROR_CUSTOM|ERROR_BAD_LENGTH;
		goto entries;
	}
	if ((memcmp(gptbentbuf, gptentbuf, entriessize) != 0) || (gptbhdr->entriescrc != gpthdr->entriescrc)) {
		fwprintf_s(stderr, L"Backup GPT entries data doesn't match primary GPT\n");
		if (dwErr == ERROR_SUCCESS) dwErr = ERROR_CUSTOM|ERROR_INVALID_DATA;
	}

entries:
	UINT t = 0;
	PGPT_ENTRY entry = gptentries;
	for (UINT i = 0; i < entrycount; i++) {
		if (printgptentry(pDiskData, entry, i))
			t++;
		entry = (PGPT_ENTRY)((PBYTE)entry + gpthdr->entrysize);
	}
	if (t < 1)
		wprintf_s(L"  No GPT partition records\n");

cleanup:
	if (gptbentbuf != NULL) delete[] gptbentbuf;
	if (gptbhdrbuf != NULL) delete[] gptbhdrbuf;
	if (gptentbuf != NULL) delete[] gptentbuf;

	return dwErr;
}

void inline printusage(LPCWSTR app)
{
	LPCWSTR sep = wcsrchr(app, '\\');
	if (sep != NULL) {
		app = sep + 1;
	}
	wprintf_s(
		L"printgpt, v%s\n"
		L"Usage: %s [-l <size>] [-p <size>] [-c] [-u[b|s]] [device|file]\n"
		L"  -l: default logical sector size.\n"
		L"  -p: default physical sector size.\n"
		L"  -c: do not colorize output.\n"
		L"  -u: units to use for human redable output, b for binary (default), s for SI.\n"
		L"Return values (errorlevel):\n"
		L"  0: no error.\n"
		L"  1: system error.\n"
		L"  2: MBR error.\n"
		L"  3: GPT error.\n\n"
		L"To access physical device use form of '\\\\.\\PhysicalDriveX'.\n"
		L"If device or file is not specified, \\\\.\\PhysicalDrive0 will be used.\n"
		L"Values with an asterisk mark are calculated values."
		, TEXT(PGPT_VERSION_DISPLAY), app);
}

void CleanDiskData(PDISK_DATA pDiskData)
{
	pDiskData->szName = NULL;
	pDiskData->ObjType = OBJ_UNKNOWN;
	pDiskData->LSS = pDiskData->PSS = 0;
	if (pDiskData->hDevice != INVALID_HANDLE_VALUE) CloseHandle(pDiskData->hDevice), pDiskData->hDevice = INVALID_HANDLE_VALUE;
	if (pDiskData->pGeometryExBuf != NULL) delete[] pDiskData->pGeometryExBuf, pDiskData->pGeometryExBuf = NULL;
	pDiskData->pDiskDetectionInfo = NULL;
	pDiskData->pDiskPartitionInfo = NULL;
	if (pDiskData->pAlignment != NULL) delete pDiskData->pAlignment, pDiskData->pAlignment = NULL;
	if (pDiskData->pDescriptorBuf != NULL) delete pDiskData->pDescriptorBuf, pDiskData->pDescriptorBuf = NULL;
	pDiskData->nLastAllocLBA = 0;
}

LPSTR StrTrimPrintable(LPSTR str)
{
	LPSTR strptr = str;
	LPSTR endptr = NULL;
	CHAR ch;
	while((ch = *str) != '\0') {
		if (isspace(ch)) {
			if (endptr == NULL)
				strptr++;
		} else {
			if (!isprint(ch)) *str = '?';
			endptr = str;
		}
		str++;
	}
	if (endptr != NULL) *(++endptr) = '\0';

	return strptr;
}

INT ProcessDisk(PDISK_DATA pDiskData)
{
	INT ret = ERR_SUCCESS;
	UINT i;
	DWORD dwErr, read;
	PBYTE hdrbuf = NULL;

	if ((dwErr = DoIO(pDiskData, 2, 0, &hdrbuf, &read)) != ERROR_SUCCESS)
		return ERR_SYSTEM;

	PBYTE bufptr = hdrbuf;
	PGPT_HEADER gpthdr = NULL;
	PMBR mbr = NULL;
	wprintf_s(L"\nAccessing %s %s ...\n", pDiskData->ObjType == OBJ_DISK?L"device":L"file", pDiskData->szName);
	if ((pDiskData->ObjType = OBJ_DISK) && (pDiskData->pDescriptor != NULL)) {
		if (pDiskData->pDescriptor->ProductIdOffset != 0) {
			LPSTR descstr = (LPSTR)&pDiskData->pDescriptorBuf[pDiskData->pDescriptor->ProductIdOffset];
			wprintf_s(L" Product ID:           %hs\n", StrTrimPrintable(descstr));
			if (pDiskData->pDescriptor->SerialNumberOffset != 0)
				descstr = (LPSTR)&pDiskData->pDescriptorBuf[pDiskData->pDescriptor->SerialNumberOffset];
				wprintf_s(L" S/N:                  %hs\n", StrTrimPrintable(descstr));
		}
	}
	wprintf_s(L" Logical sector size:  "); printhsize(pDiskData->LSS, FALSE);
	wprintf_s(L"\n%cPhysical sector size: ", pDiskData->pAlignment != NULL?L' ':L'*'); printhsize(pDiskData->PSS, FALSE);
	putwchar(L'\n');
	if (pDiskData->pGeometryEx != NULL) {
		wprintf_s(L" Reported disk size:   ");
		printhsize(pDiskData->pGeometryEx->DiskSize.QuadPart, optsi);
		wprintf_s(L"\n*Logical sectors:      %" FI64 L"d\n", pDiskData->pGeometryEx->DiskSize.QuadPart / pDiskData->LSS);
	}
	for(i = 0; i < 2; i++) { // scan for magic in first 2 logical sectors
		if (read < pDiskData->LSS) {
			fwprintf_s(stderr, L"Not enough header data"
				L" (disk/dump size should be at least %" FI32 L"u bytes)\n", pDiskData->LSS * (i + 1));
			if (i > 0) {
				if (ret == ERR_SUCCESS) // do not override MBR error since it may be MBR only disk
					ret = ERR_GPT;
			} else
				ret = ERR_MBR;
			goto cleanup;
		}
		if (((PGPT_HEADER)bufptr)->magic == GPT_MAGIC)
			gpthdr = (PGPT_HEADER)bufptr;
		else if ((i == 0) && ((PMBR)bufptr)->bootsignature == MBR_MAGIC) {
			mbr = (PMBR)bufptr;
			if ((dwErr = printmbr(pDiskData, mbr)) != ERROR_SUCCESS) {
				if (dwErr & ERROR_CUSTOM)
					ret = ERR_MBR;
				else
					ret = ERR_SYSTEM;
			}
		}
		bufptr += pDiskData->LSS;  // next sector
		read -= pDiskData->LSS;
	}
	if (gpthdr == NULL) {
		fwprintf_s(stderr, L"GPT header not found\n");
		if (ret == ERR_SUCCESS)	// it seems that it's MBR only disk, do not override any mbr error
			ret = ERR_GPT;
		goto cleanup;
	}
	if ((dwErr = printgpt(pDiskData, gpthdr)) != ERROR_SUCCESS) {
		if (dwErr & ERROR_CUSTOM)
			ret = ERR_GPT;
		else
			ret = ERR_SYSTEM;
	}

cleanup:
	if ((ret != ERR_SYSTEM) && (pDiskData->nLastAllocLBA > 0))
		wprintf_s(L"*Allocated sectors:    %" FI64 L"u%s\n", pDiskData->nLastAllocLBA + 1,
			gpthdr != NULL?L" (excluding protective MBR and GPT backup)":L"");

	if (hdrbuf != NULL) delete[] hdrbuf;

	return ret;
}

INT wmain(INT argc, LPWSTR argv[], LPWSTR envp[])
{
	LPWSTR app = argv[0];
	argv++;
	argc--;

	_wsetlocale(LC_ALL, L".OCP");

	hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
	if ((hStdOut != INVALID_HANDLE_VALUE) && GetConsoleScreenBufferInfo(hStdOut, &csbiInfo))
		DEF = csbiInfo.wAttributes; 

	UINT32 DefLogicalSector = 0;
	UINT32 DefPhysicalSector = 0;
	INT i;
	for (i = 0; i < argc; i++) {
		if ((argv[i][0] != L'-') && (argv[i][0] != L'/'))
			break;
		WCHAR op = argv[i][1];
		if (op == L'-') {
			i++;
			break;
		} else if ((op == L'l') || (op == L'p')) {
			if (i >= (argc - 1)) {
				fwprintf_s(stderr, L"Option %s requires value\n", argv[i]);
				continue;
			}
			LPWSTR endptr;
			UINT32 optval = (UINT32)wcstoul(argv[++i], &endptr, 0);
			switch (*endptr) {
			case L'k':
			case L'K':
				optval *= 1024;
				endptr++;
			}
			if ((*endptr == '\0') && (optval < 131072) && ((optval % 512) == 0)) { // sanity check
				if (op == L'l')
					DefLogicalSector = optval;
				else
					DefPhysicalSector = optval;
			} else
				fwprintf_s(stderr, L"Value '%s' is unacceptable for sector size, ignored\n", argv[i]);
		} else if (op == L'c') {
			optcolor = 0;
		} else if (op == L'u') {
			switch (argv[i][2]) {
			default:
			case L'b': optsi = FALSE; break;
			case L's': optsi = TRUE; break;
			}
		} else if ((op == L'h') || (op == L'?')) {
			printusage(app);
			return 1;
		} else
			fwprintf_s(stderr, L"Unknown option '%s', ignored\n", argv[i]);
	}
	argv += i, argc -= i;

	DISK_DATA DiskData;
	INT ret = ERR_SUCCESS;

	SecureZeroMemory(&DiskData, sizeof(DISK_DATA));
	do {
		CleanDiskData(&DiskData);
		if (DefLogicalSector > 0)
			DiskData.LSS = DefLogicalSector;
		if (DefPhysicalSector > 0)
			DiskData.PSS = DefPhysicalSector;
		if (argc > 0) {
			DiskData.szName = argv[0];
			argv++, argc--;
		} else
			DiskData.szName = DEFAULT_DEVICE;
		INT lret = ProcessDisk(&DiskData);
		if ((lret != ERR_SUCCESS) && (ret != ERR_SYSTEM)) // don't override system error
			ret = lret;
	} while(argc > 0);

	CleanDiskData(&DiskData);
	return ret;
}
