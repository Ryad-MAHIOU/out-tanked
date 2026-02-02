#include "pch.h"
#include "Network.h"

#include <ws2tcpip.h>
#include <algorithm>
#include <sstream>

Network* Network::s_pServer = nullptr;

// --------------------------------------------------------
// CONSTRUCTEUR / SINGLETON
// --------------------------------------------------------

Network::Network()
    : m_socket(INVALID_SOCKET)
    , m_isRunning(false)
    , m_hThread(NULL)
    , m_dwThreadId(0)
{
    memset(&m_defaultHeader, 0, sizeof(PacketHeader));
}

Network::~Network()
{
    Stop();
}

Network& Network::GetInstance()
{
    if (s_pServer == nullptr)
    {
        s_pServer = new Network();
    }
    return *s_pServer;
}

// --------------------------------------------------------
// START / STOP
// --------------------------------------------------------

bool Network::Start(int port)
{
    if (m_isRunning) return true;

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "[Server] Erreur WSAStartup" << std::endl;
        return false;
    }

    m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (m_socket == INVALID_SOCKET)
    {
        std::cerr << "[Server] Erreur Socket" << std::endl;
        WSACleanup();
        return false;
    }

#define SIO_UDP_CONNRESET _WSAIOW(IOC_VENDOR, 12)
    BOOL bNewBehavior = FALSE;
    DWORD dwBytesReturned = 0;
    WSAIoctl(m_socket, SIO_UDP_CONNRESET, &bNewBehavior, sizeof(bNewBehavior), NULL, 0, &dwBytesReturned, NULL, NULL);

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(port);

    if (bind(m_socket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        std::cerr << "[Server] Erreur BIND sur le port " << port << std::endl;
        closesocket(m_socket);
        WSACleanup();
        return false;
    }

    std::cout << "[Server] Serveur demarre sur le port " << port << std::endl;

    memset(&m_defaultHeader, 0, sizeof(PacketHeader));
    strncpy_s(m_defaultHeader.expediteur, "SERVER", _TRUNCATE);
    m_defaultHeader.magicNumber = 122436;
    m_defaultHeader.isReliable = false;
    m_defaultHeader.time = 0;

    m_isRunning = true;

    m_hThread = CreateThread(NULL, 0, &Network::StaticReceiveThread, this, 0, &m_dwThreadId);
    if (m_hThread == NULL)
    {
        std::cerr << "[Server] Erreur thread reception" << std::endl;
        return false;
    }

    return true;
}

void Network::Stop()
{
    if (!m_isRunning) return;

    m_isRunning = false;

    if (m_socket != INVALID_SOCKET)
    {
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
    }

    if (m_hThread != NULL)
    {
        WaitForSingleObject(m_hThread, INFINITE);
        CloseHandle(m_hThread);
        m_hThread = NULL;
    }

    WSACleanup();

    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        std::queue<ReceivedPacket> empty;
        std::swap(m_packetQueue, empty);
    }
    {
        std::lock_guard<std::mutex> lock(m_reliableMutex);
        m_pendingPackets.clear();
    }

    std::cout << "[Server] Serveur arrete." << std::endl;
}

// --------------------------------------------------------
// ENVOI & BROADCAST
// --------------------------------------------------------

void Network::PrepareHeader(PacketHeader& outHeader, PacketType type, int totalSize, bool _isReliable)
{
    outHeader = m_defaultHeader;
    outHeader.TypePacket = (int)type;
    outHeader.bodySize = totalSize - sizeof(PacketHeader);
    outHeader.isReliable = _isReliable;
    outHeader.time = m_defaultHeader.time;
    m_defaultHeader.time++;
}

void Network::SendTo(const sockaddr_in& target, const void* data, int size)
{
    if (!m_isRunning || m_socket == INVALID_SOCKET) return;

    PacketHeader* header = (PacketHeader*)data;

    // Gestion Fiabilité
    if (header->isReliable)
    {
        std::lock_guard<std::mutex> lock(m_reliableMutex);
        PendingServerPacket pp;
        pp.data.assign((const char*)data, (const char*)data + size);
        pp.sequenceID = header->time;
        pp.timeSinceLastSend = 0.0f;
        pp.retryCount = 0;
        pp.destination = target;

        m_pendingPackets.push_back(pp);
    }

    sendto(m_socket, (const char*)data, size, 0, (sockaddr*)&target, sizeof(target));
}

void Network::Broadcast(const void* data, int size, const sockaddr_in* ignoreClient)
{
    std::lock_guard<std::mutex> lock(m_clientsMutex);

    std::string ignoreKey = "";
    if (ignoreClient) ignoreKey = GetClientKey(*ignoreClient);

    for (auto const& [key, addr] : m_knownClients)
    {
        if (ignoreKey != "" && key == ignoreKey) continue;

        SendTo(addr, data, size);
    }
}

// --------------------------------------------------------
// UPDATE & DISPATCH
// --------------------------------------------------------

void Network::RegisterHandler(PacketType type, PacketHandler handler)
{
    m_handlers[(int)type] = handler;
}

void Network::UnregisterHandler(PacketType type)
{
    auto it = m_handlers.find((int)type);
    if (it != m_handlers.end()) m_handlers.erase(it);
}

