#include "server.h"

using namespace HFT;

vector<HFTServer*> HFTServerHandler::clients;

bool HFTServer::isTx() {
    // Devuelve true si el servidor esta transmitiendo; false en caso contrario
    return !modeIsUpload;
}

string HFTServer::getClientIp() {
	return clientIp;
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

int HFTServer::run() {
    // Gestiono la operacion que el cliente quiera realizar siguiendo el protocolo establecido
    if (!receive(aSock, &modeIsUpload, sizeof(bool), " el modo de operación")) return -1;
    if (!receive(aSock, &mssTestBufSize, sizeof(int32_t), " el tamaño del test de MSS")) return -1;
    if (modeIsUpload) {
        if (!receive(aSock, uuid, SIZE_UUID, " el UUID del cliente")) return -1;
        if (!receiveString(aSock, clientFileName, " el nombre del archivo en el cliente")) return -1;
        if (!receive(aSock, &fileSize, sizeof(int64_t), " el tamaño del archivo en el cliente")) return -1;
        if (!receiveString(aSock, fileLastModifyDate, " la fecha de modificación del archivo en el cliente")) return -1;
        if (!receiveString(aSock, serverFileName, " el nombre del archivo en el servidor")) return -1;

        boost::uuids::uuid uClient;
        memcpy(&uClient, uuid, SIZE_UUID); // Convierto UUID en formato char* a boost::uuids::uuid
        string serverTempFileName(serverFileName + EXTENSION_TEMP), serverUsrFileName(serverFileName + EXTENSION_USER);
        cout << "Recibida petición del cliente " << uClient << " para subir el archivo" << endl << "'" << clientFileName << "', de tamaño " << fileSize <<
            " bytes y última fecha de modificación el " << fileLastModifyDate << endl << "Desea subirlo al servidor con el nombre de '" << serverFileName << "'." << endl;

        // Compruebo si el nombre de archivo en el servidor es valido y calculo el offset por el que debe empezar a mandar el cliente
        fileOffset = isFileValid(uClient, clientFileName, fileSize, fileLastModifyDate, serverFileName, serverTempFileName, serverUsrFileName);
        isServerFileNameValid = (fileOffset >= 0);
        if (!send(aSock, &isServerFileNameValid, sizeof(bool), " la validez del nombre de archivo en el servidor")) return -1;
        if (!isServerFileNameValid) { // Si el nombre de archivo en el servidor no era valido, cierro el socket y salgo
            UDT::close(aSock);
            return -1;
        }
		if (!send(aSock, &fileOffset, sizeof(int64_t), " el progreso de archivo ya subido")) return -1;
		if (!findOptimumParams()) {	// En caso de que el cliente lo hubiese solicitado, ejecuto un test de velocidad
			errorWithUDTmsg("No se pudo completar el test de MSS!");
		}

        if (fileOffset == 0) {
            remove(serverTempFileName.c_str());                                 // Me aseguro de que no exista un archivo temporal con el mismo nombre
            fileStream.open(serverUsrFileName.c_str(), ios::out | ios::binary); // Abro el archivo USER y guardo los datos relacionados con esta subida
            fileStream << uClient << clientFileName.size() << clientFileName << fileLastModifyDate.size() << fileLastModifyDate << fileSize;
            fileStream.close();
        }

		fileStream.open(serverTempFileName.c_str(), ios::out | ios::binary | ios::app);

		mon = new Monitor(this);	// Monitoreo y comienzo la subida del archivo
#ifndef WIN32
		pthread_t monitorThread;
		pthread_create(&monitorThread, NULL, monitor, this);
		pthread_detach(monitorThread);
#else
		CreateThread(NULL, 0, monitor, this, 0, NULL);
#endif
        
        int64_t offs = 0;
        if (UDT::ERROR == UDT::recvfile(aSock, fileStream, offs, fileSize - fileOffset)) {
            mon->stop();
            errorWithUDTmsg("No se pudo completar la subida del archivo '" + clientFileName + "'");
            return -1;
        }
        
        /*const int LEN=1400;
        char buf[LEN];
        int readLen;
        while (fileStream.tellp() < fileSize) {
            readLen = UDT::recv(aSock, buf, LEN, 0);
            if (readLen == UDT::ERROR) {
                break;
            }
            fileStream.write(buf, readLen);
        }*/

		ackTransfer = (fileStream.tellp() == fileSize);
        fileStream.flush();
        fileStream.close();
        
		if (!ackTransfer) {
            errorWithUDTmsg("No se pudo completar la subida del archivo '" + clientFileName + "'");
            return -1;
        }
		// El envio suele ir unos 12MB por delante que la recepcion -> Esperar a recibir un ACK del servidor previene que cierre el socket antes de que...
        send(aSock, &ackTransfer, sizeof(bool), " la confirmacion de recepcion del archivo en el servidor"); // ... terminen de llegarle esos 12 MB
        // Por ultimo, borro el archivo '.usr' y renombro el '.temp' al nombre deseado
        if (rename(serverTempFileName.c_str(), serverFileName.c_str()) == 0) {
            if (remove(serverUsrFileName.c_str()) != 0) {
                info("El archivo se ha subido correctamente, pero ha sido imposible eliminar el archivo temporal '" + serverUsrFileName + "'.");
            }
            info("¡Enhorabuena! El archivo '" + serverFileName + "' se ha subido correctamente.");
        } else {
            info("El archivo se ha subido correctamente, pero ha sido imposible renombrarlo. Su nombre actual es '" + serverTempFileName + "'.");
		}
		mon->stop();
		while (mon->isRunning()) pause(1);
    }

    UDT::close(aSock); // Cierro el socket
    return 0;
}

bool HFTServerHandler::iniFromArgs(int argc, char* argv[]) {
    // Inicializa el servidor (port) a partir de argv, devolviendo true si todo fue correcto
    if (argc > 3) {
        error("Uso: server PuertoServidor PuertoProgreso.");
        return false;
    } else if (argc == 3) {
        if (atoi(argv[1]) <= 0) {
            error("El puerto del servidor debe ser un número mayor que 0.");
            return false;
        } else {
            port = string(argv[1]);
        }
        if (atoi(argv[2]) <= 0) {
            error("El puerto de progreso debe ser un número mayor que 0.");
            return false;
        } else {
			portProgress = string(argv[2]);
        }
    } else {
		port = DEF_SRV_PORT;
		portProgress = DEF_SRV_PROGRESS_PORT;
    }
    
    return true;
}

bool HFTServerHandler::configurePassiveSocket() {
    // Configura el socket pasivo para que escuche en el puerto port
    addrinfo addrAux, *addrServer;
    
    memset(&addrAux, 0, sizeof(struct addrinfo)); // Inicializo el socket como requiere la libreria UDT
    addrAux.ai_flags = AI_PASSIVE;
    addrAux.ai_family = AF_INET;
    addrAux.ai_socktype = SOCK_STREAM;
    
    pSock = UDT::socket(addrAux.ai_family, addrAux.ai_socktype, addrAux.ai_protocol);
	setBasicSockParams(pSock);
	UDT::setsockopt(pSock, 0, UDT_RCVSYN, new bool(false), sizeof(bool));
    
    if (0 != getaddrinfo(NULL, port.c_str(), &addrAux, &addrServer)) { // Compruebo que el puerto este accesible
        error("Puerto (" + port + ") ilegal o en uso");
        return false;
    }
    
    if (UDT::ERROR == UDT::bind(pSock, addrServer->ai_addr, addrServer->ai_addrlen)) { // Enlazo el socket pasivo al puerto port
        errorWithUDTmsg("No se pudo enlazar el socket al puerto " + port);
        return false;
    }
    
    freeaddrinfo(addrServer);
    
    if (UDT::ERROR == UDT::listen(pSock, BACKLOG)) { // Pongo el socket pasivo en modo escucha
        errorWithUDTmsg("No se pudo poner el socket en modo escucha");
        return false;
    }
    
    cout << "El servidor está a la escucha en el puerto " << port << "..." << endl;
    return true;
}

bool HFTServerHandler::configurePasiveProgressSocket() {
	// Configura el socket pasivo de progreso para que escuche en el puerto portProgress
	sockaddr_in addrServer;

	memset(&addrServer, 0, sizeof(addrServer));
	addrServer.sin_family = AF_INET;
	addrServer.sin_addr.s_addr = INADDR_ANY;
	addrServer.sin_port = htons((u_short)atoi(portProgress.c_str()));

	pProgressSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (::bind(pProgressSock, (sockaddr *)&addrServer, sizeof(addrServer)) < 0) {
		error("No se pudo enlazar el socket de progreso al puerto " + portProgress + ". Motivo: " + string(strerror(errno)));
		return false;
	}
	
	if (listen(pProgressSock, BACKLOG) < 0) {
		error("No se pudo poner el socket en modo escucha. Motivo: " + string(strerror(errno)));
		return false;
	}

#ifndef WIN32
	if (fcntl(pProgressSock, F_SETFL, O_NONBLOCK) < 0) {
#else
	u_long nonblock = 1;
	if (ioctlsocket(pProgressSock, FIONBIO, &nonblock) < 0) {
#endif
		error("No se pudo poner el socket en modo non-blocking.");
		return false;
	}

	return true;
}

#ifndef WIN32
void* HFTServerHandler::threadHandleClient(void* serv)
#else
DWORD WINAPI HFTServerHandler::threadHandleClient(LPVOID serv)
#endif
{
	HFTServer* handler = (HFTServer*)serv;
    
    handler->run();

	for (size_t i = 0; i < clients.size(); i++) {
		if (clients.at(i) == handler) {
			clients.erase(clients.begin() + i);
			break;
		}
	}

    delete handler;
    return 0;
}

int HFTServerHandler::run() {
    UDTUpDown _udt_;    // Automatically start up and clean up UDT module
	if (!configurePassiveSocket() || !configurePasiveProgressSocket()) return -1;

    sockaddr_storage addrClient;
	sockaddr_in addrProgressClient;
	int addrLen = sizeof(addrClient), addrProgressLen = sizeof(addrProgressClient);
	char clientIp[NI_MAXHOST], clientPort[NI_MAXSERV], clientProgressIp[INET_ADDRSTRLEN];
    UDTSOCKET aSock;
	int sockProgress;
	bool ipExists;
	HFTServer* serv;

	while (true) {
		if (UDT::INVALID_SOCK != (aSock = UDT::accept(pSock, (sockaddr*)&addrClient, &addrLen))) { // Acepto al cliente entrante
			UDT::setsockopt(aSock, 0, UDT_RCVSYN, new bool(true), sizeof(bool));
			// Obtengo la direccion del cliente, la muestro y creo un nuevo thread que gestione al cliente
			getnameinfo((sockaddr*)&addrClient, addrLen, clientIp, sizeof(clientIp), clientPort, sizeof(clientPort), NI_NUMERICHOST | NI_NUMERICSERV);
			cout << "Nueva conexión (" << clientIp << ":" << clientPort << ")." << endl;

			ipExists = false;
			for (size_t i = 0; i < clients.size(); i++) {
				if (clients.at(i)->getClientIp().compare(clientIp) == 0) {
					ipExists = true;
					cout << "Solo se permite una conexión desde esa ip!" << endl;
					break;
				}
			}

			if (!ipExists) {
				serv = new HFTServer(aSock, clientIp);
				clients.push_back(serv);
#ifndef WIN32
				pthread_t transferThread;
				pthread_create(&transferThread, NULL, threadHandleClient, serv);
				pthread_detach(transferThread);
#else
				CreateThread(NULL, 0, threadHandleClient, serv, 0, NULL);
#endif
			} else {
				UDT::close(aSock);
			}
		}
		if ((sockProgress = accept(pProgressSock, (sockaddr*)&addrProgressClient, (socklen_t*)&addrProgressLen)) >= 0) { // Acepto al cliente entrante
			// Obtengo la direccion del cliente, la muestro y añado el socket activo al entity que gestiona al cliente
			inet_ntop(AF_INET, &(addrProgressClient.sin_addr), clientProgressIp, INET_ADDRSTRLEN);
			cout << "Nueva conexión para el progreso (" << clientProgressIp << ":" << ntohs(addrProgressClient.sin_port) << ")." << endl;

			ipExists = false;
			for (size_t i = 0; i < clients.size(); i++) {
				if (clients.at(i)->getClientIp().compare(clientProgressIp) == 0) {
					ipExists = true;
					clients.at(i)->setSockProgress(sockProgress);
					break;
				}
			}

			if (!ipExists) {
#ifndef WIN32
				close(sockProgress);
#else
				closesocket(sockProgress);
#endif
			}
		}
	}

    UDT::close(pSock);
#ifndef WIN32
	close(pProgressSock);
#else
	closesocket(pProgressSock);
#endif
    return 0;
}

#ifndef WIN32
int main(int argc, char* argv[]) {
#else
int _tmain(int argc, _TCHAR* argv[]) {
#endif
    HFTServerHandler* server = new HFTServerHandler(argc, argv);
    int ret = server->run();
    delete server;

    return ret;
}
