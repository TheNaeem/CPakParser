export module CPakParser.Structs.Math;

export struct FMath
{
    static float Fmod(float X, float Y);
    static double Fmod(double X, double Y);
    
    template< class T > 
    __forceinline static constexpr T Abs( const T A )
    {
        return (A < (T)0) ? -A : A;
    }

    template< class T > 
    __forceinline static constexpr T Sign( const T A )
    {
        return (A > (T)0) ? (T)1 : ((A < (T)0) ? (T)-1 : (T)0);
    }

    template< class T > 
    __forceinline static constexpr T Max( const T A, const T B )
    {
        return (B < A) ? A : B;
    }

    template< class T > 
    __forceinline static constexpr T Min( const T A, const T B )
    {
        return (A < B) ? A : B;
    }
    
    __forceinline static constexpr int TruncToInt32(float F)
    {
        return (int)F;
    }
    __forceinline static constexpr int TruncToInt32(double F)
    {
        return (int)F;
    }
    __forceinline static constexpr long TruncToInt64(double F)
    {
        return (long)F;
    }
    
    __forceinline static int FloorToInt32(float F)
    {
        int I = TruncToInt32(F);
        I -= ((float)I > F);
        return I;
    }
    __forceinline static int FloorToInt32(double F)
    {
        int I = TruncToInt32(F);
        I -= ((double)I > F);
        return I;
    }
    __forceinline static long FloorToInt64(double F)
    {
        long I = TruncToInt64(F);
        I -= ((double)I > F);
        return I;
    }

    __forceinline static int RoundToInt32(float F)
    {
        return FloorToInt32(F + 0.5f);
    }
    __forceinline static int RoundToInt32(double F)
    {
        return FloorToInt32(F + 0.5);
    }
    __forceinline static long RoundToInt64(double F)
    {
        return FloorToInt64(F + 0.5);
    }
    __forceinline static int RoundToInt(float F) { return RoundToInt32(F); }
    __forceinline static long RoundToInt(double F) { return RoundToInt64(F); }

    template< class T >
    __forceinline static constexpr T GridSnap(T Location, T Grid)
    {
        return (Grid == T{}) ? Location : (Floor((Location + (Grid/(T)2)) / Grid) * Grid);
    }
};