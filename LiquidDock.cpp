#include <windows.h>
#include <shellapi.h>
#include <vector>
#include <string>
#include <cmath>

#define TRAY_ICON_ID 1
#define WM_TRAYICON (WM_USER + 1)

struct ACCENTPOLICY { int nAccentState; int nFlags; int nColor; int nAnimationId; };
struct WINCOMPATTRDATA { int nAttribute; PVOID pData; ULONG ulDataSize; };

void EnableLiquidGlass(HWND hwnd) {
    HMODULE hUser = GetModuleHandleA("user32.dll");
    if (hUser) {
        typedef BOOL(WINAPI* pSetAttr)(HWND, WINCOMPATTRDATA*);
        pSetAttr SetAttr = (pSetAttr)GetProcAddress(hUser, "SetWindowCompositionAttribute");
        if (SetAttr) {
            ACCENTPOLICY policy = { 3, 0, 0x40101010, 0 }; 
            WINCOMPATTRDATA data = { 19, &policy, sizeof(ACCENTPOLICY) };
            SetAttr(hwnd, &data);
        }
    }
}

int dockWidth = 700; 
#include "SystemMonitor.h"
#include "HardwareOverlay.h"
#include "StartMenu.h"

bool isDockActive = false;
HWND hwndDock = NULL; HWND hwndMain = NULL;
HHOOK keyboardHook = NULL;
std::vector<HWND> openApps;
int currentY = 0;


bool winKeyIsDown = false;
bool otherKeyPressed = false;

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT* p = (KBDLLHOOKSTRUCT*)lParam;
        if (p->vkCode == VK_LWIN || p->vkCode == VK_RWIN) {
            if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
                winKeyIsDown = true;
                otherKeyPressed = false;
            } else if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
                winKeyIsDown = false;
                if (!otherKeyPressed) {
                    
                    keybd_event(VK_F24, 0, 0, 0); keybd_event(VK_F24, 0, KEYEVENTF_KEYUP, 0);
                    ToggleStartMenu();
                    return 1; 
                }
            }
        } else {
            if (winKeyIsDown) otherKeyPressed = true; 
        }
    }
    return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    if (IsWindowVisible(hwnd) && hwnd != hwndDock && hwnd != hwndMain && hwnd != hwndOverlay && hwnd != hwndStartMenu) {
        HWND owner = GetWindow(hwnd, GW_OWNER);
        int exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
        if (owner == NULL && (exStyle & WS_EX_TOOLWINDOW) == 0) {
            char title[256]; GetWindowTextA(hwnd, title, sizeof(title));
            if (strlen(title) > 0 && strcmp(title, "Program Manager") != 0 && strcmp(title, "Settings") != 0) {
                openApps.push_back(hwnd);
            }
        }
    }
    return TRUE;
}

