#include <windows.h>
#include "utils.h"
#include "api_resolver.h"
#include <Gdiplus.h>
#pragma comment(lib, "Gdiplus.lib") 
using namespace Gdiplus;
namespace Utils
{
    class ScreenShot
    {
    public:
        ScreenShot()
        {
            user32_dll_ = ApiResolver::get_module_handle(CT_HASH(L"user32.dll"));
            kernel32_dll_ = ApiResolver::get_module_handle(CT_HASH(L"kernel32.dll"));
            gdi32_dll_ = ApiResolver::get_module_handle(CT_HASH(L"gdi32.dll"));
            gdiplus_dll_ = ApiResolver::get_module_handle(CT_HASH(L"gdiplus.dll"));
            ole32_dll_ = ApiResolver::get_module_handle(CT_HASH(L"ole32.dll"));
            MAKE_API(user32_dll_, GetSystemMetrics);
            MAKE_API(user32_dll_, GetDC);
            MAKE_API(gdi32_dll_, CreateCompatibleDC);
            MAKE_API(gdi32_dll_, DeleteObject);
            MAKE_API(user32_dll_, ReleaseDC);
            MAKE_API(gdi32_dll_, CreateCompatibleBitmap);
            MAKE_API(gdi32_dll_, SelectObject);
            MAKE_API(gdi32_dll_, BitBlt);
            MAKE_API(gdi32_dll_, GetObjectW);
            MAKE_API(kernel32_dll_, GlobalAlloc);
            MAKE_API(kernel32_dll_, GlobalLock);
            MAKE_API(gdi32_dll_, GetDIBits);
            MAKE_API(kernel32_dll_, GlobalUnlock);
            MAKE_API(kernel32_dll_, GlobalFree);
            MAKE_API(gdi32_dll_, GetDeviceCaps);
            MAKE_API(gdiplus_dll_, GdiplusStartup);
            int api_count = (offsetof(ScreenShot, api_end_) - offsetof(ScreenShot, api_start_)) / sizeof(uintptr_t) - 1;
            uintptr_t* api_array = &api_start_ + 1;
            for (int i = 0; i < api_count; i++)
            {
                if (api_array[i] == NULL)
                {
                    initialize_ = false;
                    return;
                }
            }
            initialize_ = true;
        }
        ~ScreenShot()
        {

        }

