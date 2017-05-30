#include "fat.h"
#include "util.h"
#include "time.h"

#pragma comment(lib, "DiskLib.lib")
using namespace std;

const char* fs = "d:\\floppy.img";

FileHandle* dwHandles[MAX_NUM] = { NULL };

u8* setzero = (u8*)calloc(512, sizeof(u8)); // 用于创建目录时清0

BPB bpb;
BPB* bpb_ptr = &bpb;

RootEntry rootEntry;
RootEntry* rootEntry_ptr = &rootEntry;


//1.创建文件api 已测试成功    是我们自己的
DWORD MyCreateFile(char *pszFolderPath, char *pszFileName)
{
	DWORD FileHandle = 0;
	u16 fstClus;
	u32 FileSize = 0; // 初始值为0
	RootEntry fileInfo;
	RootEntry* fileInfoPtr = &fileInfo;
	//按字节置位
	memset(fileInfoPtr, 0, sizeof(RootEntry));
	if (initBPB())
	{
		// 路径存在或者为根目录
		if ((fstClus = checkPath(pszFolderPath)) || strlen(pszFolderPath) == 3)
		{
			if (!checkFile(pszFileName, fstClus))
			{//文件不存在
				initFile(fileInfoPtr, pszFileName, 0x20, FileSize);
				if (writeEmptyClus(fstClus, fileInfoPtr) == TRUE)
					// 创建句柄
					FileHandle = createHandle(fileInfoPtr, fstClus);
			}
			else//文件已存在
				printf("%s\\%s has existed.", pszFolderPath, pszFileName);
		}
	}
	ShutdownDisk();
	return FileHandle;
}





//2.打开文件api 
DWORD MyOpenFile(char *pszFolderPath, char *pszFileName)
{
	DWORD FileHandle = 0;
	u16 fstClus = 0;
	BOOL checkExist = FALSE;
	char myFileName[12];
	RootEntry fileInfo;
	RootEntry* fileInfoPtr = &fileInfo;
	if (initBPB())
	{    //目录路径判断是否存在
		if ((fstClus = checkPath(pszFolderPath)) || strlen(pszFolderPath) == 3)
		{
			u16 parentClus = fstClus;
			if (checkFile(pszFileName, fstClus))
			{
				int *numbAndData;
				int daPos;
				while (1)
				{
					numbAndData = loadNumbAndData(fstClus);
					int daPos = numbAndData[0];
					//daPos代表缓冲区长度
					int num = numbAndData[1];
					//num代表循环次数;
					for (int n = 0; n < num; n++)
					{
						SetHeaderOffset(daPos, NULL, FILE_BEGIN);
						if (ReadFromDisk(fileInfoPtr, 32, NULL) != 0)
						{
							// 目录0x10，文件0x20，卷标0x28
							if (fileInfoPtr->DIR_Name[0] != 0xE5 && fileInfoPtr->DIR_Name[0] != 0 && fileInfoPtr->DIR_Name[0] != 0x2E)
							{//文件名的长度计数器
								int lenOfFileName = 0;
								int m = 0;
								//前8位读入文件的缓冲区
								for (m = 0; m < 8; m++, lenOfFileName++)
								{
									if (fileInfoPtr->DIR_Name[m] != ' ')
										myFileName[lenOfFileName] = fileInfoPtr->DIR_Name[m];
									else
									{
										myFileName[lenOfFileName] = '.';
										break;
									}
								}
								//后3位后缀的读入
								while (m<11)
								{
									m++;
									if (fileInfoPtr->DIR_Name[m] != ' ')
									{
										lenOfFileName++;
										myFileName[lenOfFileName] = fileInfoPtr->DIR_Name[m];
									}
								}
								myFileName[lenOfFileName + 1] = '\0';
								// 忽略大小写比较
								if (_stricmp(myFileName, pszFileName) == 0)
								{
									checkExist = TRUE;
									break;
								}
							}
						}
						daPos += 32;
					}
					if (checkExist)
					{
						FileHandle = createHandle(fileInfoPtr, parentClus);
						break;
					}
					//跳出条件为对簇的判断
					if ((fstClus = getNextFat(fstClus)) == 0xFFF || fstClus == 0) break;
				}
			}
		}
	}
	ShutdownDisk();
	return FileHandle;
}


//3.关闭文件api
void MyCloseFile(DWORD dwHandle) {
	free(dwHandles[dwHandle]);
	dwHandles[dwHandle] = NULL;
}


