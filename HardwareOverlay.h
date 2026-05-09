case WM_PAINT: {
            PAINTSTRUCT ps; HDC hdc = BeginPaint(hwnd, &ps);
            
            
            HBRUSH bgBrush = CreateSolidBrush(RGB(20, 20, 23)); 
            FillRect(hdc, &ps.rcPaint, bgBrush); 
            DeleteObject(bgBrush);
            
            SetBkMode(hdc, TRANSPARENT);
            
            HFONT fontData = CreateFontA(14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, "Segoe UI");
            SelectObject(hdc, fontData);
            
            int cpu = hwMonitorOverlay.GetCPUUsage();
            int ram = hwMonitorOverlay.GetRAMUsage();
            
            std::string cpuStr = "CPU: " + std::to_string(cpu) + "%";
            std::string ramStr = "RAM: " + std::to_string(ram) + "%";
            
            
            SetTextColor(hdc, (cpu > 80) ? RGB(255, 80, 80) : RGB(100, 255, 150)); 
            TextOutA(hdc, 15, 10, cpuStr.c_str(), cpuStr.length());
            
            SetTextColor(hdc, (ram > 80) ? RGB(255, 80, 80) : RGB(100, 200, 255));
            TextOutA(hdc, 15, 30, ramStr.c_str(), ramStr.length());

            DeleteObject(fontData); 
            EndPaint(hwnd, &ps); 
            return 0;
        }
