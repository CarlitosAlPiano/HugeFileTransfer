#include "HFTClientWrapper.h"

namespace HFT {
	HFTClientWrapper::HFTClientWrapper() {
		client = new HFTClient();
	}

	HFTClientWrapper::HFTClientWrapper(bool modeIsUpload, String^ serverIp, String^ serverPort, String^ serverProgressPort, String^ clientFileName, String^ serverFileName, Int32 mssTestBufSize, String^ uuidFileName)
		: HFTClientWrapper() {
		ini(modeIsUpload, serverIp, serverPort, serverProgressPort, clientFileName, serverFileName, mssTestBufSize, uuidFileName);
	}

	HFTClientWrapper::~HFTClientWrapper() {
		this->!HFTClientWrapper();
	}

	HFTClientWrapper::!HFTClientWrapper() {
		delete client;
	}

	bool HFTClientWrapper::ini(bool modeIsUpload, String^ serverIp, String^ serverPort, String^ serverProgressPort, String^ clientFileName, String^ serverFileName, Int32 mssTestBufSize, String^ uuidFileName) {
		string sIp, sPort, sProgressPort, cFileName, sFileName, uFileName;
		toNativeString(sIp, serverIp);
		toNativeString(sPort, serverPort);
		toNativeString(sProgressPort, serverProgressPort);
		toNativeString(cFileName, clientFileName);
		toNativeString(sFileName, serverFileName);
		toNativeString(uFileName, uuidFileName);
		return client->ini(modeIsUpload, sIp, sPort, sProgressPort, cFileName, sFileName, mssTestBufSize, uFileName);
	}

	bool HFTClientWrapper::obtainMonitorResults() {
		if (client->mon == NULL) return false;
		Int64 oldCurrSize = currentSize;

		KBps = client->mon->getKBps();
		remaining = client->mon->getRemaining();
		currentSize = client->mon->getCurrentSize();
		completed = client->mon->getCompleted();

		return true;
	}

	double HFTClientWrapper::getKBps() {
		return KBps;
	}

	double HFTClientWrapper::getRemaining() {
		return remaining;
	}

	Int64 HFTClientWrapper::getCurrentSize() {
		return currentSize;
	}

	double HFTClientWrapper::getCompleted() {
		return completed;
	}

	StringList^ HFTClientWrapper::getMessagesList() {
		StringList^ lstMessages = gcnew StringList();
		queue<string>* q = HFTMsg::getQ();

		while (!q->empty()) {
			lstMessages->Add(fromNativeString(q->front()));
			q->pop();
		}

		return lstMessages;
	}

	int HFTClientWrapper::run() {
		return client->run();
	}

	void HFTClientWrapper::toNativeString(string& oStr, String^ iStr) {
		oStr = marshal_as<string>(iStr);
	}

	String^ HFTClientWrapper::fromNativeString(const string& iStr) {
		return marshal_as<String^>(iStr);
	}
}