//4.文件写入api   现在是俊钦的，成功了
DWORD MyWriteFile(DWORD dwHandle, LPVOID pBuffer, DWORD dwBytesToWrite) {
	DWORD result = 0;
	FileHandle* hd = dwHandles[dwHandle];
	if (hd == NULL || initBPB() == FALSE) return -1;
	u16 FstClus = hd->fileInfo.DIR_FstClus;
	LONG offset = hd->offset; // 文件指针当前偏移
	int curClusNum = offset / BytsPerSec; // 当前指针在第几个扇区
	int curClusOffset = offset % BytsPerSec; // 当前在扇区内偏移
	while (curClusNum) {
		if (getNextFat(FstClus) == 0xFFF) {
			break;
		}
		FstClus = getNextFat(FstClus);
		curClusNum--;
	}// 获取当前指针所指扇区
	int dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (FstClus - 2) * BytsPerSec;
	int dataOffset = dataBase + curClusOffset; // 拿到文件指针所指位置
	int lenOfBuffer = dwBytesToWrite; // 缓冲区待写入长度
	char* cBuffer = (char*)malloc(sizeof(u8)*lenOfBuffer);
	memcpy(cBuffer, pBuffer, lenOfBuffer); // 复制过来
	SetHeaderOffset(dataOffset, NULL, FILE_BEGIN);
	if ((BytsPerSec - curClusOffset >= lenOfBuffer) && curClusNum == 0) {
		if (WriteToDisk(pBuffer, lenOfBuffer, &result) == 0) {
			return -1;
		}
	}
	else {
		DWORD temp;
		u16 tempClus;
		u16 bytes; // 每次读取的簇号
		u16* bytes_ptr = &bytes;
		int fatBase = RsvdSecCnt * BytsPerSec;
		int leftLen = lenOfBuffer;
		int hasWritten = 0;
		if (curClusNum == 0) {
			if (WriteToDisk(pBuffer, BytsPerSec - curClusOffset, &temp) == 0) {
				return -1;
			}
			result += temp; // 记录写入长度
			leftLen = lenOfBuffer - (BytsPerSec - curClusOffset); // 剩余长度
			hasWritten = BytsPerSec - curClusOffset;
		}
		do {
			tempClus = getNextFat(FstClus); // 尝试拿下一个FAT
			if (tempClus == 0xFFF) {
				tempClus = setFat(1);
				if (tempClus == 0) return -1; //分配簇失败
				SetHeaderOffset((fatBase + FstClus * 3 / 2), NULL, FILE_BEGIN);
				if (ReadFromDisk(bytes_ptr, 2, NULL) != 0) {
					if (FstClus % 2 == 0) {
						bytes = bytes >> 12;
						bytes = bytes << 12; // 保留高四位，低12位为0
						bytes = bytes | tempClus;
					}
					else {
						bytes = bytes << 12;
						bytes = bytes >> 12; // 保留低四位，高12位为0
						bytes = bytes | (tempClus << 4);
					}
					SetHeaderOffset((fatBase + FstClus * 3 / 2), NULL, FILE_BEGIN);
					if (WriteToDisk(bytes_ptr, 2, NULL) == 0) {
						return -1;
					}
				}
			}
			FstClus = tempClus; // 真正拿到下一个FAT
			dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (FstClus - 2) * BytsPerSec; // 刷新扇区偏移
			SetHeaderOffset(dataBase, NULL, FILE_BEGIN); // 一定是从扇区头开始写
			if (leftLen > BytsPerSec) {
				if (WriteToDisk(&cBuffer[hasWritten], BytsPerSec, &temp) == 0) {
					return -1;
				}
				hasWritten += BytsPerSec;
			}
			else {
				if (WriteToDisk(&cBuffer[hasWritten], leftLen, &temp) == 0) {
					return -1;
				}
				hasWritten += leftLen;
			}
			leftLen -= BytsPerSec;
			result += temp;
		} while (leftLen > 0);
	}
	// 刷新文件大小
	if ((offset + result) > hd->fileInfo.DIR_FileSize) {
		int dBase;
		BOOL isExist = FALSE;
		hd->fileInfo.DIR_FileSize += (offset + result) - hd->fileInfo.DIR_FileSize;
		// 遍历当前目录所有项目
		u16 parentClus = hd->parentClus;
		do {
			int loop;
			if (parentClus == 0) {
				dBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec;
				loop = RootEntCnt;
			}
			else {
				dBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (parentClus - 2) * BytsPerSec;
				loop = BytsPerSec / 32;
			}
			for (int i = 0; i < loop; i++) {
				SetHeaderOffset(dBase, NULL, FILE_BEGIN);
				if (ReadFromDisk(rootEntry_ptr, 32, NULL) != 0) {
					if (rootEntry_ptr->DIR_Attr == 0x20) {
						if (_stricmp(rootEntry_ptr->DIR_Name, hd->fileInfo.DIR_Name) == 0) {
							SetHeaderOffset(dBase, NULL, FILE_BEGIN);
							WriteToDisk(&hd->fileInfo, 32, NULL);
							isExist = TRUE;
							break;
						}
					}
				}
				dBase += 32;
			}
			if (isExist) break;
		} while ((parentClus = getNextFat(parentClus)) != 0xFFF && parentClus != 0);
		
	}
	ShutdownDisk();
	MySetFilePointer(dwHandle, result, MY_FILE_CURRENT); //偏移量刷新
	return result;
}

/*

DWORD MyWriteFile(DWORD dwHandle, LPVOID pBuffer, DWORD dwBytesToWrite)
{

	RootEntry root;
	RootEntry* rootPtr = &root;

	DWORD sign = 0;
	//获取句柄
	FileHandle* hd = dwHandles[dwHandle];
	//错误处理
	if (hd == NULL || initBPB() == FALSE) return -1;
	//获取首簇
	u16 fstClus = hd->fileInfo.DIR_FstClus;
	LONG offset = hd->offset; // 文件指针当前偏移
	int numOfClus = offset / BytsPerSec; // 当前指针在第几个扇区
	int offsetOfClus = offset % BytsPerSec; // 当前在扇区内偏移
	while (numOfClus) {
		if (getNextFat(fstClus) == 0xFFF)break;
		fstClus = getNextFat(fstClus);
		numOfClus--;
	}// 获取当前指针所指扇区
	int daPos = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (fstClus - 2) * BytsPerSec;
	int daOffset = daPos + offsetOfClus; // 拿到文件指针所指位置
	int lenOfBuffer = dwBytesToWrite; // 缓冲区待写入长度
	char* cBuffer = (char*)malloc(sizeof(u8)*lenOfBuffer);
	memcpy(cBuffer, pBuffer, lenOfBuffer); // 复制过来
	SetHeaderOffset(daOffset, NULL, FILE_BEGIN);
	//错误处理
	if ((BytsPerSec - offsetOfClus >= lenOfBuffer) && numOfClus != 0)
	{
		DWORD temp;
		u16 tempClus;
		u16 bytes; // 每次读取的簇号
		u16* bytes_ptr = &bytes;
		int fatPos = RsvdSecCnt * BytsPerSec;
		int leftLen = lenOfBuffer;
		int haveWritten = 0;
		if (numOfClus == 0)
		{
			if (WriteToDisk(pBuffer, BytsPerSec - offsetOfClus, &temp) != 0)
			{
				sign += temp; // 记录写入长度
				leftLen = lenOfBuffer - (BytsPerSec - offsetOfClus); // 剩余长度
				haveWritten = BytsPerSec - offsetOfClus;//已经写入的长度
			}
			else
				return -1;
		}
		while (1)
		{
			tempClus = getNextFat(fstClus); // 尝试拿下一个FAT
			if (tempClus == 0xFFF)
			{
				tempClus = setFat(1);
				SetHeaderOffset((fatPos + fstClus * 3 / 2), NULL, FILE_BEGIN);
				if (ReadFromDisk(bytes_ptr, 2, NULL) != 0)
				{
					if (fstClus % 2 == 0)
					{
						bytes = bytes >> 12;
						bytes = bytes << 12; // 保留高四位，低12位为0
						bytes = bytes | tempClus;
					}
					else
					{
						bytes = bytes << 12;
						bytes = bytes >> 12; // 保留低四位，高12位为0
						bytes = bytes | (tempClus << 4);
					}
					SetHeaderOffset((fatPos + fstClus * 3 / 2), NULL, FILE_BEGIN);
					if (WriteToDisk(bytes_ptr, 2, NULL))
						return -1;
				}
			}
			fstClus = tempClus; // 真正拿到下一个FAT
			daPos = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (fstClus - 2) * BytsPerSec; // 刷新扇区偏移
			SetHeaderOffset(daPos, NULL, FILE_BEGIN); // 一定是从扇区头开始写
			int little;
			little = (leftLen > BytsPerSec) ? BytsPerSec : leftLen;
			if (WriteToDisk(&cBuffer[haveWritten], little, &temp) != 0)
			{
				//已经写过的内容做增加
				haveWritten += little;
				leftLen -= BytsPerSec;
				sign += temp;
			}
			else
				return -1;
			if (leftLen <= 0) break;
		}
	}
	else
	{
		if (!WriteToDisk(pBuffer, lenOfBuffer, &sign))
			return -1;
	}
	// 刷新文件大小
	if ((offset + sign) > hd->fileInfo.DIR_FileSize)
	{
		int *numbAndData;
		BOOL checkExist = FALSE;
		hd->fileInfo.DIR_FileSize += (offset + sign) - hd->fileInfo.DIR_FileSize;
		// 遍历当前目录所有项目
		u16 parentClus = hd->parentClus;
		while (1)
		{
			numbAndData = loadNumbAndData(parentClus);
			int data = numbAndData[0];
			//daba代表缓冲区长度
			int numb = numbAndData[1];
			//numb代表循环次数
			for (int i = 0; i < numb; i++)
			{
				SetHeaderOffset(data, NULL, FILE_BEGIN);
				//从磁盘获取其中的文件信息
				if (ReadFromDisk(rootPtr, 32, NULL) != 0)
				{
					if (rootPtr->DIR_Attr == 0x20)
					{
						if (_stricmp(rootPtr->DIR_Name, hd->fileInfo.DIR_Name) == 0)
						{
							//写入磁盘
							SetHeaderOffset(data, NULL, FILE_BEGIN);
							WriteToDisk(&hd->fileInfo, 32, NULL);
							checkExist = TRUE;
							break;
						}
					}
				}
				data += 32;
			}
			if (checkExist) break;
			if ((parentClus = getNextFat(parentClus)) == 0xFFF || parentClus == 0) break;
		}
	}
	ShutdownDisk();
	MySetFilePointer(dwHandle, sign, MY_FILE_CURRENT); //偏移量刷新
	return sign;
}
*/




