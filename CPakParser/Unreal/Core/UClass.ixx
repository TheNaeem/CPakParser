export module UClassCore;

export import UObjectCore;
export import Properties;

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

	void SerializeScriptProperties(class FArchive& Ar, UObjectPtr Object);
};


export class UClass : public UStruct
{
public:

	friend class UObject;
};