export module CPakParser.Materials.Expression;

import CPakParser.Serialization.FArchive;
import CPakParser.Core.UObject;
import CPakParser.Math.Color;

export class UMaterialExpression : public UObject
{
public:

	void Serialize(FArchive& Ar) override
	{
		UObject::Serialize(Ar);
	}
};

export struct FExpressionInput
{
	friend FArchive& operator<<(FArchive& Ar, FExpressionInput& Input)
	{
		// TODO: this
		return Ar;
	}
};

export template<class InputType> 
struct FMaterialInput : FExpressionInput
{
	FMaterialInput()
	{
		UseConstant = 0;
		Constant = InputType(0);
	}

	unsigned __int32 UseConstant : 1;
	InputType Constant;
};

export struct FColorMaterialInput : FMaterialInput<FColor>
{

};
