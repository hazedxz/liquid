#pragma once
#include <windows.h>
#include <shellapi.h>

extern void EnableLiquidGlass(HWND hwnd);
extern int dockWidth; 

HWND hwndStartMenu = NULL;
bool isMenuVisible = false;

LRESULT CALLBACK StartMenuProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_PAINT: {
            PAINTSTRUCT ps; HDC hdc = BeginPaint(hwnd, &ps);
            HBRUSH bgBrush = CreateSolidBrush(RGB(12, 12, 15)); FillRect(hdc, &ps.rcPaint, bgBrush); DeleteObject(bgBrush);
            SetBkMode(hdc, TRANSPARENT);
            
            HFONT titleFont = CreateFontA(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, "Segoe UI");
            SelectObject(hdc, titleFont); SetTextColor(hdc, RGB(255, 255, 255));
            TextOutA(hdc, 20, 20, "Pinned / Recent", 15);
            
            HFONT btnFont = CreateFontA(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, "Segoe UI");
            SelectObject(hdc, btnFont); SetTextColor(hdc, RGB(200, 200, 200));
            TextOutA(hdc, 30, 80, "[ > ] Explorador de Archivos", 28);
            TextOutA(hdc, 30, 120, "[ > ] Navegador Web", 19);

           
            SetTextColor(hdc, RGB(255, 50, 50));
            TextOutA(hdc, 20, 420, "(O) Apagar Equipo", 17);
            SetTextColor(hdc, RGB(255, 150, 50));
            TextOutA(hdc, 20, 460, "(R) Reiniciar", 13);

            DeleteObject(titleFont); DeleteObject(btnFont); EndPaint(hwnd, &ps); return 0;
        }
        case WM_LBUTTONDOWN: {
            int y = HIWORD(lParam);
            if (y >= 80 && y <= 110) ShellExecuteA(NULL, "open", "explorer.exe", NULL, NULL, SW_SHOW);
            if (y >= 120 && y <= 150) ShellExecuteA(NULL, "open", "msedge.exe", NULL, NULL, SW_SHOW);
            if (y >= 420 && y <= 450) system("shutdown /s /t 0"); 
            if (y >= 460 && y <= 490) system("shutdown /r /t 0"); 
            return 0;
        }
        case WM_KILLFOCUS:
            ShowWindow(hwnd, SW_HIDE); isMenuVisible = false; return 0;
    }
    return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

void ToggleStartMenu() {
    if (!hwndStartMenu) {
        WNDCLASSA wc = {0}; wc.lpfnWndProc = StartMenuProc; wc.hInstance = GetModuleHandle(NULL); wc.lpszClassName = "StartMenuClass";
        RegisterClassA(&wc);
        int sw = GetSystemMetrics(SM_CXSCREEN); int sh = GetSystemMetrics(SM_CYSCREEN);
        int startX = (sw - dockWidth) / 2; 
        hwndStartMenu = CreateWindowExA(WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_LAYERED,
            "StartMenuClass", "StartMenu", WS_POPUP, startX, sh - 500 - 80, 350, 500, NULL, NULL, GetModuleHandle(NULL), NULL);
        SetLayeredWindowAttributes(hwndStartMenu, 0, 230, LWA_ALPHA);
        EnableLiquidGlass(hwndStartMenu);
        HRGN hRgn = CreateRoundRectRgn(0, 0, 350, 500, 20, 20); SetWindowRgn(hwndStartMenu, hRgn, TRUE);
    }
    if (isMenuVisible) ShowWindow(hwndStartMenu, SW_HIDE);
    else { ShowWindow(hwndStartMenu, SW_SHOW); SetForegroundWindow(hwndStartMenu); }
    isMenuVisible = !isMenuVisible;
}
