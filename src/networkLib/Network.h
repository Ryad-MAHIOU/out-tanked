#pragma once

#pragma region internal_dependencies
#include "Packets.h"
#include "Socket.h"
#pragma endregion
#pragma region external_dependencies
#include <string>
#include <mutex>
#include <queue>
#include <vector>
#include <atomic>
#include <map>
#include <functional>
#include <set>
#pragma endregion

struct PendingServerPacket {
    std::vector<char> data;
    int sequenceID;
    float timeSinceLastSend;
    int retryCount;
    Socket destination;
};

struct ReceivedPacket {
    std::vector<char> data;
    sockaddr_in sender;
};

using PacketHandler = std::function<void(const ReceivedPacket&)>;

class Network
{
public:
    Network(const Network&) = delete;
    Network& operator=(const Network&) = delete;
    static Network& GetInstance();

    bool Start(int port);
    void Stop();

    void PrepareHeader(PacketHeader& outHeader, PacketType type, int totalSize, bool _isReliable = false);

    void SendTo(const sockaddr_in& target, const void* data, int size);

    void Broadcast(const void* data, int size, const sockaddr_in* ignoreClient = nullptr);

    void RegisterHandler(PacketType type, PacketHandler handler);
    void UnregisterHandler(PacketType type);

    void Update(float dt);

    static std::string GetClientKey(const sockaddr_in& addr);

    void AddKnownClient(const sockaddr_in& addr);
    void RemoveKnownClient(const sockaddr_in& addr);

private:
    Network();
    ~Network();

    static Network* s_pServer;

    SOCKET m_socket;
    PacketHeader m_defaultHeader;

    HANDLE m_hThread;
    DWORD m_dwThreadId;
    std::atomic<bool> m_isRunning;

    std::queue<ReceivedPacket> m_packetQueue;
    std::mutex m_queueMutex;

    std::vector<PendingServerPacket> m_pendingPackets;
    std::mutex m_reliableMutex;

    std::map<int, PacketHandler> m_handlers;

    std::map<std::string, sockaddr_in> m_knownClients;
    std::mutex m_clientsMutex;

    bool PopPacket(ReceivedPacket& outPacket);
    void UpdateReliability(float dt);
    void HandleConfirmPacket(const ReceivedPacket& packet);

    static DWORD WINAPI StaticReceiveThread(LPVOID lpParam);
    void ReceiveLoop();
};