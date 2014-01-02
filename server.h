#include "common.h"

#define BACKLOG         10
#define DEF_SRV_PORT    "8888"

namespace HFT {
    class HFTServer : public HFTEntity {
	private:
		UDTSOCKET pSock;

		bool iniFromArgs(int argc, char* argv[]);
		bool configurePassiveSocket(UDTSOCKET& pSock);
		int64_t isFileValid(boost::uuids::uuid u, string remoteFileName, int64_t fileSize, string fileLastModifyDate, string localFileName, string tempFileName, string usrFileName);
		void handleClient(UDTSOCKET& sock);
#ifndef WIN32
		static void* threadHandleClient(void* srvr);
#else
		static DWORD WINAPI threadHandleClient(LPVOID srvr);
#endif
	public:
		HFTServer(int argc, char* argv[]) { if (!iniFromArgs(argc, argv)) serverIp = DEF_SRV_PORT; }
		virtual ~HFTServer() {}
		bool isTx();
		int run();
	};
}

