#ifndef _COMMON_H_
#define _COMMON_H_

#ifndef WIN32
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/stat.h>
#else
#include <winsock2.h>
#include <ws2tcpip.h>
#endif
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
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

struct UDTUpDown{
    UDTUpDown() {
        UDT::startup(); // Use this function to initialize the UDT library
    }
    ~UDTUpDown() {
        UDT::cleanup(); // Use this function to release the UDT library
    }
};

void info(string msg) { // Muestra un mensaje de 'info'
    cout << "INFO: " << msg << endl;
}

void error(string msg, bool close = true) {    // Muestra un mensaje de 'error'
    cout << "ERROR: " << msg << endl;
    if (close) exit(-1);
}

void errorWithUDTmsg(string msg, bool close = true) {  // Muestra un mensaje de 'error' que incluye el mensaje generado por UDT
    error(msg + ". Motivo: " + UDT::getlasterror().getErrorMessage(), close);
}

bool send(const UDTSOCKET& sock, void* buf, int len, string tag = "") { // Envía cualquier informacion por el socket
    if (UDT::ERROR == UDT::send(sock, (char*)buf, len, 0)) {
        errorWithUDTmsg("No se pudo enviar" + tag, false); // En caso de error, tag indica lo que se intentaba enviar (ej: el UUID)
        return false;
    }
    
    return true;
}

bool sendString(const UDTSOCKET& sock, string str, string tag = "") {   // Envía un string por el socket (len + string)
    uint32_t len = str.size();
    
    if(!send(sock, &len, sizeof(uint32_t), " la longitud de" + tag)) return false;
    if(!send(sock, (void*)str.c_str(), len, tag)) return false;
    
    return true;
}

bool receive(const UDTSOCKET& sock, void* buf, int len, string tag = "") {  // Recibe cualquier informacion por el socket
    if (UDT::ERROR == UDT::recv(sock, (char*)buf, len, 0)) {
        errorWithUDTmsg("No se pudo recibir" + tag, false);    // En caso de error, tag indica lo que se intentaba recibir (ej: el UUID)
        return false;
    }
    
    return true;
}

bool receiveString(const UDTSOCKET& sock, string& str, string tag = "") {   // Recibe un string por el socket (len + string)
    uint32_t len;
    char* buf;
    
    if(!receive(sock, &len, sizeof(uint32_t), " la longitud de" + tag)) return false;
    buf = (char*)malloc(len*sizeof(char));  // Reserva memoria dinamica para el char*
    if(!receive(sock, buf, len, tag)) {free(buf); return false;}
    str = string(buf, len);                 // Crea un string a partir del char*
    free(buf);                              // Y libera el char*

    return true;
}

bool readString(ifstream& ifs, string& str, string errMsg) { // Lee un string de un archivo (len + string)
    uint32_t len;

    ifs >> len;
    if(!ifs.good()) {
        ifs.close();
        info(errMsg);
        return false;
    }
    
    str.resize(len);
    ifs.read(&str[0], len);
    if(!ifs.good()) {
        ifs.close();
        info(errMsg);
        return false;
    }
    
    return true;
}

