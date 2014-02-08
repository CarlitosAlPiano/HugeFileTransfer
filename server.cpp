#include "server.h"

using namespace HFT;

bool HFTServer::iniFromArgs(int argc, char* argv[]) {
    // Inicializa el servidor (serverPort) a partir de argv, devolviendo true si todo fue correcto
    if (argc > 2) {
        error("Uso: server PuertoServidor.");
        return false;
    } else if (argc == 2) {
        if (atoi(argv[1]) <= 0) {
            error("El puerto del servidor debe ser un número mayor que 0.");
            return false;
        }
        else {
            serverPort = string(argv[1]);
        }
    } else {
        serverPort = DEF_SRV_PORT;
    }
    
    return true;
}

bool HFTServer::isTx() {
    // Devuelve true si el servidor esta transmitiendo; false en caso contrario
    return !modeIsUpload;
}

bool HFTServer::configurePassiveSocket(UDTSOCKET& pSock) {
    // Configura el socket pasivo para que escuche en el puerto serverPort
    addrinfo addrAux, *addrServer;

    memset(&addrAux, 0, sizeof(struct addrinfo)); // Inicializo el socket como requiere la libreria UDT
    addrAux.ai_flags = AI_PASSIVE;
    addrAux.ai_family = AF_INET;
    addrAux.ai_socktype = SOCK_STREAM;

    pSock = UDT::socket(addrAux.ai_family, addrAux.ai_socktype, addrAux.ai_protocol);

    if (0 != getaddrinfo(NULL, serverPort.c_str(), &addrAux, &addrServer)) { // Compruebo que el puerto este accesible
        error("Puerto (" + serverPort + ") ilegal o en uso");
        return false;
    }

#ifdef WIN32
	UDT::setsockopt(pSock, 0, UDT_MSS, new int(1052), sizeof(int)); // Windows UDP issue: For better performance, modify HKLM\System\CurrentControlSet\Services\Afd\Parameters\FastSendDatagramThreshold
#endif

    if (UDT::ERROR == UDT::bind(pSock, addrServer->ai_addr, addrServer->ai_addrlen)) { // Enlazo el socket pasivo al puerto serverPort
        errorWithUDTmsg("No se pudo enlazar el socket al puerto " + serverPort);
        return false;
    }

    freeaddrinfo(addrServer);

    if (UDT::ERROR == UDT::listen(pSock, BACKLOG)) { // Pongo el socket pasivo en modo escucha
        errorWithUDTmsg("No se pudo poner el socket en modo escucha");
        return false;
    }

    cout << "El servidor está a la escucha en el puerto " << serverPort << "..." << endl;
    return true;
}

int64_t HFTServer::isFileValid(boost::uuids::uuid u, string remoteFileName, int64_t fileSize, string fileLastModifyDate, string localFileName, string tempFileName, string usrFileName) {
    // Comprueba si el nombre de archivo remoto es valido: si ya existe un archivo con ese nombre o si existe un '.temp' pero corresponde a otro usuario, devuelvo -1. En caso contrario, el tamaño del '.temp'
    ifstream ifs(localFileName.c_str(), ios::in | ios::binary);
    if (ifs.is_open()) { // Compruebo si ya existe un archivo con ese nombre
        ifs.close();
        info("El archivo '" + localFileName + "' ya existe en el servidor.");
        return -1;
    }
    info("El archivo '" + localFileName + "' no existe en el servidor. Comprobando la existencia de una operación no concluida ('" + tempFileName + "').");

    ifs.open(usrFileName.c_str(), ios::in | ios::binary);
    if (ifs.is_open()) { // Compruebo si existe una subida incompleta con ese nombre
        boost::uuids::uuid uTemp;
        string tempLocalFileName, tempLastModifyDate;
        int64_t tempFileSize, alreadyUploaded;

        ifs >> uTemp; // Leo todos los parametros del archivo '.usr'
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

        ifs.open(tempFileName.c_str(), ios::in | ios::binary);
        if (ifs.is_open()) { // Obtengo el numero de bytes ya subidos
            ifs.seekg(0, ios::end);
            alreadyUploaded = ifs.tellg();
            ifs.close();
        } else {
            alreadyUploaded = 0;
        }

        // cout << "Parametros leídos:\n\tUUID: " << uTemp << ".\n\tLocalFileName: " << tempLocalFileName << ".\n\tLastModifyDate: " << tempLastModifyDate << ".\n\tFileSize: " << tempFileSize << "." << endl;
        // Compruebo si la subida temporal corresponde a este usuario
        if ((uTemp == u) && (tempLocalFileName.compare(remoteFileName) == 0) && (tempLastModifyDate.compare(fileLastModifyDate) == 0) && (tempFileSize == fileSize)) {
            info("Se ha detectado una subida incompleta de " + boost::lexical_cast<string>(alreadyUploaded)+" bytes subidos (quedan " + boost::lexical_cast<string>(fileSize - alreadyUploaded) + " bytes). Se continuará la subida por donde se dejó.");
            return alreadyUploaded;
        } else {
            info("El archivo '" + localFileName + "' (subida incompleta) ya existe en el servidor.");
            return -1;
        }
    }

    info("El archivo '" + tempFileName + "' tampoco existe en el servidor. Se procederá a subir el archivo.");
    return 0;
}

