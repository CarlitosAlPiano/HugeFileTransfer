#include "common.h"
#include <vector>

#define BACKLOG					10
#define DEF_SRV_PORT			"8888"
#define DEF_SRV_PROGRESS_PORT	"8889"

namespace HFT {
    class HFTServer : public HFTEntity {
	private:
		string clientIp;

		int64_t isFileValid(boost::uuids::uuid u, string remoteFileName, int64_t fileSize, string fileLastModifyDate, string localFileName, string tempFileName, string usrFileName);
	public:
		HFTServer(UDTSOCKET sock, string ip) : clientIp(ip) { this->aSock = sock; }
		virtual ~HFTServer() {}
		string getClientIp();
		bool isTx();
		int run();
	};

    class HFTServerHandler {
	private:
		UDTSOCKET pSock;
		int pProgressSock;
        string port, portProgress;
		static vector<HFTServer*> clients;

		bool iniFromArgs(int argc, char* argv[]);
		bool configurePassiveSocket();
		bool configurePasiveProgressSocket();
#ifndef WIN32
		static void* threadHandleClient(void* serv);
#else
		static DWORD WINAPI threadHandleClient(LPVOID serv);
#endif
	public:
		HFTServerHandler(int argc, char* argv[]) { if (!iniFromArgs(argc, argv)) port = DEF_SRV_PORT; }
		virtual ~HFTServerHandler() {}
		int run();
	};
}

