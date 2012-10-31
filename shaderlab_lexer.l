/*** Definition section ***/
%{

#include "shaderlab_parser.h"
#include <assert.h>

/* C code to be copied verbatim */
#define SAVE_TOKEN yylval->id = strndup(yytext, yyleng)
#define TOKEN(t) (yylval->token = t)

#define shader_start()\
{\
    yylval->shader.string_pos = 0;\
    yylval->shader.string_length = 512;\
    yylval->shader.prog = malloc(yylval->shader.string_length);\
}\

#define output_shader()\
{\
    yylval->shader.string_length = yylval->shader.string_pos = 0;\
    TOKEN(SYM_PROGGLSL);\
    yylval->id = strdup(yylval->shader.prog);\
    free(yylval->shader.prog);\
}\

#define append_to_string(c)\
{\
    if (yylval->shader.string_pos >= yylval->shader.string_length)\
    {\
        assert(yylval->shader.string_length);\
        yylval->shader.string_length *= 2;\
        yylval->shader.prog = realloc(yylval->shader.prog,\
                                    yylval->shader.string_length);\
    }\
    yylval->shader.prog[yylval->shader.string_pos++] = (c);\
    yylval->shader.prog[yylval->shader.string_pos] = '\0';\
}\

%}

/* This tells flex to read only one input file */
%option noyywrap
%option reentrant
%option bison-bridge

DIGIT    [0-9]
ID       _?[a-zA-Z0-9]+

%x IN_PROGGLSL
%x IN_PROGGLSL_VERTEX
%x IN_PROGGLSL_FRAGMENT

%%

<IN_PROGGLSL_VERTEX>{
    #endif {
        BEGIN(IN_PROGGLSL);
        output_shader();
        return SYM_PROGGLSL_VERTEX;
    }
    . { append_to_string(*yytext); }
    \n { append_to_string('\n'); }
}

<IN_PROGGLSL_FRAGMENT>{
    #endif {
        BEGIN(IN_PROGGLSL);
        output_shader();
        return SYM_PROGGLSL_FRAGMENT;
    }
    . { append_to_string(*yytext); }
    \n { append_to_string('\n'); }
}


<IN_PROGGLSL>{
    #ifdef.VERTEX {
        BEGIN(IN_PROGGLSL_VERTEX);
        shader_start();
    }
    #ifdef.FRAGMENT {
        BEGIN(IN_PROGGLSL_FRAGMENT);
        shader_start();
    }
    ENDGLSL {
        BEGIN(INITIAL);
//        return SYM_PROGGLSL;
    }
    . {  }
    \n {  }
}

<INITIAL>GLSLPROGRAM {
    yylval->shader.string_pos = yylval->shader.string_length = 0;
    yylval->shader.prog = NULL;
    BEGIN(IN_PROGGLSL);
}


\"[a-zA-Z()0-9 ]*\" { SAVE_TOKEN; return SYM_ID; }

{DIGIT}+ { SAVE_TOKEN; return SYM_INTEGER; }

{DIGIT}*\.{DIGIT}+ { SAVE_TOKEN; return SYM_DOUBLE; }



= { return SYM_SET; }
\( { return SYM_OPENC; }
\) { return SYM_CLOSEC; }
\[ { return SYM_OPENS; }
\] { return SYM_CLOSES; }
, { return SYM_COMMA; }

Blend { return SYM_BLENDSTATEMENT; }

{ID} {
    //printf("ID: %s\n", yytext);
    SAVE_TOKEN;
    return SYM_ID;
}


Material.*\{ { return SYM_MATERIAL; } 
Properties.*\{ { return SYM_PROPERTIES; } 
Pass.*\{ {  return SYM_PASS; } 
SubShader.*\{ { return SYM_SUBSHADER; } 

\{ { return SYM_OPEN; } 
\} { return SYM_CLOSE; }

\/\/.* {}

. {}

 /*.  printf("Unknown token!\n"); yyterminate();*/

%%
