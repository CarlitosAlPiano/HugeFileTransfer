#pragma once

#include <client.h>
#include <msclr\marshal_cppstd.h>

using namespace System;
using namespace msclr::interop;
typedef Collections::Generic::List<String^> StringList;

namespace HFT {
	public ref class HFTClientWrapper {
	private:
		HFTClient* client;
		double KBps, remaining, completed;
		int64_t currentSize;

		void toNativeString(string& oStr, String^ iStr);
		String^ fromNativeString(const string& iStr);
	public:
		HFTClientWrapper();
		HFTClientWrapper(bool modeIsUpload, String^ serverIp, String^ serverPort, String^ clientFileName, String^ serverFileName, String^ uuidFileName);
		~HFTClientWrapper();
		!HFTClientWrapper();
		void ini(bool modeIsUpload, String^ serverIp, String^ serverPort, String^ clientFileName, String^ serverFileName, String^ uuidFileName);
		bool obtainMonitorResults();
		double getKBps();
		double getRemaining();
		Int64 getCurrentSize();
		double getCompleted();
		StringList^ getMessagesList();
		int run();
	};
}
