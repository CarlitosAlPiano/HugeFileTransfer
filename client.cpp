#include "client.h"

using namespace HFT;

bool HFTClient::isTx() {
	// Devuelve true si el cliente esta transmitiendo; false en caso contrario
	return modeIsUpload;
}

bool HFTClient::ini(bool modeIsUpload, string serverIp, string serverPort, string serverProgressPort, string clientFileName, string serverFileName, int32_t mssTestBufSize, string uuidFileName) {
    // Inicializa el cliente, devolviendo true si todo fue correcto
    this->modeIsUpload = modeIsUpload;
    this->serverIp = serverIp;
    this->serverPort = serverPort;
	this->serverProgressPort = serverProgressPort;
    this->clientFileName = clientFileName;
    this->serverFileName = serverFileName;
    this->mssTestBufSize = mssTestBufSize;

    if (atoi(serverPort.c_str()) <= 0) {
        error("El puerto del servidor debe ser un numero mayor que 0.");
        return false;
    }
    if (atoi(serverProgressPort.c_str()) <= 0) {
        error("El puerto de progreso del servidor debe ser un numero mayor que 0.");
        return false;
    }

    ifstream ifs(clientFileName.c_str(), ios::in|ios::binary);
    if (ifs.is_open()) { // Obtengo los parametros del archivo local (tamaño, fecha de modificacion, uuid...)
        ifs.seekg(0, ios::end);
        fileSize = ifs.tellg();
        ifs.close();
#ifndef WIN32
        struct stat attrib;
        if (stat(clientFileName.c_str(), &attrib) != 0) {
            error("La fecha de modificación del archivo a enviar no ha podido ser leída.");
            return false;
        }
        fileLastModifyDate = string(asctime(gmtime(&(attrib.st_mtime)))); // Convierto la fecha a string
#else
        FILETIME lastWriteTime;
		//HANDLE fh = CreateFile(str2wstr(clientFileName).c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		HANDLE fh = CreateFile(clientFileName.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (fh == INVALID_HANDLE_VALUE) { // Abro el archivo
            error("Imposible abrir el archivo local ('" + clientFileName + "') para obtener su fecha de modificacion.");
            return false;
        }
        if (!GetFileTime(fh, NULL, NULL, &lastWriteTime)) { // Obtengo la fecha de modificacion
            error("La fecha de modificacion del archivo a enviar no ha podido ser leida.");
            return false;
        }
        CloseHandle(fh);
        fileLastModifyDate = WinFileTimeToString(lastWriteTime); // Convierto la fecha a string
#endif
        fileLastModifyDate = fileLastModifyDate.substr(0, fileLastModifyDate.size()-1); //.pop_back(); // Borro el ultimo caracter ('\n')
    } else {
        error("El archivo local no existe o no se puede leer.");
        return false;
    }

    // Obtengo el UUID del cliente
    if (uuidFileName.empty()) {
        if (!getUUID(DEF_UUID_FILENAME)) return false;
    } else {
        if (!getUUID((char*)uuidFileName.c_str())) return false;
    }

    info("El archivo '" + clientFileName + "' ocupa " + boost::lexical_cast<string>(fileSize) + " bytes y fue modificado por ultima vez el " + fileLastModifyDate + ".");
    return true;
}

bool HFTClient::iniFromArgs(int argc, char* argv[]) {
    // Inicializa el cliente a partir de argv, devolviendo true si todo fue correcto
    if (argc == 7) {
        return ini((strcmp(argv[1], "1") == 0), string(argv[2]), string(argv[3]), string(argv[4]), string(argv[5]), string(argv[6]));
    } else if (argc == 8) {
        return ini((strcmp(argv[1], "1") == 0), string(argv[2]), string(argv[3]), string(argv[4]), string(argv[5]), string(argv[6]), atoi(argv[7]));
    } else if (argc == 9) {
        return ini((strcmp(argv[1], "1") == 0), string(argv[2]), string(argv[3]), string(argv[4]), string(argv[5]), string(argv[6]), atoi(argv[7]), string(argv[8]));
    } else {
        error("Uso: client subir/descargar(1/0) IPservidor PuertoServidor PuertoProgresoServidor ArchivoLocal ArchivoRemoto [MSSTest(tamaño en KB)] [ArchivoUUID].");
        return false;
    }
}

