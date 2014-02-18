#include "common.h"

namespace HFT {
    queue<string> HFTMsg::q;

	queue<string>* HFTMsg::getQ() {
		return &q;
	}

    const string Monitor::sep = " | ";
	const double Monitor::alpha = 1.0 / 4.0;
    
	Monitor::Monitor(HFTEntity* entity) : running(true), done(false), isTx(entity->isTx()), KBps(0.0), tRemaining(INFINITY), tElapsed(0), 
		currentSize(entity->getFileOffset()), totalSize(entity->getFileSize()), fStream(entity->getFileStream()), sockProgress(entity->getSockProgress()) {}

	Monitor::~Monitor() {
		cout << endl;
	}

	void Monitor::stop() {
		running = false;
	}

    void Monitor::printHeader() {
        if (isTx) {
			cout << "SendRate(KB/s)" << sep << "Total(MB)" << sep << "Time elapsed" << sep << "Time remaining" << sep << "Completed" << endl;
		} else {
			cout << "RecvRate(KB/s)" << sep << "Total(MB)" << sep << "Time elapsed" << sep << "Time remaining" << sep << "Completed" << endl;
		}
    }
    
	void Monitor::printResult() {
		cout << fixed << setw(14) << setprecision(2) << KBps
			<< sep << setw(9) << ((double)currentSize)/1000000.0
			<< sep << setw(10) << setprecision(0) << tElapsed << " s"
			<< sep << setw(12) << setprecision(0) << round(tRemaining) << " s"
			<< sep << setw(7) << setprecision(2) << getCompleted() << " %" << '\r';
		cout.flush();
	}

	bool Monitor::isRunning() {
		return !done;
	}

	double Monitor::getKBps() {
		return KBps;
	}

	double Monitor::getRemaining() {
		return tRemaining;
	}

	int Monitor::getElapsed() {
		return tElapsed;
	}

	int64_t Monitor::getCurrentSize() {
		return currentSize;
	}

	double Monitor::getCompleted() {
		return 100.0 * ((double)currentSize) / ((double)totalSize);
	}

	void Monitor::run() {
		int64_t oldCurrSize;
		time_t tStart = time(0);

		printHeader();

		while (running) {
			tElapsed = (int) difftime(time(0), tStart);
			oldCurrSize = currentSize;

			if (*sockProgress <= 0) {
			}else if (isTx) {
				if (recv(*sockProgress, (char*)&currentSize, sizeof(int64_t), 0) < 0) {
					cout << "No se pudo recibir el progreso actual de la transferencia :(" << endl;
				}
			} else {
				currentSize = fStream->tellp();
				if (::send(*sockProgress, (char*)&currentSize, sizeof(int64_t), 0) < 0) {
					cout << "No se pudo enviar el progreso actual de la transferencia :(" << endl;
				}
			}

			KBps = (1 - alpha)*KBps + alpha*(currentSize - oldCurrSize) / 1000;
			cout << "Elapsed time: " << tElapsed << "s; Real speed: " << (((double)(currentSize - oldCurrSize)) / 1000.0) << "; Smooth speed: " << KBps << endl;
			tRemaining = ((totalSize - currentSize) / 1000.0) / KBps;

			printResult();
			pause(1);
		}

		done = true;
	}
    
    const int HFTEntity::mssLen = 7;
    int HFTEntity::mss[mssLen] = {1052, 1500, 3000, 4500, 6000, 7500, 9000};

	HFTEntity::~HFTEntity() {
		UDT::close(aSock);
#ifndef WIN32
		close(sockProgress);
#else
		closesocket(sockProgress);
#endif
		delete(mon);
	}
    
