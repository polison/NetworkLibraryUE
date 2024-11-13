// Fill out your copyright notice in the Description page of Project Settings.


#include "SocketArchive.h"

FSocketArchive::FSocketArchive()
{
    Ar = new FBufferArchive();
    Ar->SetByteSwapping(true);
    Reader = new FMemoryReader(*Ar);
    Reader->SetByteSwapping(true);
}

FSocketArchive::FSocketArchive(const FSocketArchive& other)
    : FSocketArchive()
{
    int32 Length = other.Ar->TotalSize();
    Ar->Serialize(other.Ar->GetData(), Length);
}

FSocketArchive::~FSocketArchive()
{
    if (Reader != nullptr)
    {
        Reader->Close();
        delete Reader;
        Reader = nullptr;
    }

    if (Ar != nullptr)
    {
        delete Ar;
        Ar = nullptr;
    }
}

FString FSocketArchive::Read()
{
    int32 length;
    *Reader << length;
    uint8* d = new uint8[length + 1];
    d[length] = 0;
    Reader->Serialize(d, length);
    return UTF8_TO_TCHAR((TCHAR*)d);
}

TArray<uint8> FSocketArchive::Read(int32 Length)
{
    TArray<uint8> value;
    value.SetNumUninitialized(Length);
    Reader->Serialize(value.GetData(), Length);
    return value;
}

void FSocketArchive::Write(FString& Value)
{
    FTCHARToUTF8 UTF8String(*Value);
    int32 length = UTF8String.Length();
    *Ar << length;
    Ar->Serialize((void*)UTF8String.Get(), length);
}

void FSocketArchive::Write(FSocketArchive& Value)
{
    int32 Length = Value.GetLength();
    Write(Length);
    Ar->Serialize(Value.Ar->GetData(), Length);
}

void FSocketArchive::Write(TArray<uint8>& Data, int32 Length)
{
    if (Length == 0)
        Length = Data.Num();

    Ar->Serialize(Data.GetData(), Length);
}

int32 FSocketArchive::GetLength()
{
    return Ar->TotalSize();
}

uint8* FSocketArchive::GetData()
{
    return Ar->GetData();
}

void FSocketArchive::ResetReader()
{
    Reader->Seek(0);
}

void FSocketArchive::ClearRead(int32 Length)
{
    int32 rpos = Reader->Tell();
    int32 wpos = Ar->Tell();
    if (Length == 0)
        Length = rpos;

    Ar->RemoveAt(0, Length);
    int32 w = Length > wpos ? 0 : wpos - Length;
    Ar->Seek(w);
    int32 r = Length > rpos ? 0 : rpos - Length;
    Reader->Seek(r);
}