//5.文件读取api
DWORD MyReadFile(DWORD dwHandle, LPVOID pBuffer, DWORD dwBytesToRead)
{
	DWORD sign = 0;
	//获取句柄
	FileHandle* hd = dwHandles[dwHandle];
	//错误处理
	if (hd == NULL || initBPB() == FALSE) return -1;
	//获取首簇
	u16 fstClus = hd->fileInfo.DIR_FstClus;
	LONG offset = hd->offset; // 文件指针当前偏移
	int numOfClus = offset / BytsPerSec; // 当前指针在第几个扇区
	int offsetOfClus = offset % BytsPerSec; // 当前在扇区内偏移
	while (numOfClus)
	{
		if (getNextFat(fstClus) == 0xFFF) break;
		fstClus = getNextFat(fstClus);
		numOfClus--;
	}// 获取当前指针所指扇区
	if (numOfClus > 0 || offset > hd->fileInfo.DIR_FileSize)
		return -1; // 超出文件偏移范围了
	int daPos = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (fstClus - 2) * BytsPerSec;
	int daOffset = daPos + offsetOfClus; // 拿到文件指针所指位置
	int lenOfBuffer = dwBytesToRead; // 缓冲区待读入长度
	if ((int)hd->fileInfo.DIR_FileSize - offset < lenOfBuffer)
		lenOfBuffer = hd->fileInfo.DIR_FileSize - offset;
	char* cBuffer = (char*)malloc(sizeof(u8)*lenOfBuffer); // 创建一个缓冲区
	memset(cBuffer, 0, lenOfBuffer);
	SetHeaderOffset(daOffset, NULL, FILE_BEGIN);
	// 读取
	if (BytsPerSec - offsetOfClus >= lenOfBuffer)
	{
		if (ReadFromDisk(cBuffer, lenOfBuffer, &sign))
			return -1;
	}
	else
	{
		DWORD temp;
		if (ReadFromDisk(cBuffer, BytsPerSec - offsetOfClus, &temp) != 0)
		{
			sign += temp; // 记录读取到的长度
			int leftLen = lenOfBuffer - (BytsPerSec - offsetOfClus); // 剩余长度
			int hasRead = BytsPerSec - offsetOfClus;
			while (1)
			{
				fstClus = getNextFat(fstClus); // 拿到下一个FAT
				if (fstClus == 0xFFF)
					break;
				daPos = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (fstClus - 2) * BytsPerSec; // 刷新扇区偏移
				SetHeaderOffset(daPos, NULL, FILE_BEGIN);
				int little;
				//取其中较小的那个
				little = (leftLen > BytsPerSec) ? BytsPerSec : leftLen;
				if (ReadFromDisk(&cBuffer[hasRead], little, &temp) != 0)
				{
					//已经读过的内容做增加
					hasRead += little;
					leftLen -= BytsPerSec; // 直接减掉一个扇区，只要是<=0就退出循环
					sign += temp;
				}
				else
					return -1;
				if (leftLen <= 0) break;
			}
		}
		else
			return -1;
	}
	memcpy(pBuffer, cBuffer, lenOfBuffer); // 写入缓冲区
	ShutdownDisk();
	MySetFilePointer(dwHandle, sign, MY_FILE_CURRENT); //偏移量刷新
	return sign;
}


//6.删除文件api   已测试成功
BOOL MyDeleteFile(char *pszFolderPath, char *pszFileName)
{
	//打开硬盘，初始化BPB等操作
	if (!initBPB())
		return false;

	RootEntry FileInfo;
	RootEntry* FileInfo_ptr = &FileInfo;

	//返回的标识符
	BOOL sign = false;

	//接收文件名，13位字符数组，8位名称+.+3位后缀+'\0'
	char fileName[13];

	//储存首簇号
	u16 fstClus;
	//先判断路径是否存在
	if ((fstClus = checkPath(pszFolderPath)) || strlen(pszFolderPath) == 3)
	{
		//在判断文件是否存在
		if (checkFile(pszFileName, fstClus))
		{
			int *numbAndData;
			int dataBase;
			int loop;//这里的两个变量都可以换一下名字



			while (true)
			{
				//得到偏移地址和循环次数
				numbAndData = loadNumbAndData(fstClus);
				dataBase = numbAndData[0];
				loop = numbAndData[1];

				int a;

				for (int i = 0; i < loop; i++)
				{
					SetHeaderOffset(dataBase, NULL, FILE_BEGIN);

					if (ReadFromDisk(FileInfo_ptr, 32, NULL))
					{
						//判断是否符合类比ue
						if (FileInfo_ptr->DIR_Name[0] != 0xE5 && FileInfo_ptr->DIR_Name[0] != 0 && FileInfo_ptr->DIR_Name[0] != 0x2E)
						{
							int n = 0;
							//是文件的话
							if (FileInfo_ptr->DIR_Attr == 0x20)
							{
								//将前8位截取出来
								for (a = 0; a< 8; a++)
								{
									if (FileInfo_ptr->DIR_Name[a] != ' ')
									{
										fileName[n] = FileInfo_ptr->DIR_Name[a];
										n++;
									}
									else
									{
										fileName[n] = '.';
										n++;
										break;
									}
								}
								//后三位后缀名直接赋值
								a = 8;
								while (a< 11)
								{
									fileName[n] = FileInfo_ptr->DIR_Name[a];
									a++;
									n++;

								}
								//最后一位置为\0
								fileName[n] = '\0';

								//调用字符串比较函数，如果函数返回0代表比对成功
								if (_stricmp(fileName, pszFileName) == 0)
								{
									u8 del = 0xE5;
									SetHeaderOffset(dataBase, NULL, FILE_BEGIN);
									//将删除信息写入磁盘里

									if (WriteToDisk(&del, 1, NULL))
									{
										//写入进磁盘后，还要进行回收簇的操作
										collectClus(FileInfo_ptr->DIR_FstClus);
										sign = true;
									}

								}

							}
						}

					}

					//偏移向后32字节
					dataBase += 32;
				}

				if (sign)
					break;
				if ((fstClus = getNextFat(fstClus)) == 0xFFF || fstClus == 0)
					break;

			}
		}

	}
	ShutdownDisk();
	return sign;

}



