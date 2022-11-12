export module UClass;

export import UObjectCore;
import FProperty;

export class UStruct : public UObject
{
public:

	friend class UObject;
	friend class Mappings;

	~UStruct();

private:

	UStructPtr Super;
	FProperty* PropertyLink = nullptr;

public:

	void SetSuper(UStructPtr Val);
	UStructPtr GetSuper();

	__forceinline FProperty* GetPropertyLink()
	{
		return PropertyLink;
	}

	void SerializeScriptProperties(FArchive& Ar, UObjectPtr Object);
};


export class UClass : public UStruct
{
public:

	friend class UObject;
};