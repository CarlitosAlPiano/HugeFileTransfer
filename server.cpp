#include "common.h"
#include <iomanip>

#define BACKLOG         10
#define DEF_SRV_PORT    (char*)"8888"

using namespace std;

bool modeIsUpload, isServerFileNameValid;
char *serverPort;
string clientFileName, serverFileName, fileLastModifyDate;
char uuid[SIZE_UUID];
int64_t fileSize, fileOffset;

void getArgs(int argc, char* argv[]) {
    if (argc > 2) {
        error("Uso: server PuertoServidor.");
    }
    if (argc == 2) {
        if (atoi(argv[1]) <= 0) {
            error("El puerto del servidor debe ser un número mayor que 0.");
        } else {
            serverPort = argv[1];
        }
    } else {
        serverPort = DEF_SRV_PORT;
    }
    
}

void configurePassiveSocket(UDTSOCKET& pSock) {
    addrinfo addrAux, *addrServer;

    memset(&addrAux, 0, sizeof(struct addrinfo));
    addrAux.ai_flags = AI_PASSIVE;
    addrAux.ai_family = AF_INET;
    addrAux.ai_socktype = SOCK_STREAM;
    
    pSock = UDT::socket(addrAux.ai_family, addrAux.ai_socktype, addrAux.ai_protocol);
    
    if (0 != getaddrinfo(NULL, serverPort, &addrAux, &addrServer)) {
        error("Puerto (" + string(serverPort) + ") ilegal o en uso");
    }

#ifdef WIN32
    int mss = 1052; // Windows UDP issue: For better performance, modify HKLM\System\CurrentControlSet\Services\Afd\Parameters\FastSendDatagramThreshold
    UDT::setsockopt(pSock, 0, UDT_MSS, &mss, sizeof(int));
#endif
    
    if (UDT::ERROR == UDT::bind(pSock, addrServer->ai_addr, addrServer->ai_addrlen)) {
        errorWithUDTmsg("No se pudo enlazar el socket al puerto " + string(serverPort));
    }
    
    freeaddrinfo(addrServer);
    
    if (UDT::ERROR == UDT::listen(pSock, BACKLOG)) {
        errorWithUDTmsg("No se pudo poner el socket en modo escucha");
    }
    
    cout << "El servidor está a la escucha en el puerto " << serverPort << "..." << endl;
}

#ifndef WIN32
void* handleClient(void* uSocket)
#else
DWORD WINAPI handleClient(LPVOID uSocket)
#endif
{
    UDTSOCKET aSock = *(UDTSOCKET*)uSocket;
    delete (UDTSOCKET*)uSocket;
    
    receive(aSock, &modeIsUpload, sizeof(bool), " el modo de operación");
    if (modeIsUpload) {
        if(!receive(aSock, uuid, SIZE_UUID, " el UUID del cliente")) return 0;
        if(!receiveString(aSock, clientFileName, " el nombre del archivo en el cliente")) return 0;
        if(!receive(aSock, &fileSize, sizeof(int64_t), " el tamaño del archivo en el cliente")) return 0;
        if(!receiveString(aSock, fileLastModifyDate, " la fecha de modificación del archivo en el cliente")) return 0;
        if(!receiveString(aSock, serverFileName, " el nombre del archivo en el servidor")) return 0;
        
        boost::uuids::uuid uClient;
        memcpy(&uClient, uuid, SIZE_UUID);
        string serverTempFileName(serverFileName + EXTENSION_TEMP), serverUsrFileName(serverFileName + EXTENSION_USER);
        cout << "Recibida petición del cliente " << uClient << " para subir el archivo" << endl << "'" << clientFileName << "', de tamaño " << fileSize <<
        " bytes y última fecha de modificación el " << fileLastModifyDate << endl << "Desea subirlo al servidor con el nombre de '" << serverFileName << "'." << endl;
        
        fileOffset = fileIsValid(uClient, clientFileName, fileSize, fileLastModifyDate, serverFileName, serverTempFileName, serverUsrFileName);
        isServerFileNameValid = (fileOffset >= 0);
        if(!send(aSock, &isServerFileNameValid, sizeof(bool), " la validez del nombre de archivo en el servidor")) return 0;
        if (!isServerFileNameValid) {
            UDT::close(aSock);
            return 0;
        }
        if(!send(aSock, &fileOffset, sizeof(int64_t), " el progreso de archivo ya subido")) return 0;

        fstream fileStream;
        int64_t offs = 0;
        if (fileOffset == 0) {
            remove(serverTempFileName.c_str());                         // Me aseguro de que no exista un archivo temporal con el mismo nombre
            fileStream.open(serverUsrFileName, ios::out|ios::binary);   // Abro el archivo USER y guardo los datos relacionados con esta subida
            fileStream << uClient << clientFileName.size() << clientFileName << fileLastModifyDate.size() << fileLastModifyDate << fileSize;
            fileStream.close();
        }

        fileStream.open(serverTempFileName, ios::out|ios::binary|ios::app);
        if (UDT::ERROR == UDT::recvfile(aSock, fileStream, offs, fileSize-fileOffset)) {
            cout << endl;
            errorWithUDTmsg("No se pudo completar la subida del archivo '" + clientFileName + "'", false);
            return 0;
        }
        fileStream.close();

        cout << endl;
        if (rename(serverTempFileName.c_str(), serverFileName.c_str()) == 0) {
            if (remove(serverUsrFileName.c_str()) != 0) {
                info("El archivo se ha subido correctamente, pero ha sido imposible eliminar el archivo temporal '" + serverUsrFileName + "'.");
            }
            cout << "¡Enhorabuena! El archivo '" << serverFileName << "' se ha subido correctamente." << endl;
        } else {
            info("El archivo se ha subido correctamente, pero ha sido imposible renombrarlo. Su nombre actual es '" + serverTempFileName + "'.");
        }
    }
    
    UDT::close(aSock);
    return 0;
}

int main(int argc, char* argv[]) {
    getArgs(argc, argv);
    UDTUpDown _udt_;    // Automatically start up and clean up UDT module
    UDTSOCKET pSock, aSock;
    configurePassiveSocket(pSock);

    sockaddr_storage clientaddr;
    int addrlen = sizeof(clientaddr);
    char clientIp[NI_MAXHOST];
    char clientPort[NI_MAXSERV];
    
    while (true) {
        if (UDT::INVALID_SOCK == (aSock = UDT::accept(pSock, (sockaddr*)&clientaddr, &addrlen))) {
            errorWithUDTmsg("No se pudo aceptar al cliente entrante", false);
            continue;
        }
        
        getnameinfo((sockaddr*)&clientaddr, addrlen, clientIp, sizeof(clientIp), clientPort, sizeof(clientPort), NI_NUMERICHOST|NI_NUMERICSERV);
        cout << "Nueva conexión (" << clientIp << ":" << clientPort << ")." << endl;
#ifndef WIN32
        pthread_t fileThread, monitorThread;
        pthread_create(&fileThread, NULL, handleClient, new UDTSOCKET(aSock));
        pthread_detach(fileThread);
        pthread_create(&monitorThread, NULL, monitorTx, new UDTSOCKET(aSock));
        pthread_detach(monitorThread);
#else
        CreateThread(NULL, 0, handleClient, new UDTSOCKET(aSock), 0, NULL);
        CreateThread(NULL, 0, monitorTx, new UDTSOCKET(aSock), 0, NULL);
#endif
    }
    
    UDT::close(pSock);
    return 0;
}
