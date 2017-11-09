#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include "ray.h"

internal u32
GetTotalPixelSize(image_u32 Image)
{
    u32 Result = Image.Width*Image.Height*sizeof(u32);
    return(Result);
}

internal void
WriteBitmapToFile(char* Filename, image_u32 Image)
{
    i32 BitmapSize = GetTotalPixelSize(Image);

    bitmap_header Header = {};
    Header.FileType = 0x4d42;
    Header.FileSize = BitmapSize + sizeof(bitmap_header);
    Header.BitmapOffset = sizeof(bitmap_header);
    Header.Size = 40;
    Header.Width = Image.Width;
    Header.Height = Image.Height;
    Header.Planes = 1;
    Header.BitsPerPixel = 32;
    Header.SizeOfBitmap = BitmapSize;

    FILE* Outfile = fopen(Filename, "wb");

    if (Outfile)
    {
        fwrite(&Header, sizeof(bitmap_header), 1, Outfile);
        fwrite(Image.Pixels, BitmapSize, 1, Outfile);
        fclose(Outfile);
    }
}

internal f32
RandomUnilateral(void)
{
    f32 Result = (f32)rand() / (f32)RAND_MAX;
    return Result;
}

internal f32
RandomBilateral(void)
{
    f32 Result = -1.0f + 2.0f*RandomUnilateral();
    return Result;
}

internal f32
ExactLinearToSRGB(f32 L)
{
    if (L < 0.0f)
    {
        L = 0.0f;
    }
    else if (L > 1.0f)
    {
        L = 1.0f;
    }

    f32 S = L*12.92f;
    if (L > 0.0031308f)
    {
        S = 1.055f*Pow(L, 1.0f/2.4f) - 0.055f;
    }
    return S;
}

internal v3
RayCast(world* World, v3 RayOrigin, v3 RayDirection)
{
    f32 MinHitDistance = 0.001f;
    f32 Tolerance = 0.0001f;

    v3 Result = {};
    v3 Attenuation = V3(1, 1, 1);

    for (u32 RayCount=0; RayCount<8; RayCount++)
    {
        f32 HitDistance = F32Max;
        u32 HitMatIndex = 0;
        v3 NextNormal = {};

        for (u32 PlaneIndex=0; PlaneIndex<World->PlaneCount; PlaneIndex++)
        {
            plane Plane = World->Planes[PlaneIndex];

            f32 Denom = Inner(Plane.N, RayDirection);        
            if ((Denom < -Tolerance) || (Denom > Tolerance))
            {
                f32 t = (-Plane.d - Inner(Plane.N, RayOrigin)) / Denom;
                if ((t > MinHitDistance) && (t < HitDistance))
                {
                    HitDistance = t;
                    HitMatIndex = Plane.MaterialIndex;

                    NextNormal = Plane.N;
                }
            }
        }

        for (u32 SphereIndex=0; SphereIndex<World->SphereCount; SphereIndex++)
        {
            sphere Sphere = World->Spheres[SphereIndex];

            v3 SphereRelativeRayOrigin = RayOrigin - Sphere.P;
            f32 a = Inner(RayDirection, RayDirection);
            f32 b = 2.0f * Inner(RayDirection, SphereRelativeRayOrigin);
            f32 c = Inner(SphereRelativeRayOrigin, SphereRelativeRayOrigin) - Square(Sphere.r);

            f32 Denom = 2.0f*a;
            f32 RootTerm = SquareRoot(b*b - 4.0f*a*c);
            if (RootTerm > Tolerance)
            {
                f32 tp = (-b + RootTerm) / Denom;
                f32 tn = (-b - RootTerm) / Denom;

                f32 t = tp;
                if ((tn > MinHitDistance) && (tn < tp))
                {
                    t = tn;
                }

                if ((t > MinHitDistance) && (t < HitDistance))
                {
                    HitDistance = t;
                    HitMatIndex = Sphere.MaterialIndex;

                    NextNormal = NOZ(t*RayDirection + SphereRelativeRayOrigin);
                }
            }
        }

        if (HitMatIndex)
        {
            material Mat = World->Materials[HitMatIndex];

            Result += Hadamard(Attenuation, Mat.EmitColor);
            f32 CosAtten = Inner(-RayDirection, NextNormal);
            if (CosAtten < 0)
            {
                CosAtten = 0;
            }
            Attenuation = Hadamard(Attenuation, CosAtten*Mat.RefColor);

            RayOrigin += HitDistance*RayDirection;

            v3 PureBounce = RayDirection - 2.0f*Inner(RayDirection, NextNormal)*NextNormal;
            v3 RandomBounce = NOZ(NextNormal + V3(RandomBilateral(), RandomBilateral(), RandomBilateral()));
            RayDirection = NOZ(Lerp(RandomBounce, Mat.Scatter, PureBounce));
        }
        else
        {            
            material Mat = World->Materials[HitMatIndex];
            Result += Hadamard(Attenuation, Mat.EmitColor);
            break;
        }
    }

    return Result;
}


