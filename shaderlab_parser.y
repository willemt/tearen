%{
#include <string.h>
#include <stdio.h>
#include "r_shader.h"

//    extern int yylex();
    void yyerror(void* scanner, const char *s) { printf("ERROR: %s\n", s); }
%}

%pure-parser 
%lex-param {void * scanner}
%parse-param {void * scanner}

%union {
    struct {
        int string_length;
        int string_pos;
        char* prog;
    } shader;
    int token;
    int integer;
    char* id;
}

%token <id> SYM_ID
%token <id> SYM_INTEGER SYM_DOUBLE
%token <token> SYM_OPEN SYM_CLOSE SYM_SET
%token <token> SYM_OPENC SYM_CLOSEC SYM_COMMA SYM_OPENS SYM_CLOSES
%token <token> SYM_PASS SYM_SUBSHADER SYM_PROPERTIES SYM_MATERIAL
%token <id> SYM_PROGGLSL_VERTEX SYM_PROGGLSL_FRAGMENT
%token <token> SYM_PROGGLSL
%token <token> SYM_BLENDSTATEMENT;


%start program

%%


program : id id SYM_OPEN block SYM_CLOSE { }
        ;
        
block
    :
    | SYM_PROPERTIES { } properties SYM_CLOSE block 
    | SYM_SUBSHADER { } subshader SYM_CLOSE block 
    ;

subshader
    :
    | stmt subshader
    | SYM_BLENDSTATEMENT SYM_ID SYM_ID {
        r_shader_set_blend(NULL,r_blend_str_to_int($2),r_blend_str_to_int($3));
        } subshader
    | SYM_PASS passdef SYM_CLOSE subshader
    | SYM_MATERIAL {} material SYM_CLOSE subshader
    ;

properties
    :
    | prop_function SYM_SET value prop_options properties { }
    ;

passdef
    :
    | stmt passdef
    | SYM_ID SYM_OPENS stmt SYM_CLOSES passdef
    | SYM_ID SYM_OPEN block SYM_CLOSE passdef
    | SYM_PROGGLSL_VERTEX {
        r_shader_pass_add_glsl(NULL,$1,R_GLSL_SHADER_VERTEX);
        } passdef 
    | SYM_PROGGLSL_FRAGMENT {
        r_shader_pass_add_glsl(NULL,$1,R_GLSL_SHADER_FRAGMENT);
        } passdef
    ;

material
    :
    | SYM_ID SYM_OPENS SYM_ID SYM_CLOSES material

value
    : SYM_ID
    | SYM_INTEGER
    | SYM_DOUBLE
    | SYM_OPENC tuple SYM_CLOSEC
    ;

tuple
    : func_params
    ;
    

prop_function
    : SYM_ID SYM_OPENC func_params SYM_CLOSEC {  }
    ;

prop_options
    :
    | SYM_OPEN stmt SYM_CLOSE
    ;

func_params
    : prop_function
    | value
    | func_params SYM_COMMA func_params
    ;

stmt
    :
    | id
    | id id
    | id id id
    ;

id
    : SYM_ID { }
    ;

%%

#include <stdio.h>

static char *file_contents(const char *filename, int *len_out)
{
    char *buffer = 0;
    long length;
    FILE *f = fopen(filename, "rb");

    if (f)
    {
	fseek(f, 0, SEEK_END);
	length = ftell(f);
	fseek(f, 0, SEEK_SET);
	buffer = malloc(length);
	*len_out = length;
	if (buffer)
	{
	    if (0 == fread(buffer, 1, length, f))
	    {

	    }
	}
	fclose(f);
    }
    return buffer;
}

int main(int argc, char **argv)
{
    ++argv, --argc;
    stdin = fopen( argv[0], "r" );

    char* f;
    int len;
    void* scanner;

    f = file_contents(argv[0],&len);

    yylex_init (& scanner);
    yy_scan_string(f,scanner);
    yyparse (scanner);
    yylex_destroy (scanner);
    return 0;
}
