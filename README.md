## About

UE C++ �Ա� <a href="https://github.com/polison/NetworkLibrary">NetworkLibrary<a> �Ŀ��.

## How to use

#һ��ָ��ö��, ��Ҫע��UINT_MAX.UINT_MAX����Ϊ��������źŴ�����.
```
enum ENetOpcode : uint32
{
    PropertyInfo,
};

```

#�Զ����������ͺ��Զ����¼�
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

#һ���Զ������ݰ�
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

#һ��UNetworkLibrary����
``` 

//.h

UCLASS()
class UXXNetworkLibrary : public UNetworkLibrary
{
    GENERATED_BODY()

public:
    UXXNetworkLibrary();

public:
    //һЩ�Զ���ķ���,����ʵ�ַ�������
    UFUNCTION(BlueprintCallable, Category = "NetworkLibrary")
    void GetPropertyInfo(int32 NodeId);

    //һЩ�Զ�����¼�,����ʵ�����ݷַ�
    UPROPERTY(VisibleAnywhere, BlueprintAssignable, Category = "NetworkLibrary")
    FOnInfoProperty OnPropertyInfoEnd;

private:
    //һЩ�Զ���ķ���,����ʵ��ע��������
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

#����
��ȡ UXXNetworkLibrary GameInstatnce��ϵͳ���ö�Ӧ����,�󶨶�Ӧ�¼�.
���ӷ��������� ConnectToServer 
�� OnKilled �¼���������������Ͽ�����ʱ��������.
��������ͨ���Զ��庯��,�¼���װʵ��.