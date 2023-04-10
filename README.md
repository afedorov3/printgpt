# PrintGPT

Reads and prints partitioning information from a drive or an image.  
Only MBR and GPT styles are supported.  

printgpt -h:
```
printgpt, v1.2
Usage: printgpt.exe [-l <size>] [-p <size>] [-c] [-u[b|s]] [device|file]
  -l: default logical sector size.
  -p: default physical sector size.
  -c: do not colorize output.
  -u: units to use for human redable output, b for binary (default), s for SI.
Return values (errorlevel):
  0: no error.
  1: system error.
  2: MBR error.
  3: GPT error.

To access physical device use form of '\\.\PhysicalDriveX'.
If device or file is not specified, \\.\PhysicalDrive0 will be used.
Values with an asterisk mark are calculated values.
```

Example output:
```
Accessing device \\.\PhysicalDrive2 ...
 Product ID:           VMware Virtual S
 S/N:                  VMware Virtual S
 Logical sector size:  512 bytes
 Physical sector size: 512 bytes
 Reported disk size:   40.00 GiB ( 42949672960 bytes )
*Logical sectors:      83886080
MBR:
  Disk signature:      0x00000000
  MBR entry 1:
    Type:              0xEE ( GPT protective MBR )
    Status:            0x00
    First LBA:         1
*   Last  LBA:         4294967295
    Sectors:           4294967295
*   Size:              2.00 TiB ( 2199023255040 bytes )
GPT:
  Disk GUID:           {D3DE3696-D9C5-4A5C-8315-EE92EADCFE0B}
  Table entry count:   128
  Entry size:          128
  Entries       LBA:   2
  Backup header LBA:   83886079
  First usable  LBA:   34
  Last usable   LBA:   83886046
* Total sectors:       83886080
* Total disk size:     40.00 GiB ( 42949672960 bytes )
  Header  CRC:         0x52E361BB [match]
  Entries CRC:         0xB92BA3A4 [match]
  GPT entry 1:
    Type:              {E3C9E316-0B5C-4DB8-817D-F92DF00215AE} ( Windows: Microsoft Reserved Partition (MSR) )
    Unique GUID:       {1E19DE36-7112-4B3C-A715-96FEA19EC12D}
    First LBA:         34
    Last  LBA:         32767
*   Sectors:           32734
*   Align:             1 KiB
*   Size:              15.98 MiB ( 16759808 bytes )
    Attributes:        0x0000000000000000
    Name:              Microsoft reserved partition
  GPT entry 2:
    Type:              {EBD0A0A2-B9E5-4433-87C0-68B6B72699C7} ( Windows: Basic data partition )
    Unique GUID:       {D879B5C6-63AC-4DD0-B7A9-1B7031DBCDA9}
    First LBA:         32768
    Last  LBA:         83881983
*   Sectors:           83849216
*   Align:             2 MiB
*   Size:              39.98 GiB ( 42930798592 bytes )
    Attributes:        0x0000000000000000
    Name:              Basic data partition
*Allocated sectors:    83881984 (excluding protective MBR and GPT backup)
```
