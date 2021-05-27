
internal String8
make_string(char* string){
    String8 result = {};
    result.text = string;
    result.length = strlen(string);
    return result;
}

internal String8
make_stringfv(Arena *arena, char *format, va_list args)
{
    va_list args2;
    va_copy(args2, args);
    u32 needed_bytes = vsnprintf(0, 0, format, args) + 1;
    String8 result = {0};
    result.text = (char*)push_size(arena, needed_bytes);
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

internal String8
make_stringfv(Pool* pool, char *format, va_list args)
{
    va_list args2;
    va_copy(args2, args);
    u32 needed_bytes = vsnprintf(0, 0, format, args) + 1;
    assert(needed_bytes < pool->chunk_size);
    String8 result = {0};
    result.text = (char*)pool_allocate(pool);
    result.length = vsnprintf((char*)result.text, needed_bytes, format, args2);
    result.text[result.length] = 0;
    return(result);
}

internal String8
make_stringf(Pool* pool, char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    String8 result = make_stringfv(pool, fmt, args);
    va_end(args);
    return(result);
}

internal void
replace_string(String8* string, String8 replacement){
    for(int i = 0; i < replacement.length; i++){
        string->text[i] = replacement.text[i];
    }
    string->length = replacement.length;
}

internal String8
copy_string(Arena* arena, String8 string){
    return make_stringf(arena, "%.*s", string.length, string.text);
}

internal String8
cstr_to_string(Arena* arena, char* string){
    return make_stringf(arena, "%s", string);
}

internal char*
make_cstr(Arena* arena, char* string){
    int length = strlen(string);
    char* buffer = (char*)push_size_zero(arena, length);
    memcpy(buffer, string, length);
    return buffer;
}

internal void
insert_in_string(String8* string, char* insertable, u64 index){
    if(!insertable) return;
    int length = strlen(insertable);
    if(string->length){
        for(int i = string->length-1 + length; i > index; i--){
            string->text[i] = string->text[i-length];
        }
    }
    
    for(int i = index; i < index+length; i++){
        string->text[index] = *insertable++;
    }
    string->length += length;
}

internal void
insert_in_string(String8* string, char insertable, u64 index){
    if(string->length){
        for(int i = string->length; i > index; i--){
            string->text[i] = string->text[i-1];
        }
    }
    string->text[index] = insertable;
    string->length++;
}

internal void
pop_from_string(String8* string, u64 index){
    if(string->length == 0) return;
    for(int i = index; i < string->length; i++){
        string->text[i-1] = string->text[i];
    }
    string->length--;
}

internal bool
string_eq(String8 a, char* b){
    if(!a.text || !a.length) return false;
    if(!b || !(*b)) return false;
    int b_length = strlen(b);
    if(a.length != b_length) return false;
    
    for(int i = 0; i < a.length; i++){
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
    if(a.length != b.length) return false;
    for(int i = 0; i < a.length; i++){
        if(a.text[i] != b.text[i]){
            return false;
        }
    }
    
    return true;
}

internal bool
is_strict_substring(String8 a, String8 b){
    if(a.length == 0) return false;
    if(a.length >= b.length) return false;
    for(int i = 0; i < a.length; i++){
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


internal void
advance_lexer(Lexer* l){
    l->pos++;
}

#define is_newline(x) ((x) == '\n' || (x) == '\r')
#define is_whitespace(x) ((x) == ' ' || (x) == '\t' || (x) == '\v' || (x) == '\f' || is_newline(x))
#define is_digit(x) ((x) <= '9' && (x) >= '0')
#define is_upper_hex(x) (is_digit(x) || ((x) <= 'F' && (x) >= 'A'))
#define is_lower_hex(x) (is_digit(x) || ((x) <= 'f' && (x) >= 'a'))
#define is_hex(x) (is_upper(x)  || is_lower_hex(x))
#define is_lower_alpha(x) ((x) <= 'z' && (x) >= 'a')
#define is_upper_alpha(x) ((x) <= 'Z' && (x) >= 'A')
#define is_alpha(x) ((is_lower_alpha(x) || is_upper_alpha(x)))

internal void
gobble_whitespace(Lexer* l){
    while(is_whitespace(*l->pos)){
        advance_lexer(l);
    }
}

internal String8
read_token(Lexer* l){
    gobble_whitespace(l);
    
    char c = *l->pos;
    String8 token = {};
    token.text = l->pos;
    token.length = 1;
    advance_lexer(l);
    switch(c){
        case '\0':{}break;
        case ':':{} break;
        case ',':{} break;
        case '=':{} break;
        case '\"':{
            while(*l->pos != '\"'){
                advance_lexer(l);
            }
            token.text++;
            token.length = l->pos - token.text;
            advance_lexer(l);
        } break;
        
        default:{
            if(is_alpha(c)){
                while(is_alpha(*l->pos)){advance_lexer(l); }
                token.length = l->pos - token.text;
            }else if(is_digit(c)){
                while(is_digit(*l->pos)){ advance_lexer(l); }
                token.length = l->pos - token.text;
            }else if(c == '-'){
                while(is_digit(*l->pos)){ advance_lexer(l); }
                token.length = l->pos - token.text;
            }
        }break;
    }
    return token;
}

internal void
expect_token(Lexer* l, char* string){
    assert(string_eq(read_token(l), string));
}
