#pragma once
#include <iostream>
#include "DiskLib.h"
#include "util.h"


#ifdef DLL_API  
#else  
#define DLL_API __declspec(dllexport)  
#endif

#ifdef __cplusplus
extern "C" {
#endif

	DLL_API DWORD MyCreateFile(char *pszFolderPath, char *pszFileName);
	/*
	Ҫ����ָ��Ŀ¼�´���ָ���ļ���������ļ������ڵĻ������Ŀ¼�����ڻ��ļ��Ѵ��ڣ�����ʧ�ܷ���0��
	����ɹ��򷵻�һ����ʾ���ļ��ı�ʶ��������Window�ľ�����ڲ����ݽṹ��ӳ�䷽����������
	pszFolderPath��Ŀ¼·������"C:\\Test\\Test01"�ȵ�
	pszFileName���ļ�������"Test.txt"�ȵ�
	*/

	DLL_API DWORD MyOpenFile(char *pszFolderPath, char *pszFileName);
	/*
	Ҫ�󣺴�ָ��Ŀ¼�µ�ָ���ļ������Ŀ¼�����ڻ����ļ������ڣ��򷵻�0��ʾʧ�ܣ�
	����ɹ��򷵻�һ����ʾ���ļ��ı�ʶ��������Window�ľ�����ڲ����ݽṹ��ӳ�䷽����������
	pszFolderPath��Ŀ¼·������"C:\\Test\\Test01"�ȵ�
	pszFileName���ļ�������"Test.txt"�ȵ�
	*/

	DLL_API void MyCloseFile(DWORD dwHandle);
	/*
	Ҫ�󣺹رո��ļ���
	���������
	dwHandle�������ʶ���ļ��ľ��������MyCreateFile���ص��Ǹ�
	*/

	DLL_API BOOL MyDeleteFile(char *pszFolderPath, char *pszFileName);
	/*
	Ҫ��ɾ��ָ��Ŀ¼�µ�ָ���ļ������Ŀ¼�����ڻ����ļ������ڣ��򷵻�0��ʾʧ�ܣ����򷵻�TRUE��ʾ�ɹ���
	���������
	pszFolderPath��Ŀ¼·������"C:\\Test\\Test01"�ȵ�
	pszFileName���ļ�������"Test.txt"�ȵ�
	*/

	DLL_API DWORD MyWriteFile(DWORD dwHandle, LPVOID pBuffer, DWORD dwBytesToWrite);
	/*
	Ҫ�󣺽�pBuffer��dwBytesToWrite���ȵ�����д��ָ���ļ����ļ�ָ��λ�á�
	���������
	dwHandle��MyOpenFile���ص�ֵ���������������ԭ�ⲻ���Ĵ����㣬���ڲ����ݽṹ����������
	pBuffer��ָ���д�����ݵĻ�����
	dwBytesToWrite����д�����ݵĳ���
	����ֵ���ɹ�д��ĳ��ȣ�-1��ʾʧ�ܡ�
	*/

	DLL_API DWORD MyReadFile(DWORD dwHandle, LPVOID pBuffer, DWORD dwBytesToRead);
	/*
	Ҫ�󣺶�ȡָ���ļ��С�ָ�����ȵ����ݵ�����Ļ�������
	���������
	dwHandle��MyOpenFile���ص�ֵ���������������ԭ�ⲻ���Ĵ����㣬���ڲ����ݽṹ����������
	pBuffer��ָ��������ݵĻ�����
	dwBytesToRead������ȡ���ݵĳ���
	����ֵ���ɹ���ȡ�ĳ��ȣ�-1��ʾʧ�ܡ�
	*/

	DLL_API BOOL MyCreateDirectory(char *pszFolderPath, char *pszFolderName);
	/*
	Ҫ����ָ��·���£�����ָ�����Ƶ��ļ��С����Ŀ¼�����ڻ���������ļ����Ѵ��ڣ��򷵻�FALSE��
	�����ɹ�����TRUE��
	���������
	pszFolderPath��Ŀ¼·������"C:\\Test\\Test01"�ȵ�
	pszFolderName���ļ������ƣ���"MyFolder"�ȵ�
	����ֵ�����Ŀ¼�����ڻ���������ļ����Ѵ��ڣ��򷵻�FALSE�������ɹ�����TRUE��
	*/

	DLL_API BOOL MyDeleteDirectory(char *pszFolderPath, char *pszFolderName);
	/*
	Ҫ����ָ��·���£�ɾ��ָ�����Ƶ��ļ��С����Ŀ¼�����ڻ��ɾ�����ļ��в����ڣ��򷵻�FALSE��
	ɾ���ɹ�����TRUE��
	���������
	pszFolderPath��Ŀ¼·������"C:\\Test\\Test01"�ȵ�
	pszFolderName���ļ������ƣ���"MyFolder"�ȵ�
	����ֵ�����Ŀ¼�����ڻ��ɾ�����ļ��в����ڣ��򷵻�FALSE��ɾ���ɹ�����TRUE��
	*/

	DLL_API BOOL MySetFilePointer(DWORD dwFileHandle, int nOffset, DWORD dwMoveMethod);
	/*
	��;���ƶ�ָ���ļ����ļ�ͷ����д��ͬλ�á�����ļ���������ڣ�����FALSE�����򷵻�TRUE
	dwHandle��MyOpenFile���ص�ֵ���������������ԭ�ⲻ���Ĵ����㣬���ڲ����ݽṹ����������
	nOffset��32λƫ�����������ɸ���Ϊ�㡣
	dwMoveMethod��ƫ�Ƶ���ʼλ�ã��������������Ϳ�ѡ��
	MY_FILE_BEGIN����ͷ����ʼ����ƫ��
	MY_FILE_CURRENT���ӵ�ǰ��ͷλ�ÿ�ʼ����ƫ��
	MY_FILE_END����ĩβ��ʼ����ƫ��
	*/

#ifdef __cplusplus
}
#endif