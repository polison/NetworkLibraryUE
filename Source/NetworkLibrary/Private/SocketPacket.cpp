#include "SocketPacket.h"

FSocketPacket::FSocketPacket()
{
}

uint32 FSocketPacket::GetOpcode(FSocketArchive& Ar)
{
    return Ar.Read<uint32>();
}

uint32 FSocketPacket::GetLength(FSocketArchive& Ar)
{
    return Ar.Read<uint32>();
}

void FSocketPacket::Read(FSocketArchive& Ar)
{
}

void FSocketPacket::Write(FSocketArchive& Ar)
{
    Ar.Write(PacketOpcode);
    Ar.Write(PacketBuffer);
}