//7.创建目录api
BOOL MyCreateDirectory(char *pszFolderPath, char *pszFolderName)
{
	RootEntry root;
	RootEntry* rootPtr = &root;

	u16 fstClus;
	u16 lastClus;
	BOOL sign = FALSE;
	int *numbAndData;
	int daPos;

	if (initBPB()) {
		// 路径存在或者为根目录
		if ((fstClus = checkPath(pszFolderPath)) || strlen(pszFolderPath) == 3)
		{ //保留首簇
			lastClus = fstClus;
			//文件夹路径是否已存在
			if (checkDirectory(pszFolderName, fstClus))
				cout << pszFolderPath << '\\' << pszFolderName << " has existed!" << endl;
			else {
				while (1){
					numbAndData = loadNumbAndData(fstClus);
					daPos = numbAndData[0];
					//daPos代表缓冲区长度
					int num = numbAndData[1];
					//num代表循环次数
					for (int i = 0; i < num; i++)
					{
						SetHeaderOffset(daPos, NULL, FILE_BEGIN);
						if (ReadFromDisk(rootPtr, 32, NULL) != 0)
						{
							// 目录项可用
							if (rootPtr->DIR_Name[0] == 0x00 || rootPtr->DIR_Name[0] == 0xE5)
							{// 初始化文件夹信息
								initFolder(rootPtr, pszFolderName, 0x10);
								SetHeaderOffset(daPos, NULL, FILE_BEGIN); // 磁头复位
								if (WriteToDisk(rootPtr, 32, NULL) != 0)
								{
									// 创建 . 和 ..
									int dBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (rootPtr->DIR_FstClus - 2) * BytsPerSec;
									SetHeaderOffset(dBase, NULL, FILE_BEGIN);
									//setzero在之前定义，是置空的作用
									WriteToDisk(setzero, BytsPerSec, NULL); // 目录创建初始清0
									// . 创建
									SetHeaderOffset(dBase, NULL, FILE_BEGIN);
									rootPtr->DIR_FileSize = 0;
									//. 后面的刷为0
									for (int i = 0; i < 11; i++) {
										rootPtr->DIR_Name[i] = 0x20;
									}
									//更新第一位为0
									rootPtr->DIR_Name[0] = 0x2E;
									//写入磁盘
									WriteToDisk(rootPtr, 32, NULL);
									// ..  创建
									//移动到下一个32位的位置
									SetHeaderOffset(dBase + 32, NULL, FILE_BEGIN);
									//更新第二位为2E
									rootPtr->DIR_Name[1] = 0x2E;
									rootPtr->DIR_FstClus = lastClus;
									//写入磁盘
									WriteToDisk(rootPtr, 32, NULL);
									sign = TRUE;
									break;
								}
							}
						}
						daPos += 32;
					}
					if (sign) break;
					if ((fstClus = getNextFat(fstClus)) == 0xFFF || fstClus == 0) break;
				}
			}
		}
	}
	ShutdownDisk();
	return sign;
}

//8.偏移函数，是俊钦的，到时测试替换
BOOL MySetFilePointer(DWORD dwFileHandle, int nOffset, DWORD dwMoveMethod) {
	FileHandle* hd = dwHandles[dwFileHandle];
	if (hd == NULL || initBPB() == FALSE) return FALSE; // 句柄不存在
	LONG curOffset = nOffset + hd->offset; // current模式下偏移后的位置
	u16 currentClus = hd->fileInfo.DIR_FstClus; // 首簇
	int fileSize = hd->fileInfo.DIR_FileSize; // 文件大小
	int fileBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (currentClus - 2) * BytsPerSec;
	switch (dwMoveMethod) {
	case MY_FILE_BEGIN:
		if (nOffset < 0) {
			hd->offset = 0; // 小于0，置为0
		}
		else if (nOffset > fileSize) {
			hd->offset = fileSize;
		}
		else {
			hd->offset = nOffset;
		}
		break;
	case MY_FILE_CURRENT:
		if (curOffset < 0) {
			hd->offset = 0;
		}
		else if (curOffset > fileSize) {
			hd->offset = fileSize;
		}
		else {
			hd->offset = curOffset;
		}
		break;
	case MY_FILE_END:
		if (nOffset > 0) {
			hd->offset = fileSize;
		}
		else if (nOffset < -fileSize) {
			hd->offset = 0;
		}
		else {
			hd->offset = fileSize + nOffset;
		}
		break;
	}
	ShutdownDisk();
	return TRUE;
}


//9.删除目录函数  已经换成我的了
BOOL MyDeleteDirectory(char *pszFolderPath, char *pszFolderName)
{
	//定义首簇和返回的标识符
	u16 fstClus;
	BOOL sign = FALSE;

	if (initBPB())
	{
		fstClus = checkPath(pszFolderPath);

		//簇号存在，或者直接是根目录C:\\
								if ((fstClus || strlen(pszFolderPath) == 3)&&checkDirectory(pszFolderName, FstClus))
		{
			//调用封装好的函数，接收偏移量和循环次数

			int *numbAndData;
			int dataBase;
			int loop;

			//文件名存储和回收标志
			char dirName[12];
			u8 del = 0xE5;

			//根目录项
			RootEntry delD;
			RootEntry* delD_ptr = &delD;

			int m;

			while (true)
			{
				//得到偏移地址和循环次数,偏移地址随着首簇号的改变在一直改变
				numbAndData = loadNumbAndData(fstClus);
				dataBase = numbAndData[0];
				loop = numbAndData[1];

				//进行循环搜索，搜索该目录下的所有子目录和文件
				for (int i = 0; i < loop; i++)
				{
					//先偏移到制定位置
					SetHeaderOffset(dataBase, NULL, FILE_BEGIN);
					if (ReadFromDisk(delD_ptr, 32, NULL))
					{
						//再判断标志位是否是其他
						if (delD_ptr->DIR_Name[0] != 0xE5 && delD_ptr->DIR_Name[0] != 0 && delD_ptr->DIR_Name[0] != 0x2E)
						{
							if (delD_ptr->DIR_Attr == 0x10)
							{
								//如果读取到的是子目录
								for (m = 0; m < 11; m++)
								{
									//将名称存储进数组中，遇到为空的则跳出
									if (delD_ptr->DIR_Name[m] != ' ')
									{
										dirName[m] = delD_ptr->DIR_Name[m];
									}
									else
										break;
								}

								//将最后一位置为\0
								dirName[m] = '\0';

								//比较得到的名字与传入的名字，一致的话，函数返回为0，调用递归函数删除
								if (_stricmp(dirName, pszFolderName) == 0)
								{
									//递归删除目录函数
									recursiveDeleteDirectory(delD_ptr->DIR_FstClus);

									//之后将0xE5写入
									SetHeaderOffset(dataBase, NULL, FILE_BEGIN);

									if (WriteToDisk(&del, 1, NULL))
									{
										//并进行簇的回收工作
										sign = collectClus(delD_ptr->DIR_FstClus);
										break;
									}
								}

							}

						}
					}

					dataBase += 32;
				}

				if (sign)
					break;
				fstClus = getNextFat(fstClus);
				if (fstClus == 0xFFF || fstClus == 0)
					break;
			}
		}

	}
	ShutdownDisk();
	return sign;

}


