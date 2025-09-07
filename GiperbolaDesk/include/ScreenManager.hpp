#pragma once
#include <vector>
#include <fstream>
#include <windows.h>
#include <opencv2/opencv.hpp>

class ScreenManager
{
public:
    static std::vector<uint8_t> capture_screen_as_jpg(int quality = 85)
    {
        int width = GetSystemMetrics(SM_CXSCREEN);
        int height = GetSystemMetrics(SM_CYSCREEN);

        HDC hScreen = GetDC(NULL);
        HDC hDC = CreateCompatibleDC(hScreen);
        HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, width, height);
        SelectObject(hDC, hBitmap);

        BitBlt(hDC, 0, 0, width, height, hScreen, 0, 0, SRCCOPY);

        CURSORINFO ci = { sizeof(CURSORINFO) };
        if (GetCursorInfo(&ci) && (ci.flags & CURSOR_SHOWING)) {
            ICONINFO ii;
            if (GetIconInfo(ci.hCursor, &ii)) {
                DrawIcon(hDC, ci.ptScreenPos.x - ii.xHotspot, ci.ptScreenPos.y - ii.yHotspot, ci.hCursor);
                if (ii.hbmMask) DeleteObject(ii.hbmMask);
                if (ii.hbmColor) DeleteObject(ii.hbmColor);
            }
        }

        cv::Mat img(height, width, CV_8UC3);

        BITMAPINFOHEADER bi = {};
        bi.biSize = sizeof(BITMAPINFOHEADER);
        bi.biWidth = width;
        bi.biHeight = -height;
        bi.biPlanes = 1;
        bi.biBitCount = 24;
        bi.biCompression = BI_RGB;

        GetDIBits(hDC, hBitmap, 0, height, img.data, reinterpret_cast<BITMAPINFO*>(&bi), DIB_RGB_COLORS);

        DeleteObject(hBitmap);
        DeleteDC(hDC);
        ReleaseDC(NULL, hScreen);

        std::vector<uchar> jpg_buf;
        cv::imencode(".jpg", img, jpg_buf, { cv::IMWRITE_JPEG_QUALITY, quality });

        return std::vector<uint8_t>(jpg_buf.begin(), jpg_buf.end());
    }
};