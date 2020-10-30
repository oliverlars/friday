
struct Win32_Reload {
    Permanent_Load_Callback* permanent_load;
    Hot_Load_Callback* hot_load;
    Hot_Unload_Callback* hot_unload;
    Update_Callback* update;
    HMODULE dll;
    FILETIME last_write_time;
};


internal b32
win32_code_load(Win32_Reload* app_code){
    b32 result = 1;
    
    CopyFile(global_app_dll_path, global_temp_app_dll_path, FALSE);
    app_code->dll = LoadLibraryA(global_temp_app_dll_path);
    
    app_code->last_write_time = win32_get_last_write_time(global_app_dll_path);
    
    if(!app_code->dll)
    {
        result = 0;
        goto end;
    }
    
    app_code->permanent_load    = (Permanent_Load_Callback*)GetProcAddress(app_code->dll, "permanent_load");
    app_code->hot_load          = (Hot_Load_Callback*)GetProcAddress(app_code->dll, "hot_load");
    app_code->hot_unload        = (Hot_Unload_Callback*)GetProcAddress(app_code->dll, "hot_unload");
    app_code->update            = (Update_Callback*)GetProcAddress(app_code->dll, "update");
    
    if(!app_code->permanent_load || !app_code->hot_load ||
       !app_code->hot_unload || !app_code->update)
    {
        app_code->permanent_load = permanent_load_stub;
        app_code->hot_load = hot_load_stub;
        app_code->hot_unload = hot_unload_stub;
        app_code->update = update_stub;
        result = 0;
        goto end;
    }
    
    end:;
    return result;
}

internal void
win32_code_unload(Win32_Reload* app_code)
{
    if(app_code->dll)
    {
        FreeLibrary(app_code->dll);
    }
    app_code->dll = 0;
    app_code->permanent_load = permanent_load_stub;
    app_code->hot_load = hot_load_stub;
    app_code->hot_unload = hot_unload_stub;
    app_code->update = update_stub;
}

internal void
win32_code_update(Win32_Reload* app_code)
{
    FILETIME last_write_time = win32_get_last_write_time(global_app_dll_path);
    if(CompareFileTime(&last_write_time, &app_code->last_write_time))
    {
        app_code->hot_unload();
        win32_code_unload(app_code);
        win32_code_load(app_code);
        app_code->hot_load(&global_platform);
    }
}