/*
BOOL MyDeleteDirectory(char *pszFolderPath, char *pszFolderName) {
	u16 FstClus;
	BOOL result = FALSE;
	if (strlen(pszFolderName) > 11 || strlen(pszFolderName) <= 0) return FALSE;
	if (initBPB()) {
		// 路径存在或者为根目录
		if ((FstClus = checkPath(pszFolderPath)) || strlen(pszFolderPath) == 3) {
			// 待删除目录存在
			if (checkDirectory(pszFolderName, FstClus)) {
				int dataBase;
				int loop;
				char directory[12];
				u8 del = 0xE5;
				RootEntry fd;
				RootEntry* fd_ptr = &fd;
				do {
					if (FstClus == 0) {
						// 根目录区偏移
						dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec;
						loop = RootEntCnt;
					}
					else {
						// 数据区文件首址偏移
						dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (FstClus - 2) * BytsPerSec;
						loop = BytsPerSec / 32;
					}
					for (int i = 0; i < loop; i++) {
						SetHeaderOffset(dataBase, NULL, FILE_BEGIN);
						if (ReadFromDisk(fd_ptr, 32, NULL) != 0) {
							if (fd_ptr->DIR_Name[0] != 0xE5 && fd_ptr->DIR_Name[0] != 0 && fd_ptr->DIR_Name[0] != 0x2E) {
								// 目录0x10，文件0x20，卷标0x28
								if (fd_ptr->DIR_Attr == 0x10) {
									for (int j = 0; j < 11; j++) {
										if (fd_ptr->DIR_Name[j] != ' ') {
											directory[j] = fd_ptr->DIR_Name[j];
											if (j == 10) {
												directory[11] = '\0';
												break;
											}
										}
										else {
											directory[j] = '\0';
											break;
										}
									}
									// 忽略大小写比较
									if (_stricmp(directory, pszFolderName) == 0) {
										recursiveDeleteDirectory(fd_ptr->DIR_FstClus);
										// 删除该文件夹
										SetHeaderOffset(dataBase, NULL, FILE_BEGIN);
										if (WriteToDisk(&del, 1, NULL) != 0) {
											result = collectClus(fd_ptr->DIR_FstClus); // 传入首簇，回收
											break;
										}
									}
								}
							}
						}
						dataBase += 32;
					}
					if (result) break;
				} while ((FstClus = getNextFat(FstClus)) != 0xFFF && FstClus != 0);
			}
		}
	}
	
	ShutdownDisk();
	return result;
}
*/

//1.检查文件是否存在





BOOL checkFile(char *inputFileName, u16 fstClus)
{

	RootEntry root;
	RootEntry* rootPtr = &root;

	char myfilename[13];
	int *numbAndData;
	BOOL checkExist = FALSE;
	// 遍历当前目录所有项目
	while (1)
	{
		//找到根目录区以及数据区首址
		numbAndData = loadNumbAndData(fstClus);
		int daPos = numbAndData[0];
		//daPos代表缓冲区长度
		int num = numbAndData[1];
		//num代表循环次数
		for (int m = 0; m < num; m++)
		{
			SetHeaderOffset(daPos, NULL, FILE_BEGIN);
			if (ReadFromDisk(rootPtr, 32, NULL) != 0)
			{
				// 目录0x10，文件0x20，卷标0x28
				if (rootPtr->DIR_Name[0] != 0xE5 && rootPtr->DIR_Name[0] != 0 && rootPtr->DIR_Name[0] != 0x2E)
				{
					int lenOfMyFileName = 0;
					if (rootPtr->DIR_Attr == 0x20)
					{
						int n = 0;
						while (n < 8)//前8位的文件名
						{
							if (rootPtr->DIR_Name[n] != ' ')
								myfilename[lenOfMyFileName++] = rootPtr->DIR_Name[n];
							n++;
						}
						myfilename[lenOfMyFileName++] = '.';
						n = 8;
						while (n < 11)//后3位的后缀
						{
							if (rootPtr->DIR_Name[n] != ' ')
								myfilename[lenOfMyFileName++] = rootPtr->DIR_Name[n];
							n++;
						}
						//c中字符串是以/0结尾对吧，所以加0
						myfilename[lenOfMyFileName] = '\0';
						// 忽略大小写比较，次函数调用时，如果两个字符串相等，则结果为0
						if (_stricmp(myfilename, inputFileName) == 0) {
							checkExist = TRUE;
							break;
						}
					}
				}
				daPos += 32;
			}
			if (checkExist) break;
		}
		if ((fstClus = getNextFat(fstClus)) == 0xFFF || fstClus == 0) break;
	}
	return checkExist;
}



//2.检查目录是否存在
u16 checkDirectory(char *inputFolderName, u16 fstClus) 
{

	RootEntry root;
	RootEntry* rootPtr = &root;

	char directoryName[12];
	int *numbAndData;
	u16 checkExist = 0;
	// 遍历当前目录所有项目
	while (1)
	{
		numbAndData = loadNumbAndData(fstClus);
		int daPos = numbAndData[0];
		//daPos代表缓冲区长度
		int num = numbAndData[1];
		//num代表循环次数
		for (int m = 0; m < num; m++)
		{

			//移动到偏移的位置
			SetHeaderOffset(daPos, NULL, FILE_BEGIN);

			//为什么直接读32位呢？  因为目录每项就为32位
			//返回0，代表读取失败，也就是没有目录
			if (ReadFromDisk(rootPtr, 32, NULL) != 0)
			{
				// 目录0x10，文件0x20，卷标0x28
				if (rootPtr->DIR_Name[0] != 0xE5 && rootPtr->DIR_Name[0] != 0 && rootPtr->DIR_Name[0] != 0x2E)
				{
					if (rootPtr->DIR_Attr == 0x10)
					{
						int n = 0;
						while (n<11)
						{//将对应的文件夹名转换为正常的名字与输入参数作比较
							if (rootPtr->DIR_Name[n] != ' ')
							{ //文件夹名不为空，则将名字录入到自定义的数组内
								directoryName[n] = rootPtr->DIR_Name[n];
								//直到第10位，补结束符
								if (n == 10)
								{
									directoryName[11] = '\0';
									break;
								}
							}
							//哪一位为空就加结束符，并且跳出循环
							else
							{
								directoryName[n] = '\0';
								break;
							}
							n++;
						}
						// 忽略大小写比较
						if (_stricmp(directoryName, inputFolderName) == 0)
						{
							checkExist = rootPtr->DIR_FstClus;
							break;
						}
					}
				}
			}
			//没有读到，那么偏移量+32个字节
			daPos += 32;
		}
		if (checkExist) break;
		if ((fstClus = getNextFat(fstClus)) == 0xFFF || fstClus == 0) break;
	}
	return checkExist;
}






