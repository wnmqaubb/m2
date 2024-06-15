<<<<<<< HEAD
#include "pch.h"
#include "utils.h"
#include <pe_bliss_resources.h>
#include <pe_bliss.h>
#include <ShellAPI.h>

using namespace pe_bliss;
using namespace Utils::Crypto;

namespace Utils
{
    namespace PEScan
    {
        bool calc_pe_ico_hash(std::wstring path, uint32_t* hash_val)
        {
            if (GetModuleHandleA("Shell32.dll") == NULL)
                LoadLibraryA("Shell32.dll");
            if (GetModuleHandleA("gdi32.dll") == NULL)
                LoadLibraryA("gdi32.dll");

            auto ExtractIconW = IMPORT(L"Shell32.dll", ExtractIconW);
            auto GetIconInfo = IMPORT(L"User32.dll", GetIconInfo);
            auto CopyImage = IMPORT(L"User32.dll", CopyImage);
            auto DestroyIcon = IMPORT(L"User32.dll", DestroyIcon);
            auto GetObjectW = IMPORT(L"gdi32.dll", GetObjectW);
            auto DeleteObject = IMPORT(L"gdi32.dll", DeleteObject);

            if (!ExtractIconW || !GetObjectW || !DeleteObject || !DestroyIcon || !CopyImage || !GetIconInfo)
                return false;
            bool result = false;
            HICON hicon = NULL;
            ICONINFO icon_info;
            try
            {
                HMODULE curr_hmodule = GetModuleHandle(NULL);
                if (curr_hmodule)
                {
                    hicon = ExtractIconW(curr_hmodule, path.c_str(), 0);
                    if (hicon)
                    {
                        if (GetIconInfo(hicon, &icon_info))
                        {
                            HBITMAP hbitmap;
                            DIBSECTION ds;
                            int ds_size = GetObjectW(icon_info.hbmColor, sizeof(ds), &ds);
                            if (sizeof(ds) != ds_size)
                            {
                                hbitmap = (HBITMAP)CopyImage(icon_info.hbmColor, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
                                ds_size = GetObjectW(hbitmap, sizeof(ds), &ds);
                            }

                            std::unique_ptr<unsigned char[]> icon_buffer(new unsigned char[ds.dsBmih.biSizeImage]);
                            __movsb(icon_buffer.get(), (unsigned char*)ds.dsBm.bmBits, ds.dsBmih.biSizeImage);

                            if (hash_val)
                                *hash_val = aphash(icon_buffer.get(), ds.dsBmih.biSizeImage);

                            DestroyIcon(hicon);
                            DeleteObject(hbitmap);
                            DeleteObject(icon_info.hbmColor);
                            DeleteObject(icon_info.hbmMask);

                            result = true;
                        }
                    }
                }
            }
            catch (...)
            {
                result = false;
            }

            if (hicon)
            {
                DestroyIcon(hicon);
            }

            if (icon_info.hbmColor)
            {
                DeleteObject(icon_info.hbmColor);
            }

            if (icon_info.hbmMask)
            {
                DeleteObject(icon_info.hbmMask);
            }

            return result;
        }
    }

}
=======
#include "pch.h"
#include "utils.h"
#include <pe_bliss_resources.h>
#include <pe_bliss.h>
#include <ShellAPI.h>

using namespace pe_bliss;
using namespace Utils::Crypto;

namespace Utils
{
    namespace PEScan
    {
        bool calc_pe_ico_hash(std::wstring path, uint32_t* hash_val)
        {
            if (GetModuleHandleA("Shell32.dll") == NULL)
                LoadLibraryA("Shell32.dll");
            if (GetModuleHandleA("gdi32.dll") == NULL)
                LoadLibraryA("gdi32.dll");

            auto ExtractIconW = IMPORT(L"Shell32.dll", ExtractIconW);
            auto GetIconInfo = IMPORT(L"User32.dll", GetIconInfo);
            auto CopyImage = IMPORT(L"User32.dll", CopyImage);
            auto DestroyIcon = IMPORT(L"User32.dll", DestroyIcon);
            auto GetObjectW = IMPORT(L"gdi32.dll", GetObjectW);
            auto DeleteObject = IMPORT(L"gdi32.dll", DeleteObject);

            if (!ExtractIconW || !GetObjectW || !DeleteObject || !DestroyIcon || !CopyImage || !GetIconInfo)
                return false;
            bool result = false;
            HICON hicon = NULL;
            ICONINFO icon_info;
            try
            {
                HMODULE curr_hmodule = GetModuleHandle(NULL);
                if (curr_hmodule)
                {
                    hicon = ExtractIconW(curr_hmodule, path.c_str(), 0);
                    if (hicon)
                    {
                        if (GetIconInfo(hicon, &icon_info))
                        {
                            HBITMAP hbitmap;
                            DIBSECTION ds;
                            int ds_size = GetObjectW(icon_info.hbmColor, sizeof(ds), &ds);
                            if (sizeof(ds) != ds_size)
                            {
                                hbitmap = (HBITMAP)CopyImage(icon_info.hbmColor, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
                                ds_size = GetObjectW(hbitmap, sizeof(ds), &ds);
                            }

                            std::unique_ptr<unsigned char[]> icon_buffer(new unsigned char[ds.dsBmih.biSizeImage]);
                            __movsb(icon_buffer.get(), (unsigned char*)ds.dsBm.bmBits, ds.dsBmih.biSizeImage);

                            if (hash_val)
                                *hash_val = aphash(icon_buffer.get(), ds.dsBmih.biSizeImage);

                            DestroyIcon(hicon);
                            DeleteObject(hbitmap);
                            DeleteObject(icon_info.hbmColor);
                            DeleteObject(icon_info.hbmMask);

                            result = true;
                        }
                    }
                }
            }
            catch (...)
            {
                result = false;
            }

            if (hicon)
            {
                DestroyIcon(hicon);
            }

            if (icon_info.hbmColor)
            {
                DeleteObject(icon_info.hbmColor);
            }

            if (icon_info.hbmMask)
            {
                DeleteObject(icon_info.hbmMask);
            }

            return result;
        }
    }

}
>>>>>>> 31e167db06d1caa1061611bc914b44d6767746d9