void Network::Update(float dt)
{
    if (!m_isRunning) return;

    UpdateReliability(dt);

    ReceivedPacket packet;
    while (PopPacket(packet))
    {
        if (packet.data.size() < sizeof(PacketHeader)) continue;

        PacketHeader* header = (PacketHeader*)packet.data.data();

        if (header->isReliable)
        {

            PacketConfirm confirmPck;
            PrepareHeader(confirmPck.header, PacketType::CONFIRM, sizeof(PacketConfirm), false);

            confirmPck.timeSent = header->time;

            SendTo(packet.sender, &confirmPck, sizeof(PacketConfirm));

            std::cout << "[Server] ACK sent for packet ID " << header->time << std::endl;
        }
        auto it = m_handlers.find(header->TypePacket);
        if (it != m_handlers.end())
        {
            it->second(packet);
        }
    }
}

bool Network::PopPacket(ReceivedPacket& outPacket)
{
    std::lock_guard<std::mutex> lock(m_queueMutex);
    if (m_packetQueue.empty()) return false;
    outPacket = m_packetQueue.front();
    m_packetQueue.pop();
    return true;
}

// --------------------------------------------------------
// FIABILITÉ
// --------------------------------------------------------

void Network::UpdateReliability(float dt)
{
    std::lock_guard<std::mutex> lock(m_reliableMutex);
    const float RESEND_DELAY = 1.0f;

    for (auto& packet : m_pendingPackets)
    {
        packet.timeSinceLastSend += dt;
        if (packet.timeSinceLastSend >= RESEND_DELAY)
        {
            sendto(m_socket, packet.data.data(), (int)packet.data.size(), 0,
                (sockaddr*)&packet.destination, sizeof(packet.destination));

            packet.timeSinceLastSend = 0.0f;
            packet.retryCount++;
        }
    }
}

void Network::HandleConfirmPacket(const ReceivedPacket& packet)
{
    if (packet.data.size() < sizeof(PacketConfirm)) return;
    PacketConfirm* confirm = (PacketConfirm*)packet.data.data();
    int seqID = confirm->timeSent;

    std::string senderKey = GetClientKey(packet.sender);

    std::lock_guard<std::mutex> lock(m_reliableMutex);

    auto it = std::remove_if(m_pendingPackets.begin(), m_pendingPackets.end(),
        [&](const PendingServerPacket& p) {
            return p.sequenceID == seqID && GetClientKey(p.destination) == senderKey;
        });

    if (it != m_pendingPackets.end())
    {
        m_pendingPackets.erase(it, m_pendingPackets.end());
    }
}

// --------------------------------------------------------
// RÉCEPTION
// --------------------------------------------------------

DWORD WINAPI Network::StaticReceiveThread(LPVOID lpParam)
{
    Network* pServer = (Network*)lpParam;
    if (pServer) pServer->ReceiveLoop();
    return 0;
}

void Network::ReceiveLoop()
{
    char buffer[4096];
    sockaddr_in senderAddr;
    int senderSize = sizeof(senderAddr);

    while (m_isRunning)
    {
        senderSize = sizeof(senderAddr);

        int bytesReceived = recvfrom(m_socket, buffer, sizeof(buffer), 0, (sockaddr*)&senderAddr, &senderSize);

        if (bytesReceived > 0)
        {
            if (bytesReceived < sizeof(PacketHeader)) continue;

            PacketHeader* header = (PacketHeader*)buffer;

            if (header->magicNumber != 122436) continue;

            if (header->TypePacket == PacketType::CONFIRM)
            {
                ReceivedPacket packet;
                packet.data.assign(buffer, buffer + bytesReceived);
                packet.sender = senderAddr;

                HandleConfirmPacket(packet);
                continue;
            }

            {
                std::lock_guard<std::mutex> lock(m_queueMutex);
                ReceivedPacket packet;
                packet.data.assign(buffer, buffer + bytesReceived);
                packet.sender = senderAddr;
                m_packetQueue.push(packet);
            }
        }
        else if (bytesReceived == SOCKET_ERROR)
        {
            int error = WSAGetLastError();

            if (error == WSAECONNRESET)
            {
                continue;
            }

            if (!m_isRunning || error == WSAEINTR)
            {
                break;
            }

            std::cerr << "[Server] Recv Error: " << error << std::endl;
        }
    }
}

// --------------------------------------------------------
// GESTION CLIENTS & UTILS
// --------------------------------------------------------

std::string Network::GetClientKey(const sockaddr_in& addr)
{
    char ipStr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr.sin_addr, ipStr, INET_ADDRSTRLEN);
    return std::string(ipStr) + ":" + std::to_string(ntohs(addr.sin_port));
}

void Network::AddKnownClient(const sockaddr_in& addr)
{
    std::lock_guard<std::mutex> lock(m_clientsMutex);
    m_knownClients[GetClientKey(addr)] = addr;
}

void Network::RemoveKnownClient(const sockaddr_in& addr)
{
    std::lock_guard<std::mutex> lock(m_clientsMutex);
    m_knownClients.erase(GetClientKey(addr));
}