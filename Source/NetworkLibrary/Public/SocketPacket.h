#pragma once

#include <CoreMinimal.h>

#include "SocketArchive.h"


class NETWORKLIBRARY_API FSocketPacket
{
protected:
    uint32 PacketOpcode;

    FSocketArchive PacketBuffer;

public:

    FSocketPacket();

    virtual ~FSocketPacket() = default;

    uint32 GetOpcode(FSocketArchive& Ar);

    uint32 GetLength(FSocketArchive& Ar);

    virtual void Read(FSocketArchive& Ar);

    virtual void Write(FSocketArchive& Ar);
};

