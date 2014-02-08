#include "common.h"

namespace HFT {
    queue<string> HFTMsg::q;

	queue<string>* HFTMsg::getQ() {
		return &q;
	}

    const string Monitor::sep = " | ";
    
	Monitor::Monitor() : running(false), isTx(false), KBps(0.0), remaining(INFINITY), currentSize(0), totalSize(0), fStream(NULL) {}

	Monitor::~Monitor() {
		// stop();
		cout << endl;
	}

	void Monitor::start(HFTEntity* entity) {
        isTx = entity->isTx();
        currentSize = entity->getFileOffset();
        totalSize = entity->getFileSize();
        fStream = entity->getFileStream();
		sock = entity->getSocket();
        
		running = true;
	}

	void Monitor::stop() {
		running = false;
	}

    void Monitor::printHeader() {
        if (isTx) {
			cout << "SendRate(KB/s)" << sep << "Total(MB)" << sep << "Time remaining" << sep << "Completed" << endl;
		} else {
			cout << "RecvRate(KB/s)" << sep << "Total(MB)" << sep << "Time remaining" << sep << "Completed" << endl;
		}
    }
    
	void Monitor::printResult() {
		cout << fixed << setw(14) << setprecision(2) << KBps
            << sep << setw(9) << ((double) currentSize)/1000000.0
            << sep << setw(12) << setprecision(0)  << round(remaining) << " s"
			<< sep << setw(7) << setprecision(2) << getCompleted() << " %" << '\r';
		cout.flush();
	}

	void Monitor::printResults() {
		if (!running) return;
		UDT::TRACEINFO trace;
		static int64_t resta = 0;
        int64_t oldCurrSize = currentSize;

		if (isTx) {
			if (fStream->tellg() >= 0) {
				if (oldCurrSize == 0 && resta == 0) resta = fStream->tellg();
				currentSize = fStream->tellg() - resta;
			} else {
				UDT::perfmon(*sock, &trace);
				currentSize += static_cast<int64_t>(trace.mbpsSendRate*1000000/8);
			}
		} else {
			currentSize = fStream->tellp();
		}
		KBps = ((double)(currentSize - oldCurrSize)) / 1000.0;
        remaining = ((totalSize-currentSize)/1000.0)/KBps;
        
		printResult();
	}

	double Monitor::getKBps() {
		return KBps;
	}

	double Monitor::getRemaining() {
		return remaining;
	}

	int64_t Monitor::getCurrentSize() {
		return currentSize;
	}

	double Monitor::getCompleted() {
		return 100.0 * ((double)currentSize) / ((double)totalSize);
	}

	void Monitor::run() {
		while (!running){
			pause(1);
		}

		printHeader();
		while (running) {
			printResults();
			pause(1);
		}
	}
    
    bool HFTEntity::isModeUpload() {
        return modeIsUpload;
    }
    
    string HFTEntity::getServerIp() {
        return serverIp;
    }
    
    string HFTEntity::getServerPort() {
        return serverPort;
    }
    
    string HFTEntity::getUUID() {
        boost::uuids::uuid u;
        memcpy(&u, uuid, SIZE_UUID);
        return to_string(u);
    }
    
    string HFTEntity::getClientFileName() {
        return clientFileName;
    }
    
    string HFTEntity::getServerFileName() {
        return serverFileName;
    }
    
    string HFTEntity::getFileLastModifyDate() {
        return fileLastModifyDate;
    }
    
    int64_t HFTEntity::getFileSize() {
        return fileSize;
    }
    
    int64_t HFTEntity::getFileOffset() {
        return fileOffset;
    }
    
    fstream* HFTEntity::getFileStream() {
        return &fileStream;
    }

	UDTSOCKET* HFTEntity::getSocket() {
		return &aSock;
	}

	bool HFTEntity::findOptimumParams() {
		const int bufSize = 1024000, n = 7;
		char buf[bufSize];
		int mss[n] = {1052, 1500, 3000, 4500, 6000, 7500, 9000}, minMss;
		double minTime = INFINITY, currTime;
		clock_t tStart;

		for (int i = 0; i < n; i++) {
			UDT::setsockopt(aSock, 0, UDT_MSS, new int(mss[i]), sizeof(int));
			tStart = clock();
			if (modeIsUpload) {
				if (!send(aSock, buf, bufSize, " el test numero " + boost::lexical_cast<string>(i + 1))) {
					return false;
				}
			} else {
				if (!receive(aSock, buf, bufSize, " el test numero " + boost::lexical_cast<string>(i + 1))) {
					return false;
				}
			}

			if (minTime > (currTime = double(clock() - tStart) / CLOCKS_PER_SEC)) {
				minMss = mss[i];
				minTime = currTime;
			}
			info("Test " + boost::lexical_cast<string>(i + 1) + " acabado! Tiempo: " + boost::lexical_cast<string>(currTime) + "s");
		}

		UDT::setsockopt(aSock, 0, UDT_MSS, new int(minMss), sizeof(int));
		return true;
	}

