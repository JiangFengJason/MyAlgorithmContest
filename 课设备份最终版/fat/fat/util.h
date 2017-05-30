#pragma once

#ifndef _DISK_H_
#define _DISK_H_

#define MAX_NUM 2048

typedef unsigned char u8;   //1字节  
typedef unsigned short u16; //2字节  
typedef unsigned int u32;   //4字节  

int  BytsPerSec;    //每扇区字节数  
int  SecPerClus;    //每簇扇区数  
int  RsvdSecCnt;    //Boot记录占用的扇区数  
int  NumFATs;   //FAT表个数  
int  RootEntCnt;    //根目录最大文件数  
int  TotSec; //扇区总数
int  FATSz; // FAT扇区数


#pragma pack (1) /*指定按1字节对齐*/  

/*
读取时先偏移11个字节
BS_jmpBoot 3 Bytes
BS_OEMName 8 Bytes 厂商名
*/
struct BPB {
	u16  BPB_BytsPerSec;    //每扇区字节数  
	u8   BPB_SecPerClus;    //每簇扇区数  
	u16  BPB_RsvdSecCnt;    //Boot记录占用的扇区数  
	u8   BPB_NumFATs;   //FAT表个数  
	u16  BPB_RootEntCnt;    //根目录最大文件数  
	u16  BPB_TotSec16;  // 扇区总数
	u8   BPB_Media;  // 介质描述符
	u16  BPB_FATSz16;   // 每FAT扇区数  
	u16  BPB_SecPerTrk; // 每磁道扇区数
	u16  BPB_NumHeads; // 磁头数
	u32  BPB_HiddSec; // 隐藏扇区数
	u32  BPB_TotSec32;  //如果BPB_TotSec16为0，该值为FAT扇区数  
};
/*
BPB条目结束，长度25字节
boot扇区共有512字节，结束标志0xAA55
偏移量510 Bytes,占用2 Bytes
*/

//根目录条目  
struct RootEntry {
	char DIR_Name[11]; // 文件名8字节，扩展名3字节
	u8   DIR_Attr;      //文件属性  
	char reserved[10]; //保留字段
	u16  DIR_WrtTime; //修改时间
	u16  DIR_WrtDate; // 修改日期
	u16  DIR_FstClus;   //开始簇号  
	u32  DIR_FileSize;
};
//根目录条目结束，32字节

struct FileHandle {
	RootEntry fileInfo;
	LONG offset; // 当前偏移
	u16 parentClus; //所在目录簇号，0为根目录
}; // 文件句柄内部结构

#pragma pack () /*取消指定对齐，恢复缺省对齐*/  

BOOL initBPB();
/*
用途：读取BPB块
*/

BOOL checkFile(char *pszFileName, u16 FstClus);
/*
用途：判断文件是否存在
pszFileName是文件名
FstClus是待查询文件所在目录的首簇号，0代表根目录
*/

u16  checkDirectory(char *FolderName, u16 FstClus);
/*
用途：判断目录是否存在
FolderName是目录名字
FstClus是待查询目录的上级目录的首簇号，0代表根目录
若存在，返回该目录首簇号，若不存在则返回0
*/

u16 checkPath(char *pszFolderPath);
/*
用途：判断路径是否存在
pszFolderPath是目录路径
若存在，返回路径最后一个目录的首簇号，若不存在则返回0
*/

u16 getNextFat(u16 FstClus);
/*
用途：获取文件下一簇FAT项
*/


BOOL writeEmptyClus(u16 FstClus, RootEntry* FileInfo);
/*
用途：查询可用的簇
FstClus是当前目录的首簇
RootEntry为待写入文件信息结构体指针
*/

u16 setFat(int clusNum);
/*
用途：查询可用簇，链接簇链，并初始化目录项
clusNum为需要分配的簇个数
*/

DWORD createHandle(RootEntry* FileInfo, u16 parentClus);
/*
用途：分配句柄
*/

BOOL collectClus(u16 fileClus);
/*
用途：回收簇
fileClus 首簇
*/

void recursiveDeleteDirectory(u16 fClus);
/*
用途：递归删除文件夹下的东西
fClus保存待删除文件夹首簇
*/


int* loadNumbAndData(u16 FstClus);
/*
计算循环次数与起始地址的函数
返回一个数组
*/

void initFile(RootEntry* FileInfo_ptr, char* inputFileName, u8 FileAttr, u32 FileSize);
/*

初始化文件信息，向目录项中写入各种参数，包括数据类型，名称，大小等
*/

void initFolder(RootEntry* FileInfo_ptr, char* inputFolderName, u8 FileAttr);
/*
初始化目录信息，与文件信息不同的事，不需要大小，默认为一个簇
但也需要名称，类型等信息

*/

#endif