void HFTServer::handleClient(UDTSOCKET& sock) {
#ifndef WIN32
    pthread_t monitorThread;
    pthread_create(&monitorThread, NULL, monitor, this);
    pthread_detach(monitorThread);
#else
    CreateThread(NULL, 0, monitor, this, 0, NULL);
#endif
    // Gestiono la operacion que el cliente quiera realizar siguiendo el protocolo establecido
    if (!receive(sock, &modeIsUpload, sizeof(bool), " el modo de operación")) return;
    if (modeIsUpload) {
        if (!receive(sock, uuid, SIZE_UUID, " el UUID del cliente")) return;
        if (!receiveString(sock, clientFileName, " el nombre del archivo en el cliente")) return;
        if (!receive(sock, &fileSize, sizeof(int64_t), " el tamaño del archivo en el cliente")) return;
        if (!receiveString(sock, fileLastModifyDate, " la fecha de modificación del archivo en el cliente")) return;
        if (!receiveString(sock, serverFileName, " el nombre del archivo en el servidor")) return;

        boost::uuids::uuid uClient;
        memcpy(&uClient, uuid, SIZE_UUID); // Convierto UUID en formato char* a boost::uuids::uuid
        string serverTempFileName(serverFileName + EXTENSION_TEMP), serverUsrFileName(serverFileName + EXTENSION_USER);
        cout << "Recibida petición del cliente " << uClient << " para subir el archivo" << endl << "'" << clientFileName << "', de tamaño " << fileSize <<
            " bytes y última fecha de modificación el " << fileLastModifyDate << endl << "Desea subirlo al servidor con el nombre de '" << serverFileName << "'." << endl;

        // Compruebo si el nombre de archivo en el servidor es valido y calculo el offset por el que debe empezar a mandar el cliente
        fileOffset = isFileValid(uClient, clientFileName, fileSize, fileLastModifyDate, serverFileName, serverTempFileName, serverUsrFileName);
        isServerFileNameValid = (fileOffset >= 0);
        if (!send(sock, &isServerFileNameValid, sizeof(bool), " la validez del nombre de archivo en el servidor")) return;
        if (!isServerFileNameValid) { // Si el nombre de archivo en el servidor no era valido, cierro el socket y salgo
            UDT::close(sock);
            return;
        }
        if (!send(sock, &fileOffset, sizeof(int64_t), " el progreso de archivo ya subido")) return;

        if (fileOffset == 0) {
            remove(serverTempFileName.c_str());                                 // Me aseguro de que no exista un archivo temporal con el mismo nombre
            fileStream.open(serverUsrFileName.c_str(), ios::out | ios::binary); // Abro el archivo USER y guardo los datos relacionados con esta subida
            fileStream << uClient << clientFileName.size() << clientFileName << fileLastModifyDate.size() << fileLastModifyDate << fileSize;
            fileStream.close();
        }

        fileStream.open(serverTempFileName.c_str(), ios::out | ios::binary | ios::app);
        while (mon == NULL) pause(1);
        mon->start(this); // Monitoreo y comienzo la subida del archivo
        
        /*int64_t offs = 0;
        if (UDT::ERROR == UDT::recvfile(sock, fileStream, offs, fileSize - fileOffset)) {
            mon->stop();
            errorWithUDTmsg("No se pudo completar la subida del archivo '" + clientFileName + "'");
            return;
        }*/
        
        const int LEN=1400;
        char buf[LEN];
        int readLen;
        while (fileStream.good()) {
            readLen = UDT::recv(sock, buf, LEN, 0);
            if (readLen == UDT::ERROR) {
                break;
            }
            fileStream.write(buf, readLen);
        }
        ackTransfer = fileStream.eof();
        fileStream.flush();
        fileStream.close();
		mon->stop();
        
		if (!ackTransfer) {
            errorWithUDTmsg("No se pudo completar la subida del archivo '" + clientFileName + "'");
            return;
        }
		// El envio suele ir unos 12MB por delante que la recepcion -> Esperar a recibir un ACK del servidor previene que cierre el socket antes de que...
        send(sock, &ackTransfer, sizeof(bool), " la confirmacion de recepcion del archivo en el servidor"); // ... terminen de llegarle esos 12 MB
        // Por ultimo, borro el archivo '.usr' y renombro el '.temp' al nombre deseado
        if (rename(serverTempFileName.c_str(), serverFileName.c_str()) == 0) {
            if (remove(serverUsrFileName.c_str()) != 0) {
                info("El archivo se ha subido correctamente, pero ha sido imposible eliminar el archivo temporal '" + serverUsrFileName + "'.");
            }
            cout << "¡Enhorabuena! El archivo '" << serverFileName << "' se ha subido correctamente." << endl;
        }
        else {
            info("El archivo se ha subido correctamente, pero ha sido imposible renombrarlo. Su nombre actual es '" + serverTempFileName + "'.");
        }
    }

    UDT::close(sock); // Cierro el socket
}