    void info(string msg) { // Muestra un mensaje de 'info'
		cout << "INFO: " << msg << endl;
        HFTMsg::q.push("INFO: " + msg);
	}

	void error(string msg) {    // Muestra un mensaje de 'error'
		cout << "ERROR: " << msg << endl;
        HFTMsg::q.push("ERROR: " + msg);
	}

	void errorWithUDTmsg(string msg) {  // Muestra un mensaje de 'error' que incluye el mensaje generado por UDT
		error(msg + ". Motivo: " + UDT::getlasterror().getErrorMessage());
	}

	bool send(const UDTSOCKET& sock, void* buf, int len, string tag) { // Envía cualquier informacion por el socket
		int sent = 0, totalSent = 0;

		while (totalSent < len) {
			if (UDT::ERROR == (sent = UDT::send(sock, (char*)buf+totalSent, len-totalSent, 0))) {
				errorWithUDTmsg("No se pudo enviar" + tag); // En caso de error, tag indica lo que se intentaba enviar (ej: el UUID)
				return false;
			}
			totalSent += sent;
		}

		return true;
	}

	bool sendString(const UDTSOCKET& sock, string str, string tag) {   // Envía un string por el socket (len + string)
		uint32_t len = str.size();

		if (!send(sock, &len, sizeof(uint32_t), " la longitud de" + tag)) return false;
		if (!send(sock, (void*)str.c_str(), len, tag)) return false;

		return true;
	}

	bool receive(const UDTSOCKET& sock, void* buf, int len, string tag) {  // Recibe cualquier informacion por el socket
		int recv = 0, totalRecv = 0;

		while (totalRecv < len) {
			if (UDT::ERROR == (recv = UDT::recv(sock, (char*)buf+totalRecv, len-totalRecv, 0))) {
				errorWithUDTmsg("No se pudo recibir" + tag);    // En caso de error, tag indica lo que se intentaba recibir (ej: el UUID)
				return false;
			}
			totalRecv += recv;
		}

		return true;
	}

	bool receiveString(const UDTSOCKET& sock, string& str, string tag) {   // Recibe un string por el socket (len + string)
		uint32_t len;
		char* buf;

		if (!receive(sock, &len, sizeof(uint32_t), " la longitud de" + tag)) return false;
		buf = (char*)malloc(len*sizeof(char));  // Reserva memoria dinamica para el char*
		if (!receive(sock, buf, len, tag)) { free(buf); return false; }
		str = string(buf, len);                 // Crea un string a partir del char*
		free(buf);                              // Y libera el char*

		return true;
	}

	bool readString(ifstream& ifs, string& str, string errMsg) { // Lee un string de un archivo (len + string)
		uint32_t len;

		ifs >> len;
		if (!ifs.good()) {
			ifs.close();
			info(errMsg);
			return false;
		}

		str.resize(len);
		ifs.read(&str[0], len);
		if (!ifs.good()) {
			ifs.close();
			info(errMsg);
			return false;
		}

		return true;
	}

#ifdef WIN32
	string WinFileTimeToString(const FILETIME& fileTime) {
		struct tm tm;
		SYSTEMTIME sysTime;
		FileTimeToSystemTime(&fileTime, &sysTime);
		memset(&tm, 0, sizeof(tm));

		tm.tm_year = sysTime.wYear - 1900;
		tm.tm_mon = sysTime.wMonth - 1;
		tm.tm_mday = sysTime.wDay;

		tm.tm_hour = sysTime.wHour;
		tm.tm_min = sysTime.wMinute;
		tm.tm_sec = sysTime.wSecond;

		char buf[26];
		asctime_s(buf, 26, &tm);
		return string(buf);
	}

	wstring str2wstr(const string& s) {
		int slength = (int)s.length() + 1;
		int len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
		wchar_t* buf = new wchar_t[len];
		MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
		wstring r(buf);
		delete[] buf;

		return r;
	}
#endif

#ifndef WIN32

	void* monitor(void* ntty)
#else
	DWORD WINAPI monitor(LPVOID ntty)
#endif
	{   // Monitorea la transmision/recepcion del archivo
		HFTEntity* entity = (HFTEntity*)ntty;

		entity->mon = new Monitor();
		entity->mon->run();
		delete(entity->mon);
		entity->mon = NULL;

		return 0;
	}
}
