module;

#include "Core/Defines.h"

export module CPakParser.Curves.RealCurve;
import CPakParser.Serialization.FArchive;

export enum ERichCurveInterpMode : uint8_t
{
	/** Use linear interpolation between values. */
	RCIM_Linear,
	/** Use a constant value. Represents stepped values. */
	RCIM_Constant,
	/** Cubic interpolation. See TangentMode for different cubic interpolation options. */
	RCIM_Cubic,
	/** No interpolation. */
	RCIM_None
};

export SERIALIZABLE_ENUM(ERichCurveInterpMode);