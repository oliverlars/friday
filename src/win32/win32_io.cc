internal char *
win32_cstr_from_string(String8 string)
{
    char *buffer = 0;
    buffer = (char*)arena_allocate(&platform->temporary_arena, string.length+1);
    memcpy(buffer, string.text, string.length);
    return buffer;
}

internal void
win32_save_to_file(String8 path, void* data, u64 data_len)
{
    HANDLE file = {0};
    {
        DWORD desired_access = GENERIC_READ | GENERIC_WRITE;
        DWORD share_mode = 0;
        SECURITY_ATTRIBUTES security_attributes = {
            (DWORD)sizeof(SECURITY_ATTRIBUTES),
            0,
            0,
        };
        DWORD creation_disposition = CREATE_ALWAYS;
        DWORD flags_and_attributes = 0;
        HANDLE template_file = 0;
        
        if((file = CreateFile(win32_cstr_from_string(path),
                              desired_access,
                              share_mode,
                              &security_attributes,
                              creation_disposition,
                              flags_and_attributes,
                              template_file)) != INVALID_HANDLE_VALUE) {
            
            void *data_to_write = data;
            DWORD data_to_write_size = (DWORD)data_len;
            DWORD bytes_written = 0;
            
            WriteFile(file, data_to_write, data_to_write_size, &bytes_written, 0);
            
            CloseHandle(file);
        }
        else {
            win32_output_error("File I/O Error", "Could not save to \"%s\"", path);
        }
    }
}

internal void
win32_append_to_file(String8 path, void* data, u64 data_len) {
    HANDLE file = {0};
    {
        DWORD desired_access = FILE_APPEND_DATA;
        DWORD share_mode = 0;
        SECURITY_ATTRIBUTES security_attributes = {
            (DWORD)sizeof(SECURITY_ATTRIBUTES),
            0,
            0,
        };
        DWORD creation_disposition = OPEN_ALWAYS;
        DWORD flags_and_attributes = 0;
        HANDLE template_file = 0;
        
        if((file = CreateFileA(win32_cstr_from_string(path),
                               desired_access,
                               share_mode,
                               &security_attributes,
                               creation_disposition,
                               flags_and_attributes,
                               template_file)) != INVALID_HANDLE_VALUE) {
            
            void *data_to_write = data;
            DWORD data_to_write_size = (DWORD)data_len;
            DWORD bytes_written = 0;
            
            SetFilePointer(file, 0, 0, FILE_END);
            WriteFile(file, data_to_write, data_to_write_size, &bytes_written, 0);
            
            CloseHandle(file);
        }
        else {
            win32_output_error("File I/O Error", "Could not save to \"%s\"", path);
        }
    }
}

internal void
win32_load_entire_file(Arena* arena, String8 path, void** data, u64* data_len) {
    
    *data = 0;
    *data_len = 0;
    
    HANDLE file = {0};
    
    {
        DWORD desired_access = GENERIC_READ | GENERIC_WRITE;
        DWORD share_mode = 0;
        SECURITY_ATTRIBUTES security_attributes = {
            (DWORD)sizeof(SECURITY_ATTRIBUTES),
            0,
            0,
        };
        DWORD creation_disposition = OPEN_EXISTING;
        DWORD flags_and_attributes = 0;
        HANDLE template_file = 0;
        
        if((file = CreateFile(win32_cstr_from_string(path), desired_access, share_mode, &security_attributes, creation_disposition, flags_and_attributes, template_file)) != INVALID_HANDLE_VALUE) {
            
            DWORD read_bytes = GetFileSize(file, 0);
            if(read_bytes)
            {
                void *read_data = arena_allocate(arena, read_bytes+1);
                DWORD bytes_read = 0;
                OVERLAPPED overlapped = {0};
                
                ReadFile(file, read_data, read_bytes, &bytes_read, &overlapped);
                
                ((u8 *)read_data)[read_bytes] = 0;
                
                *data = read_data;
                *data_len = (u64)bytes_read;
            }
            CloseHandle(file);
        }
    }
}

internal char *
win32_load_file_and_null_terminate(Arena* arena, String8 path)
{
    char *result = 0;
    
    HANDLE file = {0};
    {
        DWORD desired_access = GENERIC_READ | GENERIC_WRITE;
        DWORD share_mode = 0;
        SECURITY_ATTRIBUTES security_attributes = {
            (DWORD)sizeof(SECURITY_ATTRIBUTES),
            0,
            0,
        };
        DWORD creation_disposition = OPEN_EXISTING;
        DWORD flags_and_attributes = 0;
        HANDLE template_file = 0;
        
        if((file = CreateFile(win32_cstr_from_string(path), desired_access, share_mode, &security_attributes, creation_disposition, flags_and_attributes, template_file)) != INVALID_HANDLE_VALUE)
        {
            
            DWORD read_bytes = GetFileSize(file, 0);
            if(read_bytes)
            {
                result = (char*)arena_allocate(arena, read_bytes+1);
                DWORD bytes_read = 0;
                OVERLAPPED overlapped = {0};
                
                ReadFile(file, result, read_bytes, &bytes_read, &overlapped);
                
                result[read_bytes] = 0;
            }
            CloseHandle(file);
        }
        else
        {
            win32_output_error("File I/O Error", "Could not read from \"%s\"", path);
        }
    }
    
    return result;
}

internal void
win32_free_file_memory(void *data)
{
    win32_heap_free(data);
}

internal void
win32_delete_file(String8 path)
{
    DeleteFileA(win32_cstr_from_string(path));
}

internal b32
win32_make_directory(String8 path)
{
    b32 result = 1;
    if(!CreateDirectoryA(win32_cstr_from_string(path), 0))
    {
        result = 0;
    }
    return result;
}

internal b32
win32_does_file_exist(String8 path)
{
    b32 found = GetFileAttributesA(win32_cstr_from_string(path)) != INVALID_FILE_ATTRIBUTES;
    return found;
}

internal b32
win32_does_director_exist(String8 path)
{
    DWORD file_attributes = GetFileAttributesA(win32_cstr_from_string(path));
    b32 found = (file_attributes != INVALID_FILE_ATTRIBUTES &&
                 !!(file_attributes & FILE_ATTRIBUTE_DIRECTORY));
    return found;
}

internal b32
win32_copy_file(String8 dest, String8 source)
{
    b32 success = 0;
    success = CopyFile(win32_cstr_from_string(source), win32_cstr_from_string(dest), 0);
    return success;
}