    bool HFTEntity::isModeUpload() {
        return modeIsUpload;
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

	int* HFTEntity::getSockProgress() {
		return &sockProgress;
	}

	void HFTEntity::setSockProgress(int s) {
		sockProgress = s;
	}

	bool HFTEntity::findOptimumParams() {
		if (mssTestBufSize <= 0) return true;
        clock_t tStart, tEnd, minTime;
        int minMss = mss[mssLen-1];
		bool ok = true;
		// const int32_t bufSize = mssTestBufSize*1024/4;
		int32_t* buf = (int32_t*) malloc(mssTestBufSize*1024*sizeof(char));
		/*if (isTx()) {
			for (int32_t i = 0; i < bufSize; i++) {
				buf[i] = i;
			}
		}*/
        
        info("Ejecutando test para encontrar el MSS óptimo!");
        for (int i = 0; i < mssLen; i++) {
            UDT::setsockopt(aSock, 0, UDT_MSS, new int(mss[i]), sizeof(int));

            tStart = clock();
			if (isTx()) {
				if ((!send(aSock, buf, mssTestBufSize*1024, " el test numero " + boost::lexical_cast<string>(i + 1))) ||
					(!receive(aSock, &ok, sizeof(bool), " la finalización del test " + boost::lexical_cast<string>(i + 1)))) {
					free(buf);
					return false;
				}
			} else {
				if ((!receive(aSock, buf, mssTestBufSize*1024, " el test numero " + boost::lexical_cast<string>(i + 1))) ||
					(!send(aSock, &ok, sizeof(bool), " la finalización del test " + boost::lexical_cast<string>(i + 1)))) {
					free(buf);
					return false;
				}
			}
            tEnd = clock();
            
			if ((isTx()==modeIsUpload) && (i==0 || minTime>tEnd-tStart)) {
				minTime = tEnd - tStart;
				minMss = mss[i];
			}
            info("Test " + boost::lexical_cast<string>(i + 1) + " (MSS: " + boost::lexical_cast<string>(mss[i]) + "B) acabado en " + boost::lexical_cast<string>(double(tEnd-tStart)/CLOCKS_PER_SEC) + "s!");
        }
        
        info("Resultado: MSS óptimo = " + boost::lexical_cast<string>(minMss) + "B!");
        UDT::setsockopt(aSock, 0, UDT_MSS, new int(minMss), sizeof(int));
        free(buf);
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

	void setBasicSockParams(UDTSOCKET& sock) {
		linger l;
		l.l_onoff = 1;
		l.l_linger = 5; // Wait for 5s after calling close(aSock) to actually close the socket
		UDT::setsockopt(sock, 0, UDT_LINGER, &l, sizeof(linger));
		//UDT::setsockopt(sock, 0, UDT_MSS, new int(HFTEntity::mss[HFTEntity::mssLen - 1]), sizeof(int));
		//UDT::setsockopt(sock, 0, UDT_MSS, new int(9000), sizeof(int));
		//UDT::setsockopt(sock, 0, UDT_MAXBW, new int64_t(250000), sizeof(int64_t));
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
	void* adjustBw(void* ntty)
#else
	DWORD WINAPI adjustBw(LPVOID ntty)
#endif
	{   // Ajusta cada 30seg el ancho de banda de la transmision a lo que UDT estima
		HFTEntity* entity = (HFTEntity*)ntty;
		UDTSOCKET* aSock = entity->getSocket();
		UDT::TRACEINFO trace;
		int64_t bw;

		pause(20);
		while (UDT::getsockstate(*aSock) == CONNECTED) {
			UDT::perfmon(*aSock, &trace);
			bw = (int64_t) round(1.1*trace.mbpsBandwidth * 1024 / 8);
			cout << "\tEstableciendo el maximo BW a " << bw << "KB/s!" << endl;
			UDT::setsockopt(*aSock, 0, UDT_MAXBW, &bw, sizeof(int64_t));
			pause(30);
		}

		return 0;
	}

#ifndef WIN32
	void* monitor(void* ntty)
#else
	DWORD WINAPI monitor(LPVOID ntty)
#endif
	{   // Monitorea la transmision/recepcion del archivo
		HFTEntity* entity = (HFTEntity*)ntty;

		entity->mon->run();

		return 0;
	}
}
