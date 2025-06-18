#include <cstdint>
#include <cmath>

#include <Windows.h>
#include <gdiplus.h>

#include <iostream>

#pragma comment (lib, "gdiplus.lib")

using namespace Gdiplus;

using IntensityType = uint8_t;
const IntensityType MIN_INTENSITY = 0, MAX_INTENSITY = 255;
const float GAMMA = 0.45f;
const float FACTOR = std::pow(MAX_INTENSITY, 1.0f - GAMMA);

float correctPixelIntensity(float intensity) {
    return FACTOR * std::pow(intensity, GAMMA);
}

void nonlinearCorrection(int height, int width,
    int nChannels,
    IntensityType* pixels, IntensityType* result
    )
{
    for (int i = 0; i < height; i++)
        for (int j = 0; j < width; j++)
            for (int color = 0; color < nChannels; color++) {
                int index = nChannels * (i * width + j) + color;
                result[index] = correctPixelIntensity(pixels[index]);
            }
}

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid) {
    UINT num = 0, size = 0;
    GetImageEncodersSize(&num, &size);
    if (size == 0) return -1;

    ImageCodecInfo* pInfo = (ImageCodecInfo*)malloc(size);
    if (pInfo == NULL) return -1;

    GetImageEncoders(num, size, pInfo);
    for (UINT i = 0; i < num; i++) {
        if (wcscmp(pInfo[i].MimeType, format) == 0) {
            *pClsid = pInfo[i].Clsid;
            free(pInfo);
            return i;
        }
    }

    free(pInfo);
    return -1;
}

int main(int argc, const char** argv){
    ULONG_PTR gdiplusToken;
    GdiplusStartupInput gdiplusStartupInput;

    if (argc < 3) {
        std::cout << "Usage: img_proc IN_IMAGE_PATH OUT_IMAGE_PATH\n";
    }

    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    std::string in_img(argv[1]);
    int len = MultiByteToWideChar(CP_UTF8, 0, in_img.c_str(), -1, NULL, 0);
    std::wstring win_img(len, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, in_img.c_str(), -1, &win_img[0], len);
    Image image(win_img.c_str());
    UINT width = image.GetWidth();
    UINT height = image.GetHeight();

    Bitmap* bitmap = (Bitmap*)&image;

    Rect rect(0, 0, width, height);
    BitmapData bmpData;
    bitmap->LockBits(
        &rect,
        ImageLockModeRead | ImageLockModeWrite,
        PixelFormat24bppRGB, // или 32bppARGB для прозрачности
        &bmpData
    );

    BYTE* pixels = (BYTE*)bmpData.Scan0;
    int stride = bmpData.Stride; // может быть > width*3 (выровнено по 4 байта)

    nonlinearCorrection(height, width, 3, pixels, pixels);

    bitmap->UnlockBits(&bmpData);
    CLSID jpgClsid;
    GetEncoderClsid(L"image/jpeg", &jpgClsid);
    std::string out_img(argv[2]);
    len = MultiByteToWideChar(CP_UTF8, 0, out_img.c_str(), -1, NULL, 0);
    std::wstring wout_img(len, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, out_img.c_str(), -1, &wout_img[0], len);
    bitmap->Save(wout_img.c_str(), &jpgClsid, NULL);
    // GdiplusShutdown(gdiplusToken);
    return 0;
}