int64_t fileIsValid(boost::uuids::uuid u, string remoteFileName, int64_t fileSize, string fileLastModifyDate, string localFileName, string tempFileName, string usrFileName) {
// Comprueba si el nombre de archivo remoto es valido: si ya existe un archivo con ese nombre o si existe un '.temp' pero corresponde a otro usuario, devuelvo -1. En caso contrario, el tamaño del '.temp'
    ifstream ifs(localFileName, ios::in|ios::binary);
    if (ifs.is_open()) {
        ifs.close();
        info("El archivo '" + localFileName + "' ya existe en el servidor.");
        return -1;
    }
    info("El archivo '" + localFileName + "' no existe en el servidor. Comprobando la existencia de una operación no concluida ('" + tempFileName + "').");

    ifs.open(usrFileName, ios::in|ios::binary);
    if (ifs.is_open()) {
        boost::uuids::uuid uTemp;
        string tempLocalFileName, tempLastModifyDate;
        int64_t tempFileSize, alreadyUploaded;
        
        ifs >> uTemp;
        if (uTemp.is_nil() || !ifs.good()) {
            ifs.close();
            info("Error leyendo el UUID del archivo temporal '" + tempFileName + "'. Será sobreescrito por la subida actual.");
            return 0;
        }
        if (!readString(ifs, tempLocalFileName, "Error leyendo el nombre del archivo local del archivo temporal '" + tempFileName + "'. Será sobreescrito por la subida actual.")) {
            return 0;
        }
        if (!readString(ifs, tempLastModifyDate, "Error leyendo la fecha de modificación local del archivo temporal '" + tempFileName + "'. Será sobreescrito por la subida actual.")) {
            return 0;
        }
        ifs >> tempFileSize;
        if (ifs.fail()) {
            ifs.close();
            info("Error leyendo el tamaño del archivo local del archivo temporal '" + tempFileName + "'. Será sobreescrito por la subida actual.");
            return 0;
        }
        ifs.close();
        
        ifs.open(tempFileName, ios::in|ios::binary);
        if (ifs.is_open()) {
            ifs.seekg(0, ios::end);
            alreadyUploaded = ifs.tellg();
            ifs.close();
        } else {
            alreadyUploaded = 0;
        }
        
        cout << "Parametros leídos:\n\tUUID: " << uTemp << ".\n\tLocalFileName: " << tempLocalFileName << ".\n\tLastModifyDate: " << tempLastModifyDate << ".\n\tFileSize: " << tempFileSize << "." << endl;
        if ((uTemp==u) && (tempLocalFileName.compare(remoteFileName)==0) && (tempLastModifyDate.compare(fileLastModifyDate)==0) && (tempFileSize==fileSize)) {
            info("Se ha detectado una subida incompleta de " + boost::lexical_cast<string>(alreadyUploaded) + " bytes subidos (quedan " + boost::lexical_cast<string>(fileSize-alreadyUploaded) + " bytes). Se continuará la subida por donde se dejó.");
            return alreadyUploaded;
        } else {
            info("El archivo '" + localFileName + "' (subida incompleta) ya existe en el servidor.");
            return -1;
        }
    }
    
    info("El archivo '" + tempFileName + "' tampoco existe en el servidor. Se procederá a subir el archivo.");
    return 0;
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
    
    return string(asctime(&tm));
}
#endif

void monitor(void* uSock, bool monitorTx) { // Monitorea varios parametros del protocolo UDT (mbps, etc)
    UDTSOCKET sock = *(UDTSOCKET*)uSock;
    delete (UDTSOCKET*)uSock;
    UDT::TRACEINFO perf;
    
    pause(1);
    cout << "SendRate(Mb/s)\tRTT(ms)\tCWnd\tPktSndPeriod(us)\tRecvACK\tRecvNAK" << endl;
    while (true) {
        if (UDT::ERROR == UDT::perfmon(sock, &perf)) {
            //cout << endl << "Error monitoreando" << endl;
            //errorWithUDTmsg("No se pudo monitorear la transferencia de datos");
            return;
        }
        
        if (monitorTx) {
            cout << perf.mbpsSendRate;
        } else {
            cout << perf.mbpsRecvRate;
        }
        cout << "\t\t" << perf.msRTT << "\t"
        << perf.pktCongestionWindow << "\t"
        << perf.usPktSndPeriod << "\t\t\t"
        << perf.pktRecvACK << "\t"
        << perf.pktRecvNAK << "\r";
        cout.flush();
        
        pause(1);
    }
    
    UDT::close(sock);
}

#ifndef WIN32
void* monitorTx(void* uSock)
#else
DWORD WINAPI monitorTx(LPVOID uSock)
#endif
{   // Monitorea la transmision del archivo
    monitor(uSock, true);
    return 0;
}
    
#ifndef WIN32
void* monitorRx(void* uSock)
#else
DWORD WINAPI monitorRx(LPVOID uSock)
#endif
{   // Monitorea la recepcion del archivo
    monitor(uSock, false);
    return 0;
}
    
#endif
