//TCP клиент
#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <sstream>

#include <iostream>
#include <conio.h>

#pragma comment (lib, "ws2_32.lib")

#define PORT 666
//#define SERVERADDR "192.168.0.100"//"127.0.0.1"
#define SERVERADDR "127.0.0.1"

#pragma warning(disable : 4996) //_CRT_SECURE_NO_WARNINGS

using namespace std;




string ReadHostFile()  
{
	const TCHAR szFileName[] = "C:\\WINDOWS\\system32\\drivers\\etc\\hosts";
	string str = "";
	DWORD dwTemp;
	HANDLE hFile = CreateFile(szFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		DWORD errorMessageID = ::GetLastError();   cout << "Error   " << errorMessageID << endl;	_getch();
	}

	LARGE_INTEGER size_ulrg;
	unsigned int size = -1; //__int64 size = -1;
	if (!GetFileSizeEx(hFile, &size_ulrg))
	{
		CloseHandle(hFile);
		DWORD errorMessageID = ::GetLastError();   cout << "Error   " << errorMessageID << endl;	_getch();
	}
	size = size_ulrg.QuadPart;



	if (size > 0)
	{
		char* mem = new char[size];
		ReadFile(hFile, mem, size, &dwTemp, NULL);   //ReadFile(hFile, lpBuff, dwBuffSize, &dwCount, NULL);
		if (size != dwTemp)
		{
			CloseHandle(hFile);
			DWORD errorMessageID = ::GetLastError();   cout << "Error   " << errorMessageID << endl;	_getch();
		}

		str = mem;


		delete[] mem;
	}

	CloseHandle(hFile);
	return str;
}


void sendS(string str, SOCKET sock)
{
	const char *ch = strdup(str.c_str());//const char *ch = strdup(stream.str().c_str());
	send(sock, ch, str.length(), 0);
}


void SendSystemInfo(SOCKET sock)
{
	ostringstream stream;
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	
	
	char buffer[MAX_COMPUTERNAME_LENGTH + 1];
	DWORD size;
	size = sizeof(buffer);
	GetComputerName(buffer, &size);
	stream << buffer << "        computerName" << endl;
	

	GetUserName(buffer, &size);
	stream << buffer << "           userName" << endl;
	
	int n;
	char dd[4];
	DWORD dr = GetLogicalDrives();


	stream << sysinfo.dwNumberOfProcessors << "                numberOfProcessors" << endl<<endl;

	stream << "Available disk drives : " << endl;
	for (int i = 0; i < 26; i++) {
		n = ((dr >> i) & 0x00000001);
		if (n == 1) {
			dd[0] = char(65 + i);
			dd[1] = ':';
			dd[2] = '\\';
			dd[3] = '\0';
			stream << "disk " << dd << endl << "\t";

			switch (GetDriveType(dd)) {
			case DRIVE_UNKNOWN:
				stream << "Uncknown drive";
				break;
			case DRIVE_NO_ROOT_DIR:
				stream << "There's no rote directory";
				break;
			case DRIVE_REMOVABLE:
				stream << "removable drive";
				break;
			case DRIVE_FIXED:
				stream << "fixed drive";
				break;
			case DRIVE_REMOTE:
				stream << "remote drive";
				break;
			case DRIVE_CDROM:
				stream << "(CD or DVD)";
				break;
			case DRIVE_RAMDISK:
				stream << "RAM-DISK(emulated disk)";
				break;
			default:;
			}
			stream << endl;

			// ***************************************************************************************
			char VolumeNameBuffer[100];
			char FileSystemNameBuffer[100];
			unsigned long VolumeSerialNumber;

			BOOL GetVolumeInformationFlag = GetVolumeInformationA(dd, VolumeNameBuffer, 100, &VolumeSerialNumber,
				NULL, NULL, FileSystemNameBuffer, 100);
			if (GetVolumeInformationFlag != 0) {
				stream << "\tFile System is " << FileSystemNameBuffer << endl;
			}
			// ****************************************************************************************
			ULARGE_INTEGER FreeBytesAvailable;
			ULARGE_INTEGER TotalNumberOfBytes;
			ULARGE_INTEGER TotalNumberOfFreeBytes;

			BOOL GetDiskFreeSpaceFlag = GetDiskFreeSpaceEx(
				dd,                                       // directory name
				NULL,                                     // bytes available to caller
				&TotalNumberOfBytes, // bytes on disk
				NULL);                                   // free bytes on disk
			if (GetDiskFreeSpaceFlag != 0)
			{
				stream << "\tTotal Number Of Bytes = " << TotalNumberOfBytes.QuadPart
					<< "( " << TotalNumberOfBytes.QuadPart / (1024 * 1024 * 1024.0)
					<< " Gb )" << endl;

				
			}


		}
	}


	string str = stream.str();
	sendS(str, sock);
	sendS(ReadHostFile(), sock);
}


int main(int argc, char* argv[])
{

	char buff[1024];
	printf("TCP DEMO CLIENT\n");
	
	if (WSAStartup(0x202, (WSADATA *)&buff[0]))// Шаг 1  инициализация библиотеки Winsock
	{
		printf("WSAStart error %d\n", WSAGetLastError());
		_getch();
		return 1;
	}
	
	SOCKET my_sock;// Шаг 2  создание сокета
	my_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (my_sock<0)
	{
		printf("Socket() error %d\n", WSAGetLastError());
		_getch();
		return 1;
	}
	// Шаг 3  установка соединения
	// заполнение структуры sockaddr_in – указание адреса и порта сервера
	sockaddr_in dest_addr;
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(PORT);
	HOSTENT *hst;
	// преобразование IP адреса из символьного в сетевой формат
	if (inet_addr(SERVERADDR) != INADDR_NONE)
		dest_addr.sin_addr.s_addr = inet_addr(SERVERADDR);

	
		else
		{
			printf("Invalid address %s\n", SERVERADDR);
			closesocket(my_sock);
			WSACleanup();
			_getch();
			return 1;
		}

	// адрес сервера получен – пытаемся установить соединение
	if (connect(my_sock, (sockaddr *)&dest_addr, sizeof(dest_addr)))
	{
		printf("Connect error %d\n", WSAGetLastError());
		_getch();
		return 1;
	}
	printf("Connection with %s was succesfully established \n\
Type quit for quit\n\n", SERVERADDR);
	// Шаг 4  чтение и передача сообщений
	int nsize;
	while ((nsize = recv(my_sock, &buff[0], sizeof(buff) - 1, 0)) != SOCKET_ERROR)
	{
		
		buff[nsize] = 0;// ставим завершающий ноль в конце строки
		cout << buff;
		SendSystemInfo(my_sock);
		Sleep(5000);
	}
	printf("Recv error %d\n", WSAGetLastError());
	closesocket(my_sock);
	WSACleanup();
	_getch();
	return 1;
}