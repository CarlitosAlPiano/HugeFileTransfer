#ifndef _CLIENT_H_
#define _CLIENT_H_

#include "common.h"

#define DEF_UUID_FILENAME   (char*)"user.uuid"

namespace HFT {
    class HFTClientDLL_API HFTClient : public HFTEntity {
	private:
		string serverIp, serverPort, serverProgressPort;
		
        bool iniFromArgs(int argc, char* argv[]);
		bool getUUID(char* uuidFileName);
		bool connectToServer();
		bool connectToProgressServer();
	public:
		HFTClient() {}
		HFTClient(int argc, char* argv[]) { iniFromArgs(argc, argv); }
		HFTClient(bool modeIsUpload, string serverIp, string serverPort, string serverProgressPort, string clientFileName, string serverFileName, int32_t mssTestBufSize = 0, string uuidFileName = "") { ini(modeIsUpload, serverIp, serverPort, serverProgressPort, clientFileName, serverFileName, mssTestBufSize, uuidFileName); }
		virtual ~HFTClient() {}
        string getServerIp();
        string getServerPort();
        string getServerProgressPort();
		bool ini(bool modeIsUpload, string serverIp, string serverPort, string serverProgressPort, string clientFileName, string serverFileName, int32_t mssTestBufSize = 0, string uuidFileName = "");
		bool isTx();
		int run();
	};
}

#endif
