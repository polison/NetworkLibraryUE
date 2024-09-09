## About

UE C++ 对标 <a href="https://github.com/polison/NetworkLibrary">NetworkLibrary<a> 的框架.

## How to use

#一个指令枚举, 不要注册UINT_MAX.UINT_MAX被作为网络断线信号处理了.
```
enum ENetOpcode : uint32
{
    PropertyInfo,
};

```

#自定义数据类型和自定义事件
```
//.h
USTRUCT(BlueprintType)
struct FInfoProperty : public FInfoBase
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Property")
    int32 Id;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Property")
    int32 NodeId;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Property")
    FString GroupName;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Property")
    FString PropertyName;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Property")
    FString Value;

    FInfoProperty();

    virtual void Read(FSocketArchive& Ar) override;

    virtual void Write(FSocketArchive& Ar) override;
};
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInfoProperty, int32, SelectId, const TArray<FInfoProperty>&, Infos);

//.cpp
FInfoProperty::FInfoProperty()
{
    GroupName = PropertyName = Value = TEXT("-");
}

void FInfoProperty::Read(FSocketArchive& Ar)
{
    Id = Ar.Read<int32>();
    NodeId = Ar.Read<int32>();
    GroupName = Ar.Read();
    PropertyName = Ar.Read();
    Value = Ar.Read();
}

void FInfoProperty::Write(FSocketArchive& Ar)
{
    Ar.Write(Id);
    Ar.Write(NodeId);
    Ar.Write(GroupName);
    Ar.Write(PropertyName);
    Ar.Write(Value);
}
```

#一个自定义数据包
```
//.h
class FPropertyPacket : public FSocketPacket
{
public:
    int32 SelectId;

    TArray<FInfoProperty> Infos;

public:
    FPropertyPacket();

    virtual void Read(FSocketArchive& Ar) override;

    virtual void Write(FSocketArchive& Ar) override;
};

//.cpp
FPropertyPacket::FPropertyPacket()
{
    PacketOpcode = PropertyInfo;
    Infos.Empty();
}

void FPropertyPacket::Read(FSocketArchive& Ar)
{
    SelectId = Ar.Read<int32>();
    int32 Num = Ar.Read<int32>();
    for (int32 i = 0; i < Num; i++)
    {
        FInfoProperty Property;
        Property.Read(Ar);
        Infos.Add(Property);
    }
}

void FPropertyPacket::Write(FSocketArchive& Ar)
{
    PacketBuffer.Write(SelectId);
    FSocketPacket::Write(Ar);
}
```

#一个UNetworkLibrary子类
``` 

//.h

UCLASS()
class UXXNetworkLibrary : public UNetworkLibrary
{
    GENERATED_BODY()

public:
    UXXNetworkLibrary();

public:
    //一些自定义的方法,用来实现发送数据
    UFUNCTION(BlueprintCallable, Category = "NetworkLibrary")
    void GetPropertyInfo(int32 NodeId);

    //一些自定义的事件,用来实现数据分发
    UPROPERTY(VisibleAnywhere, BlueprintAssignable, Category = "NetworkLibrary")
    FOnInfoProperty OnPropertyInfoEnd;

private:
    //一些自定义的方法,用来实现注册封包处理
    void OnPropertyInfo(FSocketArchive& Ar);
};

//.cpp

UXXNetworkLibrary::USGNetworkLibrary()
    : UXXNetworkLibrary()
{
    //todo : Add PacketHandler Here;
    RegisterHandler(PropertyInfo, [ this ](FSocketArchive& Ar) { OnPropertyInfo(Ar); });
}

void UXXNetworkLibrary::GetPropertyInfo(int32 NodeId)
{
    FPropertyPacket p;
    p.SelectId = NodeId;

    SendPacket(&p);
}

void UXXNetworkLibrary::OnPropertyInfo(FSocketArchive& Ar)
{
    FPropertyPacket p;
    p.Read(Ar);

    OnPropertyInfoEnd.Broadcast(p.SelectId, p.Infos);
}
```

#调用
获取 UXXNetworkLibrary GameInstatnce子系统调用对应方法,绑定对应事件.
连接服务器调用 ConnectToServer 
绑定 OnKilled 事件可以在与服务器断开链接时触发操作.
其他可以通过自定义函数,事件封装实现.