//3.检查路径是否存在
u16 checkPath(char *pszFolderPath)
{
	//存放目录名称的字符数组，为什么是12位呢?因为要存储最后一位\0
	char dirName[12];

	//要返回的首簇号
	u16 fstClus = 0;

	//i=3，因为盘符占3位，直接去掉
	int i = 3;
	//记录dirName数组的位置
	int j = 0;

	int length = 0;

	//利用while循环，实现多级目录的遍历与检查，如果有一级目录不符，则跳出
	while (pszFolderPath[i] != '\0'&& length <= 11)
	{
		if (pszFolderPath[i] != '\\')
		{
			dirName[j] = pszFolderPath[i];
			//接收到有效字符，都偏移一位
			i++;
			j++;
			length++;

		}
		//如果遇到分割符号\，则检查之前保存的目录名，并重新开始计数
		else if (pszFolderPath[i] == '\\')
		{
			//表示目录名的结束
			dirName[j] = '\0';
			//检查是否存在这个目录名
			fstClus = checkDirectory(dirName, fstClus);
			if (fstClus)
			{

				//如果存在，把字符数组原有的项抹去，这里置为' '
				while (j >= 0)
				{
					dirName[j] = ' ';
					j--;
				}
				//同时将j恢复到0.也就是开始位置
				j++;
				i++;
				length = 0;
			}
			else
			{
				//如果没有找到该目录，那么结束判断，返回0
				return fstClus;
			}

		}
	}
	//正常跳出，再进行判断然后返回首簇号
	if (j > 0)
	{
		dirName[j] = '\0';
		fstClus = checkDirectory(dirName, fstClus);
	}

	return fstClus;

}

//4.获取下一个簇号
u16  getNextFat(u16 fstClus)
{
	//计算Boot区域大小
	int bootSize = RsvdSecCnt * BytsPerSec;

	//fat位置的具体偏移量是boot区域+簇号*3/2
	int fatPos = bootSize + fstClus * 3 / 2;

	//用两个字节存储fat信息，但是fat只有12位，所以要进行位移操作
	u16  twoByte;
	u16*  twoByte_ptr = &twoByte;

	SetHeaderOffset(fatPos, NULL, FILE_BEGIN);

	if (ReadFromDisk(twoByte_ptr, 2, NULL))
	{
		//簇号为奇数和偶数时候，是不一样的
		if (fstClus % 2 == 0)
		{
			twoByte = twoByte << 4;
			twoByte = twoByte >> 4;

		}

		else
		{
			twoByte = twoByte >> 4;

		}
		return twoByte;

	}


}


//5.初始化目录信息函数
void initFolder(RootEntry* fileInfoPtr, char* inputFolderName, u8 fileAtter)
{
	time_t tm = getTS();
	fileInfoPtr->DIR_Attr = fileAtter;
	fileInfoPtr->DIR_WrtTime = getDOSTime(tm);
	fileInfoPtr->DIR_WrtDate = getDOSDate(tm);
	fileInfoPtr->DIR_FileSize = 0;
	int n = 0;
	for (n = 0; n < 11; n++)
	{
		if (inputFolderName[n] != '\0')
			//将名字写入DIR_Name
			fileInfoPtr->DIR_Name[n] = inputFolderName[n];
		else
			break;

	}
	while (n < 11)
	{   //如果不足11位补0即0x20
		fileInfoPtr->DIR_Name[n] = 0x20;
		n++;
	}
	//把所需的簇的个数传给FAT表
	fileInfoPtr->DIR_FstClus = setFat(1);
}



//6.初始化文件信息函数
void initFile(RootEntry* fileInfoPtr, char* inputFileName, u8 fileAtter, u32 FileSize)
{
	time_t tm = getTS();
	fileInfoPtr->DIR_Attr = fileAtter;
	fileInfoPtr->DIR_WrtTime = getDOSTime(tm);
	fileInfoPtr->DIR_WrtDate = getDOSDate(tm);
	int m = 0;
	fileInfoPtr->DIR_FileSize = FileSize;
	while (inputFileName[m] != '\0')
	{
		for (int n = 0; n < 8; n++)
		{//对前8位做出操作
			if (inputFileName[m] != '.')
			{//如果没到"."的位置那么就赋值
				fileInfoPtr->DIR_Name[m] = inputFileName[m];
				m++;
			}//不足8位补0
			else
			{
				fileInfoPtr->DIR_Name[n] = 0x20;
			}
		}
		m++;
		break;
	}
	//将剩下的3位一次复制过去
	memcpy(&fileInfoPtr->DIR_Name[8], &inputFileName[m], 3);
	//所需要簇的个数
	int numOfClus;
	//对所需簇的个数计算
	numOfClus = FileSize / BytsPerSec;
	if ((FileSize % BytsPerSec) != 0 || (FileSize == 0)) {
		numOfClus++;
	}
	//把所需的簇的个数传给FAT表
	fileInfoPtr->DIR_FstClus = setFat(numOfClus);
}



