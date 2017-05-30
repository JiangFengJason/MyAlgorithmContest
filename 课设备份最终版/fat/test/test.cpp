#include <iostream>
#include <windows.h>
using namespace std;

#define MY_FILE_BEGIN           0
#define MY_FILE_CURRENT         1
#define MY_FILE_END             2

typedef DWORD(*MyCreateFile)(char *pszFolderPath, char *pszFileName);
typedef DWORD(*MyWriteFile)(DWORD dwHandle, LPVOID pBuffer, DWORD dwBytesToWrite);
typedef BOOL(*MyDeleteFile)(char *pszFolderPath, char *pszFileName);
typedef DWORD(*MyOpenFile)(char *pszFolderPath, char *pszFileName);
typedef void(*MyCloseFile)(DWORD dwHandle);
typedef DWORD(*MyReadFile)(DWORD dwHandle, LPVOID pBuffer, DWORD dwBytesToRead);
typedef BOOL(*MyCreateDirectory)(char *pszFolderPath, char *pszFolderName);
typedef BOOL(*MyDeleteDirectory)(char *pszFolderPath, char *pszFolderName);
typedef BOOL(*MySetFilePointer)(DWORD dwFileHandle, int nOffset, DWORD dwMoveMethod);

int main() {
	DWORD res;
	HMODULE h = LoadLibraryA("..\\Debug\\fat.dll");
	if (NULL == h)
	{
		cout << "can not load library..." << endl;
		return 1;
	}
	// �����ļ�   //���Գɹ�
	/*
	MyCreateFile MCF = (MyCreateFile)GetProcAddress(h, "MyCreateFile");
	res = MCF("c:\\abc\\hgj","test1.txt");
	cout << "MyCreateFile => return " << res << endl;
	//*/

	// ɾ���ļ�   //�����ݵ��ļ�ɾ��ʧ��  �����ݵ��ļ�ɾ���ɹ�  �ļ�Ŀ¼��ȹ���Ҳ��ʧ��
	/*
	MyDeleteFile MDF = (MyDeleteFile)GetProcAddress(h, "MyDeleteFile");
	res = MDF("c:\\a\\b", "test.txt");
	cout << "MyDeleteFile => return " << res << endl;
	//*/

	// ���ļ�         //���Գɹ�
	/*
	MyOpenFile MOF = (MyOpenFile)GetProcAddress(h, "MyOpenFile");
	res = MOF("c:\\abc\\", "test1.txt");
	cout << "MyOpenFile => return " << res << endl;
	//*/

	// �ر��ļ�            ���Գɹ�
	/*
	MyCloseFile MCF = (MyCloseFile)GetProcAddress(h, "MyCloseFile");
	MCF(res);
	cout << "MyCloseFile => void"<< endl;
	//*/

	// �ƶ��ļ�ָ��
	/*
	MySetFilePointer MFP = (MySetFilePointer)GetProcAddress(h, "MySetFilePointer");
	res = MFP(res, 512, MY_FILE_BEGIN);
	cout << "MySetFilePointer => return " << res << endl;
	//*/

	// д�ļ�  //���ǵ���bug      ���յ�û����
	/*
	MyWriteFile MWF = (MyWriteFile)GetProcAddress(h, "MyWriteFile");
	char pBuffer[4096] = { 0 };
	for (int i = 0; i < 4096; i++) {
	pBuffer[i] = 'a';
	}
	res = MWF(res, &pBuffer, 4096);
	cout << "MyWriteFile => return " << res << endl;
	//*/

	// ���ļ�    ���Գɹ�
	/*
	MyReadFile MRF = (MyReadFile)GetProcAddress(h, "MyReadFile");
	char rBuffer[5027] = { 0 };
	res = MRF(res, &rBuffer, 5028);
	cout << "MyReadFile => return " << res << endl;
	cout << "rBuffer => " << rBuffer << endl;
	cout << "rBuffer length => " << strlen(rBuffer) << endl;
	//*/

	// ����Ŀ¼    ���Գɹ�
	/*
	MyCreateDirectory MCD = (MyCreateDirectory)GetProcAddress(h, "MyCreateDirectory");
	res = MCD("c:\\abc", "hgj");
	cout << "MyCreateDirectory => return " << res << endl;
	//*/

	// ɾ��Ŀ¼             �ҵĻ��մغ͵ݹ�ɾ�������Ѿ����Գɹ�
	/*
	MyDeleteDirectory MCD = (MyDeleteDirectory)GetProcAddress(h, "MyDeleteDirectory");
	res = MCD("c:\\", "a");
	cout << "MyDeleteDirectory => return " << res << endl;
	//*/
	FreeLibrary(h);
	return 0;
}