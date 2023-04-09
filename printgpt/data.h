// structure
// MBR
#pragma pack(push)
#pragma pack(1)
typedef struct _CHS {
	UINT8 head;
	UINT16 sector:6;
	UINT16 cylinder:10;
} CHS, *PCHS;

typedef __declspec(align(1)) struct _MBR_ENTRY {
	UINT8 status;
	CHS firstchs;
	UINT8 type;
	CHS lastchs;
	UINT32 firstlba;
	UINT32 sectors;
} MBR_ENTRY, *PMBR_ENTRY;

typedef __declspec(align(2)) struct _MBR {
	UINT8 bootcode1[218];
	union {
		struct {
			__declspec(align(1)) struct {
				UINT16 null;
				UINT8 drive;
				UINT8 secs;
				UINT8 mins;
				UINT8 hours;
			} timestamp;
			UINT8 bootcode2[216];
		} disktimestamp;
		UINT8 bootcode2[222];
	};
	UINT32 disksignature;
	UINT16 copyprotect;
	MBR_ENTRY entries[4];
	UINT16 bootsignature;
} MBR, *PMBR;

// GPT
#pragma pack(4)
typedef __declspec(align(4)) struct _GPT_HEADER {
	UINT64 magic;
	UINT32 revision;
	UINT32 hdrsize;
	UINT32 hdrcrc;
	UINT32 reserved;
	UINT64 currentlba;
	UINT64 backuplba;
	UINT64 firstusablelba;
	UINT64 lastusablelba;
	GUID diskGUID;
	UINT64 entrieslba;
	UINT32 entrycount;
	UINT32 entrysize;
	UINT32 entriescrc;
} GPT_HEADER, *PGPT_HEADER;

typedef __declspec(align(4)) struct _GPT_ENTRY {
	GUID typeGUID;
	GUID uniqueGUID;
	UINT64 firstlba;
	UINT64 lastlba;
	UINT64 attr;
	WCHAR name[36];
} GPT_ENTRY, *PGPT_ENTRY;
#pragma pack(pop)

// data
// MBR
const UINT16 MBR_MAGIC = 0xAA55;

typedef struct _MBR_TYPE {
	LPCWSTR origin;
	LPCWSTR type;
} MBR_TYPE, *PMBR_TYPE;