//7.向空簇写入目录条信息
BOOL writeEmptyClus(u16 fstClus, RootEntry* fileInfo) {

	RootEntry root;
	RootEntry* rootPtr = &root;

	int daPos;
	u16 lastClus;
	BOOL ready = FALSE;
	while (1)
	{
		int *numbAndData;
		//计算偏移量等
		numbAndData = loadNumbAndData(fstClus);
		daPos = numbAndData[0];
		int num = numbAndData[1];
		lastClus = fstClus; // 保存非0xfff的上一个簇号
		//循环检查是否有空簇32位一组
		for (int m = 0; m < num; m++)
		{
			SetHeaderOffset(daPos, NULL, FILE_BEGIN);
			if (ReadFromDisk(rootPtr, 32, NULL) != 0)
			{
				// 如果是已删除的文件，或者空文件，说明该目录项可用
				if (rootPtr->DIR_Name[0] == 0x00 || rootPtr->DIR_Name[0] == 0xE5)
				{
					SetHeaderOffset(daPos, NULL, FILE_BEGIN);
					//将文件内容写入Disk
					if (WriteToDisk(fileInfo, 32, NULL) != 0)
					{
						ready = TRUE;
						break;
					}
				}
			}
			daPos += 32;
		}
		if (ready) break;
		if ((fstClus = getNextFat(fstClus)) == 0xFFF || fstClus == 0) break;
	}
	if (ready == FALSE && fstClus != 0) { // 目录空间不足且不是根目录
		u16 bytes;
		u16* bytes_ptr;
		int fatBase = RsvdSecCnt * BytsPerSec;
		u16 tempClus = setFat(1);
		daPos = SetHeaderOffset((fatBase + lastClus * 3 / 2), NULL, FILE_BEGIN); // 尾簇号偏移
		SetHeaderOffset(daPos, NULL, FILE_BEGIN);
		//处理奇数和偶数的不同，奇数和偶数索取的位不一样
		if (ReadFromDisk(bytes_ptr, 2, NULL) != 0)
		{
			if (lastClus % 2 == 0)
			{
				bytes = bytes >> 12;
				bytes = bytes << 12; // 保留高四位，低12位为0
				bytes = bytes | tempClus;
			}
			else
			{
				bytes = bytes << 12;
				bytes = bytes >> 12; // 保留低四位，高12位为0
				bytes = bytes | (tempClus << 4);
			}
			//尾簇号偏移
			SetHeaderOffset((fatBase + lastClus * 3 / 2), NULL, FILE_BEGIN);
			if (WriteToDisk(bytes_ptr, 2, NULL) != 0)
			{
				//找到对应的数据区，做写入
				daPos = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (tempClus - 2) * BytsPerSec;
				SetHeaderOffset(daPos, NULL, FILE_BEGIN);
				WriteToDisk(setzero, BytsPerSec, NULL); // 清0
				SetHeaderOffset(daPos, NULL, FILE_BEGIN);
				if (WriteToDisk(fileInfo, 32, NULL) != 0)
					ready = TRUE;
			}
			else
				return -1;
		}
	}
	return ready;
}



//8.设置fat的函数
u16  setFat(int clusNum)
{

	//传入的参数是需要分配的数据块个数

	//计算Boot区域大小
	int bootSize = RsvdSecCnt * BytsPerSec;

	//fat位置的具体偏移量是boot区域,前三个字节代表2个簇，簇号从2号开始，0，1不用
	int fatPos = bootSize + 3;

	//表示簇号从2号开始,fstClus代表返回的首簇号，preClus存储之前的首簇号
	u16 nowClus = 2;
	u16 fstClus;
	u16 preClus;

	u16 twoByte;
	u16*  twoByte_ptr = &twoByte;

	//进行计数，如果找到两个空簇，就把后一个的簇号写入前一个
	int n = 0;

	//这里的while应该是有条件的,比如簇号的范围之类的，这里简化了一下
	while (true)
	{
		//先移动到指定位置
		SetHeaderOffset(fatPos, NULL, FILE_BEGIN);
		if (ReadFromDisk(twoByte_ptr, 2, NULL)) {
			// 簇号分奇数偶数不同处理 
			if (nowClus % 2 == 0) {
				twoByte = twoByte << 4;
				twoByte = twoByte >> 4;
			}
			else {
				twoByte = twoByte >> 4;
			}
			//得到簇是否为空的标识符，进行判断
			if (twoByte == 0x000)
			{

				if (n == 0)
				{
					//将首簇记录下来
					fstClus = nowClus;
				}
				else if (n > 0)
				{
					//已经有两个空簇了，将第2个簇的号写入第一个簇中
					SetHeaderOffset((bootSize + preClus * 3 / 2), NULL, FILE_BEGIN);

					if (ReadFromDisk(twoByte_ptr, 2, NULL) != 0)
					{
						//因为读出来的是16位，保证前面4位不被写覆盖，所以要做一些处理
						if (preClus % 2 == 0) {
							// 保留高四位，低12位为0
							twoByte = twoByte | nowClus; // 与当前clus按位或
						}
						else {

							twoByte = twoByte | (nowClus << 4);
						}

						//处理之后，先移动到该簇的位置，再写入下一个簇的信息
						SetHeaderOffset((bootSize + preClus * 3 / 2), NULL, FILE_BEGIN);
						WriteToDisk(twoByte_ptr, 2, NULL);
					}
				}
				n++;
				//找到一个簇后，将之前的也记录下来
				preClus = nowClus;

				//如果簇的个数已经够了，那么跳出循环
				if (n == clusNum)
					break;
			}
			nowClus++;
		}

		//判断此时簇号是奇数还是偶数，然后设置fat的偏移字节
		(nowClus % 2 == 0) ? (fatPos++) : (fatPos = fatPos + 2);
	}


	//在最后一个簇中写入0xfff代表结束

	//先移动到相应位置
	SetHeaderOffset((bootSize + preClus * 3 / 2), NULL, FILE_BEGIN);
	//分奇数与偶数构造，然后写入
	if (ReadFromDisk(twoByte_ptr, 2, NULL))
	{
		if (preClus % 2 == 0) {
			twoByte = twoByte >> 12;
			twoByte = twoByte << 12; // 保留高四位，低12位为0
			twoByte = twoByte | 0x0FFF;
		}
		else {
			twoByte = twoByte << 12;
			twoByte = twoByte >> 12; // 保留低四位，高12位为0
			twoByte = twoByte | 0xFFF0;
		}
		SetHeaderOffset((bootSize + preClus * 3 / 2), NULL, FILE_BEGIN);
		WriteToDisk(twoByte_ptr, 2, NULL);
	}
	//返回首簇
	return fstClus;
}


//9.创建句柄
DWORD createHandle(RootEntry* FileInfo, u16 parentClus) {
	int i;
	//创建句柄，申请空间
	FileHandle* hd = (FileHandle*)malloc(sizeof(FileHandle)); 
	for (i = 1; i < MAX_NUM; i++)
	{
		if (dwHandles[i] == NULL) {
			memcpy(&hd->fileInfo, FileInfo, 32);
			hd->offset = 0; // 偏移量初始化为0
			hd->parentClus = parentClus;
			dwHandles[i] = hd;
			break;
		}
	}
	//返回句柄的号
	return i;
}

//10.返回循环次数和偏移位置，以数组的形式
int* loadNumbAndData(u16 fstClus)
{
	int numb[2];
	if (fstClus == 0)
	{
		// 算出根目录区的起始地址
		//RsvdSecCnt：boot记录占多大，一般为1，NumFATs：表示fat表数目，FATSz代表fat占用扇区数目
		numb[0] = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec;
		//根目录最大文件数
		numb[1] = RootEntCnt;
	}
	else
	{
		// 算出数据区文件首址
		//数据区偏移计算等于根目录区偏移+根目录区大小244*32+簇号-2乘以簇的大小，512字节
		numb[0] = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (fstClus - 2) * BytsPerSec;
		//每扇区字节数
		numb[1] = BytsPerSec / 32;
	}
	return numb;
}


