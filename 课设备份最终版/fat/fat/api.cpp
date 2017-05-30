#include "fat.h"
#include "util.h"
#include "time.h"

#pragma comment(lib, "DiskLib.lib")
using namespace std;

const char* fs = "d:\\floppy.img";

FileHandle* dwHandles[MAX_NUM] = { NULL };

u8* setzero = (u8*)calloc(512, sizeof(u8)); // ���ڴ���Ŀ¼ʱ��0

BPB bpb;
BPB* bpb_ptr = &bpb;

RootEntry rootEntry;
RootEntry* rootEntry_ptr = &rootEntry;


//1.�����ļ�api �Ѳ��Գɹ�    �������Լ���
DWORD MyCreateFile(char *pszFolderPath, char *pszFileName)
{
	DWORD FileHandle = 0;
	u16 fstClus;
	u32 FileSize = 0; // ��ʼֵΪ0
	RootEntry fileInfo;
	RootEntry* fileInfoPtr = &fileInfo;
	//���ֽ���λ
	memset(fileInfoPtr, 0, sizeof(RootEntry));
	if (initBPB())
	{
		// ·�����ڻ���Ϊ��Ŀ¼
		if ((fstClus = checkPath(pszFolderPath)) || strlen(pszFolderPath) == 3)
		{
			if (!checkFile(pszFileName, fstClus))
			{//�ļ�������
				initFile(fileInfoPtr, pszFileName, 0x20, FileSize);
				if (writeEmptyClus(fstClus, fileInfoPtr) == TRUE)
					// �������
					FileHandle = createHandle(fileInfoPtr, fstClus);
			}
			else//�ļ��Ѵ���
				printf("%s\\%s has existed.", pszFolderPath, pszFileName);
		}
	}
	ShutdownDisk();
	return FileHandle;
}





//2.���ļ�api 
DWORD MyOpenFile(char *pszFolderPath, char *pszFileName)
{
	DWORD FileHandle = 0;
	u16 fstClus = 0;
	BOOL checkExist = FALSE;
	char myFileName[12];
	RootEntry fileInfo;
	RootEntry* fileInfoPtr = &fileInfo;
	if (initBPB())
	{    //Ŀ¼·���ж��Ƿ����
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
					//daPos������������
					int num = numbAndData[1];
					//num����ѭ������;
					for (int n = 0; n < num; n++)
					{
						SetHeaderOffset(daPos, NULL, FILE_BEGIN);
						if (ReadFromDisk(fileInfoPtr, 32, NULL) != 0)
						{
							// Ŀ¼0x10���ļ�0x20�����0x28
							if (fileInfoPtr->DIR_Name[0] != 0xE5 && fileInfoPtr->DIR_Name[0] != 0 && fileInfoPtr->DIR_Name[0] != 0x2E)
							{//�ļ����ĳ��ȼ�����
								int lenOfFileName = 0;
								int m = 0;
								//ǰ8λ�����ļ��Ļ�����
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
								//��3λ��׺�Ķ���
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
								// ���Դ�Сд�Ƚ�
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
					//��������Ϊ�Դص��ж�
					if ((fstClus = getNextFat(fstClus)) == 0xFFF || fstClus == 0) break;
				}
			}
		}
	}
	ShutdownDisk();
	return FileHandle;
}


//3.�ر��ļ�api
void MyCloseFile(DWORD dwHandle) {
	free(dwHandles[dwHandle]);
	dwHandles[dwHandle] = NULL;
}


