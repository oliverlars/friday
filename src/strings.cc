
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