MBR_TYPE MBR_TYPES[256] = {
	{
		NULL,
		NULL,
	},
	{
		L"DOS 2.0+",
		L"FS: FAT12",
	},
	{
		L"XENIX",
		L"XENIX root",
	},
	{
		L"XENIX",
		L"XENIX usr",
	},
	{
		L"DOS 3.0+",
		L"FS: FAT16",
	},
	{
		L"DOS (3.2) 3.3+",
		L"Extended with CHS addressing",
	},
	{
		L"DOS 3.31+",
		L"FS: FAT16B",
	},
	{
		L"OS/2 1.2+, Windows, QNX 2",
		L"FS: IFS / HPFS / NTFS / exFAT, QNX \"qnx\"",
	},
	{
		L"AIX, QNX",
		L"AIX boot/split, QNX \"qny\"",
	},
	{
		L"AIX, QNX",
		L"AIX data/boot, QNX \"qnz\"",
	},
	{
		L"OS/2",
		L"OS/2 Boot Manager",
	},
	{
		L"DOS 7.1+",
		L"FS: FAT32 with CHS addressing",
	},
	{
		L"DOS 7.1+",
		L"FS: FAT32X with LBA",
	},
	{
		NULL,
		NULL,
	},
	{
		L"DOS 7.0+",
		L"FS: FAT16X with LBA",
	},
	{
		L"DOS 7.0+",
		L"Extended with LBA",
	},
	{
		NULL,
		NULL,
	},
	{
		L"Leading Edge MS-DOS 3.x, OS/2 Boot Manager",
		L"FS: Logical sectored FAT12 or FAT16, Hidden FAT12",
	},
	{
		L"Compaq Contura",
		L"Configuration, hibernation, diagnostics, recovery",
	},
	{
		NULL,
		NULL,
	},
	{
		L"AST MS-DOS 3.x, OS/2 Boot Manager, Maverick OS",
		L"FS: Logical sectored FAT12 or FAT16 / Hidden FAT16 / Omega",
	},
	{
		L"OS/2 Boot Manager, Maverick OS",
		L"Hidden extended with CHS addressing, swap",
	},
	{
		L"OS/2 Boot Manager",
		L"Hidden FAT16B",
	},
	{
		L"OS/2 Boot Manager",
		L"FS: Hidden IFS/HPFS/NTFS/exFAT",
	},
	{
		L"AST Windows",
		L"AST Zero Volt Suspend or SmartSleep",
	},
	{
		L"Willowtech Photon coS",
		L"Willowtech Photon coS",
	},
	{
		NULL,
		NULL,
	},
	{
		L"OS/2 Boot Manager",
		L"Hidden FAT32",
	},
	{
		L"OS/2 Boot Manager",
		L"Hidden FAT32X with LBA",
	},
	{
		NULL,
		NULL,
	},
	{
		L"OS/2 Boot Manager",
		L"Hidden FAT16X with LBA",
	},
	{
		L"OS/2 Boot Manager",
		L"Hidden extended with LBA addressing",
	},
	{
		L"Windows Mobile",
		L"Windows Mobile update XIP, Willowsoft Overture File System (OFS1)",
	},
	{
		L"Oxygen, HP",
		L"FS:Fso2(Oxygen File System), HP Volume Expansion (SpeedStor)",
	},
	{
		L"Oxygen",
		L"Oxygen Extended Partition Table",
	},
	{
		L"Windows Mobile",
		L"Windows Mobile boot XIP",
	},
	{
		L"NEC MS-DOS 3.30",
		L"Logical sectored FAT12 or FAT16",
	},
	{
		L"Windows Mobile",
		L"Windows Mobile IMGFS",
	},
	{
		NULL,
		NULL,
	},
	{
		L"Windows",
		L"FS: Windows recovery environment, MirOS, RooterBOOT kernel",
	},
	{
		NULL,
		NULL,
	},
	{
		NULL,
		NULL,
	},
	{
		L"AtheOS",
		L"AtheOS file system (AthFS, AFS)",
	},
	{
		L"SyllableOS",
		L"SyllableSecure (SylStor), a variant of AthFS",
	},
	{
		NULL,
		NULL,
	},
	{
		NULL,
		NULL,
	},
	{
		NULL,
		NULL,
	},
	{
		NULL,
		NULL,
	},
	{
		NULL,
		NULL,
	},
	{
		NULL,
		NULL,
	},
	{
		NULL,
		NULL,
	},
	{
		NULL,
		NULL,
	},
	{
		NULL,
		NULL,
	},
	{
		L"OS/2 Warp Server / eComStation",
		L"FS: JFS (OS/2 implementation of AIX Journaling Filesystem)",
	},
	{
		NULL,
		NULL,
	},
	{
		NULL,
		NULL,
	},
	{
		L"THEOS",
		L"THEOS version 3.2, 2 GB",
	},
	{
		L"Plan 9, THEOS",
		L"Plan 9 edition 3, THEOS version 4 spanned",
	},
	{
		L"THEOS",
		L"THEOS version 4, 4 GB",
	},
	{
		L"THEOS",
		L"THEOS version 4 extended",
	},
	{
		L"PartitionMagic",
		L"PqRP (PartitionMagic or DriveImage in progress)",
	},
	{
		L"PartitionMagic",
		L"Hidden NetWare",
	},
	{
		NULL,
		NULL,
	},
	{
		NULL,
		NULL,
	},
	{
		L"PICK, Venix",
		L"PICK R83, Venix 80286",
	},
	{
		L"Personal RISC, Linux, PowerPC",
		L"Personal RISC Boot, Old Linux/Minix, PPC PReP (Power PC Reference Platform) Boot",
	},
	{
		L"SFS, Linux, Windows 2k+",
		L"Secure Filesystem (SFS), Old Linux swap, Dynamic extended marker",
	},
	{
		L"Linux",
		L"Old Linux native",
	},
	{
		L"GoBack",
		L"Norton GoBack, WildFile GoBack, Adaptec GoBack, Roxio GoBack",
	},
	{
		L"Boot-US, EUMEL/ELAN",
		L"Boot-US boot manager, EUMEL/ELAN (L2)",
	},
	{
		L"EUMEL/ELAN",
		L"EUMEL/ELAN (L2)",
	},
	{
		L"EUMEL/ELAN",
		L"EUMEL/ELAN (L2)",
	},
	{
		L"EUMEL/ELAN, ERGOS L3",
		L"EUMEL/ELAN (L2), ERGOS L3",
	},
	{
		NULL,
		NULL,
	},
	{
		L"ALFS/THIN, AdaOS",
		L"FS: ALFS/THIN, Aquila",
	},
	{
		NULL,
		NULL,
	},
	{
		L"ETH Oberon",
		L"Aos (A2) filesystem",
	},
	{
		L"QNX 4.x, Neutrino",
		L"Primary QNX POSIX volume on disk",
	},
	{
		L"QNX 4.x, Neutrino",
		L"Secondary QNX POSIX volume on disk",
	},
	{
		L"QNX 4.x, Neutrino, ETH Oberon",
		L"Tertiary QNX POSIX volume on disk, boot / native filesystem",
	},
	{
		L"ETH Oberon, Disk Manager 4, LynxOS, Novell",
		L"Alternative native filesystem, Read-only partition (old), Lynx RTOS",
	},
	{
		L"Disk Manager 4-6",
		L"Read-write partition (Aux 1)",
	},
	{
		L"CP/M-80, System V/AT, V/386",
		L"FS: CP/M-80",
	},
	{
		L"Disk Manager 6",
		L"Auxiliary 3 (WO)",
	},
	{
		L"Disk Manager 6",
		L"Dynamic Drive Overlay (DDO)",
	},
	{
		L"EZ-Drive",
		L"EZ-Drive, Maxtor, MaxBlast, or DriveGuide INT 13h redirector volume",
	},
	{
		L"AT&T MS-DOS 3.x, EZ-Drive, Vfeature",
		L"Logical sectored FAT12 or FAT16, Disk Manager EZ-BIOS, VFeature partitionned volume",
	},
	{
		L"Novell",
		L"VNDI",
	},
	{
		NULL,
		NULL,
	},
	{
		NULL,
		NULL,
	},
	{
		NULL,
		NULL,
	},
	{
		NULL,
		NULL,
	},
	{
		L"EDISK",
		L"Priam EDisk Partitioned Volume",
	},
	{
		L"APTI (Alternative Partition Table Identification) conformant systems",
		L"APTI alternative",
	},
	{
		L"APTI conformant systems",
		L"APTI alternative extended",
	},
	{
		L"APTI conformant systems",
		L"APTI alternative extended (< 8 GB)",
	},
	{
		NULL,
		NULL,
	},
	{
		NULL,
		NULL,
	},
	{
		NULL,
		NULL,
	},
	{
		L"Unix",
		L"SCO Unix, ISC, UnixWare, AT&T System V/386, ix, MtXinu BSD 4.3 on Mach, GNU HURD",
	},
	{
		L"NetWare",
		L"FS: NetWare File System 286/2",
	},
	{
		L"NetWare",
		L"FS: NetWare File System 386",
	},
	{
		L"NetWare",
		L"FS: NetWare File System 386, Storage Management Services (SMS)",
	},
	{
		L"NetWare",
		L"Wolf Mountain",
	},
	{
		L"NetWare",
		NULL,
	},
	{
		L"NetWare",
		L"Novell Storage Services (NSS)",
	},
	{
		NULL,
		NULL,
	},
	{
		NULL,
		NULL,
	},
	{
		NULL,
		NULL,
	},
	{
		NULL,
		NULL,
	},
	{
		NULL,
		NULL,
	},
	{
		NULL,
		NULL,
	},
	{
		L"DiskSecure",
		L"DiskSecure multiboot",
	},
	{
		NULL,
		NULL,
	},
	{
		L"APTI conformant systems, Unix V7/x86",
		L"FS: APTI alternative FAT12 (CHS, SFN), V7/x86",
	},
	{
		NULL,
		NULL,
	},
	{
		NULL,
		L"Scramdisk",
	},
	{
		NULL,
		NULL,
	},
	{
		NULL,
		NULL,
	},
	{
		NULL,
		L"VNDI, M2FS, M2CS",
	},
	{
		NULL,
		L"XOSL bootloader filesystem",
	},
	{
		L"APTI conformant systems",
		L"FS: APTI alternative FAT16 (CHS, SFN)",
	},
	{
		L"APTI conformant systems",
		L"FS: APTI alternative FAT16X (LBA, SFN)",
	},
	{
		L"APTI conformant systems",
		L"FS: APTI alternative FAT16B (CHS, SFN)",
	},
	{
		L"APTI conformant systems",
		L"FS: APTI alternative FAT32X (LBA, SFN)",
	},
	{
		L"APTI conformant systems",
		L"FS: APTI alternative FAT32 (CHS, SFN)",
	},
	{
		NULL,
		NULL,
	},
	{
		NULL,
		NULL,
	},
	{
		L"Minix 1.1-1.4a",
		L"FS: Minix file system (old)",
	},
	{
		L"Minix 1.4b+, Linux",
		L"FS: MINIX file system, Mitac Advanced Disk Manager",
	},
	{
		NULL,
		L"Linux swap space, Solaris x86 (for Sun disklabels up to 2005), Prime",
	},
	{
		NULL,
		L"FS: Native Linux file system",
	},
	{
		L"OS/2, Windows 7",
		L"FS: Hidden C: (FAT16), APM hibernation, Rapid Start technology",
	},
	{
		NULL,
		L"Linux extended",
	},
	{
		L"Windows NT 4 Server, Linux",
		L"FS: Fault-tolerant FAT16B mirrored volume set, Linux RAID superblock with auto-detect (old)",
	},
	{
		L"Windows NT 4 Server",
		L"FS: Fault-tolerant HPFS/NTFS mirrored volume set",
	},
	{
		NULL,
		L"Linux plaintext partition table",
	},
	{
		NULL,
		NULL,
	},
	{
		L"AiR-BOOT",
		L"Linux kernel image",
	},
	{
		L"Windows NT 4 Server",
		L"FS: Legacy fault-tolerant FAT32 mirrored volume set",
	},
	{
		L"Windows NT 4 Server",
		L"FS: Legacy fault-tolerant FAT32X mirrored volume set",
	},
	{
		L"Free FDISK",
		L"FS: Hidden FAT12",
	},
	{
		L"Linux",
		L"Linux LVM",
	},
	{
		NULL,
		NULL,
	},
	{
		L"Free FDISK",
		L"FS: Hidden FAT16",
	},
	{
		L"Free FDISK",
		L"Hidden extended with CHS addressing",
	},
	{
		L"Free FDISK",
		L"FS: Hidden FAT16B",
	},
	{
		L"Linux, Amoeba",
		L"FS: Hidden Linux filesystem/Amoeba native filesystem",
	},
	{
		L"Amoeba",
		L"Amoeba bad block table",
	},
	{
		L"EXOPC",
		L"EXOPC native",
	},
	{
		L"CHRP",
		L"FS: ISO-9660 filesystem",
	},
	{
		L"Free FDISK",
		L"FS: Hidden FAT32",
	},
	{
		L"Free FDISK, ROM-DOS",
		L"FS: Hidden FAT32X/service (bootable FAT)",
	},
	{
		L"DCE376",
		L"EISA SCSI (> 1024)",
	},
	{
		L"Free FDISK",
		L"FS: Hidden FAT16X",
	},
	{
		L"Free FDISK",
		L"Hidden extended with LBA",
	},
	{
		NULL,
		NULL,
	},
	{
		NULL,
		NULL,
	},
	{
		L"ForthOS",
		L"ForthOS (eForth port)",
	},
	{
		L"BSD/OS 3.0+, BSDI",
		NULL,
	},
	{
		L"HP, Phoenix, IBM, Toshiba, Sony",
		L"Diagnostic for HP laptops, Hibernate",
	},
	{
		L"HP, Phoenix, NEC",
		L"HP Volume Expansion (SpeedStor), Hibernate",
	},
	{
		L"Cyclone V",
		L"Hard Processor System (HPS) ARM preloader",
	},
	{
		L"HP",
		L"HP Volume Expansion (SpeedStor)",
	},
	{
		L"HP",
		L"HP Volume Expansion (SpeedStor)",
	},
	{
		L"BSD",
		L"BSD slice (BSD/386, 386BSD, NetBSD (old), FreeBSD)",
	},
	{
		L"OpenBSD, HP",
		L"OpenBSD slice, HP Volume Expansion (SpeedStor)",
	},
	{
		NULL,
		L"FS: NeXTSTEP",
	},
	{
		L"Darwin, Mac OS X",
		L"FS: Apple Darwin/Mac OS X UFS",
	},
	{
		L"NetBSD",
		L"NetBSD slice",
	},
	{
		L"MS-DOS",
		L"Olivetti MS-DOS FAT12 (1.44 MB)",
	},
	{
		L"Darwin, Mac OS X, GO! OS",
		L"Apple Darwin, Mac OS X boot, GO!",
	},
	{
		NULL,
		NULL,
	},
	{
		L"RISC OS",
		L"FS: ADFS/FileCore",
	},
	{
		L"ShagOS",
		L"FS: ShagOS",
	},
	{
		L"Apple, ShagOS",
		L"Apple Mac OS X HFS and HFS+, ShagOS swap",
	},
	{
		L"Boot-Star",
		L"Boot-Star dummy",
	},
	{
		L"HP, QNX 6.x",
		L"HP Volume Expansion (SpeedStor), QNX Neutrino power-safe",
	},
	{
		L"QNX 6.x",
		L"QNX Neutrino power-safe file system",
	},
	{
		L"HP, QNX 6.x",
		L"HP Volume Expansion (SpeedStor), QNX Neutrino power-safe",
	},
	{
		L"HP",
		L"HP Volume Expansion (SpeedStor)",
	},
	{
		NULL,
		NULL,
	},
	{
		L"HP, Windows NT 4 Server",
		L"HP Volume Expansion (SpeedStor), Corrupted fault-tolerant FAT16B mirrored master volume",
	},
	{
		L"BSDI (before 3.0), Windows NT 4 Server",
		L"FS: BSDI native filesystem/swap, Corrupted fault-tolerant HPFS/NTFS mirrored master volume",
	},
	{
		L"BSDI (before 3.0)",
		L"FS: BSDI swap / native filesystem",
	},
	{
		NULL,
		NULL,
	},
	{
		NULL,
		NULL,
	},
	{
		L"BootWizard, OS Selector, Windows NT 4 Server",
		L"PTS BootWizard 4/OS Selector 5, Corrupted fault-tolerant FAT32 mirrored master volume",
	},
	{
		L"Windows NT 4 Server, Acronis, Backup Capsule",
		L"Corrupted fault-tolerant FAT32X mirrored master volume, Acronis Secure Zone (\"ACRONIS SZ\"), Backup Capsule",
	},
	{
		NULL,
		NULL,
	},
	{
		L"Solaris 8",
		L"Solaris 8 boot",
	},
	{
		L"Solaris",
		L"Solaris x86 (for Sun disklabels, since 2005)",
	},
	{
		L"DR-DOS, Multiuser DOS, REAL/32",
		L"FS: Secured FAT (smaller than 32 MB)",
	},
	{
		L"DR DOS 6.0+",
		L"FS: Secured FAT12",
	},
	{
		L"Power Boot",
		L"FS: Hidden Linux native filesystem",
	},
	{
		L"Power Boot",
		L"Hidden Linux swap",
	},
	{
		L"DR DOS 6.0+",
		L"FS: Secured FAT16",
	},
	{
		L"DR DOS 6.0+",
		L"Secured extended with CHS addressing",
	},
	{
		L"DR DOS 6.0+, Windows NT 4 Server",
		L"FS: Secured FAT16B, Corrupted fault-tolerant FAT16B mirrored slave volume",
	},
	{
		L"Syrinx, Windows NT 4 Server",
		L"Syrinx boot, Corrupted fault-tolerant HPFS/NTFS mirrored slave volume",
	},
	{
		NULL,
		NULL,
	},
	{
		NULL,
		NULL,
	},
	{
		L"DR-DOS",
		L"FS: FAT32B, a FAT32 variant with 32-bit clusters and non-standard EBPB with 64-bit sector entry",
	},
	{
		L"DR-DOS 7.0x, Windows NT 4 Server",
		L"FS: Secured FAT32, Corrupted fault-tolerant FAT32 mirrored slave volume",
	},
	{
		L"DR-DOS 7.0x, Windows NT 4 Server",
		L"FS: Secured FAT32X, Corrupted fault-tolerant FAT32X mirrored slave volume",
	},
	{
		L"CTOS",
		L"Memory dump",
	},
	{
		L"DR-DOS 7.0x",
		L"FS: Secured FAT16X",
	},
	{
		L"DR-DOS 7.0x",
		L"Secured extended with LBA",
	},
	{
		L"Multiuser DOS, REAL/32",
		L"Secured FAT (larger than 32 MB)",
	},
	{
		L"Multiuser DOS",
		L"FS: Secured FAT12",
	},
	{
		NULL,
		NULL,
	},
	{
		NULL,
		NULL,
	},
	{
		L"Multiuser DOS",
		L"FS: Secured FAT16",
	},
	{
		L"Multiuser DOS",
		L"Secured extended with CHS addressing",
	},
	{
		L"Multiuser DOS",
		L"FS: Secured FAT16B",
	},
	{
		NULL,
		NULL,
	},
	{
		NULL,
		L"FS: CP/M-86",
	},
	{
		NULL,
		NULL,
	},
	{
		L"Powercopy Backup",
		L"Shielded disk",
	},
	{
		L"CP/M-86, Concurrent DOS, CTOS, D800, DRMK",
		L"FS: CP/M-86 / Concurrent DOS, boot image for SCPU module, FAT32 system restore (DSR)",
	},
	{
		NULL,
		NULL,
	},
	{
		L"CTOS",
		L"Hidden memory dump",
	},
	{
		L"Dell",
		L"FS: FAT16 utility/diagnostic",
	},
	{
		L"DG/UX, BootIt",
		L"DG/UX virtual disk manager, EMBRM",
	},
	{
		L"STMicroelectronics ",
		L"FS: ST AVFS",
	},
	{
		L"SpeedStor",
		L"FS: Extended FAT12 (> 1023 cylinder)",
	},
	{
		NULL,
		L"FS: DOS read-only (XFDISK)",
	},
	{
		L"SpeedStor",
		L"FS: DOS read-only",
	},
	{
		L"SpeedStor",
		L"FS: Extended FAT16 (< 1024 cylinder)",
	},
	{
		L"Tandy MS-DOS",
		L"FS: Logical sectored FAT12 or FAT16",
	},
	{
		NULL,
		NULL,
	},
	{
		NULL,
		NULL,
	},
	{
		L"LUKS",
		L"Linux Unified Key Setup",
	},
	{
		NULL,
		NULL,
	},
	{
		NULL,
		NULL,
	},
	{
		L"BeOS, Haiku",
		L"FS: BFS",
	},
	{
		L"SkyOS",
		L"FS: SkyFS",
	},
	{
		L"Sprytix, EDD 4",
		L"EDC loader, GPT hybrid MBR",
	},
	{
		L"EFI",
		L"GPT protective MBR",
	},
	{
		L"EFI",
		L"EFI system partition",
	},
	{
		L"Linux, OS/32",
		L"PA-RISC Linux boot loader, floppy",
	},
	{
		NULL,
		NULL,
	},
	{
		L"Sperry IT MS-DOS 3.x, Unisys MS-DOS 3.3, DR-DOS+ 2.1",
		L"FS: Logical sectored FAT12 or FAT16",
	},
	{
		NULL,
		NULL,
	},
	{
		L"SpeedStor, Prologue",
		L"FS: SpeedStor \"large\" DOS / NGF / TwinFS",
	},
	{
		L"Prologue",
		L"FS: NGF / TwinFS",
	},
	{
		NULL,
		NULL,
	},
	{
		L"O.S.G., X1",
		L"FS: EFAT / Solid State file system",
	},
	{
		NULL,
		NULL,
	},
	{
		L"Linux",
		L"pCache ext2/ext3 persistent cache",
	},
	{
		L"Bochs",
		L"x86 emulator",
	},
	{
		L"VMware",
		L"FS: VMware VMFS",
	},
	{
		L"VMware",
		L"VMware swap / VMKCORE kernel dump",
	},
	{
		L"Linux",
		L"Linux RAID superblock with auto-detect",
	},
	{
		L"IBM, Intel, Windows NT, Linux",
		L"FS: PS/2 recovery, LANstep , PS/2 IML, Disk Administration hidden, old Linux LVM",
	},
	{
		L"XENIX",
		L"XENIX bad block table",
	}
};

