#include "SocketReader.h"
#include <Networking.h>

#include "SocketPacket.h"

void FSocketReader::Initialzie(FSocket* InSocket, TSharedPtr<FInternetAddr>& InAddr, ISocketSubsystem* InSystem)
{
    socket = InSocket;
    serverAddr = InAddr;
    socketSystem = InSystem;
}

uint32 FSocketReader::Run()
{
    IsConnected = socket->Connect(*serverAddr);
    TArray<uint8> RecvTemp;
    RecvTemp.SetNumUninitialized(1);
    int32 iReadBytes = 0;
    uint32 iPendingBytes = 0;
    ESocketErrors errorNo;
    FSocketArchive RecvArchive;
    float dStart, dUsed;
    float dDelay = normalDelay;
    while (IsConnected)
    {
        FPlatformProcess::Sleep(dDelay);
        dStart = FPlatformTime::Seconds();
        if (socket->Recv(RecvTemp.GetData(), 1, iReadBytes, ESocketReceiveFlags::Peek))
        {
            if (socket->HasPendingData(iPendingBytes))
            {
                RecvTemp.SetNumUninitialized(iPendingBytes);
                socket->Recv(RecvTemp.GetData(), iPendingBytes, iReadBytes);
                RecvArchive.Write(RecvTemp, iReadBytes);
                while (RecvArchive.GetLength() > sizeof(uint32))
                {
                    FSocketPacket packet;
                    uint32 cmd = packet.GetOpcode(RecvArchive);
                    uint32 length = packet.GetLength(RecvArchive);
                    if (maxLength < length)
                    {
                        GLog->Logf(TEXT("get a larger packet[{%d}] with length[{%d}], closed."), cmd, length);
                        Stop();
                        break;
                    }

                    uint32 dataLength = RecvArchive.GetLength();
                    if (length > dataLength)
                    {
                        RecvArchive.ResetReader();
                        break;
                    }
                    TArray<uint8> data = RecvArchive.Read(length);
                    OnData.Broadcast(cmd, data);
                    RecvArchive.ClearRead();
                }
            }
        }
        else
        {
            errorNo = socketSystem->GetLastErrorCode();
            if (errorNo != SE_EWOULDBLOCK)
            {
                GLog->Logf(TEXT("Read Failed, %d Closed!"), errorNo);
                IsConnected = false;
            }
        }

        dUsed = FPlatformTime::Seconds() - dStart;
        dDelay = (int)((dUsed <= normalDelay) ? (normalDelay - dUsed) : 0);
    }

    OnClosed.Broadcast();
    return 0;
}

void FSocketReader::Stop()
{
    IsConnected = false;
}