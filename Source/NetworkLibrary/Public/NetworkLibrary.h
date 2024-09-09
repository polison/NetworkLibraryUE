// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include <Tickable.h>
#include <Subsystems/GameInstanceSubsystem.h>

#include "SocketReader.h"
#include "SocketPacket.h"

#include "NetworkLibrary.generated.h"

USTRUCT(BlueprintType)
struct FInfoBase
{
    GENERATED_BODY()

    virtual void Read(FSocketArchive& Ar) {}

    virtual void Write(FSocketArchive& Ar) {}

    virtual ~FInfoBase() = default;
};

/*
 * 请基于此类创建子类实现自己的方法,尽量不要修改本类.
 */
UCLASS()
class NETWORKLIBRARY_API UNetworkLibrary : public UGameInstanceSubsystem, public FTickableGameObject
{
    GENERATED_BODY()

public:
    UNetworkLibrary();

    ~UNetworkLibrary();

    virtual void Deinitialize() override;

    virtual TStatId GetStatId() const override;

    virtual void Tick(float DeltaTime) override;

    virtual UWorld* GetTickableGameObjectWorld() const override;

private:
    void OnMsgData(uint32 opcode, TArray<uint8>& data);

    void OnClosed();

public:
    UFUNCTION(BlueprintCallable, Category = "NetworkLibrary")
    bool ConnectToServer(const FString& ServerIP, int32 ServerPort, int32 BufferSize = 0x4000);

    UFUNCTION(BlueprintCallable, Category = "NetworkLibrary")
    bool IsConnected();

private:
    ISocketSubsystem* SocketSystem = nullptr;

    FSocket* Socket = nullptr;

    FSocketReader* SocketReader = nullptr;

    FRunnableThread* NetworkThread = nullptr;

private:
    using HandlerFn = TFunction<void(FSocketArchive& Ar)>;
    TMap<uint32, HandlerFn> DispatchTable;

    struct FMessage
    {
        uint32 Opcode;
        HandlerFn* Handler;
        TArray<uint8> Data;
    };
    TQueue<FMessage> Messages;

public:
    void SendPacket(FSocketPacket* Packet);

    void RegisterHandler(const uint32& Opcode, const HandlerFn& Handler);

public:
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnKilled);

    UPROPERTY(EditAnywhere, BlueprintAssignable, Category = "NetworkLibrary")
    FOnKilled OnKilled;

private:
    void OnKillFromServer(FSocketArchive& Ar);
};