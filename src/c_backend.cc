// HACK(Oliver): temporary hack to get the linked list program working

internal void
compile_c_helper(FILE* file, Arc_Node* node, int indent = 0){
    if(!node) return;
    
    switch(node->ast_type){
        case AST_DECLARATION: {
            compile_c_helper(file, node->first_child->first_child);
            fprintf(file, " %.*s", expand_string(node->string));
            fprintf(file, " = ");
            if(node->last_child->first_child){
                if(node->last_child->first_child->string.length){
                    compile_c_helper(file, node->last_child->first_child);
                }else {
                    fprintf(file, "0");
                }
            }
            fprintf(file, ";\n");
        }break;
        case AST_ASSIGNMENT:{
            compile_c_helper(file, node->first_child);
            fprintf(file, " = ");
            compile_c_helper(file, node->last_child);
            fprintf(file, ";\n");
        }break;
        case AST_STRUCT: {
            fprintf(file, "struct ");
            fprintf(file, "%.*s {\n", expand_string(node->string));
            compile_c_helper(file, node->first_child, indent+1);
            fprintf(file, "};\n\n");
        }break;
        
        case AST_FUNCTION: {
            auto type = node->first_child->next_sibling->first_child;
            
            auto scope = node->last_child;
            if(type->string.length){
                compile_c_helper(file, type, indent);
            }else {
                fprintf(file, "void ");
            }
            fprintf(file, " %.*s(", expand_string(node->string));
            fprintf(file, ") {\n");
            compile_c_helper(file, scope, indent+1);
            fprintf(file, "}\n\n");
            
        }break;
        case AST_TYPE_USAGE: {
            
        }break;
        case AST_FOR: {
            
        }break;
        case AST_IF: {
            
        }break;
        case AST_NEW: {
            fprintf(file, "(");
            compile_c_helper(file, node->first_child);
            fprintf(file, "*)");
            fprintf(file, "malloc(sizeof(");
            
            compile_c_helper(file, node->first_child);
            fprintf(file, "));\n\n");
        }break;
        case AST_WHILE: {
            fprintf(file, " %.*s(", expand_string(node->string));
            compile_c_helper(file, node->first_child->first_child);
            fprintf(file, ") {\n");
            compile_c_helper(file, node->last_child);
            fprintf(file, "}\n\n");
        }break;
        case AST_SCOPE: {
            auto member = node->first_child;
            while(member){
                compile_c_helper(file, member);
                member = member->next_sibling;
            }
        }break;
        case AST_EXPR:{
            compile_c_helper(file, node->first_child);
        }break;
        case AST_TOKEN: {
            if(node->token_type == TOKEN_STRING){
                fprintf(file, "\"");
                fprintf(file, "%.*s", expand_string(node->string));
                fprintf(file, "\"");
            }else {
                fprintf(file, "%.*s", expand_string(node->string));
            }
            if(node->next_sibling && node->next_sibling->token_type == TOKEN_REFERENCE){
                if(node->reference->first_child->first_child->number_of_pointers > 0){
                    fprintf(file, "->");
                }else {
                    fprintf(file, ".");
                }
            }
            compile_c_helper(file, node->next_sibling);
        }break;
        case AST_TYPE_TOKEN: {
            fprintf(file, "%.*s ", expand_string(node->string));
            
            for(int i = 0; i < node->number_of_pointers; i++){
                fprintf(file, "*");
            }
            fprintf(file, " ");
            compile_c_helper(file, node->next_sibling);
            
        }break;
        case AST_CALL:{
            if(string_eq(node->string, "print")){
                fprintf(file, "printf(");
            }else {
                fprintf(file, "%.*s", expand_string(node->string));
            }
            auto arg = node->first_child->first_child;
            for(auto expr = arg; expr; expr = expr->next_sibling){
                compile_c_helper(file, expr->first_child);
                if(expr->next_sibling && expr->next_sibling->first_child &&
                   (expr->next_sibling->first_child->string.length)){
                    fprintf(file, ", ");
                }
            }
            fprintf(file, ");\n");
        }break;
        case AST_USING: {
            
        }break;
        case AST_RETURN:{
            
        }break;
        default: {
            compile_c_helper(file, node->first_child);
            compile_c_helper(file, node->next_sibling);
        }break;
    }
    
}

internal void
compile_c(Arc_Node* node){
    FILE* file = fopen("out.cc", "a");
    fprintf(file, "#include <stdlib.h>\n");
    fprintf(file, "#include <stdio.h>\n");
    fprintf(file, "#include <inttypes.h>\n");
    
    fprintf(file, "typedef uint8_t u8;\n");
    fprintf(file, "typedef uint16_t u16;\n");
    fprintf(file, "typedef uint32_t u32;\n");
    fprintf(file, "typedef uint64_t u64;\n");
    
    fprintf(file, "typedef int8_t s8;\n");
    fprintf(file, "typedef int16_t s16;\n");
    fprintf(file, "typedef int32_t s32;\n");
    fprintf(file, "typedef int64_t s64;\n");
    
    fprintf(file, "typedef s32 b32;\n");
    fprintf(file, "typedef s32 b32x;\n");
    
    fprintf(file, "typedef float f32;\n");
    fprintf(file, "typedef double f64;\n");
    
    fprintf(file, "\n\n\n\n");
    compile_c_helper(file, node);
    fprintf(file, "int main() { entry(); return 0; }");
    fclose(file);
}