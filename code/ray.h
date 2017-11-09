
#define internal static;
#define local_persist static;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long u64;
typedef char i8;
typedef short i16;
typedef int i32;
typedef long i64;

typedef u32 b32;

typedef float f32;
typedef double f64;

#define F32Max FLT_MAX
#define F32Min -FLT_MAX
#define Pi32 3.141592653589793238462643

#include "ray_math.h"

#define ArrayCount(X) ((sizeof((X)) / sizeof((X)[0])))

#pragma pack(push, 1)
struct bitmap_header
{   
    u16 FileType;
    u32 FileSize;
    u16 Reserved1;
    u16 Reserved2;
    u32 BitmapOffset;
    u32 Size;
    i32 Width;
    i32 Height;
    u16 Planes;
    u16 BitsPerPixel;
    u32 Compression;
    u32 SizeOfBitmap;
    i32 HorzResolution;
    i32 VertResolution;
    u32 ColorsUsed;
    u32 ColorsImportant;
};
#pragma pack(pop)

struct image_u32
{
    u32 Width;
    u32 Height;
    u32* Pixels;
};

struct material
{
    f32 Scatter;
    v3 EmitColor;
    v3 RefColor;
};

struct plane
{
    // NOTE(scott): Plane Equation: (N^T)*p + d = 0
    // ax + by + cz = 0

    v3 N;
    f32 d;

    u32 MaterialIndex;
};

struct sphere
{
    v3 P;  
    f32 r;

    u32 MaterialIndex;
};

struct world
{
    u32 MaterialCount;
    material* Materials;

    u32 PlaneCount;
    plane* Planes;

    u32 SphereCount;
    sphere* Spheres;
};