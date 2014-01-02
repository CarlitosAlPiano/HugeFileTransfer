#ifndef _COMMON_H_
#define _COMMON_H_

#ifndef WIN32
	#include <arpa/inet.h>
	#include <netdb.h>
	#include <sys/stat.h>
	#define HFTClientDLL_API
#else
	#pragma once
	#pragma comment (lib, "Ws2_32.lib")
	#define _SCL_SECURE_NO_WARNINGS
	#define HFTClientDLL_EXPORTS
	#ifdef HFTClientDLL_EXPORTS
		#define HFTClientDLL_API __declspec(dllexport) 
	#else
		#define HFTClientDLL_API __declspec(dllimport) 
	#endif
	#include <winsock2.h>
	#include <ws2tcpip.h>
	#include <time.h>
#endif
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <iomanip>
#include <queue>
#include <cmath>
#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <udt.h>

#define SIZE_UUID           16
#define EXTENSION_TEMP      ".temp"
#define EXTENSION_USER      ".usr"
#ifndef WIN32
#define pause(x)            sleep(x);
#else
#define pause(x)            Sleep(1000*x);
#endif

using namespace std;

namespace HFT {
	struct UDTUpDown{
		bool clean;
		UDTUpDown() : clean(false) {
			UDT::startup(); // Inicializa la libreria UDT
		}
		~UDTUpDown() {
			if (clean) {
				UDT::cleanup(); // Libera la libreria UDT
			}
		}
	};
    
	class HFTClientDLL_API HFTMsg {
    public:
        static queue<string> q;
		static queue<string>* getQ();
    };
    
    class HFTEntity;

	class HFTClientDLL_API Monitor {
	private:
		bool running, isTx;
		double KBps, remaining;
		int64_t currentSize, totalSize;
        fstream* fStream;
		UDTSOCKET* sock;
        
		void printResult();
	public:
        static const string sep;

        Monitor();
		~Monitor();
		void printHeader();
		void printResults();
		double getKBps();
		double getRemaining();
		int64_t getCurrentSize();
		double getCompleted();
		void start(HFTEntity* entity);
		void stop();
		void run();
	};

	class HFTClientDLL_API HFTEntity {
	protected:
		bool modeIsUpload, isServerFileNameValid, ackTransfer;
		string serverIp, serverPort;
		char uuid[SIZE_UUID];
		string clientFileName, serverFileName, fileLastModifyDate;
		int64_t fileSize, fileOffset;
		fstream fileStream;
		UDTSOCKET aSock;

		virtual bool iniFromArgs(int argc, char* argv[]) = 0;
	public:
		Monitor* mon;

		HFTEntity() : modeIsUpload(true), isServerFileNameValid(false), ackTransfer(false), serverIp(""), serverPort(""), clientFileName(""), serverFileName(""), fileLastModifyDate(""), fileSize(0), fileOffset(0), mon(NULL) {}
        bool isModeUpload();
        string getServerIp();
        string getServerPort();
        string getUUID();
        string getClientFileName();
        string getServerFileName();
        string getFileLastModifyDate();
        int64_t getFileSize();
        int64_t getFileOffset();
        fstream* getFileStream();
        UDTSOCKET* getSocket();
		virtual bool isTx() = 0;
		virtual int run() = 0;
	};

    void info(string msg);
	void error(string msg);
	void errorWithUDTmsg(string msg);
	bool send(const UDTSOCKET& sock, void* buf, int len, string tag = "");
	bool sendString(const UDTSOCKET& sock, string str, string tag = "");
	bool receive(const UDTSOCKET& sock, void* buf, int len, string tag = "");
	bool receiveString(const UDTSOCKET& sock, string& str, string tag = "");
	bool readString(ifstream& ifs, string& str, string errMsg);
#ifdef WIN32
	string WinFileTimeToString(const FILETIME& fileTime);
	wstring str2wstr(const string& s);
	DWORD WINAPI monitor(LPVOID ntty);
#else
	void* monitor(void* ntty);
#endif
}

#endif
