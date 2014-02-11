#include "common.h"

#define BACKLOG         10
#define DEF_SRV_PORT    "8888"

namespace HFT {
    class HFTServer : public HFTEntity {
	private:
		int64_t isFileValid(boost::uuids::uuid u, string remoteFileName, int64_t fileSize, string fileLastModifyDate, string localFileName, string tempFileName, string usrFileName);
	public:
		HFTServer(UDTSOCKET sock) { this->aSock = sock; }
		virtual ~HFTServer() {}
		bool isTx();
		int run();
	};

    class HFTServerHandler {
	private:
		UDTSOCKET pSock;
        string port;

		bool iniFromArgs(int argc, char* argv[]);
		bool configurePassiveSocket();
#ifndef WIN32
		static void* threadHandleClient(void* sock);
#else
		static DWORD WINAPI threadHandleClient(LPVOID sock);
#endif
	public:
		HFTServerHandler(int argc, char* argv[]) { if (!iniFromArgs(argc, argv)) port = DEF_SRV_PORT; }
		virtual ~HFTServerHandler() {}
		int run();
	};
}

