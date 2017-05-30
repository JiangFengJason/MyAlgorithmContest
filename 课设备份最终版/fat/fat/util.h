#pragma once

#ifndef _DISK_H_
#define _DISK_H_

#define MAX_NUM 2048

typedef unsigned char u8;   //1�ֽ�  
typedef unsigned short u16; //2�ֽ�  
typedef unsigned int u32;   //4�ֽ�  

int  BytsPerSec;    //ÿ�����ֽ���  
int  SecPerClus;    //ÿ��������  
int  RsvdSecCnt;    //Boot��¼ռ�õ�������  
int  NumFATs;   //FAT�����  
int  RootEntCnt;    //��Ŀ¼����ļ���  
int  TotSec; //��������
int  FATSz; // FAT������


#pragma pack (1) /*ָ����1�ֽڶ���*/  

/*
��ȡʱ��ƫ��11���ֽ�
BS_jmpBoot 3 Bytes
BS_OEMName 8 Bytes ������
*/
struct BPB {
	u16  BPB_BytsPerSec;    //ÿ�����ֽ���  
	u8   BPB_SecPerClus;    //ÿ��������  
	u16  BPB_RsvdSecCnt;    //Boot��¼ռ�õ�������  
	u8   BPB_NumFATs;   //FAT�����  
	u16  BPB_RootEntCnt;    //��Ŀ¼����ļ���  
	u16  BPB_TotSec16;  // ��������
	u8   BPB_Media;  // ����������
	u16  BPB_FATSz16;   // ÿFAT������  
	u16  BPB_SecPerTrk; // ÿ�ŵ�������
	u16  BPB_NumHeads; // ��ͷ��
	u32  BPB_HiddSec; // ����������
	u32  BPB_TotSec32;  //���BPB_TotSec16Ϊ0����ֵΪFAT������  
};
/*
BPB��Ŀ����������25�ֽ�
boot��������512�ֽڣ�������־0xAA55
ƫ����510 Bytes,ռ��2 Bytes
*/

//��Ŀ¼��Ŀ  
struct RootEntry {
	char DIR_Name[11]; // �ļ���8�ֽڣ���չ��3�ֽ�
	u8   DIR_Attr;      //�ļ�����  
	char reserved[10]; //�����ֶ�
	u16  DIR_WrtTime; //�޸�ʱ��
	u16  DIR_WrtDate; // �޸�����
	u16  DIR_FstClus;   //��ʼ�غ�  
	u32  DIR_FileSize;
};
//��Ŀ¼��Ŀ������32�ֽ�

struct FileHandle {
	RootEntry fileInfo;
	LONG offset; // ��ǰƫ��
	u16 parentClus; //����Ŀ¼�غţ�0Ϊ��Ŀ¼
}; // �ļ�����ڲ��ṹ

#pragma pack () /*ȡ��ָ�����룬�ָ�ȱʡ����*/  

BOOL initBPB();
/*
��;����ȡBPB��
*/

BOOL checkFile(char *pszFileName, u16 FstClus);
/*
��;���ж��ļ��Ƿ����
pszFileName���ļ���
FstClus�Ǵ���ѯ�ļ�����Ŀ¼���״غţ�0�����Ŀ¼
*/

u16  checkDirectory(char *FolderName, u16 FstClus);
/*
��;���ж�Ŀ¼�Ƿ����
FolderName��Ŀ¼����
FstClus�Ǵ���ѯĿ¼���ϼ�Ŀ¼���״غţ�0�����Ŀ¼
�����ڣ����ظ�Ŀ¼�״غţ����������򷵻�0
*/

u16 checkPath(char *pszFolderPath);
/*
��;���ж�·���Ƿ����
pszFolderPath��Ŀ¼·��
�����ڣ�����·�����һ��Ŀ¼���״غţ����������򷵻�0
*/

u16 getNextFat(u16 FstClus);
/*
��;����ȡ�ļ���һ��FAT��
*/


BOOL writeEmptyClus(u16 FstClus, RootEntry* FileInfo);
/*
��;����ѯ���õĴ�
FstClus�ǵ�ǰĿ¼���״�
RootEntryΪ��д���ļ���Ϣ�ṹ��ָ��
*/

u16 setFat(int clusNum);
/*
��;����ѯ���ôأ����Ӵ���������ʼ��Ŀ¼��
clusNumΪ��Ҫ����Ĵظ���
*/

DWORD createHandle(RootEntry* FileInfo, u16 parentClus);
/*
��;��������
*/

BOOL collectClus(u16 fileClus);
/*
��;�����մ�
fileClus �״�
*/

void recursiveDeleteDirectory(u16 fClus);
/*
��;���ݹ�ɾ���ļ����µĶ���
fClus�����ɾ���ļ����״�
*/


int* loadNumbAndData(u16 FstClus);
/*
����ѭ����������ʼ��ַ�ĺ���
����һ������
*/

void initFile(RootEntry* FileInfo_ptr, char* inputFileName, u8 FileAttr, u32 FileSize);
/*

��ʼ���ļ���Ϣ����Ŀ¼����д����ֲ����������������ͣ����ƣ���С��
*/

void initFolder(RootEntry* FileInfo_ptr, char* inputFolderName, u8 FileAttr);
/*
��ʼ��Ŀ¼��Ϣ�����ļ���Ϣ��ͬ���£�����Ҫ��С��Ĭ��Ϊһ����
��Ҳ��Ҫ���ƣ����͵���Ϣ

*/

#endif