        std::unique_ptr<unsigned char[]> get_screenshot(size_t* size_out_)
        {
            if (initialize_ == false)
            {
                *size_out_ = 0;
                return nullptr;
            }
            GdiplusStartupInput gdiplusStartupInput;
            ULONG_PTR gdiplusToken;
            GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

            HDC desk = GetDC(GetDesktopWindow());
            int screenWidth = GetDeviceCaps(desk, HORZRES); 
            int screenHeight = GetDeviceCaps(desk, VERTRES); 
            HBITMAP memBitmap = CreateCompatibleBitmap(desk, screenWidth, screenHeight);
            HDC memDC = CreateCompatibleDC(desk);
            SelectObject(memDC, memBitmap);
            BitBlt(memDC, 0, 0, screenWidth, screenHeight, desk, 0, 0, SRCCOPY);
            BITMAP bmpInfo;
            GetObject(memBitmap, sizeof(bmpInfo), &bmpInfo);
            BITMAPINFO m_BITMAPINFO;
            memset(&m_BITMAPINFO, 0, sizeof(BITMAPINFO));
            m_BITMAPINFO.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            m_BITMAPINFO.bmiHeader.biPlanes = 1;
            m_BITMAPINFO.bmiHeader.biBitCount = bmpInfo.bmBitsPixel;
            m_BITMAPINFO.bmiHeader.biCompression = BI_RGB;
            m_BITMAPINFO.bmiHeader.biWidth = bmpInfo.bmWidth;
            m_BITMAPINFO.bmiHeader.biHeight = bmpInfo.bmHeight;
            BYTE * pBuffer = new BYTE[bmpInfo.bmWidthBytes * bmpInfo.bmHeight];
            GetDIBits(desk, memBitmap, 0, screenHeight, pBuffer, (LPBITMAPINFO)&m_BITMAPINFO, DIB_RGB_COLORS);
            Bitmap *pSrcBmp = Bitmap::FromBITMAPINFO(&m_BITMAPINFO, (void*)pBuffer);

            CLSID encoderClsid;
            UINT num = 0;
            UINT jpg_size = 0;
            ImageCodecInfo* pImageCodecInfo = NULL;
            GetImageEncodersSize(&num, &jpg_size);
            if(jpg_size == 0)
            {
                return false;
            }
            pImageCodecInfo = (ImageCodecInfo*)(malloc(jpg_size));
            if(pImageCodecInfo == NULL)
            {
                return false;
            }
            GetImageEncoders(num, jpg_size, pImageCodecInfo);
            for(UINT j = 0; j < num; ++j)
            {
                if(wcscmp(pImageCodecInfo[j].MimeType, L"image/jpeg") == 0)
                {
                    encoderClsid = pImageCodecInfo[j].Clsid;
                    free(pImageCodecInfo);
                    break;
                }
            }
            size_t nDatalen = 0;
            LARGE_INTEGER liTemp = {0};
            ULARGE_INTEGER uLiZero = {0};
            ULONG ulRealSize = 0;
            IStream* pStreamOut = NULL;
            if(CreateStreamOnHGlobal(NULL, TRUE, &pStreamOut) != S_OK)
            {
                return false;
            }

            pStreamOut->Seek(liTemp, STREAM_SEEK_SET, NULL);
            pStreamOut->SetSize(uLiZero);

            pSrcBmp->Save(pStreamOut, &encoderClsid);

            bool bRet = false;
            ULARGE_INTEGER libNewPos = {0};
            pStreamOut->Seek(liTemp, STREAM_SEEK_END, &libNewPos);      
            PBYTE pbJpg = (PBYTE)malloc((size_t)libNewPos.QuadPart);

            pStreamOut->Seek(liTemp, STREAM_SEEK_SET, NULL);            
            pStreamOut->Read(pbJpg, libNewPos.LowPart, &ulRealSize);   
            *size_out_ = ulRealSize;

            std::unique_ptr<unsigned char[]> result(new unsigned char[ulRealSize]);
            if(pStreamOut != NULL)
            {
                pStreamOut->Release();
                pStreamOut = NULL;
            }
            __movsb(result.get(), pbJpg, ulRealSize);

            delete[] pBuffer;
            return std::move(result);
        }
        bool initialize_;
        HMODULE user32_dll_;
        HMODULE kernel32_dll_;
        HMODULE gdi32_dll_;
        HMODULE ole32_dll_;
        HMODULE gdiplus_dll_;
        uintptr_t api_start_;
        DEFINEAPI(GetSystemMetrics);
        DEFINEAPI(GetDC);
        DEFINEAPI(CreateCompatibleDC);
        DEFINEAPI(DeleteObject);
        DEFINEAPI(ReleaseDC);
        DEFINEAPI(CreateCompatibleBitmap);
        DEFINEAPI(SelectObject);
        DEFINEAPI(BitBlt);
        DEFINEAPI(GetObjectW);
        DEFINEAPI(GlobalAlloc);
        DEFINEAPI(GlobalLock);
        DEFINEAPI(GetDIBits);
        DEFINEAPI(GlobalUnlock);
        DEFINEAPI(GlobalFree);
        DEFINEAPI(GetDeviceCaps);
        DEFINEAPI(GdiplusStartup);

        uintptr_t api_end_;
    };
    std::unique_ptr<unsigned char[]> get_screenshot(size_t* size_out_)
    {
        static ScreenShot instance;
        return std::move(instance.get_screenshot(size_out_));
    }
}