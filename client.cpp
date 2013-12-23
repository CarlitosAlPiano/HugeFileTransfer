#include "common.h"
#include <boost/uuid/uuid_generators.hpp>

#define DEF_UUID_FILENAME   (char*)"user.uuid"

using namespace std;

bool modeIsUpload, isServerFileNameValid;
char *serverIp, *serverPort;
string clientFileName, serverFileName, fileLastModifyDate;
char uuid[SIZE_UUID];
int64_t fileSize, fileOffset;

void getUUID(char* uuidFileName) {
    boost::uuids::uuid u = boost::uuids::nil_generator()();
    ifstream ifsUuid(uuidFileName, ios::in|ios::binary);
    if (ifsUuid.is_open()) {
        ifsUuid >> u;
        ifsUuid.close();
        cout << "Leído el siguiente UUID: " << u << endl;
    } else {
        cout << "No se encontró un UUID para este usuario. Se procederá a generar uno aleatoriamente." << endl;
    }
    
    if (u.is_nil()) {
        u = boost::uuids::random_generator()();
        ofstream ofsUuid(uuidFileName, ios::out|ios::binary|ios::trunc);
        if (ofsUuid.is_open()) {
            ofsUuid << u;
            ofsUuid.close();
            cout << "Creado el UUID: " << u << endl;
        } else {
            error("No se pudo salvar el UUID creado para este usuario en el archivo '" + string(uuidFileName) + "'.\n\t¿Está el archivo abierto o no tiene derechos para escribir en este directorio?");
        }
    }
    memcpy(uuid, &u, SIZE_UUID);
}

void getArgs(int argc, char* argv[]) {
    if (!((argc==6) || (argc==7))) {
        error("Uso: client subir/descargar(1/0) IPservidor PuertoServidor ArchivoLocal ArchivoRemoto.");
    }
    modeIsUpload = (strcmp(argv[1], "1") == 0);
    serverIp = argv[2];
    if (atoi(argv[3]) <= 0) {
        error("El puerto del servidor debe ser un número mayor que 0.");
    }
    serverPort = argv[3];
    clientFileName = string(argv[4]);
    ifstream ifs(clientFileName, ios::in|ios::binary);
    if (ifs.is_open()) {
        ifs.seekg(0, ios::end);
        fileSize = ifs.tellg();
        ifs.close();
#ifndef WIN32
        struct stat attrib;
        if (stat(clientFileName.c_str(), &attrib) != 0) {
            error("La fecha de modificación del archivo a enviar no ha podido ser leída.");
        }
        fileLastModifyDate = string(asctime(gmtime(&(attrib.st_mtime))));
#else
        FILETIME lastWriteTime;
        HANDLE fh = CreateFile(clientFileName.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (fh == INVALID_HANDLE_VALUE) {
            error("Imposible abrir el archivo local para obtener su fecha de modificación.");
        }
        if (!GetFileTime(fh, NULL, NULL, &lastWriteTime)) {
            error("La fecha de modificación del archivo a enviar no ha podido ser leída.");
        }
        CloseHandle(fh);
        fileLastModifyDate = WinFileTimeToString(lastWriteTime);
#endif
        fileLastModifyDate.pop_back();  // Get rid of trailing '\n'
    } else {
        error("El archivo local no existe o no se puede leer.");
    }
    serverFileName = string(argv[5]);
    if (argc == 6) {
        getUUID(DEF_UUID_FILENAME);
    } else {
        getUUID(argv[6]);
    }
    
    info("El archivo '" + clientFileName + "' ocupa " + boost::lexical_cast<string>(fileSize) + "B y fue modificado por última vez el " + fileLastModifyDate + ".");
}

void connectToServer(UDTSOCKET& sock) {
    addrinfo addrAux, *addrServer;
    
    memset(&addrAux, 0, sizeof(struct addrinfo));
    addrAux.ai_flags = AI_PASSIVE;
    addrAux.ai_family = AF_INET;
    addrAux.ai_socktype = SOCK_STREAM;
    
    sock = UDT::socket(addrAux.ai_family, addrAux.ai_socktype, addrAux.ai_protocol);
    
    if (0 != getaddrinfo(serverIp, serverPort, &addrAux, &addrServer)) {
        error("Dirección del servidor incorrecta (" + string(serverIp) + ":" + string(serverPort) + ").");
    }
    
    if (UDT::ERROR == UDT::connect(sock, addrServer->ai_addr, addrServer->ai_addrlen)) {
        errorWithUDTmsg("No se pudo conectar con el servidor");
    }
    
    freeaddrinfo(addrServer);
}

int main(int argc, char* argv[]) {
    getArgs(argc, argv);
    UDTUpDown _udt_;    // Automatically start up and clean up UDT module
    UDTSOCKET sock;
    connectToServer(sock);
    
#ifndef WIN32
    pthread_t monitorThread;
    pthread_create(&monitorThread, NULL, monitorTx, new UDTSOCKET(sock));
    pthread_detach(monitorThread);
#else
    CreateThread(NULL, 0, monitorTx, new UDTSOCKET(sock), 0, NULL);
#endif
    if(!send(sock, &modeIsUpload, sizeof(bool), " el modo de operación")) return -1;
    if (modeIsUpload) {
        if(!send(sock, uuid, SIZE_UUID, " el UUID del cliente")) return -1;
        if(!sendString(sock, clientFileName, " el nombre del archivo en el cliente")) return -1;
        if(!send(sock, &fileSize, sizeof(int64_t), " el tamaño del archivo en el cliente")) return -1;
        if(!sendString(sock, fileLastModifyDate, " la fecha de modificación del archivo en el cliente")) return -1;
        if(!sendString(sock, serverFileName, " el nombre del archivo en el servidor")) return -1;
        
        if(!receive(sock, &isServerFileNameValid, sizeof(bool), " la validez del nombre de archivo en el servidor")) return -1;
        if (!isServerFileNameValid) {
            error("El nombre de archivo en el servidor que ha especificado ya existe. Por favor, pruebe con otro nombre.");
        }
        if(!receive(sock, &fileOffset, sizeof(int64_t), " el progreso de archivo ya subido")) return -1;
        info("Se va a proceder a subir el archivo '" + clientFileName + "' -> '" + serverFileName + "', que lleva ya " + boost::lexical_cast<string>(fileOffset) + " bytes subidos (quedan " + boost::lexical_cast<string>(fileSize-fileOffset) + " bytes).");

        fstream fileStream(clientFileName, ios::in|ios::binary);
        if (UDT::ERROR == UDT::sendfile(sock, fileStream, fileOffset, fileSize-fileOffset)) {
            errorWithUDTmsg("No se pudo completar la subida del archivo '"+ clientFileName + "'");
        }
        fileStream.close();
        cout << endl << "¡Enhorabuena! El archivo '" << serverFileName << "' se ha subido correctamente." << endl;;
    }
    
    UDT::close(sock);
    return 0;
}
