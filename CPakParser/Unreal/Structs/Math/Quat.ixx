export module CPakParser.Math.Quat;

import <type_traits>;
import CPakParser.Serialization.FArchive;
import CPakParser.Structs.Math;

export template <typename T>
struct TQuat
{
    static_assert(std::is_floating_point<T>::value, "TQuat only supports float and double types.");

    /** The quaternion's X-component. */
    T X;

    /** The quaternion's Y-component. */
    T Y;

    /** The quaternion's Z-component. */
    T Z;

    /** The quaternion's W-component. */
    T W;

    /** Identity quaternion. */
    static const TQuat Identity;

    __forceinline TQuat() = default;

    __forceinline TQuat(T InX, T InY, T InZ, T InW)
        : X(InX), Y(InY), Z(InZ), W(InW)
    {
    }

    __forceinline explicit TQuat(T V)
        : X(V), Y(V), Z(V), W(V)
    {
    }
    
    friend inline FArchive& operator<<(FArchive& Ar, TQuat& F)
    {
        if (Ar.UEVer() >= EUnrealEngineObjectUE5Version::LARGE_WORLD_COORDINATES)
        {
            Ar << F.X << F.Y << F.Z << F.W;
        }
        else
        {
            // Stored as floats, so serialize float and copy.
            float X, Y, Z, W;
            Ar << X << Y << Z << W;
            F = TQuat<double>(X, Y, Z, W);
        }
        return Ar;
    }
};

export typedef TQuat<double> FQuat;
export typedef TQuat<double> FQuat4d;
export typedef TQuat<float> FQuat4f;