// GPT
const UINT64 GPT_MAGIC = 0x5452415020494645ULL; // EFI PART

LPCWSTR GPT_GEN_ATTR[48] = { 
	L"System partition",
	L"Ignore partition",
	L"Legacy BIOS bootable",
	/* [3] - [47] */
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
};

typedef struct _GPT_TYPE {
	GUID typeGUID;
	LPCWSTR os;
	LPCWSTR type;
	LPCWSTR attr[16];
} GPT_TYPE, *PGPT_TYPE;

GPT_TYPE GPT_TYPES[] = 
{
	{   // Must exist and be first
		{0x00000000, 0x0000, 0x0000, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},
		NULL,
		L"Unused entry"
	},
	{
		{0x024DEE41, 0x33E7, 0x11D3, {0x9D, 0x69, 0x00, 0x08, 0xC7, 0x81, 0xF3, 0x9F}},
		NULL,
		L"MBR partition scheme"
	},
	{
		{0xC12A7328, 0xF81F, 0x11D2, {0xBA, 0x4B, 0x00, 0xA0, 0xC9, 0x3E, 0xC9, 0x3B}},
		NULL,
		L"EFI System partition"
	},
	{
		{0x21686148, 0x6449, 0x6E6F, {0x74, 0x4E, 0x65, 0x65, 0x64, 0x45, 0x46, 0x49}},
		NULL,
		L"BIOS Boot partition"
	},
	{
		{0xD3BFE2DE, 0x3DAF, 0x11DF, {0xBA, 0x40, 0xE3, 0xA5, 0x56, 0xD8, 0x95, 0x93}},
		NULL,
		L"Intel Fast Flash (iFFS) partition (for Intel Rapid Start technology)"
	},
	{
		{0xF4019732, 0x066E, 0x4E12, {0x82, 0x73, 0x34, 0x6C, 0x56, 0x41, 0x49, 0x4F}},
		NULL,
		L"Sony boot partition"
	},
	{
		{0xBFBFAFE7, 0xA34F, 0x448A, {0x9A, 0x5B, 0x62, 0x13, 0xEB, 0x73, 0x6C, 0x22}},
		NULL,
		L"Lenovo boot partition"
	},
	{
		{0xE3C9E316, 0x0B5C, 0x4DB8, {0x81, 0x7D, 0xF9, 0x2D, 0xF0, 0x02, 0x15, 0xAE}},
		L"Windows",
		L"Microsoft Reserved Partition (MSR)"
	},
	{
		{0xEBD0A0A2, 0xB9E5, 0x4433, {0x87, 0xC0, 0x68, 0xB6, 0xB7, 0x26, 0x99, 0xC7}},
		L"Windows",
		L"Basic data partition",
		{
			NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,	NULL, NULL,
			L"Read-only",		/* [60] */
			L"Shadow copy",		/* [61] */
			L"Hidden",			/* [62] */
			L"Do not automount"	/* [63] */
		}
	},
	{
		{0x5808C8AA, 0x7E8F, 0x42E0, {0x85, 0xD2, 0xE1, 0xE9, 0x04, 0x34, 0xCF, 0xB3}},
		L"Windows",
		L"Logical Disk Manager (LDM) metadata partition"
	},
	{
		{0xAF9B60A0, 0x1431, 0x4F62, {0xBC, 0x68, 0x33, 0x11, 0x71, 0x4A, 0x69, 0xAD}},
		L"Windows",
		L"Logical Disk Manager data partition"
	},
	{
		{0xDE94BBA4, 0x06D1, 0x4D40, {0xA1, 0x6A, 0xBF, 0xD5, 0x01, 0x79, 0xD6, 0xAC}},
		L"Windows",
		L"Windows Recovery Environment"
	},
	{
		{0x37AFFC90, 0xEF7D, 0x4E96, {0x91, 0xC3, 0x2D, 0x7A, 0xE0, 0x55, 0xB1, 0x74}},
		L"Windows",
		L"IBM General Parallel File System (GPFS) partition"
	},
	{
		{0xE75CAF8F, 0xF680, 0x4CEE, {0xAF, 0xA3, 0xB0, 0x01, 0xE5, 0x6E, 0xFC, 0x2D}},
		L"Windows",
		L"Storage Spaces partition"
	},
	{
		{0x75894C1E, 0x3AEB, 0x11D3, {0xB7, 0xC1, 0x7B, 0x03, 0xA0, 0x00, 0x00, 0x00}},
		L"HP-UX",
		L"Data partition"
	},
	{
		{0xE2A1E728, 0x32E3, 0x11D6, {0xA6, 0x82, 0x7B, 0x03, 0xA0, 0x00, 0x00, 0x00}},
		L"HP-UX",
		L"Service Partition"
	},
	{
		{0x0FC63DAF, 0x8483, 0x4772, {0x8E, 0x79, 0x3D, 0x69, 0xD8, 0x47, 0x7D, 0xE4}},
		L"Linux",
		L"Linux filesystem data"
	},
	{
		{0xA19D880F, 0x05FC, 0x4D3B, {0xA0, 0x06, 0x74, 0x3F, 0x0F, 0x84, 0x91, 0x1E}},
		L"Linux",
		L"RAID partition"
	},
	{
		{0x0657FD6D, 0xA4AB, 0x43C4, {0x84, 0xE5, 0x09, 0x33, 0xC8, 0x4B, 0x4F, 0x4F}},
		L"Linux",
		L"Swap partition"
	},
	{
		{0xE6D6D379, 0xF507, 0x44C2, {0xA2, 0x3C, 0x23, 0x8F, 0x2A, 0x3D, 0xF9, 0x28}},
		L"Linux",
		L"Logical Volume Manager (LVM) partition"
	},
	{
		{0x933AC7E1, 0x2EB4, 0x4F13, {0xB8, 0x44, 0x0E, 0x14, 0xE2, 0xAE, 0xF9, 0x15}},
		L"Linux",
		L"/home partition"
	},
	{
		{0x3B8F8425, 0x20E0, 0x4F3B, {0x90, 0x7F, 0x1A, 0x25, 0xA7, 0x6F, 0x98, 0xE8}},
		L"Linux",
		L"/srv (server data) partition"
	},
	{
		{0x7FFEC5C9, 0x2D00, 0x49B7, {0x89, 0x41, 0x3E, 0xA1, 0x0A, 0x55, 0x86, 0xB7}},
		L"Linux",
		L"Plain dm-crypt partition"
	},
	{
		{0xCA7D7CCB, 0x63ED, 0x4C53, {0x86, 0x1C, 0x17, 0x42, 0x53, 0x60, 0x59, 0xCC}},
		L"Linux",
		L"LUKS partition"
	},
	{
		{0x8DA63339, 0x0007, 0x60C0, {0xC4, 0x36, 0x08, 0x3A, 0xC8, 0x23, 0x09, 0x08}},
		L"Linux",
		L"Reserved"
	},
	{
		{0x83BD6B9D, 0x7F41, 0x11DC, {0xBE, 0x0B, 0x00, 0x15, 0x60, 0xB8, 0x4F, 0x0F}},
		L"FreeBSD",
		L"Boot partition"
	},
	{
		{0x516E7CB4, 0x6ECF, 0x11D6, {0x8F, 0xF8, 0x00, 0x02, 0x2D, 0x09, 0x71, 0x2B}},
		L"FreeBSD",
		L"Data partition"
	},
	{
		{0x516E7CB5, 0x6ECF, 0x11D6, {0x8F, 0xF8, 0x00, 0x02, 0x2D, 0x09, 0x71, 0x2B}},
		L"FreeBSD",
		L"Swap partition"
	},
	{
		{0x516E7CB6, 0x6ECF, 0x11D6, {0x8F, 0xF8, 0x00, 0x02, 0x2D, 0x09, 0x71, 0x2B}},
		L"FreeBSD",
		L"Unix File System (UFS) partition"
	},
	{
		{0x516E7CB8, 0x6ECF, 0x11D6, {0x8F, 0xF8, 0x00, 0x02, 0x2D, 0x09, 0x71, 0x2B}},
		L"FreeBSD",
		L"Vinum volume manager partition"
	},
	{
		{0x516E7CBA, 0x6ECF, 0x11D6, {0x8F, 0xF8, 0x00, 0x02, 0x2D, 0x09, 0x71, 0x2B}},
		L"FreeBSD",
		L"ZFS partition"
	},
	{
		{0x48465300, 0x0000, 0x11AA, {0xAA, 0x11, 0x00, 0x30, 0x65, 0x43, 0xEC, 0xAC}},
		L"Mac OS X",
		L"Hierarchical File System Plus (HFS+) partition"
	},
	{
		{0x55465300, 0x0000, 0x11AA, {0xAA, 0x11, 0x00, 0x30, 0x65, 0x43, 0xEC, 0xAC}},
		L"Mac OS X",
		L"Apple UFS"
	},
	{
		{0x6A898CC3, 0x1DD2, 0x11B2, {0x99, 0xA6, 0x08, 0x00, 0x20, 0x73, 0x66, 0x31}},
		L"Mac OS X",
		L"ZFS"
	},
	{
		{0x52414944, 0x0000, 0x11AA, {0xAA, 0x11, 0x00, 0x30, 0x65, 0x43, 0xEC, 0xAC}},
		L"Mac OS X",
		L"Apple RAID partition"
	},
	{
		{0x52414944, 0x5F4F, 0x11AA, {0xAA, 0x11, 0x00, 0x30, 0x65, 0x43, 0xEC, 0xAC}},
		L"Mac OS X",
		L"Apple RAID partition, offline"
	},
	{
		{0x426F6F74, 0x0000, 0x11AA, {0xAA, 0x11, 0x00, 0x30, 0x65, 0x43, 0xEC, 0xAC}},
		L"Mac OS X",
		L"Apple Boot partition"
	},
	{
		{0x4C616265, 0x6C00, 0x11AA, {0xAA, 0x11, 0x00, 0x30, 0x65, 0x43, 0xEC, 0xAC}},
		L"Mac OS X",
		L"Apple Label"
	},
	{
		{0x5265636F, 0x7665, 0x11AA, {0xAA, 0x11, 0x00, 0x30, 0x65, 0x43, 0xEC, 0xAC}},
		L"Mac OS X",
		L"Apple TV Recovery partition"
	},
	{
		{0x53746F72, 0x6167, 0x11AA, {0xAA, 0x11, 0x00, 0x30, 0x65, 0x43, 0xEC, 0xAC}},
		L"Mac OS X",
		L"Apple Core Storage (i.e. Lion FileVault) partition"
	},
	{
		{0x6A82CB45, 0x1DD2, 0x11B2, {0x99, 0xA6, 0x08, 0x00, 0x20, 0x73, 0x66, 0x31}},
		L"Solaris",
		L"Boot partition"
	},
	{
		{0x6A85CF4D, 0x1DD2, 0x11B2, {0x99, 0xA6, 0x08, 0x00, 0x20, 0x73, 0x66, 0x31}},
		L"Solaris",
		L"Root partition"
	},
	{
		{0x6A87C46F, 0x1DD2, 0x11B2, {0x99, 0xA6, 0x08, 0x00, 0x20, 0x73, 0x66, 0x31}},
		L"Solaris",
		L"Swap partition"
	},
	{
		{0x6A8B642B, 0x1DD2, 0x11B2, {0x99, 0xA6, 0x08, 0x00, 0x20, 0x73, 0x66, 0x31}},
		L"Solaris",
		L"Backup partition"
	},
	{
		{0x6A898CC3, 0x1DD2, 0x11B2, {0x99, 0xA6, 0x08, 0x00, 0x20, 0x73, 0x66, 0x31}},
		L"Solaris",
		L"/usr partition"
	},
	{
		{0x6A8EF2E9, 0x1DD2, 0x11B2, {0x99, 0xA6, 0x08, 0x00, 0x20, 0x73, 0x66, 0x31}},
		L"Solaris",
		L"/var partition"
	},
	{
		{0x6A90BA39, 0x1DD2, 0x11B2, {0x99, 0xA6, 0x08, 0x00, 0x20, 0x73, 0x66, 0x31}},
		L"Solaris",
		L"/home partition"
	},
	{
		{0x6A9283A5, 0x1DD2, 0x11B2, {0x99, 0xA6, 0x08, 0x00, 0x20, 0x73, 0x66, 0x31}},
		L"Solaris",
		L"Alternate sector"
	},
	{
		{0x6A945A3B, 0x1DD2, 0x11B2, {0x99, 0xA6, 0x08, 0x00, 0x20, 0x73, 0x66, 0x31}},
		L"Solaris",
		L"Reserved partition"
	},
	{
		{0x6A9630D1, 0x1DD2, 0x11B2, {0x99, 0xA6, 0x08, 0x00, 0x20, 0x73, 0x66, 0x31}},
		L"Solaris",
		L"Reserved partition"
	},
	{
		{0x6A980767, 0x1DD2, 0x11B2, {0x99, 0xA6, 0x08, 0x00, 0x20, 0x73, 0x66, 0x31}},
		L"Solaris",
		L"Reserved partition"
	},
	{
		{0x6A96237F, 0x1DD2, 0x11B2, {0x99, 0xA6, 0x08, 0x00, 0x20, 0x73, 0x66, 0x31}},
		L"Solaris",
		L"Reserved partition"
	},
	{
		{0x6A8D2AC7, 0x1DD2, 0x11B2, {0x99, 0xA6, 0x08, 0x00, 0x20, 0x73, 0x66, 0x31}},
		L"Solaris",
		L"Reserved partition"
	},
	{
		{0x49F48D32, 0xB10E, 0x11DC, {0xB9, 0x9B, 0x00, 0x19, 0xD1, 0x87, 0x96, 0x48}},
		L"NetBSD",
		L"Swap partition"
	},
	{
		{0x49F48D5A, 0xB10E, 0x11DC, {0xB9, 0x9B, 0x00, 0x19, 0xD1, 0x87, 0x96, 0x48}},
		L"NetBSD",
		L"FFS partition"
	},
	{
		{0x49F48D82, 0xB10E, 0x11DC, {0xB9, 0x9B, 0x00, 0x19, 0xD1, 0x87, 0x96, 0x48}},
		L"NetBSD",
		L"LFS partition"
	},
	{
		{0x49F48DAA, 0xB10E, 0x11DC, {0xB9, 0x9B, 0x00, 0x19, 0xD1, 0x87, 0x96, 0x48}},
		L"NetBSD",
		L"RAID partition"
	},
	{
		{0x2DB519C4, 0xB10F, 0x11DC, {0xB9, 0x9B, 0x00, 0x19, 0xD1, 0x87, 0x96, 0x48}},
		L"NetBSD",
		L"Concatenated partition"
	},
	{
		{0x2DB519EC, 0xB10F, 0x11DC, {0xB9, 0x9B, 0x00, 0x19, 0xD1, 0x87, 0x96, 0x48}},
		L"NetBSD",
		L"Encrypted partition"
	},
	{
		{0xFE3A2A5D, 0x4F32, 0x41A7, {0xB7, 0x25, 0xAC, 0xCC, 0x32, 0x85, 0xA3, 0x09}},
		L"ChromeOS",
		L"ChromeOS kernel"
	},
	{
		{0x3CB8E202, 0x3B7E, 0x47DD, {0x8A, 0x3C, 0x7F, 0xF2, 0xA1, 0x3C, 0xFC, 0xEC}},
		L"ChromeOS",
		L"ChromeOS rootfs"
	},
	{
		{0x2E0A753D, 0x9E48, 0x43B0, {0x83, 0x37, 0xB1, 0x51, 0x92, 0xCB, 0x1B, 0x5E}},
		L"ChromeOS",
		L"ChromeOS future use"
	},
	{
		{0x42465331, 0x3BA3, 0x10F1, {0x80, 0x2A, 0x48, 0x61, 0x69, 0x6B, 0x75, 0x21}},
		L"Haiku",
		L"Haiku BFS"
	},
	{
		{0x85D5E45E, 0x237C, 0x11E1, {0xB4, 0xB3, 0xE8, 0x9A, 0x8F, 0x7F, 0xC3, 0xA7}},
		L"MidnightBSD",
		L"Boot partition"
	},
	{
		{0x85D5E45A, 0x237C, 0x11E1, {0xB4, 0xB3, 0xE8, 0x9A, 0x8F, 0x7F, 0xC3, 0xA7}},
		L"MidnightBSD",
		L"Data partition"
	},
	{
		{0x85D5E45B, 0x237C, 0x11E1, {0xB4, 0xB3, 0xE8, 0x9A, 0x8F, 0x7F, 0xC3, 0xA7}},
		L"MidnightBSD",
		L"Swap partition"
	},
	{
		{0x0394EF8B, 0x237E, 0x11E1, {0xB4, 0xB3, 0xE8, 0x9A, 0x8F, 0x7F, 0xC3, 0xA7}},
		L"MidnightBSD",
		L"Unix File System (UFS) partition"
	},
	{
		{0x85D5E45C, 0x237C, 0x11E1, {0xB4, 0xB3, 0xE8, 0x9A, 0x8F, 0x7F, 0xC3, 0xA7}},
		L"MidnightBSD",
		L"Vinum volume manager partition"
	},
	{
		{0x85D5E45D, 0x237C, 0x11E1, {0xB4, 0xB3, 0xE8, 0x9A, 0x8F, 0x7F, 0xC3, 0xA7}},
		L"MidnightBSD",
		L"ZFS partition"
	},
	{
		{0xBFBFAFE7, 0xA34F, 0x448A, {0x9A, 0x5B, 0x62, 0x13, 0xEB, 0x73, 0x6C, 0x22}},
		L"Ceph",
		L"Ceph Journal"
	},
	{
		{0x45B0969E, 0x9B03, 0x4F30, {0xB4, 0xC6, 0x5E, 0xC0, 0x0C, 0xEF, 0xF1, 0x06}},
		L"Ceph",
		L"Ceph dm-crypt Encrypted Journal"
	},
	{
		{0x4FBD7E29, 0x9D25, 0x41B8, {0xAF, 0xD0, 0x06, 0x2C, 0x0C, 0xEF, 0xF0, 0x5D}},
		L"Ceph",
		L"Ceph OSD"
	},
	{
		{0x4FBD7E29, 0x9D25, 0x41B8, {0xAF, 0xD0, 0x5E, 0xC0, 0x0C, 0xEF, 0xF0, 0x5D}},
		L"Ceph",
		L"Ceph dm-crypt OSD"
	},
	{
		{0x89C57F98, 0x2FE5, 0x4DC0, {0x89, 0xC1, 0xF3, 0xAD, 0x0C, 0xEF, 0xF2, 0xBE}},
		L"Ceph",
		L"Ceph disk in creation"
	},
	{
		{0x89C57F98, 0x2FE5, 0x4DC0, {0x89, 0xC1, 0x5E, 0xC0, 0x0C, 0xEF, 0xF2, 0xBE}},
		L"Ceph",
		L"Ceph dm-crypt disk in creation"
	},
	{
		{0x9e1a2d38, 0xc612, 0x4316, {0xaa, 0x26, 0x8b, 0x49, 0x52, 0x1e, 0x5a, 0x8b}},
		L"PowerPC Reference Platform",
		L"PReP boot partition"
	},
};

