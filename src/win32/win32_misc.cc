internal FILETIME
win32_get_last_write_time(char* filename){
    FILETIME last_write_time = {0};
    WIN32_FIND_DATA find_data;
    HANDLE find_handle = FindFirstFileA(filename, &find_data);
    if(find_handle != INVALID_HANDLE_VALUE)
    {
        FindClose(find_handle);
        last_write_time = find_data.ftLastWriteTime;
    }
    return last_write_time;
}

internal void*
win32_heap_alloc(u32 size){
    return HeapAlloc(GetProcessHeap(), 0, size);
}

internal void
win32_heap_free(void* data){
    HeapFree(GetProcessHeap(), 0, data);
}

internal void*
win32_reserve(u64 size){
    return VirtualAlloc(0, size, MEM_RESERVE, PAGE_NOACCESS);
}

internal void
win32_release(void* memory){
    VirtualFree(memory, 0, MEM_RELEASE);
}

internal void
win32_commit(void* memory, u64 size){
    VirtualAlloc(memory, size, MEM_COMMIT, PAGE_READWRITE);
}

internal void
win32_uncommit(void* memory, u64 size){
    VirtualFree(memory, size, MEM_DECOMMIT);
}
internal void
win32_output_error(char *title, char *format, ...)
{
    va_list args;
    va_start(args, format);
    u32 required_characters = vsnprintf(0, 0, format, args)+1;
    va_end(args);
    
    char text[4096] = {0};
    if(required_characters > 4096){
        required_characters = 4096;
    }
    
    va_start(args, format);
    vsnprintf(text, required_characters, format, args);
    va_end(args);
    
    text[required_characters-1] = 0;
    MessageBoxA(0, text, title, MB_OK);
}

internal void
win32_toggle_fullscreen(HWND hwnd)
{
    local_persist WINDOWPLACEMENT last_window_placement = {
        sizeof(last_window_placement)
    };
    
    DWORD window_style = GetWindowLong(hwnd, GWL_STYLE);
    if(window_style & WS_OVERLAPPEDWINDOW) {
        
        MONITORINFO monitor_info = { sizeof(monitor_info) };
        if(GetWindowPlacement(hwnd, &last_window_placement) &&
           GetMonitorInfo(MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY),
                          &monitor_info)) {
            
            SetWindowLong(hwnd, GWL_STYLE,
                          window_style & ~WS_OVERLAPPEDWINDOW);
            
            SetWindowPos(hwnd, HWND_TOP,
                         monitor_info.rcMonitor.left,
                         monitor_info.rcMonitor.top,
                         monitor_info.rcMonitor.right -
                         monitor_info.rcMonitor.left,
                         monitor_info.rcMonitor.bottom -
                         monitor_info.rcMonitor.top,
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    }
    else {
        SetWindowLong(hwnd, GWL_STYLE,
                      window_style | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(hwnd, &last_window_placement);
        SetWindowPos(hwnd, 0, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                     SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
}