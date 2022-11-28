module;

#include "Core/Defines.h"

export module CPakParser.Curves.RichCurve;

import CPakParser.Serialization.FArchive;
import CPakParser.Curves.RealCurve;
import CPakParser.Versioning.Custom.UE5MainStreamObjectVersion;
import CPakParser.MathUtil;

export enum ERichCurveTangentMode : uint8_t
{
	/** Automatically calculates tangents to create smooth curves between values. */
	RCTM_Auto,
	/** User specifies the tangent as a unified tangent where the two tangents are locked to each other, presenting a consistent curve before and after. */
	RCTM_User,
	/** User specifies the tangent as two separate broken tangents on each side of the key which can allow a sharp change in evaluation before or after. */
	RCTM_Break,
	/** No tangents. */
	RCTM_None
};

export SERIALIZABLE_ENUM(ERichCurveTangentMode);

enum ERichCurveTangentWeightMode : uint8_t
{
	/** Don't take tangent weights into account. */
	RCTWM_WeightedNone,
	/** Only take the arrival tangent weight into account for evaluation. */
	RCTWM_WeightedArrive,
	/** Only take the leaving tangent weight into account for evaluation. */
	RCTWM_WeightedLeave,
	/** Take both the arrival and leaving tangent weights into account for evaluation. */
	RCTWM_WeightedBoth
};

export SERIALIZABLE_ENUM(ERichCurveTangentWeightMode);

export struct FRichCurveKey
{
	FRichCurveKey()
		: InterpMode(RCIM_Linear)
		, TangentMode(RCTM_Auto)
		, TangentWeightMode(RCTWM_WeightedNone)
		, Time(0.f)
		, Value(0.f)
		, ArriveTangent(0.f)
		, ArriveTangentWeight(0.f)
		, LeaveTangent(0.f)
		, LeaveTangentWeight(0.f)
	{ }

	FRichCurveKey(float InTime, float InValue)
		: InterpMode(RCIM_Linear)
		, TangentMode(RCTM_Auto)
		, TangentWeightMode(RCTWM_WeightedNone)
		, Time(InTime)
		, Value(InValue)
		, ArriveTangent(0.f)
		, ArriveTangentWeight(0.f)
		, LeaveTangent(0.f)
		, LeaveTangentWeight(0.f)
	{ }

	FRichCurveKey(float InTime, float InValue, float InArriveTangent, const float InLeaveTangent, ERichCurveInterpMode InInterpMode)
		: InterpMode(InInterpMode)
		, TangentMode(RCTM_Auto)
		, TangentWeightMode(RCTWM_WeightedNone)
		, Time(InTime)
		, Value(InValue)
		, ArriveTangent(InArriveTangent)
		, ArriveTangentWeight(0.f)
		, LeaveTangent(InLeaveTangent)
		, LeaveTangentWeight(0.f)
	{ }

	/** Interpolation mode between this key and the next */
	ERichCurveInterpMode InterpMode;

	/** Mode for tangents at this key */
	ERichCurveTangentMode TangentMode;

	/** If either tangent at this key is 'weighted' */
	ERichCurveTangentWeightMode TangentWeightMode;

	/** Time at this key */
	float Time;

	/** Value at this key */
	float Value;

	/** If RCIM_Cubic, the arriving tangent at this key */
	float ArriveTangent;

	/** If RCTWM_WeightedArrive or RCTWM_WeightedBoth, the weight of the left tangent */
	float ArriveTangentWeight;

	/** If RCIM_Cubic, the leaving tangent at this key */
	float LeaveTangent;

	/** If RCTWM_WeightedLeave or RCTWM_WeightedBoth, the weight of the right tangent */
	float LeaveTangentWeight;

	friend FArchive& operator<<(FArchive& Ar, FRichCurveKey& P)
	{
		if (Ar.UEVer() < VER_UE4_SERIALIZE_RICH_CURVE_KEY)
		{
			return Ar;
		}

		Ar << P.InterpMode;
		Ar << P.TangentMode;
		Ar << P.TangentWeightMode;
		Ar << P.Time;
		Ar << P.Value;
		Ar << P.ArriveTangent;
		Ar << P.ArriveTangentWeight;
		Ar << P.LeaveTangent;
		Ar << P.LeaveTangentWeight;

		if (P.TangentWeightMode != RCTWM_WeightedNone &&
			P.TangentWeightMode != RCTWM_WeightedArrive &&
			P.TangentWeightMode != RCTWM_WeightedLeave &&
			P.TangentWeightMode != RCTWM_WeightedBoth &&
			Ar.CustomVer(FUE5MainStreamObjectVersion::GUID) < FUE5MainStreamObjectVersion::RichCurveKeyInvalidTangentMode)
		{
			P.TangentWeightMode = RCTWM_WeightedNone;
			
			// No valid weight mode - find correct one one
			const bool bHasArriveWeight = !CPP_Math::IsNearlyZero(P.ArriveTangentWeight);
			const bool bHasLeaveWeight = !CPP_Math::IsNearlyZero(P.LeaveTangentWeight);

			if (bHasArriveWeight && bHasLeaveWeight)
			{
				P.TangentWeightMode = RCTWM_WeightedBoth;
			}
			else if (bHasArriveWeight)
			{
				P.TangentWeightMode = RCTWM_WeightedArrive;
			}
			else if (bHasLeaveWeight)
			{
				P.TangentWeightMode = RCTWM_WeightedLeave;
			}
		}

		return Ar;
	}
};