#ifndef WIN32
void* HFTServer::threadHandleClient(void* srvr)
#else
DWORD WINAPI HFTServer::threadHandleClient(LPVOID srvr)
#endif
{
    HFTServer* server = (HFTServer*)srvr;

	info("Ejecutando test para encontrar el MSS óptimo!");
	server->findOptimumParams();

    UDTSOCKET* sock = new UDTSOCKET(server->aSock);
	int mss;
	UDT::getsockopt(server->aSock, 0, UDT_MSS, &mss, sizeof(int));
	UDT::setsockopt(*sock, 0, UDT_MSS, &mss, sizeof(int));
    server->handleClient(*sock);
    delete sock;
    return 0;
}

int HFTServer::run() {
    UDTUpDown _udt_;    // Automatically start up and clean up UDT module
    if (!configurePassiveSocket(pSock)) return -1;

    sockaddr_storage clientaddr;
    int addrlen = sizeof(clientaddr);
    char clientIp[NI_MAXHOST];
    char clientPort[NI_MAXSERV];

    while (true) {
        if (UDT::INVALID_SOCK == (aSock = UDT::accept(pSock, (sockaddr*)&clientaddr, &addrlen))) { // Acepto al cliente entrante
            errorWithUDTmsg("No se pudo aceptar al cliente entrante");
            continue;
        }

        // Obtengo la direccion del cliente, la muestro y creo un nuevo thread que gestione al cliente
        getnameinfo((sockaddr*)&clientaddr, addrlen, clientIp, sizeof(clientIp), clientPort, sizeof(clientPort), NI_NUMERICHOST | NI_NUMERICSERV);
        cout << "Nueva conexión (" << clientIp << ":" << clientPort << ")." << endl;
#ifndef WIN32
        pthread_t fileThread;
        pthread_create(&fileThread, NULL, threadHandleClient, this);
        pthread_detach(fileThread);
#else
        CreateThread(NULL, 0, threadHandleClient, this, 0, NULL);
#endif
    }

    UDT::close(pSock);
    return 0;
}

int main(int argc, char* argv[]) {
    HFTServer* server = new HFTServer(argc, argv);
    int ret = server->run();
    delete server;

    return ret;
}