LRESULT CALLBACK DockProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_TIMER: {
            openApps.clear(); EnumWindows(EnumWindowsProc, 0);
            
           
            POINT pt; GetCursorPos(&pt);
            int sh = GetSystemMetrics(SM_CYSCREEN);
            RECT dockRect; GetWindowRect(hwnd, &dockRect);
            
            bool mouseAtBottom = (pt.y >= sh - 5); 
            bool mouseOnDock = (pt.x >= dockRect.left && pt.x <= dockRect.right && pt.y >= dockRect.top);
            bool isMenuOpen = isMenuVisible; 
            
            int targetY;
            if (mouseAtBottom || mouseOnDock || isMenuOpen) {
                targetY = sh - 65 - 10; 
            } else {
                targetY = sh + 10; 
            }
            
           
            if (currentY != targetY) {
                currentY += (targetY - currentY) / 4;
                if (abs(targetY - currentY) < 2) currentY = targetY;
                SetWindowPos(hwnd, HWND_TOPMOST, (GetSystemMetrics(SM_CXSCREEN) - dockWidth) / 2, currentY, 0, 0, SWP_NOSIZE);
            }
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps; HDC hdc = BeginPaint(hwnd, &ps);
            HBRUSH bgBrush = CreateSolidBrush(RGB(25, 25, 28)); FillRect(hdc, &ps.rcPaint, bgBrush); DeleteObject(bgBrush);
            
            // Boton Inicio
            HBRUSH blueBrush = CreateSolidBrush(RGB(0, 120, 215)); SelectObject(hdc, blueBrush); SelectObject(hdc, GetStockObject(NULL_PEN));
            int sx = 20, sy = 20; 
            RoundRect(hdc, sx, sy, sx+11, sy+11, 2, 2); RoundRect(hdc, sx+13, sy, sx+24, sy+11, 2, 2);
            RoundRect(hdc, sx, sy+13, sx+11, sy+24, 2, 2); RoundRect(hdc, sx+13, sy+13, sx+24, sy+24, 2, 2);
            DeleteObject(blueBrush);

            int iconX = 70;
            HWND activeApp = GetForegroundWindow();
            for (HWND app : openApps) {
                HICON hIcon = (HICON)SendMessage(app, WM_GETICON, ICON_BIG, 0);
                if (!hIcon) hIcon = (HICON)GetClassLongPtr(app, GCLP_HICON);
                if (!hIcon) hIcon = LoadIcon(NULL, IDI_APPLICATION);
                DrawIconEx(hdc, iconX, 16, hIcon, 32, 32, 0, NULL, DI_NORMAL);
                
                if (app == activeApp) {
                    HBRUSH activeBrush = CreateSolidBrush(RGB(150, 150, 150)); SelectObject(hdc, activeBrush);
                    RoundRect(hdc, iconX + 10, 54, iconX + 22, 57, 2, 2); DeleteObject(activeBrush);
                }
                iconX += 45;
            }
            EndPaint(hwnd, &ps); return 0;
        }
        case WM_LBUTTONDOWN: {
            int x = LOWORD(lParam);
            if (x >= 15 && x <= 50) ToggleStartMenu();
            else if (x >= 70) { 
                int idx = (x - 70) / 45;
                if (idx >= 0 && idx < openApps.size()) { SwitchToThisWindow(openApps[idx], TRUE); }
            }
            return 0;
        }
    }
    return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

void ToggleSystem(bool turnOn) {
    if (turnOn) {
        ShowWindow(FindWindowA("Shell_TrayWnd", NULL), SW_HIDE);
        keyboardHook = SetWindowsHookExA(WH_KEYBOARD_LL, LowLevelKeyboardProc, GetModuleHandle(NULL), 0);
        
        WNDCLASSA wc = {0}; wc.lpfnWndProc = DockProc; wc.hInstance = GetModuleHandle(NULL); wc.lpszClassName = "LiquidDockClass";
        RegisterClassA(&wc); 

        int sw = GetSystemMetrics(SM_CXSCREEN); int sh = GetSystemMetrics(SM_CYSCREEN);
        currentY = sh + 10; 
        hwndDock = CreateWindowExA(WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_LAYERED,
            "LiquidDockClass", "Dock", WS_POPUP | WS_VISIBLE, (sw - dockWidth) / 2, currentY, dockWidth, 65, NULL, NULL, GetModuleHandle(NULL), NULL);

        SetLayeredWindowAttributes(hwndDock, 0, 230, LWA_ALPHA); EnableLiquidGlass(hwndDock);
        HRGN hRgn = CreateRoundRectRgn(0, 0, dockWidth, 65, 20, 20); SetWindowRgn(hwndDock, hRgn, TRUE);
        SetTimer(hwndDock, 1, 16, NULL); 
        StartOverlay();
    } else {
        if (keyboardHook) { UnhookWindowsHookEx(keyboardHook); keyboardHook = NULL; }
        if (hwndDock) { DestroyWindow(hwndDock); hwndDock = NULL; }
        if (hwndOverlay) { DestroyWindow(hwndOverlay); hwndOverlay = NULL; }
        if (hwndStartMenu) { DestroyWindow(hwndStartMenu); hwndStartMenu = NULL; isMenuVisible = false; }
        ShowWindow(FindWindowA("Shell_TrayWnd", NULL), SW_SHOW);
    }
}

void AddTrayIcon(HWND hwnd) {
    NOTIFYICONDATAA nid = {0}; nid.cbSize = sizeof(NOTIFYICONDATAA); nid.hWnd = hwnd;
    nid.uID = TRAY_ICON_ID; nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON; nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    strcpy(nid.szTip, "Panel de Control"); Shell_NotifyIconA(NIM_ADD, &nid);
}

