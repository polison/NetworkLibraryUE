// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetworkLibrary.h"
#include <Modules/ModuleManager.h>
#include <SocketSubsystem.h>
#include <Networking.h>

IMPLEMENT_MODULE(FDefaultModuleImpl, NetworkLibrary)

UNetworkLibrary::UNetworkLibrary()
{
    SocketSystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);

    SocketReader = new FSocketReader();
    SocketReader->OnData.AddUObject(this, &UNetworkLibrary::OnMsgData);
    SocketReader->OnClosed.AddUObject(this, &UNetworkLibrary::OnClosed);

    RegisterHandler(UINT_MAX, [ this ](FSocketArchive& Ar) { OnKillFromServer(Ar); });
}

UNetworkLibrary::~UNetworkLibrary()
{
    if (NetworkThread != nullptr)
    {
        delete NetworkThread;
        NetworkThread = nullptr;
    }

    if (SocketReader != nullptr)
    {
        delete SocketReader;
        SocketReader = nullptr;
    }
}

void UNetworkLibrary::Deinitialize()
{
    if (SocketReader->IsConnected)
    {
        SocketReader->Stop();
    }
}

TStatId UNetworkLibrary::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UNetworkLibrary, STATGROUP_Tickables);
}

void UNetworkLibrary::Tick(float DeltaTime)
{
    FMessage Msg;
    while (Messages.Dequeue(Msg))
    {
        FSocketArchive Ar;
        Ar.Write(Msg.Data);
        (*Msg.Handler)(Ar);
    }
}

UWorld* UNetworkLibrary::GetTickableGameObjectWorld() const
{
    return GetWorld();
}

void UNetworkLibrary::OnMsgData(uint32 opcode, TArray<uint8>& data)
{
    auto Handler = DispatchTable.Find(opcode);
    if (Handler != nullptr)
    {
        FMessage Message = {
                opcode,
                Handler,
                data	 // The message data
        };
        Messages.Enqueue(Message);
    }
    else
    {
        GLog->Logf(TEXT("No handler registered for message with id [%d]"), opcode);
    }
}

void UNetworkLibrary::OnClosed()
{
    if (Socket != nullptr)
    {
        TArray<uint8> data;
        OnMsgData(-1, data);

        SocketSystem->DestroySocket(Socket);
        Socket = nullptr;
    }
}

bool UNetworkLibrary::ConnectToServer(FString& errorMsg, const FString& ServerIP, int32 ServerPort, int32 BufferSize)
{
    errorMsg = FString();
    if (SocketReader->IsConnected)
        return true;

    OnClosed();
    Socket = SocketSystem->CreateSocket(NAME_Stream, TEXT("SOCKET"), FNetworkProtocolTypes::IPv4);
    if (!Socket)
    {
        errorMsg = TEXT("Create Socket Error!");
        GLog->Log(errorMsg);
        return false;
    }

    Socket->SetNoDelay();
    Socket->SetNonBlocking();
    Socket->SetSendBufferSize(BufferSize, BufferSize);
    Socket->SetReceiveBufferSize(BufferSize, BufferSize);

    TSharedPtr<FInternetAddr> serverAddr = SocketSystem->CreateInternetAddr(FNetworkProtocolTypes::IPv4);
#if ENGINE_MAJOR_VERSION >= 5 || (ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION >= 23)
    FAddressInfoResult result = SocketSystem->GetAddressInfo(*ServerIP, nullptr, EAddressInfoFlags::Default, FNetworkProtocolTypes::IPv4);
    if (result.ReturnCode != ESocketErrors::SE_NO_ERROR)
    {
        errorMsg = FString::Printf(TEXT("Get server ip with error [%d]"), result.ReturnCode);
        GLog->Log(errorMsg);
        return false;
    }
    serverAddr = result.Results[0].Address;
#else
    SocketSystem->GetHostByName(TCHAR_TO_ANSI(*ServerIP), *serverAddr);
    if (!serverAddr.IsValid())
    {
        errorMsg = TEXT("Get server ip with error ");
        GLog->Log(errorMsg);
        return false;
    }
#endif
    serverAddr->SetPort(ServerPort);
    SocketReader->Initialzie(Socket, serverAddr, SocketSystem);

    if (NetworkThread != nullptr)
    {
        delete NetworkThread;
        NetworkThread = nullptr;
    }

    NetworkThread = FRunnableThread::Create(SocketReader, TEXT("Socket Network Thread"));

    return true;
}

bool UNetworkLibrary::IsConnected()
{
    return SocketReader->IsConnected;
}

void UNetworkLibrary::SendPacket(FSocketPacket* Packet)
{
    if (SocketReader->IsConnected)
    {
        FSocketArchive Ar;
        Packet->Write(Ar);
        int32 iSendBytes = 0;
        Socket->Send(Ar.GetData(), Ar.GetLength(), iSendBytes);
    }
}

void UNetworkLibrary::RegisterHandler(const uint32& Opcode, const HandlerFn& Handler)
{
    DispatchTable.Add(Opcode, Handler);
}

void UNetworkLibrary::OnKillFromServer(FSocketArchive& Ar)
{
    OnKilled.Broadcast();
}