

union v3
{
    struct
    {
        f32 X, Y, Z;
    };
    struct 
    {
        f32 R, G, B;
    };
};


union v4
{
    struct
    {
        union
        {
            v3 XYZ;
            struct
            {
                f32 X, Y, Z;
            };
        };

        f32 W;
    };
    struct
    {
        union
        {
            v3 RGB;
            struct
            {
                f32 R, G, B;
            };
        };

        f32 A;        
    };
};


//
// Scalar Operations
//

inline u32
RoundF32ToU32(f32 A)
{
    u32 Result = (u32)(A + 0.5f);
    return Result;
}

inline f32
Square(f32 A)
{
    f32 Result = A * A;
    return Result;
}

inline f32
SquareRoot(f32 A)
{
    f32 Result = (f32)sqrt(A);
    return Result;
}

inline f32
Pow(f32 A, f32 B)
{
    f32 Result = (f32)pow(A, B);
    return Result;
}

inline f32
Lerp(f32 A, f32 t, f32 B)
{
    f32 Result = (1.0f - t)*A + B*t;
    return Result; 
}


//
// v3 Operations
// 

inline v3
V3(f32 X, f32 Y, f32 Z)
{
    v3 Result;

    Result.X = X;
    Result.Y = Y;
    Result.Z = Z;

    return Result;
}

inline v3
operator+(v3 A, v3 B)
{
    v3 Result = A;

    Result.X += B.X;
    Result.Y += B.Y;
    Result.Z += B.Z;

    return Result;
}

inline v3 &
operator+=(v3 &A, v3 B)
{
    A = A + B;

    return(A);
}

inline v3
operator-(v3 A, v3 B)
{
    v3 Result;

    Result.X = A.X - B.X;
    Result.Y = A.Y - B.Y;
    Result.Z = A.Z - B.Z;

    return(Result);
}

inline v3
operator*(f32 A, v3 B)
{
    v3 Result;

    Result.X = A*B.X;
    Result.Y = A*B.Y;
    Result.Z = A*B.Z;
    
    return(Result);
}

inline v3
operator*(v3 B, f32 A)
{
    v3 Result = A*B;

    return(Result);
}

inline v3
operator-(v3 A)
{
    v3 Result;

    Result.X = -A.X;
    Result.Y = -A.Y;
    Result.Z = -A.Z;

    return Result;
}

inline v3
Hadamard(v3 A, v3 B)
{
    v3 Result = {};

    Result.X = A.X*B.X;
    Result.Y = A.Y*B.Y;
    Result.Z = A.Z*B.Z;

    return Result;
}

inline f32 
Inner(v3 A, v3 B)
{
    f32 Result = A.X*B.X + A.Y*B.Y + A.Z*B.Z;
    return Result;
}

inline v3
Cross(v3 A, v3 B)
{
    v3 Result;

    Result.X = A.Y*B.Z - A.Z*B.Y;
    Result.Y = A.Z*B.X - A.X*B.Z;
    Result.Z = A.X*B.Y - A.Y*B.X;

    return Result;
}

inline f32 
LengthSq(v3 A)
{
    f32 Result = Inner(A, A);
    return Result;
}

inline f32
Length(v3 A)
{
    f32 Result = SquareRoot(LengthSq(A));
    return Result;
}

inline v3
Normalize(v3 A)
{
    v3 Result = A * (1.0f / Length(A));
    return Result;
}

inline v3
NOZ(v3 A)
{
    v3 Result = {};

    f32 LenSq = LengthSq(A);
    if (LenSq > Square(0.0001f))
    {
        Result = A * (1.0f / SquareRoot(LenSq));
    }

    return Result;
}

inline v3
Lerp(v3 A, f32 t, v3 B)
{
    v3 Result = (1.0f - t)*A + B*t;
    return Result;
}

//
// v4 Operations
//

inline v4
V4(v3 XYZ, f32 W)
{
    v4 Result;

    Result.XYZ = XYZ;
    Result.W = W;

    return Result;
}

inline u32
RGBAPack4x8(v4 Unpacked)
{
    u32 Result = ((RoundF32ToU32(Unpacked.A) << 24) |
                  (RoundF32ToU32(Unpacked.B) << 16) |
                  (RoundF32ToU32(Unpacked.G) << 8)  |
                  (RoundF32ToU32(Unpacked.R) << 0));
    return Result;
}

inline u32
BGRAPack4x8(v4 Unpacked)
{
    u32 Result = ((RoundF32ToU32(Unpacked.A) << 24) |
                  (RoundF32ToU32(Unpacked.R) << 16) |
                  (RoundF32ToU32(Unpacked.G) << 8)  |
                  (RoundF32ToU32(Unpacked.B) << 0));
    return Result;
}

inline v4
Linear1ToSRGB255(v4 C)
{
    v4 Result;

    f32 One255 = 255.0f;

    Result.R = One255*SquareRoot(C.R);
    Result.G = One255*SquareRoot(C.G);
    Result.B = One255*SquareRoot(C.B);
    Result.A = One255*C.A;

    return Result;
}