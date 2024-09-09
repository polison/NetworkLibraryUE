// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include <CoreMinimal.h>

class NETWORKLIBRARY_API FSocketReader : public FRunnable
{
public:
    DECLARE_MULTICAST_DELEGATE_TwoParams(FOnRecvData, uint32, TArray<uint8>&);//, PacketId , PacketData);
    DECLARE_MULTICAST_DELEGATE(FOnClosed);//

public:

    virtual uint32 Run() override;

    virtual void Stop() override;

private:

    const uint32 maxLength = 0x4000;

    const float normalDelay = 0.1f;

    FSocket* socket = nullptr;

    ISocketSubsystem* socketSystem = nullptr;

    TSharedPtr<FInternetAddr> serverAddr = nullptr;

public:
    void Initialzie(FSocket* InSocket, TSharedPtr<FInternetAddr>& InAddr, ISocketSubsystem* InSystem);

public:

    bool IsConnected = false;

    FOnRecvData OnData;

    FOnClosed OnClosed;
};