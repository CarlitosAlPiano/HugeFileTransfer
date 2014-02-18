#ifndef _COMMON_H_
#define _COMMON_H_

#ifndef WIN32
	#include <arpa/inet.h>
	#include <netdb.h>
	#include <sys/stat.h>
	#include <fcntl.h>
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
	#include <tchar.h>
	#include <winsock2.h>
	#include <ws2tcpip.h>
#endif
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <cerrno>
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
		bool running, done, isTx;
		double KBps, tRemaining;
		int tElapsed;
		int64_t currentSize, totalSize;
		fstream* fStream;
		int* sockProgress;
	public:
        static const string sep;
		static const double alpha;

        Monitor(HFTEntity* entity);
		~Monitor();
		void printHeader();
		void printResult();
		bool isRunning();
		double getKBps();
		double getRemaining();
		int getElapsed();
		int64_t getCurrentSize();
		double getCompleted();
		void stop();
		void run();
	};

	class HFTClientDLL_API HFTEntity {
	protected:
		bool modeIsUpload, isServerFileNameValid, ackTransfer;
		char uuid[SIZE_UUID];
		string clientFileName, serverFileName, fileLastModifyDate;
		int64_t fileSize, fileOffset;
		fstream fileStream;
		UDTSOCKET aSock;
		int sockProgress;
        int32_t mssTestBufSize;
	public:
        static const int mssLen;
        static int mss[];
		Monitor* mon;

		HFTEntity() : modeIsUpload(true), isServerFileNameValid(false), ackTransfer(false), clientFileName(""), serverFileName(""), fileLastModifyDate(""), fileSize(0), fileOffset(0), sockProgress(-1), mssTestBufSize(0), mon(NULL) {}
		~HFTEntity();
		bool isModeUpload();
        string getUUID();
        string getClientFileName();
        string getServerFileName();
        string getFileLastModifyDate();
        int64_t getFileSize();
        int64_t getFileOffset();
        fstream* getFileStream();
        UDTSOCKET* getSocket();
		int* getSockProgress();
		void setSockProgress(int s);
		bool findOptimumParams();
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
	void setBasicSockParams(UDTSOCKET& sock);
#ifdef WIN32
	string WinFileTimeToString(const FILETIME& fileTime);
	wstring str2wstr(const string& s);
	DWORD WINAPI adjustBw(LPVOID ntty);
	DWORD WINAPI monitor(LPVOID ntty);
#else
	void* adjustBw(void* ntty);
	void* monitor(void* ntty);
#endif
}

#endif
