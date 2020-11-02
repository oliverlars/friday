
internal String8
make_string(char* string){
    String8 result = {};
    result.text = string;
    result.length = strlen(string);
    return result;
}

internal char*
cstr_to_string(Arena* arena, String8 string){
    char* result = (char*)arena_allocate(arena, string.length+1);
    result[string.length] = 0;
    for(int i = 0; i < string.length; i++){
        result[i] = string.text[i];
    }
    
    return result;
}

internal String8
make_stringfv(Arena *arena, char *format, va_list args)
{
    va_list args2;
    va_copy(args2, args);
    u32 needed_bytes = vsnprintf(0, 0, format, args) + 1;
    String8 result = {0};
    result.text = (char*)arena_allocate(arena, needed_bytes);
    result.length = vsnprintf((char*)result.text, needed_bytes, format, args2);
    result.text[result.length] = 0;
    return(result);
}

internal String8
make_stringf(Arena *arena, char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    String8 result = make_stringfv(arena, fmt, args);
    va_end(args);
    return(result);
}

internal bool
string_eq(String8 a, char* b){
    if(!a.text || !a.length) return false;
    if(!b || !(*b)) return false;
    int b_length = strlen(b);
    int min_length = a.length > b_length ? b_length : a.length;
    
    for(int i = 0; i < min_length; i++){
        if(a.text[i] != b[i]){
            return false;
        }
    }
    
    return true;
}

internal bool
string_eq(String8 a, String8 b){
    if(!a.text || !a.length) return false;
    if(!b.text || !b.length) return false;
    int min_length = a.length > b.length ? b.length : a.length;
    for(int i = 0; i < min_length; i++){
        if(a.text[i] != b.text[i]){
            return false;
        }
    }
    
    return true;
}

internal int
string_to_int(String8 string){
    
    int result = 0;
    bool is_negative = string.text[0] == '-';
    for(int i = is_negative; i < string.length; i++){
        result = result*10 + (string.text[i] - '0');
    }
    
    if(is_negative){
        result *= -1;
    }
    return result;
}

internal String8
string_from_cstr(char* string){
    String8 result = {};
    result.text = string;
    result.length = strlen(string);
    return result;
}
