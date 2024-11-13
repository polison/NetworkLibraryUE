// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <CoreMinimal.h>
#include <Serialization/BufferArchive.h>

/**
 *
 */
class NETWORKLIBRARY_API FSocketArchive
{
public:
    FSocketArchive();

    FSocketArchive(const FSocketArchive& other);

    ~FSocketArchive();

private:
    FBufferArchive* Ar = nullptr;
    FMemoryReader* Reader = nullptr;

public:
    /// <summary>
    /// 按类型读取:主要使用数字类型
    /// </summary>
    template<typename T>
    T Read()
    {
        T value;
        *Reader << value;
        return value;
    }

    FString Read();

    TArray<uint8> Read(int32 Length);

    /// <summary>
   /// 按类型写入:主要使用数字类型
   /// </summary>
    template<typename T>
    void Write(T& Value)
    {
        *Ar << Value;
    }

    void Write(FString& Value);

    void Write(FSocketArchive& Value);

    void Write(TArray<uint8>& Data, int32 Length = 0);

    int32 GetLength();

    uint8* GetData();

    void ResetReader();

    void ClearRead(int32 length = 0);
};