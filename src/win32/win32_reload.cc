
struct Win32_Reload {
    Permanent_Load_Callback* permanent_load;
    Hot_Load_Callback* hot_load;
    Update_Callback* update;
    HMODULE dll;
    FILETIME last_write_time;
};


internal b32
win32_load_code(Win32_Reload* reload){
    
    b32 result = 1;
    
    CopyFile(global_app_dll_path, global_temp_app_dll_path, FALSE);
    reload->dll = LoadLibraryA(global_temp_app_dll_path);
    
    
    reload->last_write_time = win32_get_last_write_time(global_app_dll_path);
}