string HFTClient::getServerIp() {
    return serverIp;
}

string HFTClient::getServerPort() {
    return serverPort;
}

string HFTClient::getServerProgressPort() {
    return serverProgressPort;
}

bool HFTClient::getUUID(char* uuidFileName) {
    // Leo el UUID del cliente guardado en el archivo uuidFileName
    boost::uuids::uuid u = boost::uuids::nil_generator()();
    ifstream ifsUuid(uuidFileName, ios::in | ios::binary);
    if (ifsUuid.is_open()) { // Si existe el archivo, leo el UUID
        ifsUuid >> u;
        ifsUuid.close();
        info("Leido el siguiente UUID: " + to_string(u));
    } else {
        info("No se encontro un UUID para este usuario. Se procedera a generar uno aleatoriamente.");
    }

    if (u.is_nil()) { // Si no he leido un UUID o el que he leido no era valido, genero uno aleatoriamente
        u = boost::uuids::random_generator()();
        ofstream ofsUuid(uuidFileName, ios::out | ios::binary | ios::trunc);
        if (ofsUuid.is_open()) { // Guardo en el archivo uuidFileName el UUID que acabo de generar
            ofsUuid << u;
            ofsUuid.close();
            info("Creado el UUID: " + to_string(u));
        } else {
            error("No se pudo salvar el UUID creado para este usuario en el archivo '" + string(uuidFileName) + "'.\n\t¿Esta el archivo abierto o no tiene derechos para escribir en este directorio?");
            return false;
        }
    }
    
    memcpy(uuid, &u, SIZE_UUID); // Obtengo la representacion en chars del UUID
    return true;
}

bool HFTClient::connectToServer() {
    // Conecta el socket aSock al servidor, leyendo la direccion de las variables serverIp y serverPort
    addrinfo addrAux, *addrServer;

    memset(&addrAux, 0, sizeof(struct addrinfo)); // Inicializo el socket como requiere la libreria UDT
    addrAux.ai_flags = AI_PASSIVE;
    addrAux.ai_family = AF_INET;
    addrAux.ai_socktype = SOCK_STREAM;

    aSock = UDT::socket(addrAux.ai_family, addrAux.ai_socktype, addrAux.ai_protocol);
	setBasicSockParams(aSock);

    if (0 != getaddrinfo(serverIp.c_str(), serverPort.c_str(), &addrAux, &addrServer)) { // Compruebo que la direccion del servidor sea valida
        error("Direccion del servidor incorrecta (" + serverIp + ":" + serverPort + ").");
        return false;
    }
	if (UDT::ERROR == UDT::connect(aSock, addrServer->ai_addr, addrServer->ai_addrlen)) { // Y conecto el socket al servidor
        errorWithUDTmsg("No se pudo conectar con el servidor");
        return false;
    }

    freeaddrinfo(addrServer);
    return true;
}