//4.�ļ�д��api   �����ǿ��յģ��ɹ���
DWORD MyWriteFile(DWORD dwHandle, LPVOID pBuffer, DWORD dwBytesToWrite) {
	DWORD result = 0;
	FileHandle* hd = dwHandles[dwHandle];
	if (hd == NULL || initBPB() == FALSE) return -1;
	u16 FstClus = hd->fileInfo.DIR_FstClus;
	LONG offset = hd->offset; // �ļ�ָ�뵱ǰƫ��
	int curClusNum = offset / BytsPerSec; // ��ǰָ���ڵڼ�������
	int curClusOffset = offset % BytsPerSec; // ��ǰ��������ƫ��
	while (curClusNum) {
		if (getNextFat(FstClus) == 0xFFF) {
			break;
		}
		FstClus = getNextFat(FstClus);
		curClusNum--;
	}// ��ȡ��ǰָ����ָ����
	int dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (FstClus - 2) * BytsPerSec;
	int dataOffset = dataBase + curClusOffset; // �õ��ļ�ָ����ָλ��
	int lenOfBuffer = dwBytesToWrite; // ��������д�볤��
	char* cBuffer = (char*)malloc(sizeof(u8)*lenOfBuffer);
	memcpy(cBuffer, pBuffer, lenOfBuffer); // ���ƹ���
	SetHeaderOffset(dataOffset, NULL, FILE_BEGIN);
	if ((BytsPerSec - curClusOffset >= lenOfBuffer) && curClusNum == 0) {
		if (WriteToDisk(pBuffer, lenOfBuffer, &result) == 0) {
			return -1;
		}
	}
	else {
		DWORD temp;
		u16 tempClus;
		u16 bytes; // ÿ�ζ�ȡ�Ĵغ�
		u16* bytes_ptr = &bytes;
		int fatBase = RsvdSecCnt * BytsPerSec;
		int leftLen = lenOfBuffer;
		int hasWritten = 0;
		if (curClusNum == 0) {
			if (WriteToDisk(pBuffer, BytsPerSec - curClusOffset, &temp) == 0) {
				return -1;
			}
			result += temp; // ��¼д�볤��
			leftLen = lenOfBuffer - (BytsPerSec - curClusOffset); // ʣ�೤��
			hasWritten = BytsPerSec - curClusOffset;
		}
		do {
			tempClus = getNextFat(FstClus); // ��������һ��FAT
			if (tempClus == 0xFFF) {
				tempClus = setFat(1);
				if (tempClus == 0) return -1; //�����ʧ��
				SetHeaderOffset((fatBase + FstClus * 3 / 2), NULL, FILE_BEGIN);
				if (ReadFromDisk(bytes_ptr, 2, NULL) != 0) {
					if (FstClus % 2 == 0) {
						bytes = bytes >> 12;
						bytes = bytes << 12; // ��������λ����12λΪ0
						bytes = bytes | tempClus;
					}
					else {
						bytes = bytes << 12;
						bytes = bytes >> 12; // ��������λ����12λΪ0
						bytes = bytes | (tempClus << 4);
					}
					SetHeaderOffset((fatBase + FstClus * 3 / 2), NULL, FILE_BEGIN);
					if (WriteToDisk(bytes_ptr, 2, NULL) == 0) {
						return -1;
					}
				}
			}
			FstClus = tempClus; // �����õ���һ��FAT
			dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (FstClus - 2) * BytsPerSec; // ˢ������ƫ��
			SetHeaderOffset(dataBase, NULL, FILE_BEGIN); // һ���Ǵ�����ͷ��ʼд
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
	// ˢ���ļ���С
	if ((offset + result) > hd->fileInfo.DIR_FileSize) {
		int dBase;
		BOOL isExist = FALSE;
		hd->fileInfo.DIR_FileSize += (offset + result) - hd->fileInfo.DIR_FileSize;
		// ������ǰĿ¼������Ŀ
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
	MySetFilePointer(dwHandle, result, MY_FILE_CURRENT); //ƫ����ˢ��
	return result;
}

/*

DWORD MyWriteFile(DWORD dwHandle, LPVOID pBuffer, DWORD dwBytesToWrite)
{

	RootEntry root;
	RootEntry* rootPtr = &root;

	DWORD sign = 0;
	//��ȡ���
	FileHandle* hd = dwHandles[dwHandle];
	//������
	if (hd == NULL || initBPB() == FALSE) return -1;
	//��ȡ�״�
	u16 fstClus = hd->fileInfo.DIR_FstClus;
	LONG offset = hd->offset; // �ļ�ָ�뵱ǰƫ��
	int numOfClus = offset / BytsPerSec; // ��ǰָ���ڵڼ�������
	int offsetOfClus = offset % BytsPerSec; // ��ǰ��������ƫ��
	while (numOfClus) {
		if (getNextFat(fstClus) == 0xFFF)break;
		fstClus = getNextFat(fstClus);
		numOfClus--;
	}// ��ȡ��ǰָ����ָ����
	int daPos = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (fstClus - 2) * BytsPerSec;
	int daOffset = daPos + offsetOfClus; // �õ��ļ�ָ����ָλ��
	int lenOfBuffer = dwBytesToWrite; // ��������д�볤��
	char* cBuffer = (char*)malloc(sizeof(u8)*lenOfBuffer);
	memcpy(cBuffer, pBuffer, lenOfBuffer); // ���ƹ���
	SetHeaderOffset(daOffset, NULL, FILE_BEGIN);
	//������
	if ((BytsPerSec - offsetOfClus >= lenOfBuffer) && numOfClus != 0)
	{
		DWORD temp;
		u16 tempClus;
		u16 bytes; // ÿ�ζ�ȡ�Ĵغ�
		u16* bytes_ptr = &bytes;
		int fatPos = RsvdSecCnt * BytsPerSec;
		int leftLen = lenOfBuffer;
		int haveWritten = 0;
		if (numOfClus == 0)
		{
			if (WriteToDisk(pBuffer, BytsPerSec - offsetOfClus, &temp) != 0)
			{
				sign += temp; // ��¼д�볤��
				leftLen = lenOfBuffer - (BytsPerSec - offsetOfClus); // ʣ�೤��
				haveWritten = BytsPerSec - offsetOfClus;//�Ѿ�д��ĳ���
			}
			else
				return -1;
		}
		while (1)
		{
			tempClus = getNextFat(fstClus); // ��������һ��FAT
			if (tempClus == 0xFFF)
			{
				tempClus = setFat(1);
				SetHeaderOffset((fatPos + fstClus * 3 / 2), NULL, FILE_BEGIN);
				if (ReadFromDisk(bytes_ptr, 2, NULL) != 0)
				{
					if (fstClus % 2 == 0)
					{
						bytes = bytes >> 12;
						bytes = bytes << 12; // ��������λ����12λΪ0
						bytes = bytes | tempClus;
					}
					else
					{
						bytes = bytes << 12;
						bytes = bytes >> 12; // ��������λ����12λΪ0
						bytes = bytes | (tempClus << 4);
					}
					SetHeaderOffset((fatPos + fstClus * 3 / 2), NULL, FILE_BEGIN);
					if (WriteToDisk(bytes_ptr, 2, NULL))
						return -1;
				}
			}
			fstClus = tempClus; // �����õ���һ��FAT
			daPos = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (fstClus - 2) * BytsPerSec; // ˢ������ƫ��
			SetHeaderOffset(daPos, NULL, FILE_BEGIN); // һ���Ǵ�����ͷ��ʼд
			int little;
			little = (leftLen > BytsPerSec) ? BytsPerSec : leftLen;
			if (WriteToDisk(&cBuffer[haveWritten], little, &temp) != 0)
			{
				//�Ѿ�д��������������
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
	// ˢ���ļ���С
	if ((offset + sign) > hd->fileInfo.DIR_FileSize)
	{
		int *numbAndData;
		BOOL checkExist = FALSE;
		hd->fileInfo.DIR_FileSize += (offset + sign) - hd->fileInfo.DIR_FileSize;
		// ������ǰĿ¼������Ŀ
		u16 parentClus = hd->parentClus;
		while (1)
		{
			numbAndData = loadNumbAndData(parentClus);
			int data = numbAndData[0];
			//daba������������
			int numb = numbAndData[1];
			//numb����ѭ������
			for (int i = 0; i < numb; i++)
			{
				SetHeaderOffset(data, NULL, FILE_BEGIN);
				//�Ӵ��̻�ȡ���е��ļ���Ϣ
				if (ReadFromDisk(rootPtr, 32, NULL) != 0)
				{
					if (rootPtr->DIR_Attr == 0x20)
					{
						if (_stricmp(rootPtr->DIR_Name, hd->fileInfo.DIR_Name) == 0)
						{
							//д�����
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
	MySetFilePointer(dwHandle, sign, MY_FILE_CURRENT); //ƫ����ˢ��
	return sign;
}
*/




//5.�ļ���ȡapi
DWORD MyReadFile(DWORD dwHandle, LPVOID pBuffer, DWORD dwBytesToRead)
{
	DWORD sign = 0;
	//��ȡ���
	FileHandle* hd = dwHandles[dwHandle];
	//������
	if (hd == NULL || initBPB() == FALSE) return -1;
	//��ȡ�״�
	u16 fstClus = hd->fileInfo.DIR_FstClus;
	LONG offset = hd->offset; // �ļ�ָ�뵱ǰƫ��
	int numOfClus = offset / BytsPerSec; // ��ǰָ���ڵڼ�������
	int offsetOfClus = offset % BytsPerSec; // ��ǰ��������ƫ��
	while (numOfClus)
	{
		if (getNextFat(fstClus) == 0xFFF) break;
		fstClus = getNextFat(fstClus);
		numOfClus--;
	}// ��ȡ��ǰָ����ָ����
	if (numOfClus > 0 || offset > hd->fileInfo.DIR_FileSize)
		return -1; // �����ļ�ƫ�Ʒ�Χ��
	int daPos = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (fstClus - 2) * BytsPerSec;
	int daOffset = daPos + offsetOfClus; // �õ��ļ�ָ����ָλ��
	int lenOfBuffer = dwBytesToRead; // �����������볤��
	if ((int)hd->fileInfo.DIR_FileSize - offset < lenOfBuffer)
		lenOfBuffer = hd->fileInfo.DIR_FileSize - offset;
	char* cBuffer = (char*)malloc(sizeof(u8)*lenOfBuffer); // ����һ��������
	memset(cBuffer, 0, lenOfBuffer);
	SetHeaderOffset(daOffset, NULL, FILE_BEGIN);
	// ��ȡ
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
			sign += temp; // ��¼��ȡ���ĳ���
			int leftLen = lenOfBuffer - (BytsPerSec - offsetOfClus); // ʣ�೤��
			int hasRead = BytsPerSec - offsetOfClus;
			while (1)
			{
				fstClus = getNextFat(fstClus); // �õ���һ��FAT
				if (fstClus == 0xFFF)
					break;
				daPos = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (fstClus - 2) * BytsPerSec; // ˢ������ƫ��
				SetHeaderOffset(daPos, NULL, FILE_BEGIN);
				int little;
				//ȡ���н�С���Ǹ�
				little = (leftLen > BytsPerSec) ? BytsPerSec : leftLen;
				if (ReadFromDisk(&cBuffer[hasRead], little, &temp) != 0)
				{
					//�Ѿ�����������������
					hasRead += little;
					leftLen -= BytsPerSec; // ֱ�Ӽ���һ��������ֻҪ��<=0���˳�ѭ��
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
	memcpy(pBuffer, cBuffer, lenOfBuffer); // д�뻺����
	ShutdownDisk();
	MySetFilePointer(dwHandle, sign, MY_FILE_CURRENT); //ƫ����ˢ��
	return sign;
}


//6.ɾ���ļ�api   �Ѳ��Գɹ�
BOOL MyDeleteFile(char *pszFolderPath, char *pszFileName)
{
	//��Ӳ�̣���ʼ��BPB�Ȳ���
	if (!initBPB())
		return false;

	RootEntry FileInfo;
	RootEntry* FileInfo_ptr = &FileInfo;

	//���صı�ʶ��
	BOOL sign = false;

	//�����ļ�����13λ�ַ����飬8λ����+.+3λ��׺+'\0'
	char fileName[13];

	//�����״غ�
	u16 fstClus;
	//���ж�·���Ƿ����
	if ((fstClus = checkPath(pszFolderPath)) || strlen(pszFolderPath) == 3)
	{
		//���ж��ļ��Ƿ����
		if (checkFile(pszFileName, fstClus))
		{
			int *numbAndData;
			int dataBase;
			int loop;//������������������Ի�һ������



			while (true)
			{
				//�õ�ƫ�Ƶ�ַ��ѭ������
				numbAndData = loadNumbAndData(fstClus);
				dataBase = numbAndData[0];
				loop = numbAndData[1];

				int a;

				for (int i = 0; i < loop; i++)
				{
					SetHeaderOffset(dataBase, NULL, FILE_BEGIN);

					if (ReadFromDisk(FileInfo_ptr, 32, NULL))
					{
						//�ж��Ƿ�������ue
						if (FileInfo_ptr->DIR_Name[0] != 0xE5 && FileInfo_ptr->DIR_Name[0] != 0 && FileInfo_ptr->DIR_Name[0] != 0x2E)
						{
							int n = 0;
							//���ļ��Ļ�
							if (FileInfo_ptr->DIR_Attr == 0x20)
							{
								//��ǰ8λ��ȡ����
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
								//����λ��׺��ֱ�Ӹ�ֵ
								a = 8;
								while (a< 11)
								{
									fileName[n] = FileInfo_ptr->DIR_Name[a];
									a++;
									n++;

								}
								//���һλ��Ϊ\0
								fileName[n] = '\0';

								//�����ַ����ȽϺ����������������0����ȶԳɹ�
								if (_stricmp(fileName, pszFileName) == 0)
								{
									u8 del = 0xE5;
									SetHeaderOffset(dataBase, NULL, FILE_BEGIN);
									//��ɾ����Ϣд�������

									if (WriteToDisk(&del, 1, NULL))
									{
										//д������̺󣬻�Ҫ���л��մصĲ���
										collectClus(FileInfo_ptr->DIR_FstClus);
										sign = true;
									}

								}

							}
						}

					}

					//ƫ�����32�ֽ�
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



//7.����Ŀ¼api
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
		// ·�����ڻ���Ϊ��Ŀ¼
		if ((fstClus = checkPath(pszFolderPath)) || strlen(pszFolderPath) == 3)
		{ //�����״�
			lastClus = fstClus;
			//�ļ���·���Ƿ��Ѵ���
			if (checkDirectory(pszFolderName, fstClus))
				cout << pszFolderPath << '\\' << pszFolderName << " has existed!" << endl;
			else {
				while (1){
					numbAndData = loadNumbAndData(fstClus);
					daPos = numbAndData[0];
					//daPos������������
					int num = numbAndData[1];
					//num����ѭ������
					for (int i = 0; i < num; i++)
					{
						SetHeaderOffset(daPos, NULL, FILE_BEGIN);
						if (ReadFromDisk(rootPtr, 32, NULL) != 0)
						{
							// Ŀ¼�����
							if (rootPtr->DIR_Name[0] == 0x00 || rootPtr->DIR_Name[0] == 0xE5)
							{// ��ʼ���ļ�����Ϣ
								initFolder(rootPtr, pszFolderName, 0x10);
								SetHeaderOffset(daPos, NULL, FILE_BEGIN); // ��ͷ��λ
								if (WriteToDisk(rootPtr, 32, NULL) != 0)
								{
									// ���� . �� ..
									int dBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (rootPtr->DIR_FstClus - 2) * BytsPerSec;
									SetHeaderOffset(dBase, NULL, FILE_BEGIN);
									//setzero��֮ǰ���壬���ÿյ�����
									WriteToDisk(setzero, BytsPerSec, NULL); // Ŀ¼������ʼ��0
									// . ����
									SetHeaderOffset(dBase, NULL, FILE_BEGIN);
									rootPtr->DIR_FileSize = 0;
									//. �����ˢΪ0
									for (int i = 0; i < 11; i++) {
										rootPtr->DIR_Name[i] = 0x20;
									}
									//���µ�һλΪ0
									rootPtr->DIR_Name[0] = 0x2E;
									//д�����
									WriteToDisk(rootPtr, 32, NULL);
									// ..  ����
									//�ƶ�����һ��32λ��λ��
									SetHeaderOffset(dBase + 32, NULL, FILE_BEGIN);
									//���µڶ�λΪ2E
									rootPtr->DIR_Name[1] = 0x2E;
									rootPtr->DIR_FstClus = lastClus;
									//д�����
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

//8.ƫ�ƺ������ǿ��յģ���ʱ�����滻
BOOL MySetFilePointer(DWORD dwFileHandle, int nOffset, DWORD dwMoveMethod) {
	FileHandle* hd = dwHandles[dwFileHandle];
	if (hd == NULL || initBPB() == FALSE) return FALSE; // ���������
	LONG curOffset = nOffset + hd->offset; // currentģʽ��ƫ�ƺ��λ��
	u16 currentClus = hd->fileInfo.DIR_FstClus; // �״�
	int fileSize = hd->fileInfo.DIR_FileSize; // �ļ���С
	int fileBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (currentClus - 2) * BytsPerSec;
	switch (dwMoveMethod) {
	case MY_FILE_BEGIN:
		if (nOffset < 0) {
			hd->offset = 0; // С��0����Ϊ0
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


//9.ɾ��Ŀ¼����  �Ѿ������ҵ���
BOOL MyDeleteDirectory(char *pszFolderPath, char *pszFolderName)
{
	//�����״غͷ��صı�ʶ��
	u16 fstClus;
	BOOL sign = FALSE;

	if (initBPB())
	{
		fstClus = checkPath(pszFolderPath);

		//�غŴ��ڣ�����ֱ���Ǹ�Ŀ¼C:\\
								if ((fstClus || strlen(pszFolderPath) == 3)&&checkDirectory(pszFolderName, FstClus))
		{
			//���÷�װ�õĺ���������ƫ������ѭ������

			int *numbAndData;
			int dataBase;
			int loop;

			//�ļ����洢�ͻ��ձ�־
			char dirName[12];
			u8 del = 0xE5;

			//��Ŀ¼��
			RootEntry delD;
			RootEntry* delD_ptr = &delD;

			int m;

			while (true)
			{
				//�õ�ƫ�Ƶ�ַ��ѭ������,ƫ�Ƶ�ַ�����״غŵĸı���һֱ�ı�
				numbAndData = loadNumbAndData(fstClus);
				dataBase = numbAndData[0];
				loop = numbAndData[1];

				//����ѭ��������������Ŀ¼�µ�������Ŀ¼���ļ�
				for (int i = 0; i < loop; i++)
				{
					//��ƫ�Ƶ��ƶ�λ��
					SetHeaderOffset(dataBase, NULL, FILE_BEGIN);
					if (ReadFromDisk(delD_ptr, 32, NULL))
					{
						//���жϱ�־λ�Ƿ�������
						if (delD_ptr->DIR_Name[0] != 0xE5 && delD_ptr->DIR_Name[0] != 0 && delD_ptr->DIR_Name[0] != 0x2E)
						{
							if (delD_ptr->DIR_Attr == 0x10)
							{
								//�����ȡ��������Ŀ¼
								for (m = 0; m < 11; m++)
								{
									//�����ƴ洢�������У�����Ϊ�յ�������
									if (delD_ptr->DIR_Name[m] != ' ')
									{
										dirName[m] = delD_ptr->DIR_Name[m];
									}
									else
										break;
								}

								//�����һλ��Ϊ\0
								dirName[m] = '\0';

								//�Ƚϵõ��������봫������֣�һ�µĻ�����������Ϊ0�����õݹ麯��ɾ��
								if (_stricmp(dirName, pszFolderName) == 0)
								{
									//�ݹ�ɾ��Ŀ¼����
									recursiveDeleteDirectory(delD_ptr->DIR_FstClus);

									//֮��0xE5д��
									SetHeaderOffset(dataBase, NULL, FILE_BEGIN);

									if (WriteToDisk(&del, 1, NULL))
									{
										//�����дصĻ��չ���
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
		// ·�����ڻ���Ϊ��Ŀ¼
		if ((FstClus = checkPath(pszFolderPath)) || strlen(pszFolderPath) == 3) {
			// ��ɾ��Ŀ¼����
			if (checkDirectory(pszFolderName, FstClus)) {
				int dataBase;
				int loop;
				char directory[12];
				u8 del = 0xE5;
				RootEntry fd;
				RootEntry* fd_ptr = &fd;
				do {
					if (FstClus == 0) {
						// ��Ŀ¼��ƫ��
						dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec;
						loop = RootEntCnt;
					}
					else {
						// �������ļ���ַƫ��
						dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (FstClus - 2) * BytsPerSec;
						loop = BytsPerSec / 32;
					}
					for (int i = 0; i < loop; i++) {
						SetHeaderOffset(dataBase, NULL, FILE_BEGIN);
						if (ReadFromDisk(fd_ptr, 32, NULL) != 0) {
							if (fd_ptr->DIR_Name[0] != 0xE5 && fd_ptr->DIR_Name[0] != 0 && fd_ptr->DIR_Name[0] != 0x2E) {
								// Ŀ¼0x10���ļ�0x20�����0x28
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
									// ���Դ�Сд�Ƚ�
									if (_stricmp(directory, pszFolderName) == 0) {
										recursiveDeleteDirectory(fd_ptr->DIR_FstClus);
										// ɾ�����ļ���
										SetHeaderOffset(dataBase, NULL, FILE_BEGIN);
										if (WriteToDisk(&del, 1, NULL) != 0) {
											result = collectClus(fd_ptr->DIR_FstClus); // �����״أ�����
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

//1.����ļ��Ƿ����





BOOL checkFile(char *inputFileName, u16 fstClus)
{

	RootEntry root;
	RootEntry* rootPtr = &root;

	char myfilename[13];
	int *numbAndData;
	BOOL checkExist = FALSE;
	// ������ǰĿ¼������Ŀ
	while (1)
	{
		//�ҵ���Ŀ¼���Լ���������ַ
		numbAndData = loadNumbAndData(fstClus);
		int daPos = numbAndData[0];
		//daPos������������
		int num = numbAndData[1];
		//num����ѭ������
		for (int m = 0; m < num; m++)
		{
			SetHeaderOffset(daPos, NULL, FILE_BEGIN);
			if (ReadFromDisk(rootPtr, 32, NULL) != 0)
			{
				// Ŀ¼0x10���ļ�0x20�����0x28
				if (rootPtr->DIR_Name[0] != 0xE5 && rootPtr->DIR_Name[0] != 0 && rootPtr->DIR_Name[0] != 0x2E)
				{
					int lenOfMyFileName = 0;
					if (rootPtr->DIR_Attr == 0x20)
					{
						int n = 0;
						while (n < 8)//ǰ8λ���ļ���
						{
							if (rootPtr->DIR_Name[n] != ' ')
								myfilename[lenOfMyFileName++] = rootPtr->DIR_Name[n];
							n++;
						}
						myfilename[lenOfMyFileName++] = '.';
						n = 8;
						while (n < 11)//��3λ�ĺ�׺
						{
							if (rootPtr->DIR_Name[n] != ' ')
								myfilename[lenOfMyFileName++] = rootPtr->DIR_Name[n];
							n++;
						}
						//c���ַ�������/0��β�԰ɣ����Լ�0
						myfilename[lenOfMyFileName] = '\0';
						// ���Դ�Сд�Ƚϣ��κ�������ʱ����������ַ�����ȣ�����Ϊ0
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



//2.���Ŀ¼�Ƿ����
u16 checkDirectory(char *inputFolderName, u16 fstClus) 
{

	RootEntry root;
	RootEntry* rootPtr = &root;

	char directoryName[12];
	int *numbAndData;
	u16 checkExist = 0;
	// ������ǰĿ¼������Ŀ
	while (1)
	{
		numbAndData = loadNumbAndData(fstClus);
		int daPos = numbAndData[0];
		//daPos������������
		int num = numbAndData[1];
		//num����ѭ������
		for (int m = 0; m < num; m++)
		{

			//�ƶ���ƫ�Ƶ�λ��
			SetHeaderOffset(daPos, NULL, FILE_BEGIN);

			//Ϊʲôֱ�Ӷ�32λ�أ�  ��ΪĿ¼ÿ���Ϊ32λ
			//����0�������ȡʧ�ܣ�Ҳ����û��Ŀ¼
			if (ReadFromDisk(rootPtr, 32, NULL) != 0)
			{
				// Ŀ¼0x10���ļ�0x20�����0x28
				if (rootPtr->DIR_Name[0] != 0xE5 && rootPtr->DIR_Name[0] != 0 && rootPtr->DIR_Name[0] != 0x2E)
				{
					if (rootPtr->DIR_Attr == 0x10)
					{
						int n = 0;
						while (n<11)
						{//����Ӧ���ļ�����ת��Ϊ����������������������Ƚ�
							if (rootPtr->DIR_Name[n] != ' ')
							{ //�ļ�������Ϊ�գ�������¼�뵽�Զ����������
								directoryName[n] = rootPtr->DIR_Name[n];
								//ֱ����10λ����������
								if (n == 10)
								{
									directoryName[11] = '\0';
									break;
								}
							}
							//��һλΪ�վͼӽ���������������ѭ��
							else
							{
								directoryName[n] = '\0';
								break;
							}
							n++;
						}
						// ���Դ�Сд�Ƚ�
						if (_stricmp(directoryName, inputFolderName) == 0)
						{
							checkExist = rootPtr->DIR_FstClus;
							break;
						}
					}
				}
			}
			//û�ж�������ôƫ����+32���ֽ�
			daPos += 32;
		}
		if (checkExist) break;
		if ((fstClus = getNextFat(fstClus)) == 0xFFF || fstClus == 0) break;
	}
	return checkExist;
}






//3.���·���Ƿ����
u16 checkPath(char *pszFolderPath)
{
	//���Ŀ¼���Ƶ��ַ����飬Ϊʲô��12λ��?��ΪҪ�洢���һλ\0
	char dirName[12];

	//Ҫ���ص��״غ�
	u16 fstClus = 0;

	//i=3����Ϊ�̷�ռ3λ��ֱ��ȥ��
	int i = 3;
	//��¼dirName�����λ��
	int j = 0;

	int length = 0;

	//����whileѭ����ʵ�ֶ༶Ŀ¼�ı������飬�����һ��Ŀ¼������������
	while (pszFolderPath[i] != '\0'&& length <= 11)
	{
		if (pszFolderPath[i] != '\\')
		{
			dirName[j] = pszFolderPath[i];
			//���յ���Ч�ַ�����ƫ��һλ
			i++;
			j++;
			length++;

		}
		//��������ָ����\������֮ǰ�����Ŀ¼���������¿�ʼ����
		else if (pszFolderPath[i] == '\\')
		{
			//��ʾĿ¼���Ľ���
			dirName[j] = '\0';
			//����Ƿ�������Ŀ¼��
			fstClus = checkDirectory(dirName, fstClus);
			if (fstClus)
			{

				//������ڣ����ַ�����ԭ�е���Ĩȥ��������Ϊ' '
				while (j >= 0)
				{
					dirName[j] = ' ';
					j--;
				}
				//ͬʱ��j�ָ���0.Ҳ���ǿ�ʼλ��
				j++;
				i++;
				length = 0;
			}
			else
			{
				//���û���ҵ���Ŀ¼����ô�����жϣ�����0
				return fstClus;
			}

		}
	}
	//�����������ٽ����ж�Ȼ�󷵻��״غ�
	if (j > 0)
	{
		dirName[j] = '\0';
		fstClus = checkDirectory(dirName, fstClus);
	}

	return fstClus;

}

//4.��ȡ��һ���غ�
u16  getNextFat(u16 fstClus)
{
	//����Boot�����С
	int bootSize = RsvdSecCnt * BytsPerSec;

	//fatλ�õľ���ƫ������boot����+�غ�*3/2
	int fatPos = bootSize + fstClus * 3 / 2;

	//�������ֽڴ洢fat��Ϣ������fatֻ��12λ������Ҫ����λ�Ʋ���
	u16  twoByte;
	u16*  twoByte_ptr = &twoByte;

	SetHeaderOffset(fatPos, NULL, FILE_BEGIN);

	if (ReadFromDisk(twoByte_ptr, 2, NULL))
	{
		//�غ�Ϊ������ż��ʱ���ǲ�һ����
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


//5.��ʼ��Ŀ¼��Ϣ����
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
			//������д��DIR_Name
			fileInfoPtr->DIR_Name[n] = inputFolderName[n];
		else
			break;

	}
	while (n < 11)
	{   //�������11λ��0��0x20
		fileInfoPtr->DIR_Name[n] = 0x20;
		n++;
	}
	//������Ĵصĸ�������FAT��
	fileInfoPtr->DIR_FstClus = setFat(1);
}



//6.��ʼ���ļ���Ϣ����
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
		{//��ǰ8λ��������
			if (inputFileName[m] != '.')
			{//���û��"."��λ����ô�͸�ֵ
				fileInfoPtr->DIR_Name[m] = inputFileName[m];
				m++;
			}//����8λ��0
			else
			{
				fileInfoPtr->DIR_Name[n] = 0x20;
			}
		}
		m++;
		break;
	}
	//��ʣ�µ�3λһ�θ��ƹ�ȥ
	memcpy(&fileInfoPtr->DIR_Name[8], &inputFileName[m], 3);
	//����Ҫ�صĸ���
	int numOfClus;
	//������صĸ�������
	numOfClus = FileSize / BytsPerSec;
	if ((FileSize % BytsPerSec) != 0 || (FileSize == 0)) {
		numOfClus++;
	}
	//������Ĵصĸ�������FAT��
	fileInfoPtr->DIR_FstClus = setFat(numOfClus);
}



//7.��մ�д��Ŀ¼����Ϣ
BOOL writeEmptyClus(u16 fstClus, RootEntry* fileInfo) {

	RootEntry root;
	RootEntry* rootPtr = &root;

	int daPos;
	u16 lastClus;
	BOOL ready = FALSE;
	while (1)
	{
		int *numbAndData;
		//����ƫ������
		numbAndData = loadNumbAndData(fstClus);
		daPos = numbAndData[0];
		int num = numbAndData[1];
		lastClus = fstClus; // �����0xfff����һ���غ�
		//ѭ������Ƿ��пմ�32λһ��
		for (int m = 0; m < num; m++)
		{
			SetHeaderOffset(daPos, NULL, FILE_BEGIN);
			if (ReadFromDisk(rootPtr, 32, NULL) != 0)
			{
				// �������ɾ�����ļ������߿��ļ���˵����Ŀ¼�����
				if (rootPtr->DIR_Name[0] == 0x00 || rootPtr->DIR_Name[0] == 0xE5)
				{
					SetHeaderOffset(daPos, NULL, FILE_BEGIN);
					//���ļ�����д��Disk
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
	if (ready == FALSE && fstClus != 0) { // Ŀ¼�ռ䲻���Ҳ��Ǹ�Ŀ¼
		u16 bytes;
		u16* bytes_ptr;
		int fatBase = RsvdSecCnt * BytsPerSec;
		u16 tempClus = setFat(1);
		daPos = SetHeaderOffset((fatBase + lastClus * 3 / 2), NULL, FILE_BEGIN); // β�غ�ƫ��
		SetHeaderOffset(daPos, NULL, FILE_BEGIN);
		//����������ż���Ĳ�ͬ��������ż����ȡ��λ��һ��
		if (ReadFromDisk(bytes_ptr, 2, NULL) != 0)
		{
			if (lastClus % 2 == 0)
			{
				bytes = bytes >> 12;
				bytes = bytes << 12; // ��������λ����12λΪ0
				bytes = bytes | tempClus;
			}
			else
			{
				bytes = bytes << 12;
				bytes = bytes >> 12; // ��������λ����12λΪ0
				bytes = bytes | (tempClus << 4);
			}
			//β�غ�ƫ��
			SetHeaderOffset((fatBase + lastClus * 3 / 2), NULL, FILE_BEGIN);
			if (WriteToDisk(bytes_ptr, 2, NULL) != 0)
			{
				//�ҵ���Ӧ������������д��
				daPos = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (tempClus - 2) * BytsPerSec;
				SetHeaderOffset(daPos, NULL, FILE_BEGIN);
				WriteToDisk(setzero, BytsPerSec, NULL); // ��0
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



//8.����fat�ĺ���
u16  setFat(int clusNum)
{

	//����Ĳ�������Ҫ��������ݿ����

	//����Boot�����С
	int bootSize = RsvdSecCnt * BytsPerSec;

	//fatλ�õľ���ƫ������boot����,ǰ�����ֽڴ���2���أ��غŴ�2�ſ�ʼ��0��1����
	int fatPos = bootSize + 3;

	//��ʾ�غŴ�2�ſ�ʼ,fstClus�����ص��״غţ�preClus�洢֮ǰ���״غ�
	u16 nowClus = 2;
	u16 fstClus;
	u16 preClus;

	u16 twoByte;
	u16*  twoByte_ptr = &twoByte;

	//���м���������ҵ������մأ��ͰѺ�һ���Ĵغ�д��ǰһ��
	int n = 0;

	//�����whileӦ������������,����غŵķ�Χ֮��ģ��������һ��
	while (true)
	{
		//���ƶ���ָ��λ��
		SetHeaderOffset(fatPos, NULL, FILE_BEGIN);
		if (ReadFromDisk(twoByte_ptr, 2, NULL)) {
			// �غŷ�����ż����ͬ���� 
			if (nowClus % 2 == 0) {
				twoByte = twoByte << 4;
				twoByte = twoByte >> 4;
			}
			else {
				twoByte = twoByte >> 4;
			}
			//�õ����Ƿ�Ϊ�յı�ʶ���������ж�
			if (twoByte == 0x000)
			{

				if (n == 0)
				{
					//���״ؼ�¼����
					fstClus = nowClus;
				}
				else if (n > 0)
				{
					//�Ѿ��������մ��ˣ�����2���صĺ�д���һ������
					SetHeaderOffset((bootSize + preClus * 3 / 2), NULL, FILE_BEGIN);

					if (ReadFromDisk(twoByte_ptr, 2, NULL) != 0)
					{
						//��Ϊ����������16λ����֤ǰ��4λ����д���ǣ�����Ҫ��һЩ����
						if (preClus % 2 == 0) {
							// ��������λ����12λΪ0
							twoByte = twoByte | nowClus; // �뵱ǰclus��λ��
						}
						else {

							twoByte = twoByte | (nowClus << 4);
						}

						//����֮�����ƶ����ôص�λ�ã���д����һ���ص���Ϣ
						SetHeaderOffset((bootSize + preClus * 3 / 2), NULL, FILE_BEGIN);
						WriteToDisk(twoByte_ptr, 2, NULL);
					}
				}
				n++;
				//�ҵ�һ���غ󣬽�֮ǰ��Ҳ��¼����
				preClus = nowClus;

				//����صĸ����Ѿ����ˣ���ô����ѭ��
				if (n == clusNum)
					break;
			}
			nowClus++;
		}

		//�жϴ�ʱ�غ�����������ż����Ȼ������fat��ƫ���ֽ�
		(nowClus % 2 == 0) ? (fatPos++) : (fatPos = fatPos + 2);
	}


	//�����һ������д��0xfff�������

	//���ƶ�����Ӧλ��
	SetHeaderOffset((bootSize + preClus * 3 / 2), NULL, FILE_BEGIN);
	//��������ż�����죬Ȼ��д��
	if (ReadFromDisk(twoByte_ptr, 2, NULL))
	{
		if (preClus % 2 == 0) {
			twoByte = twoByte >> 12;
			twoByte = twoByte << 12; // ��������λ����12λΪ0
			twoByte = twoByte | 0x0FFF;
		}
		else {
			twoByte = twoByte << 12;
			twoByte = twoByte >> 12; // ��������λ����12λΪ0
			twoByte = twoByte | 0xFFF0;
		}
		SetHeaderOffset((bootSize + preClus * 3 / 2), NULL, FILE_BEGIN);
		WriteToDisk(twoByte_ptr, 2, NULL);
	}
	//�����״�
	return fstClus;
}


//9.�������
DWORD createHandle(RootEntry* FileInfo, u16 parentClus) {
	int i;
	//�������������ռ�
	FileHandle* hd = (FileHandle*)malloc(sizeof(FileHandle)); 
	for (i = 1; i < MAX_NUM; i++)
	{
		if (dwHandles[i] == NULL) {
			memcpy(&hd->fileInfo, FileInfo, 32);
			hd->offset = 0; // ƫ������ʼ��Ϊ0
			hd->parentClus = parentClus;
			dwHandles[i] = hd;
			break;
		}
	}
	//���ؾ���ĺ�
	return i;
}

//10.����ѭ��������ƫ��λ�ã����������ʽ
int* loadNumbAndData(u16 fstClus)
{
	int numb[2];
	if (fstClus == 0)
	{
		// �����Ŀ¼������ʼ��ַ
		//RsvdSecCnt��boot��¼ռ���һ��Ϊ1��NumFATs����ʾfat����Ŀ��FATSz����fatռ��������Ŀ
		numb[0] = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec;
		//��Ŀ¼����ļ���
		numb[1] = RootEntCnt;
	}
	else
	{
		// ����������ļ���ַ
		//������ƫ�Ƽ�����ڸ�Ŀ¼��ƫ��+��Ŀ¼����С244*32+�غ�-2���ԴصĴ�С��512�ֽ�
		numb[0] = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (fstClus - 2) * BytsPerSec;
		//ÿ�����ֽ���
		numb[1] = BytsPerSec / 32;
	}
	return numb;
}


//11.�ҵĻ��մصĺ�����ѭ��������־����λ��Ϊ��,ע��ƫ�Ƶ���
BOOL collectClus(u16 fstClus)
{
	//�����ֽڶ�ȡ��
	u16 twoByte;
	u16* twoByte_ptr = &twoByte;


	while (fstClus != 0xFFF)
	{
		int clusPos = RsvdSecCnt * BytsPerSec + fstClus * 3 / 2;

		//�ݴ�λ���洢��һ���غ�
		u16 temp = getNextFat(fstClus);

		//ÿ�ν��ж�д����ǰ���������ƫ������
		SetHeaderOffset(clusPos, NULL, FILE_BEGIN);
		if (ReadFromDisk(twoByte_ptr, 2, NULL))
		{
			if (fstClus % 2 == 0)
			{
				//��������ż����ͬ����
				twoByte = twoByte >> 12;
				twoByte = twoByte << 12;

			}

			else
			{
				twoByte = twoByte << 12;
				twoByte = twoByte >> 12;
			}

			//д�����
			SetHeaderOffset(clusPos, NULL, FILE_BEGIN);
			WriteToDisk(twoByte_ptr, 2, NULL);

		}
		//��ǰ�غ������ƶ������Ѿ��洢��temp��ֵ��ȥ
		fstClus = temp;

	}
	return TRUE;

}



//12.�ݹ�ɾ��Ŀ¼ �������ҵ�
void recursiveDeleteDirectory(u16 fstClus)
{
	//ɾ����Ŀ¼����Ϊ0xe5
	u8 deleteSign = 0xe5;

	//����fat��ƫ����
	int fatPos = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (fstClus - 2) * BytsPerSec;

	//����һ���ļ��������
	RootEntry delF;
	RootEntry* delF_ptr = &delF;
	while (true)
	{
		//Ŀ¼�������
		int n = BytsPerSec / 32;
		for (int i = 0; i < n; i++)
		{
			//�Ƚ���λ�õ�ƫ��
			SetHeaderOffset(fatPos, NULL, FILE_BEGIN);
			//Ȼ����ж�ȡ
			if (ReadFromDisk(delF_ptr, 32, NULL))
			{
				//����ļ�����Ϊ��Щ������ֵ����
				if (delF_ptr->DIR_Name[0] != 0xe5 && delF_ptr->DIR_Name[0] != 0 && delF_ptr->DIR_Name[0] != 0x2E)
				{
					if (delF_ptr->DIR_Attr == 0x10)
					{
						//�Ȱ�Ŀ¼��д��0xe5
						SetHeaderOffset(fatPos, NULL, FILE_BEGIN);
						WriteToDisk(&delF_ptr, 1, NULL);

						//�����¼����Ŀ¼����ݹ����ɾ�������������մ�
						recursiveDeleteDirectory(delF_ptr->DIR_FstClus);
						collectClus(delF_ptr->DIR_FstClus);
					}

					else
					if (delF_ptr->DIR_Attr == 0x20)
					{
						//������ļ���ֱ��д��0xe5��Ȼ����մؾͿ�����
						SetHeaderOffset(fatPos, NULL, FILE_BEGIN);
						WriteToDisk(&delF_ptr, 1, NULL);
						collectClus(delF_ptr->DIR_FstClus);
					}
				}


			}

			//��ƫ��32λ
			fatPos = fatPos + 32;
		}
		//һ��Ŀ¼����ϣ�ָ����һ���أ�����غ�Ϊfff����˵���Ѿ�����
		fstClus = getNextFat(fstClus);
		if (fstClus == 0xfff)
			break;

	}

}



/*

void recursiveDeleteDirectory(u16 fClus) {
	u8 del = 0xE5;
	// �ݹ�ɾ���ļ����µ��ļ���Ŀ¼
	// fClus �����ɾ���ļ����״�
	RootEntry fdd;
	RootEntry* fdd_ptr = &fdd;
	int fBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (fClus - 2) * BytsPerSec; // �ҵ����ļ��е�����ƫ��
	// ������ɾ��Ŀ¼�µ�����Ŀ¼��ɾ����
	do {
		for (int k = 0; k < BytsPerSec / 32; k++) {
			SetHeaderOffset(fBase, NULL, FILE_BEGIN);
			if (ReadFromDisk(fdd_ptr, 32, NULL) != 0) {
				// �ļ���ֱ�Ӱѵ�һ�ֽڸ��˾ͳ�
				if (fdd_ptr->DIR_Name[0] != 0xE5 && fdd_ptr->DIR_Name[0] != 0 && fdd_ptr->DIR_Name[0] != 0x2E) {
					if (fdd_ptr->DIR_Attr == 0x20) {
						SetHeaderOffset(fBase, NULL, FILE_BEGIN);
						WriteToDisk(&del, 1, NULL);
						collectClus(fdd_ptr->DIR_FstClus); // �����ļ���
					}
					else if (fdd_ptr->DIR_Attr == 0x10) {
						// �ļ��еݹ����ɾ������µ�Ŀ¼��
						SetHeaderOffset(fBase, NULL, FILE_BEGIN);
						WriteToDisk(&del, 1, NULL);
						recursiveDeleteDirectory(fdd_ptr->DIR_FstClus); // �ݹ����
						collectClus(fdd_ptr->DIR_FstClus); // ����Ŀ¼��
					}
				}
			}
			fBase += 32;
		}
	} while ((fClus = getNextFat(fClus)) != 0xFFF);
}
*/







//13.��ȡ������ں���
int getDOSDate(time_t ts) {
	struct tm *res;

	res = localtime(&ts);

	return (res->tm_year - 80) * 512 + (res->tm_mon + 1) * 32 + res->tm_mday;
}

//14.��ȡʱ�亯��
int getDOSTime(time_t ts) {
	struct tm *res;

	res = localtime(&ts);

	return res->tm_hour * 2048 + res->tm_min * 32 + res->tm_sec / 2;
}

//15.��ʼ��BPB����

BOOL initBPB() {

	if (StartupDisk(fs)) {
		cout << "start up disk..." << endl;
		//����BPB,ƫ��11���ֽڶ�ȡ
		SetHeaderOffset(11, NULL, FILE_BEGIN);
		if (ReadFromDisk(bpb_ptr, 25, NULL) != 0) {
			//��ʼ������ȫ�ֱ���  
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
			cout << "ÿ�����ֽ�����" << BytsPerSec << endl; // 512
			cout << "ÿ����������" << SecPerClus << endl; // 1
			cout << "Boot��¼ռ�õ���������" << RsvdSecCnt << endl; // 1
			cout << "FAT�������" << NumFATs << endl; // 2
			cout << "��Ŀ¼����ļ�����" << RootEntCnt << endl; // 224
			cout << "����������" << TotSec << endl; // 2880
			cout << "ÿFAT��������" << FATSz << endl; // 9
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
