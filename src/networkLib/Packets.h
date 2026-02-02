#pragma once

#pragma pack(push, 1)

enum PacketType {
    
    // -- GENERAL --
    CONFIRM,
};

struct PacketHeader {
    char expediteur[16];
    char destinataire[16];
    int magicNumber;
    bool isReliable;
    int TypePacket;
    int bodySize;
    int time;
};

struct PacketConfirm {
    PacketHeader header;
    int timeSent;
};

#pragma pack(pop)