int main(int ArgCount, char** Args)
{
    printf("Raycasting...");

    image_u32 Image;
    Image.Width = 1280;
    Image.Height = 720;

    i32 BitmapSize = GetTotalPixelSize(Image);
    Image.Pixels = (u32*)malloc(BitmapSize);

#if 1
    material Materials[7] = {};
    Materials[0].EmitColor = V3(0.3f, 0.4f, 0.5f);
    Materials[1].RefColor = V3(0.5f, 0.5f, 0.5f);
    Materials[2].RefColor = V3(0.7f, 0.5f, 0.3f);
    Materials[3].EmitColor = V3(0.4f, 0.0f, 0.0f);
    Materials[4].RefColor = V3(0.2f, 0.8f, 0.2f);
    Materials[4].Scatter = 0.7f;
    Materials[5].RefColor = V3(0.4f, 0.8f, 0.9f);
    Materials[5].Scatter = 0.85f;    
    Materials[6].RefColor = V3(0.95f, 0.95f, 0.95f);
    Materials[6].Scatter = 1.0f;

    plane Planes[1] = {};
    Planes[0].N = V3(0, 0, 1);
    Planes[0].d = 0;
    Planes[0].MaterialIndex = 1;

    sphere Spheres[5] = {};
    Spheres[0].P = V3(0, 0, 0);
    Spheres[0].r = 1.0f;
    Spheres[0].MaterialIndex = 2;
    Spheres[1].P = V3(3, -2, 0);
    Spheres[1].r = 1.0f;
    Spheres[1].MaterialIndex = 3;
    Spheres[2].P = V3(-2, -1, 2);
    Spheres[2].r = 1.0f;
    Spheres[2].MaterialIndex = 4;    
    Spheres[3].P = V3(1, -1, 3);
    Spheres[3].r = 1.0f;
    Spheres[3].MaterialIndex = 5;
    Spheres[4].P = V3(-2, 3, 0);
    Spheres[4].r = 2.0f;
    Spheres[4].MaterialIndex = 6;
#else
    material Materials[3] = {};
    Materials[0].EmitColor = V3(0.3f, 0.4f, 0.5f);
    Materials[1].RefColor = V3(0.5f, 0.5f, 0.5f);    
    Materials[2].RefColor = V3(0.95f, 0.95f, 0.95f);
    Materials[2].Scatter = 1.0f;

    plane Planes[1] = {};
    Planes[0].N = V3(0, 0, 1);
    Planes[0].d = 0;
    Planes[0].MaterialIndex = 1;

    sphere Spheres[6] = {};
    Spheres[0].P = V3(-1.5f, -1, 1);
    Spheres[0].r = 1;
    Spheres[0].MaterialIndex = 2;
    Spheres[1].P = V3(-1.5f, 0, 1);
    Spheres[1].r = 1;
    Spheres[1].MaterialIndex = 2;
    Spheres[2].P = V3(-1.5f, 1, 1);
    Spheres[2].r = 1;
    Spheres[2].MaterialIndex = 2;
    Spheres[3].P = V3(1.5f, -1, 1);
    Spheres[3].r = 1;
    Spheres[3].MaterialIndex = 2;
    Spheres[4].P = V3(1.5f, 0, 1);
    Spheres[4].r = 1;
    Spheres[4].MaterialIndex = 2;
    Spheres[5].P = V3(1.5f, 1, 1);
    Spheres[5].r = 1;
    Spheres[5].MaterialIndex = 2;
#endif

    world World = {};
    World.MaterialCount = ArrayCount(Materials);
    World.Materials = Materials;
    World.PlaneCount = ArrayCount(Planes);
    World.Planes = Planes;
    World.SphereCount = ArrayCount(Spheres);
    World.Spheres = Spheres;

#if 1
    v3 CameraP = V3(0, -10, 1);
#else
    v3 CameraP = V3(0, -5, 3);
#endif
    v3 CameraZ = NOZ(CameraP);
    v3 CameraX = NOZ(Cross(V3(0, 0, 1), CameraZ));
    v3 CameraY = NOZ(Cross(CameraZ, CameraX));


    f32 FilmDist = 1.0f;
    f32 FilmW = 1.0f;
    f32 FilmH = 1.0f;
    if (Image.Width > Image.Height)
    {
        FilmH = FilmW * (f32)Image.Height / (f32)Image.Width;
    }
    else if (Image.Height > Image.Width)
    {
        FilmW = FilmH * (f32)Image.Width / (f32)Image.Height;
    }
    f32 HalfFilmW = 0.5f * FilmW;
    f32 HalfFilmH = 0.5f * FilmH;
    v3 FilmCenter = CameraP - FilmDist*CameraZ;

    f32 HalfPixW = 0.5f / Image.Width;
    f32 HalfPixH = 0.5f / Image.Height;

    u32 RaysPerPixel = 256;    
    u32 PreviousPercent = 0;
    u32* Pixel = Image.Pixels;
    for (i32 Y=0; Y<Image.Height; Y++)
    {
        f32 FilmY = -1.0f + 2.0f*((f32)Y / (f32)Image.Height);

        for (i32 X=0; X<Image.Width; X++)
        {
            f32 FilmX = -1.0f + 2.0f*((f32)X / (f32)Image.Width);;

            v3 Color = {};
            f32 Contrib = 1.0f / (f32)RaysPerPixel;
            for (u32 RayIndex=0; RayIndex<RaysPerPixel; RayIndex++) 
            {
                f32 OffX = FilmX + RandomBilateral() * HalfPixW;
                f32 OffY = FilmY + RandomBilateral() * HalfPixH;
                v3 FilmP = FilmCenter + OffX*HalfFilmW*CameraX + OffY*HalfFilmH*CameraY;
                v3 RayOrigin = CameraP;
                v3 RayDirection = NOZ(FilmP - CameraP);

                Color += Contrib*RayCast(&World, RayOrigin, RayDirection);
            }

            v4 BMPColor = 
            {
                255.0f*ExactLinearToSRGB(Color.R),
                255.0f*ExactLinearToSRGB(Color.G),
                255.0f*ExactLinearToSRGB(Color.B),
                255.0f
            };                
            u32 BMPValue = BGRAPack4x8(BMPColor);

            *Pixel++ = BMPValue;
        }

        u32 Percent = (u32)(100.0f*(f32)Y/(f32)Image.Height);
        if (PreviousPercent != Percent)
        {            
            printf("\rRaycasting line %4d of %4d (%d%%)", Y, Image.Height, Percent);
            fflush(stdout);
            PreviousPercent = Percent;
        }

    }


    WriteBitmapToFile("test.bmp", Image);

    printf("Done.\n");

    return 0;
}