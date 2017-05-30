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
	// 创建文件   //测试成功
	/*
	MyCreateFile MCF = (MyCreateFile)GetProcAddress(h, "MyCreateFile");
	res = MCF("c:\\abc\\hgj","test1.txt");
	cout << "MyCreateFile => return " << res << endl;
	//*/

	// 删除文件   //有内容的文件删除失败  无内容的文件删除成功  文件目录深度过大也会失败
	/*
	MyDeleteFile MDF = (MyDeleteFile)GetProcAddress(h, "MyDeleteFile");
	res = MDF("c:\\a\\b", "test.txt");
	cout << "MyDeleteFile => return " << res << endl;
	//*/

	// 打开文件         //测试成功
	/*
	MyOpenFile MOF = (MyOpenFile)GetProcAddress(h, "MyOpenFile");
	res = MOF("c:\\abc\\", "test1.txt");
	cout << "MyOpenFile => return " << res << endl;
	//*/

	// 关闭文件            测试成功
	/*
	MyCloseFile MCF = (MyCloseFile)GetProcAddress(h, "MyCloseFile");
	MCF(res);
	cout << "MyCloseFile => void"<< endl;
	//*/

	// 移动文件指针
	/*
	MySetFilePointer MFP = (MySetFilePointer)GetProcAddress(h, "MySetFilePointer");
	res = MFP(res, 512, MY_FILE_BEGIN);
	cout << "MySetFilePointer => return " << res << endl;
	//*/

	// 写文件  //我们的有bug      俊钦的没问题
	/*
	MyWriteFile MWF = (MyWriteFile)GetProcAddress(h, "MyWriteFile");
	char pBuffer[4096] = { 0 };
	for (int i = 0; i < 4096; i++) {
	pBuffer[i] = 'a';
	}
	res = MWF(res, &pBuffer, 4096);
	cout << "MyWriteFile => return " << res << endl;
	//*/

	// 读文件    测试成功
	/*
	MyReadFile MRF = (MyReadFile)GetProcAddress(h, "MyReadFile");
	char rBuffer[5027] = { 0 };
	res = MRF(res, &rBuffer, 5028);
	cout << "MyReadFile => return " << res << endl;
	cout << "rBuffer => " << rBuffer << endl;
	cout << "rBuffer length => " << strlen(rBuffer) << endl;
	//*/

	// 创建目录    测试成功
	/*
	MyCreateDirectory MCD = (MyCreateDirectory)GetProcAddress(h, "MyCreateDirectory");
	res = MCD("c:\\abc", "hgj");
	cout << "MyCreateDirectory => return " << res << endl;
	//*/

	// 删除目录             我的回收簇和递归删除函数已经测试成功
	/*
	MyDeleteDirectory MCD = (MyDeleteDirectory)GetProcAddress(h, "MyDeleteDirectory");
	res = MCD("c:\\", "a");
	cout << "MyDeleteDirectory => return " << res << endl;
	//*/
	FreeLibrary(h);
	return 0;
}