// Console attributes
// Colors
// Foreground
#define FBK 0
#define FBKB FOREGROUND_INTENSITY
#define FRD FOREGROUND_RED
#define FRDB FOREGROUND_RED|FOREGROUND_INTENSITY
#define FGR FOREGROUND_GREEN
#define FGRB FOREGROUND_GREEN|FOREGROUND_INTENSITY
#define FBL FOREGROUND_BLUE
#define FBLB FOREGROUND_BLUE|FOREGROUND_INTENSITY
#define FWH FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE
#define FWHB FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_INTENSITY
#define FCY FOREGROUND_GREEN|FOREGROUND_BLUE
#define FCYB FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_INTENSITY
#define FYL FOREGROUND_RED|FOREGROUND_GREEN
#define FYLB FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_INTENSITY
#define FMG FOREGROUND_RED|FOREGROUND_BLUE
#define FMGB FOREGROUND_RED|FOREGROUND_BLUE|FOREGROUND_INTENSITY
// Background
#define BBK 0
#define BBKB BACKGROUND_INTENSITY
#define BRD BACKGROUND_RED
#define BRDB BACKGROUND_RED|BACKGROUND_INTENSITY
#define BGR BACKGROUND_GREEN
#define BGRB BACKGROUND_GREEN|BACKGROUND_INTENSITY
#define BBL BACKGROUND_BLUE
#define BBLB BACKGROUND_BLUE|BACKGROUND_INTENSITY
#define BWH BACKGROUND_RED|BACKGROUND_GREEN|BACKGROUND_BLUE
#define BWHB BACKGROUND_RED|BACKGROUND_GREEN|BACKGROUND_BLUE|BACKGROUND_INTENSITY
#define BCY BACKGROUND_GREEN|BACKGROUND_BLUE
#define BCYB BACKGROUND_GREEN|BACKGROUND_BLUE|BACKGROUND_INTENSITY
#define BYL BACKGROUND_RED|BACKGROUND_GREEN
#define BYLB BACKGROUND_RED|BACKGROUND_GREEN|BACKGROUND_INTENSITY
#define BMG BACKGROUND_RED|BACKGROUND_BLUE
#define BMGB BACKGROUND_RED|BACKGROUND_BLUE|BACKGROUND_INTENSITY