//11.我的回收簇的函数，循环，将标志的两位置为空,注意偏移的量
BOOL collectClus(u16 fstClus)
{
	//两个字节读取簇
	u16 twoByte;
	u16* twoByte_ptr = &twoByte;


	while (fstClus != 0xFFF)
	{
		int clusPos = RsvdSecCnt * BytsPerSec + fstClus * 3 / 2;

		//暂存位，存储下一个簇号
		u16 temp = getNextFat(fstClus);

		//每次进行读写操作前，必须进行偏移设置
		SetHeaderOffset(clusPos, NULL, FILE_BEGIN);
		if (ReadFromDisk(twoByte_ptr, 2, NULL))
		{
			if (fstClus % 2 == 0)
			{
				//分奇数与偶数不同操作
				twoByte = twoByte >> 12;
				twoByte = twoByte << 12;

			}

			else
			{
				twoByte = twoByte << 12;
				twoByte = twoByte >> 12;
			}

			//写入操作
			SetHeaderOffset(clusPos, NULL, FILE_BEGIN);
			WriteToDisk(twoByte_ptr, 2, NULL);

		}
		//当前簇号向下移动，将已经存储的temp赋值回去
		fstClus = temp;

	}
	return TRUE;

}



//12.递归删除目录 现在是我的
void recursiveDeleteDirectory(u16 fstClus)
{
	//删除的目录项置为0xe5
	u8 deleteSign = 0xe5;

	//计算fat的偏移量
	int fatPos = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (fstClus - 2) * BytsPerSec;

	//设立一个文件项，并引用
	RootEntry delF;
	RootEntry* delF_ptr = &delF;
	while (true)
	{
		//目录项最大数
		int n = BytsPerSec / 32;
		for (int i = 0; i < n; i++)
		{
			//先进行位置的偏移
			SetHeaderOffset(fatPos, NULL, FILE_BEGIN);
			//然后进行读取
			if (ReadFromDisk(delF_ptr, 32, NULL))
			{
				//如果文件名不为这些其他的值，空
				if (delF_ptr->DIR_Name[0] != 0xe5 && delF_ptr->DIR_Name[0] != 0 && delF_ptr->DIR_Name[0] != 0x2E)
				{
					if (delF_ptr->DIR_Attr == 0x10)
					{
						//先把目录名写成0xe5
						SetHeaderOffset(fatPos, NULL, FILE_BEGIN);
						WriteToDisk(&delF_ptr, 1, NULL);

						//如果记录的是目录，则递归调用删除函数，最后回收簇
						recursiveDeleteDirectory(delF_ptr->DIR_FstClus);
						collectClus(delF_ptr->DIR_FstClus);
					}

					else
					if (delF_ptr->DIR_Attr == 0x20)
					{
						//如果是文件，直接写如0xe5，然后回收簇就可以了
						SetHeaderOffset(fatPos, NULL, FILE_BEGIN);
						WriteToDisk(&delF_ptr, 1, NULL);
						collectClus(delF_ptr->DIR_FstClus);
					}
				}


			}

			//再偏移32位
			fatPos = fatPos + 32;
		}
		//一个目录项完毕，指向下一个簇，如果簇号为fff，则说明已经到底
		fstClus = getNextFat(fstClus);
		if (fstClus == 0xfff)
			break;

	}

}



/*

void recursiveDeleteDirectory(u16 fClus) {
	u8 del = 0xE5;
	// 递归删除文件夹下的文件和目录
	// fClus 保存待删除文件夹首簇
	RootEntry fdd;
	RootEntry* fdd_ptr = &fdd;
	int fBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (fClus - 2) * BytsPerSec; // 找到该文件夹的扇区偏移
	// 遍历待删除目录下的所有目录项删除掉
	do {
		for (int k = 0; k < BytsPerSec / 32; k++) {
			SetHeaderOffset(fBase, NULL, FILE_BEGIN);
			if (ReadFromDisk(fdd_ptr, 32, NULL) != 0) {
				// 文件就直接把第一字节改了就成
				if (fdd_ptr->DIR_Name[0] != 0xE5 && fdd_ptr->DIR_Name[0] != 0 && fdd_ptr->DIR_Name[0] != 0x2E) {
					if (fdd_ptr->DIR_Attr == 0x20) {
						SetHeaderOffset(fBase, NULL, FILE_BEGIN);
						WriteToDisk(&del, 1, NULL);
						collectClus(fdd_ptr->DIR_FstClus); // 回收文件簇
					}
					else if (fdd_ptr->DIR_Attr == 0x10) {
						// 文件夹递归调用删除其底下的目录项
						SetHeaderOffset(fBase, NULL, FILE_BEGIN);
						WriteToDisk(&del, 1, NULL);
						recursiveDeleteDirectory(fdd_ptr->DIR_FstClus); // 递归调用
						collectClus(fdd_ptr->DIR_FstClus); // 回收目录簇
					}
				}
			}
			fBase += 32;
		}
	} while ((fClus = getNextFat(fClus)) != 0xFFF);
}
*/







//13.获取年份日期函数
int getDOSDate(time_t ts) {
	struct tm *res;

	res = localtime(&ts);

	return (res->tm_year - 80) * 512 + (res->tm_mon + 1) * 32 + res->tm_mday;
}

//14.获取时间函数
int getDOSTime(time_t ts) {
	struct tm *res;

	res = localtime(&ts);

	return res->tm_hour * 2048 + res->tm_min * 32 + res->tm_sec / 2;
}

//15.初始化BPB函数

BOOL initBPB() {

	if (StartupDisk(fs)) {
		cout << "start up disk..." << endl;
		//载入BPB,偏移11个字节读取
		SetHeaderOffset(11, NULL, FILE_BEGIN);
		if (ReadFromDisk(bpb_ptr, 25, NULL) != 0) {
			//初始化各个全局变量  
			BytsPerSec = bpb_ptr->BPB_BytsPerSec;
			SecPerClus = bpb_ptr->BPB_SecPerClus;
			RsvdSecCnt = bpb_ptr->BPB_RsvdSecCnt;
			NumFATs = bpb_ptr->BPB_NumFATs;
			RootEntCnt = bpb_ptr->BPB_RootEntCnt;
			if (bpb_ptr->BPB_TotSec16 != 0) {
				TotSec = bpb_ptr->BPB_TotSec16;
			}
			else {
				TotSec = bpb_ptr->BPB_TotSec32;
			}
			FATSz = bpb_ptr->BPB_FATSz16;
			/*
			cout << "每扇区字节数：" << BytsPerSec << endl; // 512
			cout << "每簇扇区数：" << SecPerClus << endl; // 1
			cout << "Boot记录占用的扇区数：" << RsvdSecCnt << endl; // 1
			cout << "FAT表个数：" << NumFATs << endl; // 2
			cout << "根目录最大文件数：" << RootEntCnt << endl; // 224
			cout << "扇区总数：" << TotSec << endl; // 2880
			cout << "每FAT扇区数：" << FATSz << endl; // 9
			*/
			return TRUE;
		}
		else {
			cout << "read BPB fail..." << endl;
		}
	}
	else {
		cout << "cannot start up disk..." << endl;
	}
	return FALSE;
}
