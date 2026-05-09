#pragma once
#include <windows.h>
#include <shellapi.h>

extern void EnableLiquidGlass(HWND hwnd);

HWND hwndStartMenu = NULL;
bool isMenuVisible = false;

LRESULT CALLBACK StartMenuProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_PAINT: {
            PAINTSTRUCT ps; HDC hdc = BeginPaint(hwnd, &ps);
            
            HBRUSH bgBrush = CreateSolidBrush(RGB(32, 32, 36)); 
            FillRect(hdc, &ps.rcPaint, bgBrush); DeleteObject(bgBrush);
            SetBkMode(hdc, TRANSPARENT);
            
            
            HBRUSH searchBrush = CreateSolidBrush(RGB(45, 45, 50));
            SelectObject(hdc, searchBrush); SelectObject(hdc, GetStockObject(NULL_PEN));
            RoundRect(hdc, 30, 30, 420, 65, 10, 10);
            HFONT fontSearch = CreateFontA(15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, "Segoe UI");
            SelectObject(hdc, fontSearch); SetTextColor(hdc, RGB(160, 160, 160));
            TextOutA(hdc, 45, 38, "Buscar aplicaciones, configuraciones y documentos", 49);
            DeleteObject(searchBrush);

            
            HFONT titleFont = CreateFontA(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, "Segoe UI");
            SelectObject(hdc, titleFont); SetTextColor(hdc, RGB(255, 255, 255));
            TextOutA(hdc, 40, 95, "Anclado", 7);

            
            HICON hEdge = LoadIcon(NULL, IDI_APPLICATION); 
            DrawIconEx(hdc, 60, 140, hEdge, 32, 32, 0, NULL, DI_NORMAL); TextOutA(hdc, 50, 180, "Navegador", 9);
            DrawIconEx(hdc, 210, 140, hEdge, 32, 32, 0, NULL, DI_NORMAL); TextOutA(hdc, 205, 180, "Archivos", 8);
            DrawIconEx(hdc, 360, 140, hEdge, 32, 32, 0, NULL, DI_NORMAL); TextOutA(hdc, 355, 180, "Terminal", 8);

            
            HPEN linePen = CreatePen(PS_SOLID, 1, RGB(60, 60, 65)); SelectObject(hdc, linePen);
            MoveToEx(hdc, 0, 480, NULL); LineTo(hdc, 450, 480); DeleteObject(linePen);

            
            SetTextColor(hdc, RGB(255, 255, 255)); TextOutA(hdc, 40, 505, "Usuario", 7);
            
            HBRUSH pwrBrush = CreateSolidBrush(RGB(255, 60, 60)); SelectObject(hdc, pwrBrush);
            RoundRect(hdc, 320, 500, 370, 530, 5, 5); // Apagar
            DeleteObject(pwrBrush);
            SetTextColor(hdc, RGB(255, 255, 255)); TextOutA(hdc, 335, 505, "OFF", 3);

            HBRUSH resBrush = CreateSolidBrush(RGB(255, 150, 50)); SelectObject(hdc, resBrush);
            RoundRect(hdc, 380, 500, 430, 530, 5, 5); // Reiniciar
            DeleteObject(resBrush);
            TextOutA(hdc, 395, 505, "RES", 3);

            DeleteObject(titleFont); DeleteObject(fontSearch); EndPaint(hwnd, &ps); return 0;
        }
        case WM_LBUTTONDOWN: {
            int x = LOWORD(lParam); int y = HIWORD(lParam);
            
            if (x >= 40 && x <= 100 && y >= 130 && y <= 190) ShellExecuteA(NULL, "open", "msedge.exe", NULL, NULL, SW_SHOW);
            if (x >= 190 && x <= 250 && y >= 130 && y <= 190) ShellExecuteA(NULL, "open", "explorer.exe", NULL, NULL, SW_SHOW);
            if (x >= 340 && x <= 400 && y >= 130 && y <= 190) ShellExecuteA(NULL, "open", "cmd.exe", NULL, NULL, SW_SHOW);
            
            if (x >= 320 && x <= 370 && y >= 500 && y <= 530) system("shutdown /s /t 0");
            if (x >= 380 && x <= 430 && y >= 500 && y <= 530) system("shutdown /r /t 0");
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
        int startX = (sw - 450) / 2; 
        hwndStartMenu = CreateWindowExA(WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_LAYERED,
            "StartMenuClass", "StartMenu", WS_POPUP, startX, sh - 550 - 80, 450, 550, NULL, NULL, GetModuleHandle(NULL), NULL);
        SetLayeredWindowAttributes(hwndStartMenu, 0, 240, LWA_ALPHA);
        EnableLiquidGlass(hwndStartMenu);
        HRGN hRgn = CreateRoundRectRgn(0, 0, 450, 550, 15, 15); SetWindowRgn(hwndStartMenu, hRgn, TRUE);
    }
    if (isMenuVisible) ShowWindow(hwndStartMenu, SW_HIDE);
    else { ShowWindow(hwndStartMenu, SW_SHOW); SetForegroundWindow(hwndStartMenu); }
    isMenuVisible = !isMenuVisible;
}