bool HFTClient::connectToProgressServer() {
	// Conecta el socket sockProgress al servidor, leyendo la direccion de las variables serverIp y serverProgressPort
	sockaddr_in addrServer;
	hostent	*serverIpAddr;

	memset(&addrServer, 0, sizeof(addrServer));
	addrServer.sin_family = AF_INET;
	addrServer.sin_port = htons((u_short)atoi(serverProgressPort.c_str()));
	if (serverIpAddr = gethostbyname(serverIp.c_str())) {	// Obtengo la direccion ip si serverIp representa el nombre del host
		memcpy(&addrServer.sin_addr, serverIpAddr->h_addr, serverIpAddr->h_length);
	} else if (INADDR_NONE == (addrServer.sin_addr.s_addr = inet_addr(serverIp.c_str()))) { // Compruebo que la direccion ip (en formato 0.0.0.0) sea valida
		error("Direccion del servidor incorrecta (" + serverIp + ":" + serverPort + ").");
		return false;
	}

	if ((sockProgress = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		error("No se pudo conectar con el servidor. Motivo: " + string(strerror(errno)));
		return false;
	}

	if (connect(sockProgress, (sockaddr *)&addrServer, sizeof(addrServer)) < 0) {
		error("No se pudo conectar con el servidor. Motivo: " + string(strerror(errno)));
		return false;
	}

	return true;
}

int HFTClient::run() {
    UDTUpDown _udt_;    // Automatically start up and clean up UDT module
	if (!connectToServer() || !connectToProgressServer()) return -1;
	_udt_.clean = true;

    // Gestiono la operacion que el cliente quiera realizar siguiendo el protocolo establecido
    if (!send(aSock, &modeIsUpload, sizeof(bool), " el modo de operacion")) return -1;
    if (!send(aSock, &mssTestBufSize, sizeof(int32_t), " el tamaño del test de MSS")) return -1;
    if (modeIsUpload) {
        if (!send(aSock, uuid, SIZE_UUID, " el UUID del cliente")) return -1;
        if (!sendString(aSock, clientFileName, " el nombre del archivo en el cliente")) return -1;
        if (!send(aSock, &fileSize, sizeof(int64_t), " el tamaño del archivo en el cliente")) return -1;
        if (!sendString(aSock, fileLastModifyDate, " la fecha de modificacion del archivo en el cliente")) return -1;
        if (!sendString(aSock, serverFileName, " el nombre del archivo en el servidor")) return -1;

        if (!receive(aSock, &isServerFileNameValid, sizeof(bool), " la validez del nombre de archivo en el servidor")) return -1;
        if (!isServerFileNameValid) {
            error("El nombre de archivo en el servidor que ha especificado ya existe. Por favor, pruebe con otro nombre.");
            return -1;
        }
        if (!receive(aSock, &fileOffset, sizeof(int64_t), " el progreso de archivo ya subido")) return -1;
		if (!findOptimumParams()) {	// En caso de que el cliente lo hubiese solicitado, ejecuto un test de velocidad
			errorWithUDTmsg("No se pudo completar el test de MSS!");
		}
        info("Se va a proceder a subir el archivo '" + clientFileName + "' -> '" + serverFileName + "', que lleva ya " + boost::lexical_cast<string>(fileOffset) + " bytes subidos (quedan " + boost::lexical_cast<string>(fileSize - fileOffset) + " bytes).");

		fileStream.open(clientFileName.c_str(), ios::in | ios::binary);

		mon = new Monitor(this);	// Monitoreo y comienzo la subida del archivo
#ifndef WIN32
		pthread_t bwThread, monitorThread;
		pthread_create(&bwThread, NULL, adjustBw, this);
		pthread_detach(bwThread);
		pthread_create(&monitorThread, NULL, monitor, this);
		pthread_detach(monitorThread);
#else
		CreateThread(NULL, 0, adjustBw, this, 0, NULL);
		CreateThread(NULL, 0, monitor, this, 0, NULL);
#endif
        
        if (UDT::ERROR == UDT::sendfile(aSock, fileStream, fileOffset, fileSize - fileOffset, 1408)) {
			mon->stop();
			fileStream.close();
            errorWithUDTmsg("No se pudo completar la subida del archivo '" + clientFileName + "'");
            return -1;
        }
        
        /*const int LEN=1400;
        char buf[LEN];
        int64_t alreadySent, sentLen, bufRead;
        fileStream.seekg(fileOffset);
        while (!fileStream.eof()) {
            fileStream.read(buf, LEN);
            alreadySent = 0;
            bufRead = fileStream.gcount();
            while (alreadySent < bufRead) {
                sentLen = UDT::send(aSock, buf+alreadySent, bufRead-alreadySent, 0);
                if (sentLen == UDT::ERROR) {
                    fileStream.close();
                    mon->stop();
                    errorWithUDTmsg("No se pudo completar la subida del archivo '" + clientFileName + "'");
                    return -1;
                }
                alreadySent += sentLen;
            }
        }*/

		// El envio suele ir unos 12MB por delante que la recepcion -> Esperar a recibir un ACK del servidor previene que cierre el socket antes de que...
		receive(aSock, &ackTransfer, sizeof(bool), " la confirmacion de recepcion del archivo en el servidor"); // ... terminen de llegarle esos 12 MB
		info("¡Enhorabuena! El archivo '" + serverFileName + "' se ha subido correctamente.");
        fileStream.close();
		mon->stop();
		while (mon->isRunning()) pause(1);
    }

    UDT::close(aSock); // Cierro el socket
    return 0;
}
	
#ifndef WIN32
int main(int argc, char* argv[]) {
	HFTClient* client = new HFTClient(argc, argv);
	int ret = client->run();
	delete client;

	return ret;
}
#endif
