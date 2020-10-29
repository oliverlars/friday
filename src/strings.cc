
internal String8
make_string(char* string){
    String8 result = {};
    result.text = string;
    result.length = strlen(string);
    return result;
}


internal String8
make_stringf(Arena* arena, char* string, ...){
    va_list args;
    va_start(args, string);
    
    char* pointer = string;
    while(pointer && *pointer){
        pointer++;
    }
    
    char* text = (char*)arena_allocate(arena, pointer-string);
    int length = vsnprintf(text, 256, string, args);
    String8 result;
    result.text = text;
    result.length = length;
    return result;
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