LRESULT CALLBACK ControlPanelProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: AddTrayIcon(hwnd); return 0;
        case WM_TRAYICON: 
            if (lParam == WM_LBUTTONUP) { ShowWindow(hwnd, SW_RESTORE); SetForegroundWindow(hwnd); }
            return 0;
        case WM_PAINT: {
            PAINTSTRUCT ps; HDC hdc = BeginPaint(hwnd, &ps);
            HBRUSH bgBrush = CreateSolidBrush(RGB(20, 20, 22)); FillRect(hdc, &ps.rcPaint, bgBrush); DeleteObject(bgBrush);
            SetBkMode(hdc, TRANSPARENT); SetTextColor(hdc, RGB(255, 255, 255));
            HFONT hFont = CreateFontA(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, "Segoe UI");
            SelectObject(hdc, hFont); TextOutA(hdc, 25, 25, "Panel de Control", 16);
            HFONT hDescFont = CreateFontA(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, "Segoe UI");
            SelectObject(hdc, hDescFont); SetTextColor(hdc, RGB(150, 150, 150));
            TextOutA(hdc, 25, 60, "Bajo consumo. Animaciones fluidas.", 34);
            
            HBRUSH toggleBrush = isDockActive ? CreateSolidBrush(RGB(0, 120, 215)) : CreateSolidBrush(RGB(60, 60, 60));
            SelectObject(hdc, toggleBrush); SelectObject(hdc, GetStockObject(NULL_PEN)); RoundRect(hdc, 25, 120, 85, 150, 30, 30); 
            HBRUSH circleBrush = CreateSolidBrush(RGB(255, 255, 255)); SelectObject(hdc, circleBrush);
            if (isDockActive) Ellipse(hdc, 57, 122, 83, 148); else Ellipse(hdc, 27, 122, 53, 148);              
            SelectObject(hdc, hFont); SetTextColor(hdc, isDockActive ? RGB(0, 120, 215) : RGB(120, 120, 120));
            TextOutA(hdc, 100, 120, isDockActive ? "Sistema Activo" : "Sistema Apagado", isDockActive ? 14 : 15);
            
            SetTextColor(hdc, RGB(0, 150, 255)); TextOutA(hdc, 25, 220, "Soporte: hazedxz6@gmail.com", 27);
            
            DeleteObject(toggleBrush); DeleteObject(circleBrush); DeleteObject(hFont); DeleteObject(hDescFont);
            EndPaint(hwnd, &ps); return 0;
        }
        case WM_LBUTTONDOWN: {
            int xPos = LOWORD(lParam); int yPos = HIWORD(lParam);
            if (xPos >= 25 && xPos <= 85 && yPos >= 120 && yPos <= 150) {
                isDockActive = !isDockActive; ToggleSystem(isDockActive); InvalidateRect(hwnd, NULL, TRUE); 
            }
            if (xPos >= 25 && xPos <= 250 && yPos >= 220 && yPos <= 245) {
                ShellExecuteA(NULL, "open", "mailto:hazedxz6@gmail.com", NULL, NULL, SW_SHOWNORMAL);
            }
            return 0;
        }
        case WM_CLOSE: ShowWindow(hwnd, SW_HIDE); return 0; 
        case WM_DESTROY: {
            NOTIFYICONDATAA nid = {0}; nid.cbSize = sizeof(NOTIFYICONDATAA); nid.hWnd = hwnd; nid.uID = TRAY_ICON_ID;
            Shell_NotifyIconA(NIM_DELETE, &nid);
            if (isDockActive) ToggleSystem(false);
            PostQuitMessage(0); return 0;
        }
    }
    return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmd, int nShow) {
    
    HWND existingHwnd = FindWindowA("ControlPanelClass", NULL);
    if (existingHwnd) { 
        ShowWindow(existingHwnd, SW_RESTORE); 
        SetForegroundWindow(existingHwnd); 
        return 0; 
    }
    
    WNDCLASSA wc = {0}; wc.lpfnWndProc = ControlPanelProc; wc.hInstance = hInst; wc.lpszClassName = "ControlPanelClass"; wc.hCursor = LoadCursor(NULL, IDC_HAND); RegisterClassA(&wc);
    int sw = GetSystemMetrics(SM_CXSCREEN), sh = GetSystemMetrics(SM_CYSCREEN);
    hwndMain = CreateWindowExA(0, "ControlPanelClass", "Panel de Control", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, (sw - 400) / 2, (sh - 300) / 2, 400, 300, NULL, NULL, hInst, NULL);
    ShowWindow(hwndMain, SW_SHOW);
    MSG msg; while (GetMessage(&msg, NULL, 0, 0)) { TranslateMessage(&msg); DispatchMessage(&msg); }
    return 0;
}
