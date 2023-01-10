export module CPakParser.Math.Rotator;

import <type_traits>;
import CPakParser.Serialization.FArchive;
import CPakParser.Structs.Math;

export template <typename T>
struct TRotator
{
    static_assert(std::is_floating_point<T>::value, "TRotator only supports float and double types.");

    /** Rotation around the right axis (around Y axis), Looking up and down (0=Straight Ahead, +Up, -Down) */
    T Pitch;

    /** Rotation around the up axis (around Z axis), Turning around (0=Forward, +Right, -Left)*/
    T Yaw;
            
    /** Rotation around the forward axis (around X axis), Tilting your head, (0=Straight, +Clockwise, -CCW) */
    T Roll;

    /** A rotator of zero degrees on each axis. */
    static const TRotator ZeroRotator;

    __forceinline TRotator() = default;

    __forceinline explicit TRotator(T InF)
        :	Pitch(InF), Yaw(InF), Roll(InF)
    {
    }

    __forceinline TRotator(T InPitch, T InYaw, T InRoll)
        :	Pitch(InPitch), Yaw(InYaw), Roll(InRoll) 
    {
    }

    friend inline FArchive& operator<<(FArchive& Ar, TRotator& R)
    {
        if (Ar.UEVer() >= EUnrealEngineObjectUE5Version::LARGE_WORLD_COORDINATES)
        {
            Ar << R.Pitch << R.Yaw << R.Roll;
        }
        else
        {
            // Stored as floats, so serialize float and copy.
            float Pitch, Yaw, Roll;
            Ar << Pitch << Yaw << Roll;
            R = TRotator<double>(Pitch, Yaw, Roll);
        }
        return Ar;
    }

    __forceinline TRotator operator+(const TRotator& R) const
    {
        return TRotator( Pitch+R.Pitch, Yaw+R.Yaw, Roll+R.Roll );
    }

    __forceinline TRotator operator-(const TRotator& R) const
    {
        return TRotator( Pitch-R.Pitch, Yaw-R.Yaw, Roll-R.Roll );
    }

    __forceinline TRotator operator*(float Scale)
    {
        return TRotator(Pitch * Scale, Yaw * Scale, Roll * Scale);
    }

    __forceinline bool operator==(const TRotator& R) const
    {
        return Pitch==R.Pitch && Yaw==R.Yaw && Roll==R.Roll;
    }

    __forceinline bool operator!=(const TRotator& V) const
    {
        return Pitch!=V.Pitch || Yaw!=V.Yaw || Roll!=V.Roll;
    }
    
    __forceinline TRotator operator+=(const TRotator& R)
    {
        Pitch += R.Pitch; Yaw += R.Yaw; Roll += R.Roll;
        return *this;
    }

    __forceinline TRotator operator-=(const TRotator& R)
    {
        Pitch -= R.Pitch; Yaw -= R.Yaw; Roll -= R.Roll;
        return *this;
    }

    __forceinline TRotator operator*=(float Scale)
    {
        Pitch = Pitch * Scale; Yaw = Yaw * Scale; Roll = Roll * Scale;
        return *this;
    }

    __forceinline bool IsNearlyZero(T Tolerance) const
    {
        return
        FMath::Abs(NormalizeAxis(Pitch))<=Tolerance
        &&	FMath::Abs(NormalizeAxis(Yaw))<=Tolerance
        &&	FMath::Abs(NormalizeAxis(Roll))<=Tolerance;
    }

    __forceinline bool IsZero() const
    {
        return (ClampAxis(Pitch)==0.f) && (ClampAxis(Yaw)==0.f) && (ClampAxis(Roll)==0.f);
    }
    
    __forceinline bool Equals(const TRotator& R, T Tolerance) const
    {
        return (FMath::Abs(NormalizeAxis(Pitch - R.Pitch)) <= Tolerance) 
        && (FMath::Abs(NormalizeAxis(Yaw - R.Yaw)) <= Tolerance) 
        && (FMath::Abs(NormalizeAxis(Roll - R.Roll)) <= Tolerance);
    }

    __forceinline TRotator Add(T DeltaPitch, T DeltaYaw, T DeltaRoll)
    {
        Yaw   += DeltaYaw;
        Pitch += DeltaPitch;
        Roll  += DeltaRoll;
        return *this;
    }

    __forceinline TRotator GridSnap(const TRotator& RotGrid) const
    {
        return TRotator
            (
            FMath::GridSnap(Pitch,RotGrid.Pitch),
            FMath::GridSnap(Yaw,  RotGrid.Yaw),
            FMath::GridSnap(Roll, RotGrid.Roll)
            );
    }

    __forceinline TRotator Clamp() const
    {
        return TRotator(ClampAxis(Pitch), ClampAxis(Yaw), ClampAxis(Roll));
    }

    __forceinline T ClampAxis(T Angle)
    {
        // returns Angle in the range (-360,360)
        Angle = FMath::Fmod(Angle, (T)360.0);

        if (Angle < (T)0.0)
        {
            // shift to [0,360) range
            Angle += (T)360.0;
        }

        return Angle;
    }

    __forceinline T NormalizeAxis(T Angle)
    {
        // returns Angle in the range [0,360)
        Angle = ClampAxis(Angle);

        if (Angle > (T)180.0)
        {
            // shift to (-180,180]
            Angle -= (T)360.0;
        }

        return Angle;
    }

    __forceinline uint8_t CompressAxisToByte(T Angle)
    {
        // map [0->360) to [0->256) and mask off any winding
        return FMath::RoundToInt(Angle * (T)256.f / (T)360.f) & 0xFF;
    }

    __forceinline T DecompressAxisFromByte(uint8_t Angle)
    {
        // map [0->256) to [0->360)
        return (Angle * (T)360.f / (T)256.f);
    }

    __forceinline uint16_t CompressAxisToShort(T Angle)
    {
        // map [0->360) to [0->65536) and mask off any winding
        return FMath::RoundToInt(Angle * (T)65536.f / (T)360.f) & 0xFFFF;
    }

    __forceinline T DecompressAxisFromShort(uint16_t Angle)
    {
        // map [0->65536) to [0->360)
        return (Angle * (T)360.f / (T)65536.f);
    }

    __forceinline TRotator GetNormalized() const
    {
        TRotator Rot = *this;
        Rot.Normalize();
        return Rot;
    }

    __forceinline TRotator GetDenormalized() const
    {
        TRotator Rot = *this;
        Rot.Pitch	= ClampAxis(Rot.Pitch);
        Rot.Yaw		= ClampAxis(Rot.Yaw);
        Rot.Roll	= ClampAxis(Rot.Roll);
        return Rot;
    }
    
    __forceinline void Normalize()
    {
        Pitch = NormalizeAxis(Pitch);
        Yaw = NormalizeAxis(Yaw);
        Roll = NormalizeAxis(Roll);
    }
};

export typedef TRotator<double> FRotator;
export typedef TRotator<double> FRotator3d;
export typedef TRotator<float> FRotator3f;