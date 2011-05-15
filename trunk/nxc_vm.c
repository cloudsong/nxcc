/**
 * this file implement all ...
 * @date:2010-8-31
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS HEADER.
 *
 * Copyright 2011 songyuan, aboutin.me@gmail.com
 *
 * 
 * NXCC is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * NXCC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with NXCC.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "nxc_vm.h"

/**===========================================================================*/
/**============================== Lexer ======================================*/
/**===========================================================================*/

/**
 * token-name mapping table ...
 */
char *nxc_token_name_table[]=
{
    "bad",
    "sizeof",
    
    "var",
    "func",
    "typedef",
    "struct",
    "class",
    "union",
    "const",
    "api",
    
    "id",           ///identifier:var",function",typename...

    "const_long",
    "const_float",
    "const_string", ///64bit unsigned

    "goto",
    "break",
    "continue",

    "if",
    "else",

    "for",
    "do",
    "while",

    "return",
    ///-------------assignment_operators------------
    "=",            ///"="
    "+= ",
    "-= ",
    "*= ",
    "/= ",
    "%= ",
    "|= ",
    "&= ",
    "^= ",
    "<<=",
    ">>=",
    ///--------------binary_operators---------------
    "||",           ///"||"
    "&&",           ///"&&"
    "|",            ///"|"
    "&",            ///"&"
    "^",            ///"^"
    "==",           ///"=="
    "!=",           ///"!="
    ">",            ///">"
    "<",            ///"<"
    ">=",           ///">="
    "<=",           ///"<="
    "<<",           ///"<<"
    ">>",           ///">>"
    "+",            ///"+"
    "-",            ///"-"
    "*",            ///"*"
    "/",            ///"/"
    "%",            ///"%"
    ///---------------unary_operator---------------
    "cast",         ///"cast"
    "++",           ///"++"
    "--",           ///"--"
    "!",            ///"!"
    "~",            ///"~"
    "@",            ///@  dummy stub ...
    "(",            ///"("
    ")",            ///")"
    "[",            ///"["
    "]",            ///"]"
    "[[",           ///"[["
    "]]",           ///"]]"
    "[[[",          ///"[["
    "]]]",          ///"]]"
    "{",            ///"{"
    "}",            ///"}"
    ".",            ///"."
    "->",           ///"->"
    "...",          ///"..."
    ",",            ///","
    ":",            ///":"
    ";",            ///";"
                    ///EOF="\x80"
};

___fast char *nxc_get_token_name(int token)
{
    if(token == nxc_token_eof) return "___EOF___";
    if(token<0 || token > nxc_token_semicolon) return "N/A";
    return nxc_token_name_table[token];
}

/**===========================================================================*/

/**
 * show fatal error here ...
 */
static void nxc_error(nxc_compiler_t* const compiler, const char *format, ...)
{
    nxc_va_list ap;
    nxc_lexer_t *lexer;
    
    lexer = compiler->lexer;
    nxc_printf(compiler,"(%s,%d):Error:",lexer->fname?lexer->fname:"N/A",lexer->line_number+1);
    nxc_va_start(ap, format);
    nxc_vprintf(compiler, format, ap);
    nxc_printf(compiler, "\n");
    nxc_va_end(ap);
}


/**
 * show fatal error for given ast node ...
 */
static void nxc_ast_error(nxc_compiler_t *const compiler,
                          nxc_ast_t *ast,
                          const char *format,
                          ...)
{
    nxc_va_list ap;

    if(ast){
        nxc_printf(compiler, "(%s,%d):",ast->fname?ast->fname:"N/A",ast->line_number+1);
    }else{
        nxc_printf(compiler, "fatal:");
    }
    nxc_printf(compiler, "error:");
    nxc_va_start(ap, format);
    nxc_vprintf(compiler, format, ap);
    nxc_printf(compiler, "\n");
    nxc_va_end(ap);
}



















/**===========================================================================*/
///conver string to float ...
double nxc_atof(char *s)
{
    double a = 0.0;
    int e = 0;
    int c;
    while ((c = *s++) != '\0' && nxc_isdigit(c)) {
        a = a*10.0 + (c - '0');
    }
    if (c == '.') {
        while ((c = *s++) != '\0' && nxc_isdigit(c)) {
            a = a*10.0 + (c - '0');
            e = e-1;
        }
    }
    if (c == 'e' || c == 'E') {
        int sign = 1;
        int i = 0;
        c = *s++;
        if (c == '+')
            c = *s++;
        else if (c == '-') {
            c = *s++;
            sign = -1;
        }
        while (nxc_isdigit(c)) {
            i = i*10 + (c - '0');
            c = *s++;
        }
        e += i*sign;
    }
    while (e > 0) {
        a *= 10.0;
        e--;
    }
    while (e < 0) {
        a *= 0.1;
        e++;
    }
    return a;
}

/**===========================================================================*/

/**
 * scan space white ...
 */
static int nxc_lex_scan_space(nxc_lexer_t *lexer) 
{ 
    int c;
    do 
    {
        c = nxc_lex_getc(lexer); 
        if (c == '/') ///comment 1
        { 
            c = nxc_lex_getc(lexer); 
            if (c == '/')      ///style ' / / '
            {
                do{c = nxc_lex_getc(lexer);}while(c && c != '\n' && c != '\r'); 
            }
            else if (c == '*') ///stype '/ *'
            {
                while (1) 
                { 
                    c = nxc_lex_getc(lexer); 
                    if (c == 0) break;
                    if (c == '*') 
                    {
                        if ((c = nxc_lex_getc(lexer)) == '/') 
                        {
                            c = ' '; 
                            break; 
                        }
                        else 
                            nxc_lex_ungetc(lexer); 
                    }
                } 
            }
            else
            { 
                nxc_lex_ungetc(lexer); 
                c = '/'; 
            } 
        }///end of  [ if (c == '/') ]
        else if (c == '#') ///treat as comment ...
        {
            do{c = nxc_lex_getc(lexer);}while(c && c != '\n' && c != '\r'); 
        }
    } while (nxc_isspace(c));

    nxc_lex_ungetc(lexer);
    return c; 
} 

/**
 * scan '\XX' {'\r' '\n' '\t' ...}
 */
static int nxc_lex_scan_escap_char(nxc_lexer_t *lexer)
{
    int v;
    int c;

    c = nxc_lex_getc(lexer);
    switch (c)
    {
    case 'a':       return '\a';
    case 'b':       return '\b';
    case 'f':       return '\f';
    case 'n':       return '\n';
    case 'r':       return '\r';
    case 't':       return '\t';
    case 'v':       return '\v';
    case '\'':
    case '"':
    case '\\':
    case '\?':      return c;
    case 'x':
        /** \xAB */
        c = nxc_lex_getc(lexer);
        if (! nxc_ishexdigit(c))
        {
            nxc_lex_set_err_msg(lexer,"Expect hex digit");
            return 'x';
        }
        v = 0;
        ///first Hex char ...
        if (nxc_isdigit(c)) 
            v = (v << 4) + c - '0';
        else 
            v = (v << 4) + nxc_toupper(c) - 'A' + 10;
        ///second Hex Char ...
        c = nxc_lex_getc(lexer);
        if(nxc_ishexdigit(c))
        {
            if (nxc_isdigit(c))
                v = (v << 4) + c - '0';
            else
                v = (v << 4) + nxc_toupper(c) - 'A' + 10;
        }
        else{
            nxc_lex_ungetc(lexer);
        }
        return v;
    case'0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':
        v = c - '0';
        c = nxc_lex_getc(lexer);
        if (nxc_isoctdigit(c))
        {
            v = (v << 3) + c - '0';
            c = nxc_lex_getc(lexer);
            if (nxc_isoctdigit(c))
                v = (v << 3) + c - '0';
            else
                nxc_lex_ungetc(lexer);
        }
        else{
            nxc_lex_ungetc(lexer);
        }
        return v;
    default:
        nxc_lex_set_err_msg(lexer, "Unrecognized escape sequence:");
        return c;
    }
}

/**===========================================================================*/

/**
 * scan a float and return it's token type ...
 */
static int nxc_lex_scan_float(nxc_lexer_t *lexer)
{
    int c;

    for (; ;) { 
        c = nxc_lex_getc(lexer);
        if ('0' <= c && c <= '9') 
            nxc_lex_push_char(lexer,c);
        else 
            break; 
    } 
    
    if (c == 'E' || c == 'e') { 
        nxc_lex_push_char(lexer,c);
        c = nxc_lex_getc(lexer);
        if (c == '+' || c == '-') { 
            nxc_lex_push_char(lexer,c);
            c = nxc_lex_getc(lexer);
        } 
        
        while ('0' <= c && c <= '9') { 
            nxc_lex_push_char(lexer,c);
            c = nxc_lex_getc(lexer);
        } 
    }
    if (c != 'F' && c != 'f') {
        nxc_lex_ungetc(lexer); 
    }
    
    ///>>>save value ...
    nxc_lex_push_char(lexer,0);//zero terminated it ...
    nxc_token_set_float_val(lexer->token,(float)nxc_atof(lexer->parse_buff));
    nxc_token_set_len(lexer->token,sizeof(float));
    
    return nxc_token_const_float;
}

/**===========================================================================*/

/**
 * scan '%char%' ...
 */
___fast int nxc_lex_scan_char_number(nxc_lexer_t *lexer)
{
    int val,c;

    val = 0;
    c = nxc_lex_getc(lexer);
    while (c != '\'') 
    {
        if (c == '\n' || c == 0) break;
        if (c == '\\') c = nxc_lex_scan_escap_char(lexer);
        val = (val<<8) | c;
        c = nxc_lex_getc(lexer);
    }
    if (c != '\'') 
    {
        nxc_lex_set_err_msg(lexer,"Expect '");  
        return nxc_token_bad;
    }

    ///>>>save value ...
    nxc_token_set_long_val(lexer->token,val);
    nxc_token_set_len(lexer->token,sizeof(val));

    return nxc_token_const_long;
}

/**
 * scan a number ...
 */
static int nxc_lex_scan_number(nxc_lexer_t *lexer,int c) 
{ 
    long value = 0; 
    int c2;

    ///get next ...
    c2 = nxc_lex_getc(lexer);
    if (c == '0') 
    {
        if (c2 == 'X' || c2 == 'x') ///hex
        { 
            for (; ;) 
            { 
                c = nxc_lex_getc(lexer); 
                if ('0' <= c && c <= '9') 
                    value = value * 16 + (long) (c - '0'); 
                else if ('A' <= c && c <= 'F') 
                    value = value * 16 + (long) (c - 'A' + 10); 
                else if ('a' <= c && c <= 'f') 
                    value = value * 16 + (long) (c - 'a' + 10); 
                else 
                { 
                    ///save value ...
                    nxc_token_set_long_val(lexer->token,value);
                    nxc_token_set_len(lexer->token,sizeof(value));
                    if (c == 'L' || c == 'l'){
                        return nxc_token_const_long; 
                    }
                    else 
                    { 
                        nxc_lex_ungetc(lexer); 
                        return nxc_token_const_long;
                    } 
                } 
            } 
        }
        else if ('0' <= c2 && c2 <= '7') ///octect
        { 
            value = c2 - '0'; 
            for (; ;) 
            { 
                c = nxc_lex_getc(lexer); 
                if ('0' <= c && c <= '7') 
                {
                    value = value * 8 + (long) (c - '0'); 
                }
                else 
                { 
                    ///save value ...
                    nxc_token_set_long_val(lexer->token,value);
                    nxc_token_set_len(lexer->token,sizeof(value));

                    if (c == 'L' || c == 'l') {
                        return nxc_token_const_long/*LongConstant*/; 
                    }
                    else 
                    { 
                        nxc_lex_ungetc(lexer); 
                        return nxc_token_const_long/*IntConstant*/; 
                    } 
                } 
            } 
        }
    }
    
    ///push 'c' incase of float ...
    nxc_lex_push_char(lexer,c);

    value = c - '0';
    while ('0' <= c2 && c2 <= '9') 
    { 
        ///push c2 ...
        nxc_lex_push_char(lexer,c2);

        value = value * 10 + c2 - '0'; 
        c2 = nxc_lex_getc(lexer); 
    } 
    
    ///save value ...
    nxc_token_set_long_val(lexer->token,value);
    nxc_token_set_len(lexer->token,sizeof(value));

    if (c2 == 'F' || c2 == 'f') 
    { 
        ///conver to float ...
        nxc_token_set_float_val(lexer->token,(float)value);
        nxc_token_set_len(lexer->token,sizeof(value));

        return nxc_token_const_float/*FloatConstant*/; 
    }
    else if (c2 == '.') { 
        ///append the '.'
        nxc_lex_push_char(lexer,'.');
        ///goto scan-float ...
        return nxc_lex_scan_float(lexer);
    }
    else if (c2 == 'L' || c2 == 'l'){
        return nxc_token_const_long/*LongConstant*/; 
    }

    nxc_lex_ungetc(lexer); 
    return nxc_token_const_long/*IntConstant*/; 
}

/**
 * scan a string ...
 * return token type ...
 */
___fast int nxc_lex_scan_string(nxc_lexer_t *lexer)
{
    int c;

    ///reset name-buffer ...
    for(;;)
    {
        c = nxc_lex_getc(lexer);
        while (c != '"')
        {
            if (c == '\n' || c == 0) break;
            if(c=='\\') c = nxc_lex_scan_escap_char(lexer);
            nxc_lex_push_char(lexer,c);
            c = nxc_lex_getc(lexer);
        }
        if (c != '"')
        {
            nxc_lex_set_err_msg(lexer,"Expect '\"'");
            return nxc_token_bad;
        }

        nxc_lex_scan_space(lexer); ///skip $space ...

        /**
         * combine string segment ...
         */
        c = nxc_lex_getc(lexer);
        if(c != '"'){
            nxc_lex_ungetc(lexer);
            break;
        }
    }

    ///set value ...
    __nxc_token_set_str_val(lexer->token,lexer->parse_buff);
    nxc_token_set_len(lexer->token,lexer->name_len);

    return nxc_token_const_string;
}

/**
 * scan a identifier , filter the keywords ...
 * @c : prefetched char ...
 */
___fast int nxc_lex_scan_identifier(nxc_lexer_t *lexer,int c)
{
    int c1,c2;
    while (nxc_is_letter_or_digit(c))
    {
        nxc_lex_push_char(lexer,c);///push char
        c = nxc_lex_getc(lexer);
    }
    if (c==':')
    {
        c1 = nxc_lex_getc(lexer);
        c2 = nxc_lex_getc(lexer);
        if (c1 == ':' && nxc_isletter(c2))
        {
            nxc_lex_push_char(lexer,':');///push char
            nxc_lex_push_char(lexer,':');///push char
            while (nxc_is_letter_or_digit(c2))
            {
                nxc_lex_push_char(lexer,c2);///push char
                c2 = nxc_lex_getc(lexer);
            }
        }
        else
        {
            nxc_lex_ungetc(lexer);///rollback
            nxc_lex_ungetc(lexer);///rollback
        }
    }
    nxc_lex_ungetc(lexer);///rollback

    ///set value ...
    __nxc_token_set_str_val(lexer->token,lexer->parse_buff);///internal only ...
    nxc_token_set_len(lexer->token,lexer->name_len);

    return nxc_token_id;
}

/**===========================================================================*/

/**
 * scan '=' '=='
 */
___fast int nxc_lex_scan_equal(nxc_lexer_t *lexer)
{
    if(nxc_lex_getc(lexer) == '=')  return nxc_token_equal;
    nxc_lex_ungetc(lexer);
    return nxc_token_assign;
}

/**
 * scan '+' '++' '+='
 */
___fast int nxc_lex_scan_plus(nxc_lexer_t *lexer)
{
    int c;

    c = nxc_lex_getc(lexer);
    if (c == '+')   return nxc_token_inc;
    if (c == '=')   return nxc_token_add_assign;
    nxc_lex_ungetc(lexer);
    return nxc_token_add;
}

/**
 * scan '-' '--' '-=' '->'
 */
___fast int nxc_lex_scan_minus(nxc_lexer_t *lexer)
{
    int c;
    
    c = nxc_lex_getc(lexer);
    if (c == '-')   return nxc_token_dec;
    if (c == '=')   return nxc_token_sub_assign;
    if (c == '>')   return nxc_token_ptr_right;
    nxc_lex_ungetc(lexer);
    return nxc_token_sub;
}

/**
 * scan '*' '*='
 */
___fast int nxc_lex_scan_star(nxc_lexer_t *lexer)
{
    if (nxc_lex_getc(lexer) == '=') return nxc_token_mul_assign;
    nxc_lex_ungetc(lexer);
    return nxc_token_mul;
}

/**
 * scan '/' '/='
 */
___fast int nxc_lex_scan_slash(nxc_lexer_t *lexer)
{
    if (nxc_lex_getc(lexer) == '=') return nxc_token_div_assign;
    nxc_lex_ungetc(lexer);
    return nxc_token_div;
}

/**
 * scan '%' '%='
 */
___fast int nxc_lex_scan_percent(nxc_lexer_t *lexer)
{
    if (nxc_lex_getc(lexer) == '=') return nxc_token_mod_assign;
    nxc_lex_ungetc(lexer);
    return nxc_token_mod;
}

/**
 * scan '|' '||' '|='
 */
___fast int nxc_lex_scan_or(nxc_lexer_t *lexer)
{
    int c;
    c = nxc_lex_getc(lexer);
    if (c == '|')   return nxc_token_or;
    if (c == '=')   return nxc_token_bitor_assign;
    nxc_lex_ungetc(lexer);
    return nxc_token_bit_or;
}

/**
 * scan '&' '&&' '&='
 */
___fast int nxc_lex_scan_and(nxc_lexer_t *lexer)
{
    int c;

    c = nxc_lex_getc(lexer);
    if (c == '&')   return nxc_token_and;
    if (c == '=')   return nxc_token_bitand_assign;
    nxc_lex_ungetc(lexer);
    return nxc_token_bit_and;
}

/**
 * scan '^' '^='
 */
___fast int nxc_lex_scan_caret(nxc_lexer_t *lexer)
{
    if (nxc_lex_getc(lexer) == '=') return nxc_token_xor_assign;
    nxc_lex_ungetc(lexer);
    return nxc_token_xor;
}

/**
 * scan '<' '<<' '<<=' '<='
 */
___fast int nxc_lex_scan_less(nxc_lexer_t *lexer)
{
    int c;

    c = nxc_lex_getc(lexer);
    if (c == '<')
    {
        c = nxc_lex_getc(lexer);
        if (c == '=')  return nxc_token_lshift_assign;
        nxc_lex_ungetc(lexer);
        return nxc_token_left_shift;
    }
    if (c == '=') return nxc_token_less_equal;

    nxc_lex_ungetc(lexer);
    return nxc_token_less_than;
}

/**
 * scan '>' '>>' '>>=' '>='
 */
___fast int nxc_lex_scan_great(nxc_lexer_t *lexer)
{
    int c;

    c = nxc_lex_getc(lexer);
    if (c == '>')
    {
        c = nxc_lex_getc(lexer);
        if (c == '=') return nxc_token_rshift_assign;
        nxc_lex_ungetc(lexer);
        return nxc_token_right_shift;
    }
    if (c == '=') return nxc_token_great_equal;

    nxc_lex_ungetc(lexer);
    return nxc_token_great_than;
}

/**
 * scan '!' '!='
 */
___fast int nxc_lex_scan_exclamation(nxc_lexer_t *lexer)
{
    if (nxc_lex_getc(lexer) == '=') return nxc_token_unequal;
    nxc_lex_ungetc(lexer);
    return nxc_token_not;
}

/**
 * scan '.' '.123' '...'
 */
___fast int nxc_lex_scan_dot(nxc_lexer_t *lexer)
{
    int c;

    c = nxc_lex_getc(lexer);
    if (nxc_isdigit(c))
    {
        nxc_lex_push_char(lexer,'.');
        nxc_lex_push_char(lexer,c);
        return nxc_lex_scan_float(lexer);
    }
    if (c == '.')
    {
        c = nxc_lex_getc(lexer);
        if(c == '.') return nxc_token_ellipsis;
        nxc_lex_ungetc(lexer);
    }

    nxc_lex_ungetc(lexer);
    return nxc_token_dot;
}

/**
 * scan '[' '[[' '[[['
 */
___fast int nxc_lex_scan_left_bracket(nxc_lexer_t *lexer)
{
    int c;

    c = nxc_lex_getc(lexer);
    if (c == '[')
    {
        c = nxc_lex_getc(lexer);
        if (c == '[')   return nxc_token_left_bracket2;
        nxc_lex_ungetc(lexer);
        return nxc_token_left_bracket1;
    }
    nxc_lex_ungetc(lexer);
    return nxc_token_left_bracket;
}

/**
 * scan ']' ']]' ']]]'
 */
___fast int nxc_lex_scan_right_bracket(nxc_lexer_t *lexer)
{
    int c;

    c = nxc_lex_getc(lexer);
    if (c == ']')
    {
        c = nxc_lex_getc(lexer);
        if (c == ']')   return nxc_token_right_bracket2;
        nxc_lex_ungetc(lexer);
        return nxc_token_right_bracket1;
    }
    nxc_lex_ungetc(lexer);
    return nxc_token_right_bracket;
}

/**===========================================================================*/
/**===========================================================================*/

/**
 * run lex and get next token info ...
 * @return next token type ...
 */
___fast int __nxc_do_lex(nxc_lexer_t *lexer)
{
    int c;
    int ret;

    nxc_lex_set_err_msg(lexer,0);
    nxc_lex_clear_namebuf(lexer);
    ///------------------------------
    c = nxc_lex_scan_space(lexer);   ///skip space first ...
    if (!c){///save result ...
        nxc_lex_set_token_type(lexer,nxc_token_eof);
        return nxc_token_eof;
    }
    
    c = nxc_lex_getc(lexer);         ///read a char and trnna dispatch
    switch (c)
    {
    case 'a':case 'b':case 'c':case 'd':case 'e':case 'f':case 'g':case 'h':
    case 'i':case 'j':case 'k':case 'l':case 'm':case 'n':case 'o':case 'p':
    case 'q':case 'r':case 's':case 't':case 'u':case 'v':case 'w':case 'x':
    case 'y':case 'z':
    case 'A':case 'B':case 'C':case 'D':case 'E':case 'F':case 'G':case 'H':
    case 'I':case 'J':case 'K':case 'L':case 'M':case 'N':case 'O':case 'P':
    case 'Q':case 'R':case 'S':case 'T':case 'U':case 'V':case 'W':case 'X':
    case 'Y':case 'Z':
    case '_':
        ret= nxc_lex_scan_identifier(lexer,c);
        break;
    case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':
    case '8':case '9':
        ret = nxc_lex_scan_number(lexer,c);
        break;  
    case '"':   ret = nxc_lex_scan_string(lexer);       break;
    case '\'':  ret = nxc_lex_scan_char_number(lexer);  break;
    case '(':   ret = nxc_token_left_paren;             break;
    case ')':   ret = nxc_token_right_paren;            break;
    case '[':   ret = nxc_lex_scan_left_bracket(lexer); break;
    case ']':   ret = nxc_lex_scan_right_bracket(lexer);break;
    case '{':   ret = nxc_token_left_brace;             break;
    case '}':   ret = nxc_token_right_brace;            break;
    ///-------------------------------------------------------------------------
    case '+':   ret = nxc_lex_scan_plus(lexer);         break;
    case '-':   ret = nxc_lex_scan_minus(lexer);        break;
    case '*':   ret = nxc_lex_scan_star(lexer);         break;
    case '/':   ret = nxc_lex_scan_slash(lexer);        break;
    case '%':   ret = nxc_lex_scan_percent(lexer);      break;
    case '&':   ret = nxc_lex_scan_and(lexer);          break;
    case '|':   ret = nxc_lex_scan_or(lexer);           break;
    case '!':   ret = nxc_lex_scan_exclamation(lexer);  break;
    case '^':   ret = nxc_lex_scan_caret(lexer);        break;
    case '<':   ret = nxc_lex_scan_less(lexer);         break;
    case '=':   ret = nxc_lex_scan_equal(lexer);        break;
    case '>':   ret = nxc_lex_scan_great(lexer);        break;
    case '.':   ret = nxc_lex_scan_dot(lexer);          break;
    ///---
    case '~':   ret = nxc_token_compensation;   break;
    case '@':   ret = nxc_token_at;             break;
    ///---
    case ',':   ret = nxc_token_comma;          break;
    case ':':   ret = nxc_token_colon;          break;
    case ';':   ret = nxc_token_semicolon;      break;
    ///'#' treat as a comment line ...
    case '#':   ret = nxc_token_bad;            break;
    case '$':   ret = nxc_token_bad;            break;
    case '?':   ret = nxc_token_bad;            break;
    default:    ret = nxc_token_bad;            break;
    }

    ///zero terminate the name - string ...
    lexer->parse_buff[lexer->name_len] = 0;
    ///save result ...
    nxc_lex_set_token_type(lexer,ret);

    return ret;
}

/**===========================================================================*/
/**===========================================================================*/














/**===========================================================================*/
/**============================ compiler =====================================*/
/**===========================================================================*/

/**
 * parse the variable declaration 
 * return non-zero mean error ...
 * >>>Will prefetch next token<<<
 * 'var a,b[100],c[[10]],def;'
 */
static int nxc_parse_var_declaration(nxc_compiler_t*const compiler,int sym_type);

/**
 * parse the constant declaration 
 * return non-zero mean error ...
 * >>>Will prefetch next token<<<
 */
static int nxc_parse_const_declaration(nxc_compiler_t*const compiler);

/**
 * find const in specific table ...
 */
___fast nxc_const_node_t*nxc_find_const(nxc_compiler_t*const compiler,char*name,int len);

/**
 * unary-expression:
 *      prefix-expression  : & * + - ! ~ unary-expression
 *      postfix-expression : . ( [ expression
 *      ( type-name ) unary-expression
 *      sizeof ( type-name )
 * @return 0 means error ...
 * >>>Will prefetch next token<<<
 */
static nxc_ast_t *nxc_parse_unary(nxc_compiler_t*const compiler);

/**
 * trnna parse binary expression ...
 *     unary op expression
 *
 * >>>will prefetch the next token<<<
 */
static nxc_ast_t *nxc_parse_binary_expression(nxc_compiler_t*const compiler,int curr_level);

/**
 * parse expression and generate ast list ...
 * return 0 means error ...
 * >>>Will prefetch next token<<<
 */
static nxc_ast_t *nxc_parse_expression(nxc_compiler_t*const compiler);

/**
 * parse a statement and generate the ast list ...
 * return 0 means error ...
 * >>>Will prefetch next token<<<
 */
static nxc_ast_t *nxc_parse_statement(nxc_compiler_t*const compiler);

/**
 * parse statement block ...
 *    [1].var declaration ...
 *    [2].normal statement ...
 *
 * >>>Will prefetch next token<<<
 */
static nxc_ast_t *nxc_parse_statement_block(nxc_compiler_t*const compiler);


/**
 * translate ast to marco-instruction ...
 * return non-zero means error ...
 */
static int nxc_translate(nxc_compiler_t*const compiler,nxc_ast_t *ast,int calc_lvalue);


/**===========================================================================*/

/**
 * privilege table for AST-opcode ...
 */
static char __ast_opcode_level[]=
{
    0,///nxc_op_bad
    ///---
    0,///nxc_op_comma,        ///"," ///for expression level ...
    ///---
    1,///nxc_op_assign,       ///"=" ///for binary-expression level ...
    1,///nxc_op_add_assign,   ///"+="
    1,///nxc_op_sub_assign,   ///"-="
    1,///nxc_op_mul_assign,   ///"*="
    1,///nxc_op_div_assign,   ///"/="
    1,///nxc_op_mod_assign,   ///"%="
    1,///nxc_op_bitor_assign, ///"|="
    1,///nxc_op_bitand_assign,///"&="
    1,///nxc_op_xor_assign,   ///"^="
    1,///nxc_op_lshift_assign,///"<<="
    1,///nxc_op_rshift_assign,///">>="
    ///------------------binary-operator---------
    2,///nxc_op_or,           ///"||"
    3,///nxc_op_and,          ///"&&"
    4,///nxc_op_bit_or,       ///"|"
    5,///nxc_op_bit_and,      ///"&"
    6,///nxc_op_xor,          ///"^"
    7,///nxc_op_equal,        ///"=="
    7,///nxc_op_unequal,      ///"!="
    8,///nxc_op_great,        ///">"
    8,///nxc_op_less,         ///"<"
    8,///nxc_op_great_equal,  ///">="
    8,///nxc_op_less_equal,   ///"<="
    9,///nxc_op_left_shift,   ///"<<"
    9,///nxc_op_right_shift,  ///">>"
    10,///nxc_op_add,          ///"+"
    10,///nxc_op_sub,          ///"-"
    11,///nxc_op_mul,          ///"*"
    11,///nxc_op_div,          ///"/"
    11,///nxc_op_mod,          ///"%"
    ///------------------unary operator-------- 
    12,///nxc_op_inc,          ///"++"
    12,///nxc_op_dec,          ///"--"
    12,///nxc_op_pos,          ///"+"
    12,///nxc_op_negative,     ///"-"
    12,///nxc_op_compensation, ///"~"
    12,///nxc_op_not,          ///"!"
    ///-------------------------------------------
    12,///nxc_op_address,      ///"&"
    12,///nxc_op_sizeof,       ///"sizeof"
    12,///nxc_op_cast,         ///"cast"
    ///-------------------------------------------
    13,///nxc_op_index8,       ///"["   ---
    13,///nxc_op_index16,      ///"[["  ---
    13,///nxc_op_index32       ///"[[[" ---
    13,///nxc_op_index_l       ///"[[[["---
    ///-------------------------------------------
    13,///nxc_op_call,         ///"call"
    13,///nxc_op_member,       ///"."
    13,///nxc_op_ptr_right,    ///"->"
    ///-------------------------------------------
    14,///nxc_op_id,           ///"$ID"
    14,///nxc_op_const,        ///"const"
    ///-------------------------------------------
    15,///nxc_op_expression,
    15,///nxc_op_if,
    15,///nxc_op_else,
    15,///nxc_op_for,
    15,///nxc_op_while,
    15,///nxc_op_do,
    15,///nxc_op_break,
    15,///nxc_op_continue,
    15,///nxc_op_return,
    15,///nxc_op_block,
    100,///nxc_op_max,
};
/**
 * AST opcode name table ...
 */
static char *__ast_opcode_name[] = 
{
    "bad",
    ///------------------------------
    ",",            ///","
    ///-------------binary opcode-----------------
    "=",            ///"="
    "+=",           ///"+="
    "-=",           ///"-="
    "*=",           ///"*="
    "/=",           ///"/="
    "%=",           ///"%="
    "|=",           ///"|="
    "&=",           ///"&="
    "^=",           ///"^="
    "<<=",          ///"<<="
    ">>=",          ///">>="
    ///------------------------------
    "||",           ///"||"
    "&&",           ///"&&"
    "|",            ///"|"
    "&",            ///"&"
    "^",            ///"^"
    "==",           ///"=="
    "!=",           ///"!="
    ">",            ///">"
    "<",            ///"<"
    ">=",           ///">="
    "<=",           ///"<="
    "<<",           ///"<<"
    ">>",           ///">>"
    "+",            ///"+"
    "-",            ///"-"
    "x",            ///"*"
    "/",            ///"/"
    "%",            ///"%"
    ///-------------unary opcode-----------------
    "++",           ///"++"
    "--",           ///"--"
    "+pos",         ///"+"
    "-neg",         ///"-"
    "~",            ///"~"
    "!",            ///"!"
    ///-------------------------------------------
    "&addr",        ///"&"
    "sizeof",       ///"sizeof"
    "cast",         ///"cast"
    ////------------------------------------------------------------------------
    "[]char",       ///"[char]"     ---
    "[]short",      ///"[short]"    ---
    "[]int",        ///"[int]"   ---
    "[]long",       ///"[long]"
    ///-------------------------------------------
    "call",         ///"call"
    ".member",      ///"."
    "->",           ///"->"
    ///------------------factor------------------
    "id",           ///"$ID"
    "const",        ///"const"
    ///------------------statement---------------
    "expr"
    "if",
    "else",
    "for",
    "while",
    "do",
    "break",
    "continue",
    "return",
    "block",
    "max",
};
/**
 * VM Opcode-name table ...
 */
static char *__nxc_vm_opcode_name[] = 
{
    "halt",
    ///------------------------assignment expression----------------------------
    "load8",                ///"*(char* )"
    "load16",               ///"*(short*)"
    "load32",               ///"*(int*  )"
    "load_long",            ///"*(long* )"
    ///-------------------------------------------
    "loadx8",               ///reg1 = *(char *)(reg1+offset)
    "loadx16",              ///reg1 = *(short*)(reg1+offset)
    "loadx32",              ///reg1 = *(int  *)(reg1+offset)
    "loadx_l",              ///reg1 = *(long *)(reg1+offset)
    ///-------------------------------------------------------------------------
    "index_8",              ///"[]char"   ---
    "index_16",             ///"[]short"  ---
    "index_32",             ///"[]int"    ---
    "index_L",              ///"[]long"   ---
    ///------------------------------factor-------------------------------------
    "load_lvar",            ///marco-opcode ...
    "load_lvar_addr",
    "load_gvar",
    "load_gvar_addr",
    "load_immed",           ///func addr ", const str ", number ...
    "add_immed",            ///reg=reg+immediate
    ///-------------------------------------------
    "st_lvar",              ///"="
    "add_st_lvar",          ///"+="
    "sub_st_lvar",          ///"-="
    "mul_st_lvar",          ///"*="
    "div_st_lvar",          ///"/="
    "mod_st_lvar",          ///"%="
    "bor_st_lvar",          ///"|="
    "band_st_lvar",         ///"&="
    "xor_st_lvar",          ///"^="
    "lshift_st_lvar",       ///"<<="
    "rshift_st_lvar",       ///">>="
    ///-------------------------STOTE---------------------------
    "assign_char",          ///"="  
    "add_assign_char",      ///"+=" 
    "sub_assign_char",      ///"-=" 
    "mul_assign_char",      ///"*=" 
    "div_assign_char",      ///"/=" 
    "mod_assign_char",      ///"%=" 
    "bitor_assign_char",    ///"|=" 
    "bitand_assign_char",   ///"&=" 
    "xor_assign_char",      ///"^=" 
    "lshift_assign_char",   ///"<<="
    "rshift_assign_char",   ///">>="
    ///--------------   
    "assign_short",         ///"="  
    "add_assign_short",     ///"+=" 
    "sub_assign_short",     ///"-=" 
    "mul_assign_short",     ///"*=" 
    "div_assign_short",     ///"/=" 
    "mod_assign_short",     ///"%=" 
    "bitor_assign_short",   ///"|=" 
    "bitand_assign_short",  ///"&=" 
    "xor_assign_short",     ///"^=" 
    "lshift_assign_short",  ///"<<="
    "rshift_assign_short",  ///">>="
    ///--------------
    "assign_int",           ///"="  
    "add_assign_int",       ///"+=" 
    "sub_assign_int",       ///"-=" 
    "mul_assign_int",       ///"*=" 
    "div_assign_int",       ///"/=" 
    "mod_assign_int",       ///"%=" 
    "bitor_assign_int",     ///"|=" 
    "bitand_assign_int",    ///"&=" 
    "xor_assign_int",       ///"^=" 
    "lshift_assign_int",    ///"<<="
    "rshift_assign_int",    ///">>="
    ///--------------
    "assign_long",          ///"="  
    "add_assign_long",      ///"+=" 
    "sub_assign_long",      ///"-=" 
    "mul_assign_long",      ///"*=" 
    "div_assign_long",      ///"/=" 
    "mod_assign_long",      ///"%=" 
    "bitor_assign_long",    ///"|=" 
    "bitand_assign_long",   ///"&=" 
    "xor_assign_long",      ///"^=" 
    "lshift_assign_long",   ///"<<="
    "rshift_assign_long",   ///">>="
    ///--------------
    "++",                   ///"++"
    "--",                   ///"--"
    ///-----------------------------binary operator-----------------------------
    "||",                   ///"||"
    "&&",                   ///"&&"
    "|",                    ///"|"
    "&",                    ///"&"
    "^",                    ///"^"
    "==",                   ///"=="
    "!=",                   ///"!="
    ">",                    ///">"
    "<",                    ///"<"
    ">=",                   ///">="
    "<=",                   ///"<="
    "<<",                   ///"<<"
    ">>",                   ///">>"
    "+",                    ///"+"
    "-",                    ///"-"
    "*",                    ///"*"
    "/",                    ///"/"
    "%",                    ///"%"
    ///---
    "||",                   ///"||" const
    "&&",                   ///"&&" const
    "|",                    ///"|"  const
    "&",                    ///"&"  const
    "^",                    ///"^"  const
    "==",                   ///"==" const
    "!=",                   ///"!=" const
    ">",                    ///">"  const
    "<",                    ///"<"  const
    ">=",                   ///">=" const
    "<=",                   ///"<=" const
    "<<",                   ///"<<" const
    ">>",                   ///">>" const
    "+",                    ///"+"  const
    "-",                    ///"-"  const
    "*",                    ///"*"  const
    "/",                    ///"/"  const
    "%",                    ///"%"  const
    ///---
    "||",                   ///const "||"
    "&&",                   ///const "&&"
    "|",                    ///const "|"
    "&",                    ///const "&"
    "^",                    ///const "^"
    "==",                   ///const "=="
    "!=",                   ///const "!="
    ">",                    ///const ">"
    "<",                    ///const "<"
    ">=",                   ///const ">="
    "<=",                   ///const "<="
    "<<",                   ///const "<<"
    ">>",                   ///const ">>"
    "+",                    ///const "+"
    "-",                    ///const "-"
    "*",                    ///const "*"
    "/",                    ///const "/"
    "%",                    ///const "%"
    ///-------------------------------------------------------------------------
    "f_>",                  ///":>"
    "f_<",                  ///":<"
    "f_>=",                 ///":>="
    "f_<=",                 ///":<="
    "f_+",                  ///":+"
    "f_-",                  ///":-"
    "f_*",                  ///":*"
    "f_/",                  ///":/"
    "f_-",                  ///":-"
    ///---
    "f_>",                  ///":>"
    "f_<",                  ///":<"
    "f_>=",                 ///":>="
    "f_<=",                 ///":<="
    "f_+",                  ///":+"
    "f_-",                  ///":-"
    "f_*",                  ///":*"
    "f_/",                  ///":/"
    ///---
    "f_>",                  ///":>"
    "f_<",                  ///":<"
    "f_>=",                 ///":>="
    "f_<=",                 ///":<="
    "f_+",                  ///":+"
    "f_-",                  ///":-"
    "f_*",                  ///":*"
    "f_/",                  ///":/"
    ///----------------------------unary operator-------------------------------
    "cast",                 ///"cast"
    "positive",             ///"+"
    "negative",             ///"-"
    "~",                    ///"~"
    "!",                    ///"!"
    ///------------------------------flow-control-------------------------------
    "addr_array8",          ///"&[[["    left + index
    "addr_array16",         ///"&[[[["   left + index * sizeof(short)
    "addr_array32",         ///"&[["     left + index * sizeof(int)
    "addr_array_l",         ///"&["      left + index * sizeof(long)
    ///------------------------------------------------
    "call",                 ///"call    $reg"
    "postcall",             ///do post work after call(mov reg,eax;add esp,xxx)
    "trap",                 ///"trap    $reg"
    "fast_trap",            ///"fasttrap $reg"
    "jmp",                  ///"jmp     $imme_addr"
    "jz",                   ///"jmp if zero"
    "jnz",                  ///"jmp if non-zero"
    "return_val",           ///"return  $expr"
    "ret",                  ///"ret"
    "push",                 ///"push     $reg"
	"pushi",                 ///"push     $immediate"
    "pop",                  ///"pop
    "enter",                ///enter func ///this will be done automatically ...
    "leave",                ///leave func ///this is done at return statement ..
    "nop",
    "max",
};
/**===========================================================================*/

___fast char *nxc_get_vmop_name(int opcode){
    if(opcode<nxc_vmop_halt || opcode> nxc_vmop_max) return "N/A";
    return __nxc_vm_opcode_name[opcode];
}
___fast char *nxc_get_opcode_name(int opcode){
    if(opcode<nxc_op_bad || opcode> nxc_op_max) return "N/A";
    return __ast_opcode_name[opcode];
}

/**
 * calc level from opcode ...
 */
___fast int nxc_get_opcode_level(int opcode){
    return __ast_opcode_level[opcode];
}
/**
 * check if token is a binary operator ...
 */
___fast int nxc_is_binary_operator(int token){
    return ((token>=nxc_token_or) && (token <= nxc_token_mod));
}
/**
 * calc opcode from token ...
 * the token is a binary operator ...
 */
___fast int nxc_binary_operator_to_opcode(int token){
    return nxc_op_assign + (token - nxc_token_assign);
}
/**
 * check if token is a assignment operator ...
 */
___fast int nxc_is_assignment_operator(int token){
    return (token >= nxc_token_assign) && (token <=nxc_token_rshift_assign);
}
///convet token to opcode ...
___fast int nxc_assignment_operator_to_opcode(int token){
    return nxc_op_assign + (token - nxc_token_assign);
}

///generate full name for member $class_name::$member_name ...
///@return fullname length ...
___fast int nxc_gen_member_fullname(char*obuf,char*class_name,char*member_name)
{
    int len,len1;

    len = nxc_strlen(class_name);
    if (len>60) len = 60;      ///truncate
    len1 = nxc_strlen(member_name);
    if (len1>60) len1 = 60;    ///truncate
    nxc_memcpy(obuf,class_name,len);
    obuf[len++] = ':';
    obuf[len++] = ':';
    nxc_memcpy(obuf+len,member_name,len1);
    len+=len1;
    obuf[len] = 0;
    return len;
}

/**===========================================================================*/

/**
 * add string (\x00 enabled) to string table ...
 * @return 0 means error ...
 */
static nxc_str_node_t* nxc_lex_add_string(nxc_compiler_t *const compiler,
                                          char*name,int namelen,
                                          int xtype)
{
    int total_len;
    nxc_hash_node_t *n;
    nxc_hash_node_t dummy;
    nxc_str_node_t *str_node;

    dummy.key  = name;
    dummy.klen = namelen;

    ///trnna fix existed ...
    n = nxc_hash_find(compiler->str_table,&dummy);
    if(n) return (nxc_str_node_t *)n;

    total_len = sizeof(nxc_str_node_t) + namelen + 1;
    ///create a new keyword-node (add extra zero to target string)
    str_node = (nxc_str_node_t *) nxc_compiler_malloc(compiler,total_len);
    if(!str_node) return 0;
    ///reset mem
    nxc_memset(str_node,0,total_len);
    ///set payload ...
    nxc_init_hash_node(&str_node->hash_node,&str_node[1],namelen,0);
    nxc_init_dlist_head(&str_node->trace_node);
    ///copy name ...
    nxc_str_node_set_str(str_node,(char *)&str_node[1]);
    nxc_str_node_set_len(str_node,namelen);
    nxc_memcpy(nxc_str_node_get_str(str_node),name,namelen);
    nxc_str_node_get_str(str_node)[namelen] = 0; ///zero-terminated it ...
    nxc_str_node_set_type(str_node,xtype);

    nxc_hash_add(compiler->str_table,&str_node->hash_node);
    return str_node;
}

/**
 * return 0 means error ...
 */
static nxc_str_node_t* nxc_lex_add_keyword(nxc_compiler_t *const compiler,
                                           char*name,
                                           int token_type)
{
    return nxc_lex_add_string(compiler,name,nxc_strlen(name),token_type);
}

/**
 * find static string by name ...
 */
static nxc_str_node_t* nxc_lex_find_str(nxc_compiler_t*const compiler,
                                        char *name,int namelen)
{
    nxc_hash_node_t *n;
    nxc_hash_node_t dummy;
    
    dummy.key  = name;
    dummy.klen = namelen;
    
    ///trnna fix existed ...
    n = nxc_hash_find(compiler->str_table,&dummy);
    if(!n) return 0;

    return (nxc_str_node_t*)n;
}

/**
 * find static string by name and create new one if find failed ...
 * @return 0 means error ...
 */
static nxc_str_node_t* nxc_lex_create_str(nxc_compiler_t*const compiler,
                                          char *name,int namelen)
{
    nxc_str_node_t *str_node;
    nxc_hash_node_t *n;
    nxc_hash_node_t dummy;
    
    dummy.key  = name;
    dummy.klen = namelen;
    
    ///trnna fix existed ...
    n = nxc_hash_find(compiler->str_table,&dummy);
    if(!n) 
    {
        str_node = nxc_lex_add_string(compiler,name,namelen,0);
        return str_node;
    }

    return (nxc_str_node_t*)n;
}
/**
 * build keywords table ...
 * @return 0 means okay ...
 */
static int nxc_lex_init_keywords(nxc_compiler_t *const compiler)
{
    nxc_str_node_t *node;

    node = nxc_lex_add_keyword(compiler,"sizeof"   ,nxc_token_sizeof);
    node = nxc_lex_add_keyword(compiler,"typedef"  ,nxc_token_typedef);
    node = nxc_lex_add_keyword(compiler,"struct"   ,nxc_token_struct);
    node = nxc_lex_add_keyword(compiler,"class"    ,nxc_token_class);
    node = nxc_lex_add_keyword(compiler,"union"    ,nxc_token_union);
    node = nxc_lex_add_keyword(compiler,"const"    ,nxc_token_const);
    node = nxc_lex_add_keyword(compiler,"api"      ,nxc_token_api);

    node = nxc_lex_add_keyword(compiler,"var"      ,nxc_token_var);
    node = nxc_lex_add_keyword(compiler,"func"     ,nxc_token_func);

    node = nxc_lex_add_keyword(compiler,"break"    ,nxc_token_break);
    node = nxc_lex_add_keyword(compiler,"continue" ,nxc_token_continue);
    node = nxc_lex_add_keyword(compiler,"if"       ,nxc_token_if);
    node = nxc_lex_add_keyword(compiler,"else"     ,nxc_token_else);
    node = nxc_lex_add_keyword(compiler,"for"      ,nxc_token_for);
    node = nxc_lex_add_keyword(compiler,"do"       ,nxc_token_do);
    node = nxc_lex_add_keyword(compiler,"while"    ,nxc_token_while);
    node = nxc_lex_add_keyword(compiler,"return"   ,nxc_token_return);
    node = nxc_lex_add_keyword(compiler,"___eof___",nxc_token_eof);

    return 0;
}


/**===========================================================================*/

/**
 * run lex and get next token info ...
 * @return next token type ...
 */
static int nxc_do_lex(nxc_compiler_t *compiler)
{
    int token;
    nxc_lexer_t *lexer;
    nxc_str_node_t *str_node;
    nxc_const_node_t *const_node;
    char *name_buff;
    int namelen=0;
    int token_type;

    lexer = compiler->lexer;
    name_buff = lexer->parse_buff;
    ///tell lex do parsing token and fill result to name buffer ...
    token = __nxc_do_lex(lexer);
    ///check error msg ...
    if (nxc_lex_get_err_msg(lexer)){
        nxc_printf(compiler,"error:%s",nxc_lex_get_err_msg(lexer));
        return nxc_token_bad;
    }

    if(token == nxc_token_id)
    {
        ///1.check if overflow ...
        namelen = nxc_lex_get_namelen(compiler->lexer);
        if (namelen >= 1023){
            nxc_error(compiler,"token length over flow!!!");
            return -1;
        }
        ///2.check keyword or create string ...
        str_node = nxc_lex_create_str(compiler,name_buff,namelen);
        if (!str_node) return -1;

        ///3.do alias replacing ...
        const_node = nxc_find_const(compiler,name_buff,namelen);
        if (const_node)
        {
            ///do replacing ...
            nxc_lex_copy_token(lexer,nxc_const_node_get_token(const_node));

            return lexer->token_type;
        }

        ///3.get string type ...
        token_type = nxc_str_node_get_type(str_node);
        ///maybe it's a static-string ,treat as id ...
        if (!token_type) token_type = nxc_token_id;
        ///fix the current token type ...
        nxc_lex_set_token_type(lexer,token_type);
        ///fix curr-token str-val
        nxc_token_set_str_node(lexer->token,str_node);
        ///here , it must be keywords-string ...
        return token_type;
    }
    
    if (token == nxc_token_const_string)
    {
        ///1.check if overflow ...
        namelen = nxc_lex_get_namelen(compiler->lexer);
        if (namelen >= 1023)
        {
            nxc_error(compiler,"token length over flow!!!");
            return -1;
        }
        ///generate string ...
        str_node = nxc_lex_create_str(compiler,name_buff,namelen);
        if (!str_node) return -1;
        ///fix curr-token str-val
        nxc_token_set_str_node(lexer->token,str_node);

        ///trace the string and alloc the data-seg space for it ...
        nxc_compiler_trace_const_str(compiler,str_node);

        return nxc_token_const_string;
    }

    return token;
}

/**===========================================================================*/

/**
 * return current token type ...
 */
___fast int nxc_cur_tok_type(nxc_compiler_t*const compiler){
    return nxc_lex_get_token_type(compiler->lexer);
}
/**
 * return current token value ...
 */
___fast nxc_token_t* nxc_cur_tok(nxc_compiler_t*const compiler){
    return &compiler->lexer->token[0];
}

/**
 * do lex and test if next token is expected one ...
 * @return 0 means okay ...
 */
___fast int nxc_expect(nxc_compiler_t*const compiler,int expect_token_type)
{
    int token;
    token = nxc_do_lex(compiler);
    if(token!=expect_token_type)
    {
        nxc_error(compiler,"syntex error : '%s' is Expected.",
                  nxc_get_token_name(expect_token_type));
        return -1;
    }
    return 0;
}

/**
 * check if current token is the one expected ...
 * return 0 means okay ...
 */
___fast int nxc_expect_current(nxc_compiler_t*const compiler,int expect_token_type)
{
    int token;
    token = nxc_cur_tok_type(compiler);
    if(token!=expect_token_type)
    {
        nxc_error(compiler,"syntex error : '%s' is Expected.",
                    nxc_get_token_name(expect_token_type));
        return -1;
    }
    return 0;
}

/**===========================================================================*/
/**
 * alloc space for specific kind of variable ...
 * what the allocator returned , is the finally value for vm!!!
 */
static 
int nxc_alloc_space_for_var(nxc_compiler_t*const compiler,int sym_type,int size)
{
    int start_addr;
    switch (sym_type)
    {
    case nxc_sym_global_var:
        ///nxc_compiler_alloc_dseg_space() will align the size !!!
        ///alloc dseg space for global variable ...
        start_addr = nxc_compiler_alloc_dseg_space(compiler,size);
        ///return a aligned address
        return +start_addr;
    case nxc_sym_global_function:
        return 0;
    case nxc_sym_local_param:
        ///align the size
        size = nxc_align(size,sizeof(long));
        ///find existed start addr ...
        start_addr = nxc_sym_get_param_size(compiler->curr_function);
        ///update total size ...
        nxc_sym_set_param_size(compiler->curr_function,start_addr+size);
        ///return a aligned address
        return +start_addr;
    case nxc_sym_local_var:
        ///align the size
        size = nxc_align(size,sizeof(long));
        ///local var is a reverse direction allocation !!!
        start_addr = nxc_sym_get_func_lvar_size(compiler->curr_function) + size;
        ///update total size ...
        nxc_sym_set_func_lvar_size(compiler->curr_function,start_addr);
        /**
         * +----------+------------+-----------+
         * |  var2    |    var1    |  ebp      |
         * +----------^------------+-----------+
         *            |
         *    var1----+  (var1_offset = sizeof(long))
         */
        ///return reverse address ...
        return -start_addr;
    case nxc_sym_member:
        ///alloc member offset ...
        start_addr = compiler->curr_struct_size; ///save start address
        compiler->curr_struct_size += size;      ///update total size ...
        return +start_addr;
        break;
    default:
        return 0;
    }
}

/**===========================================================================*/
/**
 * create a symbol and add it to specific sym-table ...
 * return 0 means error ...
 */
static nxc_sym_t *__nxc_new_sym(nxc_compiler_t*const compiler,
                                nxc_sym_t *sym,
                                nxc_str_node_t *name_info,
                                int sym_type,
                                int type_size,
                                int sym_flag)
{
    int xaddr,namelen;
    char *name;
    
    if(!sym) return 0;
    name    = nxc_str_node_get_str(name_info);
    namelen = nxc_str_node_get_len(name_info);
    ///do initialize ...
    nxc_memset(sym,0,sizeof(*sym));
    nxc_sym_set_name(sym,name);
    nxc_sym_set_namelen(sym,namelen);
    nxc_sym_set_str_node(sym,name_info); ///associate symbol to string-node ...
    nxc_sym_set_type(sym,sym_type);
    nxc_sym_set_typesize(sym,type_size);
    nxc_sym_add_flag(sym,sym_flag);

    switch (sym_type)
    {
    case nxc_sym_global_function:
        ///init instruction list ...
        nxc_sym_init_local_sym_table(sym);

    case nxc_sym_global_var:
        ///allocate the data-space for from data-seg ...
        xaddr = nxc_alloc_space_for_var(compiler,sym_type,type_size);
        nxc_sym_set_var_offset(sym,xaddr);
        ///... add sym to global-sym-table ...
        nxc_hash_add(compiler->sym_table,&sym->hash_node);///add to global 

        ///trace global func and var for EXP-Table ...
        if(!nxc_sym_has_flag(sym,nxc_symflag_extern)){///skip extern symbol ...
            nxc_compiler_trace_global_sym(compiler,sym);///local g_sym
        }else{
            ///extern symbol will be traced and fixed at 'load-instruction'
        }

        ///trace the string and alloc the data-seg space for it(For IMP&EXP)...
        nxc_compiler_trace_const_str(compiler,name_info);

        break;
    case nxc_sym_local_param:
    case nxc_sym_local_var:
        ///allocate the data-space for local-frame ...
        xaddr = nxc_alloc_space_for_var(compiler,sym_type,type_size);
        nxc_sym_set_var_offset(sym,xaddr);
        ///... add sym to local-sym-table-table ...
        nxc_sym_add_local_sym(compiler->curr_function,sym);   ///add to local
        break;
    case nxc_sym_member:
        ///do nothing ...
        xaddr = nxc_alloc_space_for_var(compiler,sym_type,type_size);
        nxc_sym_set_var_offset(sym,xaddr);
        ///... add sym to global-sym-table ...
        nxc_hash_add(compiler->sym_table,&sym->hash_node);///add to global 
        break;
    default:
        nxc_error(compiler,"unknown symbol-type!!");
        return 0;
        break;
    }

    return sym;
}

/**
 * find symbol in specific table ...
 */
___fast nxc_sym_t *nxc_find_global_symbol(nxc_compiler_t*const compiler,
                                          char*name,int len)
{
    nxc_hash_table_t *table;
    nxc_hash_node_t dummy;
    table = compiler->sym_table;
    dummy.key = name;
    dummy.klen = len;
    return (nxc_sym_t *)nxc_hash_find(table,&dummy);
}

/**
 * find local symbol ...
 */
___fast nxc_sym_t *nxc_find_local_symbol(nxc_compiler_t*const compiler,
                                         char*name,int len)
{
    nxc_sym_t *p;
    nxc_dlist_head_t *_p;
    nxc_dlist_for_each(_p,nxc_sym_get_local_sym_table(compiler->curr_function))
    {
        p = nxc_dlist_entry(_p,nxc_sym_t,hash_node.list_node);
        if( len == nxc_sym_get_namelen(p) &&
            !nxc_memcmp(nxc_sym_get_name(p),name,len)) return p;
    }
    return 0;
}

/**
 * find symbol by name ...
 * [1].find local sym-table
 * [2].find global sym-table
 */
___fast nxc_sym_t*nxc_find_symbol(nxc_compiler_t*const compiler,
                                  char*name,int len)
{
    nxc_sym_t *ret = 0;

    ///[1].trnna find local first ...
    if(nxc_in_local_zone(compiler))
    {
        ret = nxc_find_local_symbol(compiler,name,len);
    }
    ///[2].trnna find global ...
    if(!ret)
    {
        ret = nxc_find_global_symbol(compiler,name,len);
    }

    return ret;
}

/**
 * create a symbol and add it to specific sym-table ...
 * return 0 means error ...
 */
___fast nxc_sym_t *nxc_new_sym(nxc_compiler_t*const compiler,
                               nxc_str_node_t *name_info,
                               int sym_type,
                               int type_size,
                               int sym_flag)
{
    nxc_sym_t *sym;
    ///create a new symbol object ...
    sym = (nxc_sym_t *)nxc_compiler_malloc(compiler,sizeof(*sym));
    if (!sym)   return 0;
    return __nxc_new_sym(compiler,sym,name_info,sym_type,type_size,sym_flag);
}

/**
 * create a symbol and add it to specific sym-table ...
 * return 0 means error ...
 */
___fast nxc_sym_t *nxc_new_sym_ex(nxc_compiler_t*const compiler,
                                  char *name,int namelen,
                                  int sym_type,
                                  int type_size,
                                  int sym_flag)
{
    nxc_sym_t *sym;
    nxc_str_node_t *str_node;
    ///do unique check ...
    sym = nxc_find_symbol(compiler,name,namelen);
    if (sym){
        nxc_error(compiler,"multi-symbol instance:'%s'",name);
        return 0;
    }

    ///generate name info ...
    str_node = nxc_lex_create_str(compiler,name,namelen);
    if (!str_node) return 0;
    return nxc_new_sym(compiler,str_node,sym_type,type_size,sym_flag);
}

/**===========================================================================*/

/**
 * find const in specific table ...
 */
___fast nxc_const_node_t*nxc_find_const(nxc_compiler_t*const compiler,
                                        char*name,int len)
{
    nxc_hash_node_t dummy;
    dummy.key  = name;
    dummy.klen = len;
    return (nxc_const_node_t *)nxc_hash_find(compiler->const_table,&dummy);
}

/**
 * create a symbol and add it to specific sym-table ...
 * return 0 means error ...
 */
static nxc_const_node_t *nxc_new_const_ex(nxc_compiler_t*const compiler,
                                          char*name,int namelen,
                                          int token_type,
                                          int token_len,
                                          void*token_val)
{
    nxc_const_node_t *node;
    nxc_str_node_t *str_node;
    
    ///uniqueness check ...
    if(nxc_find_const(compiler,name,namelen)){
        nxc_error(compiler,"multi-defination of '%s'",name);
        return 0;
    }
    ///create name node ...
    str_node = nxc_lex_create_str(compiler,name,namelen);
    if (!str_node)return 0;
    ///create a new const-node object ...
    node = (nxc_const_node_t *)nxc_compiler_malloc(compiler,sizeof(*node));
    if(!node) return 0;
    ///do initialize ...
    nxc_memset(node,0,sizeof(*node));
    nxc_const_node_set_name(node,nxc_str_node_get_str(str_node));///set name
    nxc_const_node_set_namelen(node,namelen);                    ///namelen
    nxc_token_set_type(node->const_token,token_type);            ///valueType
    nxc_token_set_len(node->const_token,token_len);
    nxc_token_set_long_val(node->const_token,(long)token_val);   ///value

    nxc_hash_add(compiler->const_table,&node->hash_node);        ///add to table 
    return node;
}

/**
 * #############################################################################
 * #############################################################################
 * #############################################################################
 */

/**
 * create a empty ast node ...
 * return 0 means error ...
 */
static nxc_ast_t * nxc_new_ast_node(nxc_compiler_t*const compiler)
{
    nxc_ast_t *ast;

    ast = (nxc_ast_t *)nxc_compiler_malloc(compiler,sizeof(*ast));
    if(ast)
    {
        nxc_memset(ast,0,sizeof(*ast));///zero all as default !!!
        nxc_init_dlist_head(&ast->list_head);
        nxc_ast_set_opcode(ast,nxc_op_bad);
        nxc_ast_set_line_num(ast,compiler->lexer->line_number);
        nxc_ast_set_fname(ast,compiler->lexer->fname);
        nxc_ast_set_father_node(ast,compiler->curr_ast_node);

        return ast;
    }
    nxc_error(compiler,"***not enough memory***");

    return 0;
}

/**===========================================================================*/

/**
 * parse a id factor and generate a ast 
 * return ast node created for the factor ...
 * return 0 means error ...
 * >>>Won't prefetch next token<<<
 */
static nxc_ast_t *__nxc_parse_id_factor(nxc_compiler_t*const compiler,char*name)
{
    nxc_ast_t *ast_node;
    
    ///create ast node ...
    ast_node = nxc_new_ast_node(compiler);
    if(!ast_node) return 0;
    ///set info ...
    nxc_ast_set_opcode(ast_node,nxc_op_id);
    /**
     * while reached here , the symbol of id might NOT available !!!
     * !!!'var/func declaration' might stand after reference-statement !!!
     */
    nxc_id_ast_set_name(ast_node,name);//save id name.

    ///===alloc xseg space for prolog===
    nxc_compiler_alloc_xseg_space(compiler,sizeof(nxc_instr_t));//ld $R,$id
    return ast_node;
}
/**
 * parse a id factor and generate a ast 
 * return ast node created for the factor ...
 * return 0 means error ...
 * >>>Won't prefetch next token<<<
 */
___fast 
nxc_ast_t *nxc_parse_id_factor(nxc_compiler_t*const compiler,nxc_token_t *tok)
{
    return __nxc_parse_id_factor(compiler,nxc_token_get_str_val(tok));
}

/**
 * parse a const factor and generate a ast 
 * return ast node created for the factor ...
 * return 0 means error ...
 * >>>Won't prefetch next token<<<
 */
static 
nxc_ast_t *nxc_parse_const_factor(nxc_compiler_t*const compiler,nxc_token_t*tok)
{
    int token;
    nxc_ast_t *ast_node;

    token = nxc_token_get_type(tok);
    switch (token)
    {
    case nxc_token_const_long:
    case nxc_token_const_float:
        ///create ast node ...
        ast_node = nxc_new_ast_node(compiler);
        if(!ast_node) return 0;
        ///set info ...
        nxc_ast_set_opcode(ast_node,nxc_op_const);
        ///save const value ...
        nxc_const_ast_set_type(ast_node,token);                 ///copy type
        nxc_const_ast_set_len(ast_node,nxc_token_get_len(tok)); ///copy namelen ...
        nxc_const_ast_set_long_val(ast_node,nxc_token_get_long_val(tok));///data
        
        ///===alloc xseg space for prolog===
        nxc_compiler_alloc_xseg_space(compiler,sizeof(nxc_instr_t));//ld $R,$num
        
        return ast_node;
    case nxc_token_const_string:
        ///create ast node ...
        ast_node = nxc_new_ast_node(compiler);
        if(!ast_node) return 0;
        nxc_ast_set_opcode(ast_node,nxc_op_const);///set info ...
        ///save const value ...
        nxc_const_ast_set_type(ast_node,token);                 ///copy type
        nxc_const_ast_set_len(ast_node,nxc_token_get_len(tok)); ///copy namelen ...
        nxc_const_ast_set_str_node(ast_node,nxc_token_get_str_node(tok));///data
        
        ///===alloc xseg space for prolog===
        nxc_compiler_alloc_xseg_space(compiler,sizeof(nxc_instr_t));//ld $R,num
        
        return ast_node;
    default:
        nxc_error(compiler,"invalid factor '%s'!!!",nxc_get_token_name(token));
    }
    return 0;
}

/**
 * parse expression factor ...
 * expression-factor format:
 *    ID
 *    constant
 *    ( expression )
 * return ast node created for the factor ...
 * return 0 means error ...
 * >>>Won't prefetch next token<<<
 */
static nxc_ast_t *nxc_parse_factor(nxc_compiler_t*const compiler)
{
    int token;
    nxc_token_t *tok;
    nxc_ast_t *ast_node;
    
    //token = nxc_cur_tok_type(compiler);
    tok   = nxc_cur_tok(compiler);
    token = nxc_token_get_type(tok);
    switch (token)
    {
    case nxc_token_id:
        return nxc_parse_id_factor(compiler,tok);
    case nxc_token_const_long:
    case nxc_token_const_float:
    case nxc_token_const_string:
        return nxc_parse_const_factor(compiler,tok);
    case nxc_token_left_paren:///'('
        nxc_do_lex(compiler);///skip the '('
        ///this is a expression , larger than binary expression ...
        ast_node = nxc_parse_expression(compiler);
        ///check ')'
        if(nxc_expect_current(compiler,nxc_token_right_paren)) return 0;
        return ast_node;
    default:
        nxc_error(compiler,"invalid factor '%s'!!!",nxc_get_token_name(token));
    }
    return 0;
}

/**===========================================================================*/
/**
 * parse '$Unary $start_token $expression $endtoken'
 * return 0 means error ...
 * >>>Won't prefetch next<<<
 */
static nxc_ast_t*nxc_postfix_do_index(nxc_compiler_t*const compiler,
                                      nxc_ast_t *left_factor,
                                      int opcode,
                                      int end_token)
{
    nxc_ast_t *expr;
    nxc_ast_t *index;

    ///this op-nodde's father need to be fixed!!!
    expr = nxc_new_ast_node(compiler);
    if(!expr) return 0;
    nxc_ast_set_opcode(expr,opcode);
    nxc_array_ast_set_base(expr,left_factor);
    nxc_do_lex(compiler);
    ///>>>(fix)set left factor as child ...
    nxc_ast_set_father_node(left_factor,expr);
    ///>>>set curr ast node to myself ...
    compiler->curr_ast_node = expr;
    ///>>>index is a expression , larger than binary expression ...
    index = nxc_parse_expression(compiler);
    if(!index) return 0;
    nxc_array_ast_set_index(expr,index);
    ///check '$end_token'
    if(nxc_expect_current(compiler,end_token)) return 0;

    ///===alloc xseg space for prolog===///index $R_base,$R_index 
    nxc_compiler_alloc_xseg_space(compiler,sizeof(nxc_instr_t));
    
    return expr;
}

/**
 * parse $Unary ( {$expr {, $expr}} ) ...
 * return 0 means error ...
 * >>>Won't prefetch next<<<
 */
static nxc_ast_t *nxc_postfix_do_call(nxc_compiler_t*const compiler,
                                      nxc_ast_t *left_factor)
{
    int token,parm_cnt;
    nxc_ast_t *ast;
    nxc_ast_t *param;

    ast = nxc_new_ast_node(compiler);
    if(!ast) return 0;
    nxc_ast_set_opcode(ast,nxc_op_call);
    nxc_call_ast_set_func(ast,left_factor);///left is function call ...
    nxc_call_ast_init_parm_list(ast);      ///right is param list ...

    ///>>>(fix)set left factor as son ...
    nxc_ast_set_father_node(left_factor,ast);

    parm_cnt = 0;
    token = nxc_do_lex(compiler);
    if(token!= nxc_token_right_paren)
    {
        for (;;)
        {
            ///>>>set curr ast node to call-node ...
            compiler->curr_ast_node = ast;
            ///parse param-expr ...
            param = nxc_parse_binary_expression(compiler,0);
            if(!param) return 0;
            ///add param to param-list ...
            nxc_call_ast_add_parm(ast,param);
            parm_cnt++;

            ///check ')'
            token = nxc_cur_tok_type(compiler);
            if(token == nxc_token_right_paren) break;
            ///check ','
            if(nxc_expect_current(compiler,nxc_token_comma)) return 0;
            ///skip ','
            nxc_do_lex(compiler);
        }
    }
    ///save parameters count 
    nxc_call_ast_set_parmcnt(ast,parm_cnt);

    ///---alloc xseg space for instruction--- ///push $parm
    if (parm_cnt)
    {
        nxc_compiler_alloc_xseg_space(compiler,sizeof(nxc_instr_t)*parm_cnt);
    }

    ///---alloc xseg space for instruction---///call instr
    nxc_compiler_alloc_xseg_space(compiler,sizeof(nxc_instr_t));

    ///===alloc xseg space for prolog===///poscall instr
    nxc_compiler_alloc_xseg_space(compiler,sizeof(nxc_instr_t));

    return ast;
}

/**
 * parse member operation $obj . $member 
 * return 0 means error ...
 * >>>Won't prefetch next<<<
 */
static nxc_ast_t *nxc_postfix_do_member(nxc_compiler_t*const compiler,
                                        nxc_ast_t *left_factor)
{
    nxc_ast_t *ast;
    nxc_token_t *tok;

    ast = nxc_new_ast_node(compiler);
    if(!ast) return 0;
    nxc_ast_set_opcode(ast,nxc_op_member);
    ///>>>set curr ast node to me ...
    compiler->curr_ast_node = ast;

    ///>>>(fix)set left factor as son ...
    nxc_ast_set_father_node(left_factor,ast);

    ///save left ast ...
    nxc_member_ast_set_obj(ast,left_factor);

    ///next should be a token ...
    if(nxc_expect(compiler,nxc_token_id))return 0;

    ///save member name !!!
    tok = nxc_cur_tok(compiler);
    nxc_member_ast_set_member_name(ast,nxc_token_get_str_val(tok));

    ///===alloc xseg space for prolog===///push $obj_result_reg
    nxc_compiler_alloc_xseg_space(compiler,sizeof(nxc_instr_t));

    ///===alloc xseg space for prolog===///load reg,reg+offset | [reg+offset]
    nxc_compiler_alloc_xseg_space(compiler,sizeof(nxc_instr_t));

    return ast;
}

/**
 * parse postfix unary :
 *   [ [[
 *
 * postfix-expression:
 *      primary-expression
 *      postfix-expression [ expression ]
 *      postfix-expression [[ expression ]]
 *      postfix-expression ( [argument-expression-list] )
 * >>>Will Prefetch next token<<<
 */
static nxc_ast_t *nxc_parse_postfix(nxc_compiler_t*const compiler)
{
    int token;
    nxc_ast_t *left_factor,*unary,*father;

    ///save father node first ...
    father = compiler->curr_ast_node;

    ///get left factor first ...
    left_factor = nxc_parse_factor(compiler);
    if(!left_factor) return 0;

    ///parse right(postfix) token then ...
    for (;;)
    {
        token = nxc_do_lex(compiler);
        switch (token)
        {
        case nxc_token_left_paren:    ///'('
            ///auto fix left_factor's father node ...
            unary = nxc_postfix_do_call(compiler,left_factor);
            if(!unary) return 0;
            left_factor = unary;
            break;
        case nxc_token_left_bracket:  ///'['
            ///auto fix left_factor's father node ...
            unary = nxc_postfix_do_index(compiler,
                                         left_factor,
                                         nxc_op_index_l,
                                         nxc_token_right_bracket);
            if(!unary) return 0;
            left_factor = unary;
            break;
        case nxc_token_left_bracket1: ///'[['
            ///auto fix left_factor's father node ...
            unary = nxc_postfix_do_index(compiler,
                                         left_factor,
                                         nxc_op_index8,
                                         nxc_token_right_bracket1);
            if(!unary) return 0;
            left_factor = unary;
            break;
        case nxc_token_left_bracket2: ///'[[['
            ///auto fix left_factor's father node ...
            unary = nxc_postfix_do_index(compiler,
                                         left_factor,
                                         nxc_op_index32,
                                         nxc_token_right_bracket2);
            if(!unary) return 0;
            left_factor = unary;
            break;
        case nxc_token_dot:           ///'.'
            ///auto fix left_factor's father node ...
            unary = nxc_postfix_do_member(compiler,left_factor);
            if(!unary) return 0;
            left_factor = unary;
            break;
        case nxc_token_colon:
            ///do type cast 
            nxc_error(compiler,"type-cast is not supported!!!");
            ///a = ((b+c):TTT).ccc;
            break;
        default:
            ///fix whole factor's father node ...
            nxc_ast_set_father_node(left_factor,father);
            ///return whole 'factor' directly ...
            return left_factor;
        }
    }

    return 0;
}

/**===========================================================================*/

/**
 * parse {'+' '-' '*' '&' '~' '!'} prefix unary ...
 * prefix : '+' '-' '*' '&' '~' '!'
 *  $Unary :
 *           $prefix $Unary
 * >>>Will prefetch next token<<<
 */
static nxc_ast_t *nxc_prefix_do_prefix(nxc_compiler_t*const compiler,int opcode)
{
    nxc_ast_t *expr;
    nxc_ast_t *unary;

    expr = nxc_new_ast_node(compiler);
    if(!expr) return 0;
    nxc_ast_set_opcode(expr,opcode);
    ///set current ast node to myself ...
    compiler->curr_ast_node = expr;
    ///trnna get next prefix or factor ...
    nxc_do_lex(compiler);
    ///do rescurses call ...
    unary = nxc_parse_unary(compiler);
    if (!unary) return 0;
    nxc_prefix_ast_set_unary(expr,unary);

    ///---alloc xseg space for instruction---
    nxc_compiler_alloc_xseg_space(compiler,sizeof(nxc_instr_t));

    return expr;
}

/**
 * parse ($Type) $Unary ...
 * if it's not cast , then it;s must be a (postfix)unary , we pass it to 
 * postfix directly !!! 
 * return 0 means error ...
 * >>>Will prefetch next token<<<
 */
static nxc_ast_t *nxc_prefix_do_cast(nxc_compiler_t*const compiler)
{
    int token;
    nxc_ast_t *expr;
    
    token = nxc_do_lex(compiler);
    ///inside the paren should be treat as expression
    expr = nxc_parse_expression(compiler);
    if(nxc_expect_current(compiler,nxc_token_right_paren)) return 0;

    ///prefetch next to adjust the unary ...
    nxc_do_lex(compiler);
    
    return expr;
}

/**
 * unary-expression:
 *      prefix-expression  : & * + - ! ~ @ unary-expression
 *      postfix-expression : . ( [ expression
 *      ( type-name ) unary-expression
 *      sizeof ( type-name )
 * @return ast node created ...
 * @return 0 means error ...
 * >>>Will prefetch next token<<<
 */
static nxc_ast_t *nxc_parse_unary(nxc_compiler_t*const compiler)
{
    int token;

    token = nxc_cur_tok_type(compiler);
    ///parse prefix unary ...
    switch (token)
    {
    case nxc_token_bit_and:     ///& , address
        return nxc_prefix_do_prefix(compiler,nxc_op_address);
    case nxc_token_add:         ///+ , positive
        return nxc_prefix_do_prefix(compiler,nxc_op_positive);
    case nxc_token_sub:         ///- , negative
        return nxc_prefix_do_prefix(compiler,nxc_op_negative);
    case nxc_token_mul:         ///* ,
    {
        return nxc_parse_postfix(compiler);
    }
    case nxc_token_inc:         ///"++"
        return nxc_prefix_do_prefix(compiler,nxc_op_inc);
    case nxc_token_dec:         ///"--"
        return nxc_prefix_do_prefix(compiler,nxc_op_dec);
    case nxc_token_not:         ///! , not
        return nxc_prefix_do_prefix(compiler,nxc_op_not);
    case nxc_token_compensation:///~ , compensation
        return nxc_prefix_do_prefix(compiler,nxc_op_compensation);
    default:
        ///finally,this expression might be a postfix unary or factor ...
        return nxc_parse_postfix(compiler);
    }

    return 0;
}

/**===========================================================================*/
/**
 * trnna parse binary expression ...
 * format : $unary $op $expression
 *                 |                       ^
 *             [binary]                    |
 *            /        \                   |
 *         [unary] op [unary]              |
 *                       |                 |
 *                    [prefix]             |
 *                       |                 |
 *                    [postfix]            |
 *                       |                 |
 *                    [factor]----+        |
 *                   /     \       \       |
 *                [ID]  [Constant]  [ (Expression) ]
 *
 * >>>will prefetch the next token<<<
 */
static nxc_ast_t *nxc_parse_binary_expression(nxc_compiler_t*const compiler,
                                              int curr_level)
{
    int token,opcode,new_level;
    nxc_ast_t *left,*right,*bin,*father;

    ///save father node first ...
    father = compiler->curr_ast_node;

    ///parse left unary ...
    left = nxc_parse_unary(compiler);
    if(!left) return 0;
    ///nxc_parse_unary() will prefetch next token ...
    token = nxc_cur_tok_type(compiler);
    ///parse right expression ...
    while(nxc_is_binary_operator(token))///make sure we just parse scope of binary !!!
    {
        opcode = nxc_binary_operator_to_opcode(token);
        new_level = nxc_get_opcode_level(opcode);
        if(new_level < curr_level) break; ///new < current     , do left-join
        bin = nxc_new_ast_node(compiler); ///new >= curr_level , do right-join
        if(!bin) return 0;
        nxc_ast_set_opcode(bin,opcode);
        ///low level expr is treat as a left factor ...
        nxc_expr_ast_set_left(bin,left);
        ///set current ast node to myself ...
        compiler->curr_ast_node = bin;
        ///>>>fix left's father_node ...
        nxc_ast_set_father_node(left,bin);
        ///fetch next token and parse binary ...
        nxc_do_lex(compiler); ///skip '$operator' , fetch next token

        ///---alloc xseg space for instruction---/// instr $R bin_op $R
        nxc_compiler_alloc_xseg_space(compiler,sizeof(nxc_instr_t));

        ///high privilege expr is set as right child
        ///+1 means calc left factor first ...
        ///+0 means calc right factor if we meet the same-level-operator ...
        right = nxc_parse_binary_expression(compiler,new_level+1);
        if (!right) return 0;
        nxc_expr_ast_set_right(bin,right);
        ///save previous expression result ...
        ///current binary a left , recursely process rest (binary) as right
        left = bin;
        ///fetch current (low-level) token after high-level expre return ...
        token = nxc_cur_tok_type(compiler);
    }

    ///fix father node of whole expression ...
    nxc_ast_set_father_node(left,father);

    return left;
}

/**
 * parse assignment expression ...
 * !!!assignment expression will calculate right first!!!
 * return 0 means error 
 * >>>Will prefetch next token<<<
 * ::: a=b=c
 *      = 
 *     / \
 *    a   =
 *       / \
 *      b   c
 */
static nxc_ast_t* nxc_parse_assignment_expression(nxc_compiler_t*const compiler)
{
    int token,opcode;
    nxc_ast_t *left,*right,*assign_ast,*father;

    ///save father node first ...
    father = compiler->curr_ast_node;

    ///>>>left part should be a "binary-expression".
    left = nxc_parse_binary_expression(compiler,0);
    if(!left) return 0;

    token = nxc_cur_tok_type(compiler);
    if(nxc_is_assignment_operator(token)) ///if it's a assignment ...
    {
        ///create a assignment ast-node ...
        assign_ast = nxc_new_ast_node(compiler);
        if(!assign_ast) return 0;
        nxc_do_lex(compiler); ///skip $operator,fetch next token ...
        opcode = nxc_assignment_operator_to_opcode(token);///set opcode
        nxc_ast_set_opcode(assign_ast,opcode);
        nxc_expr_ast_set_left(assign_ast,left);///set left expr

        ///---alloc xseg space for instruction---///instr $R = $R
        nxc_compiler_alloc_xseg_space(compiler,sizeof(nxc_instr_t));

        ///set current ast node to myself ...
        compiler->curr_ast_node = assign_ast;
        ///>>>fix left's father_node to current ...
        nxc_ast_set_father_node(left,assign_ast);
        ///>>>right part should be a "assignment expression".
        ///this will cause a recurse call
        ///and right expression will be calculated first!!!
        ///a = b = c;  then b = c will be calculated first.
        right = nxc_parse_assignment_expression(compiler);///right
        if(!right) return 0;
        nxc_expr_ast_set_right(assign_ast,right);
        ///save current tree as left factor and process right recursesly...
        left = assign_ast;
    }

    ///fix whole expression's father node ...
    nxc_ast_set_father_node(left,father);

    return left;
}

/**
 * parse (comma)expression and generate ast ...
 * >>>Will prefetch next token<<<
 * @rev:2010-9-26 12:43:31
 * 
 * x = (a,b,c); ===>a ; b ; x = c;
 *     [1]             [2]
 *      ,               ,
 *     / \             / \
 *    ,    c          c   ,
 *   / \                 / \
 *  a   b               b   a
 * ----------------------------------
 * [1] will cause a confilect of register allocation while translate ...
 * that is , we always drop right child result-register , but [1] require drop 
 * left child's result register , this will cause a bad confilect with stack-like
 * register allocation (though we could do another allocation , but we just want
 * to keep it compatible with stack virtual machine ...)
 * so , we should generate [2] tree and make it run smoothly ...
 * @rev:2010-9-26 13:01:59
 * it's not necessary ...
 * (a,b,c)  ===>  [1]
 * while [a] complete ,  we release reg associate with a , then [b]
 * then reg_of_b == reg_of_a , we just don't release [b] and go on 
 * then the final result is [b] ...
 * @rev:2010-10-3---------------------------------------------------------------
 * layered expression:
 *   $expression (comma expression)
 *   |   +-->$assignment                  ,   $assignment , ...
 *   |       +->($binary Bop $binary op...)       +->($binary op $binary op ...)
 *   |             +-->$unary
 *   |                  +-->($prefix $factor $postfix)
 */
static nxc_ast_t *nxc_parse_expression(nxc_compiler_t*const compiler)
{
    nxc_ast_t *left_expr,*right,*ast,*father;

    ///save father node first ...
    father = compiler->curr_ast_node;

    ///trnna parse left as assignment expression ...
    left_expr = nxc_parse_assignment_expression(compiler);
    if(!left_expr) return 0;
    ///loop and parse next binary expr if we meet ',' ...
    while(nxc_cur_tok_type(compiler) == nxc_token_comma)
    {
        ast = nxc_new_ast_node(compiler); ///create a new ast ...
        nxc_ast_set_opcode(ast,nxc_op_comma);
        ///>>>left expr is trapped ...
        nxc_expr_ast_set_left(ast,left_expr);
        ///set current ast node to myself ...
        compiler->curr_ast_node = ast;
        ///>>>fix left node's father_node ...
        nxc_ast_set_father_node(left_expr,ast);
        ///skip '$operator' and fetch next token ...
        nxc_do_lex(compiler);
        ///>>>do parse right ...
        right = nxc_parse_assignment_expression(compiler);
        if(!right) return 0;
        ///set right node ...
        nxc_expr_ast_set_right(ast,right);
        ///save last expression result ...
        ///this will generate a left-first-tree ...
        ///and last binary is the most right tree-node ...
        left_expr = ast;
    }

    ///>>>fix whole expression's father node ...
    nxc_ast_set_father_node(left_expr,father);
    
    return left_expr;
}

/**===========================================================================*/

/**
 * parse '$expression ;'
 * and prefetch next ...
 * Expression-Statement AST will release unused register
 * Expression-AST will hold a register to hold result !!!
 */
static nxc_ast_t *nxc_parse_expression_statement(nxc_compiler_t*const compiler)
{
    nxc_ast_t *ast;
    nxc_ast_t *expr;

    ast = nxc_new_ast_node(compiler);
    if(!ast) return  0;
    nxc_ast_set_opcode(ast,nxc_op_expression);

    ///set current ast node to myself ...
    compiler->curr_ast_node = ast;

    ///parse expr as child ...
    expr = nxc_parse_expression(compiler);
    if(!expr) return 0;
    nxc_expr_stmt_ast_set_expr(ast,expr);
    if(nxc_expect_current(compiler,nxc_token_semicolon)) return 0;

    nxc_do_lex(compiler);

    return ast;
}

/**
 * if - then - else statement ...
 * and prefetch next ...
 */
static nxc_ast_t *nxc_parse_if_statement(nxc_compiler_t*const compiler)
{
    int token;
    nxc_ast_t *ast,*cond,*then_stmt,*else_stmt;

    if(nxc_expect(compiler,nxc_token_left_paren)) return 0;///check '('
    ast = nxc_new_ast_node(compiler);
    if(!ast) return 0;
    nxc_ast_set_opcode(ast,nxc_op_if);

    ///set current ast node to myself ...
    compiler->curr_ast_node = ast;

    ///===alloc xseg space for prolog===///max 2 instr ... 
    nxc_compiler_alloc_xseg_space(compiler,2 * sizeof(nxc_instr_t));

    ///parse $Condition
    nxc_do_lex(compiler);///prefetch next to do-expression ...
    cond = nxc_parse_expression(compiler);
    if(!cond) return 0;
    nxc_if_stmt_ast_set_cond(ast,cond);

    if(nxc_expect_current(compiler,nxc_token_right_paren)) return 0;///check ')'

    token = nxc_do_lex(compiler);
    if(token != nxc_token_semicolon)///parse then stmt ...
    {
        ///set current ast node to myself ...
        compiler->curr_ast_node = ast;

        ///parse $then
        then_stmt = nxc_parse_statement(compiler);
        if(!then_stmt) return 0;
        nxc_if_stmt_ast_set_then_stmt(ast,then_stmt);
    }
    else ///if(expr) ; ===> this is a empty statement !!!
    {   ///if we meet semincolon , skip it ...
        nxc_do_lex(compiler);
    }

    token = nxc_cur_tok_type(compiler);
    if(token != nxc_token_else) return ast; ///no else found ...

    ///parse else stmt ...
    token = nxc_do_lex(compiler);
    if(token != nxc_token_semicolon)
    {
        ///set current ast node to myself ...
        compiler->curr_ast_node = ast;

        ///parse $else
        else_stmt = nxc_parse_statement(compiler);
        if(!else_stmt) return 0;
        nxc_if_stmt_ast_set_else_stmt(ast,else_stmt);
    }
    else
    {   ///prefetch next token ...
        nxc_do_lex(compiler);
    }

    return ast;
}

/**
 * parse ' for (init;cond;next) loop_stmt; '
 * and prefetch next ...
 */
static nxc_ast_t *nxc_parse_for_statement(nxc_compiler_t*const compiler)
{
    int token;
    nxc_ast_t *ast,*init_stmt,*cond,*next_stmt,*loop_stmt,*old_loop;

    if(nxc_expect(compiler,nxc_token_left_paren)) return 0;///check '('

    ///===alloc xseg space for prolog===///max 3 Instruction
    nxc_compiler_alloc_xseg_space(compiler,3 * sizeof(nxc_instr_t));

    ///create a loop node ...
    ast = nxc_new_ast_node(compiler);
    if(!ast) return 0;
    nxc_ast_set_opcode(ast,nxc_op_for);
    nxc_loop_stmt_ast_init_break_continue_list(ast);

    token = nxc_do_lex(compiler);
    if(token!=nxc_token_semicolon)
    {
        ///set current ast node to myself ...
        compiler->curr_ast_node = ast;

        ///parse $INIT
        init_stmt = nxc_parse_expression(compiler);
        if(!init_stmt) return 0;
        nxc_loop_stmt_ast_set_init_stmt(ast,init_stmt);

        if(nxc_expect_current(compiler,nxc_token_semicolon)) return 0;
    }

    token = nxc_do_lex(compiler);
    if(token != nxc_token_semicolon)
    {
        ///set current ast node to myself ...
        compiler->curr_ast_node = ast;

        ///parse $Condition
        cond = nxc_parse_expression(compiler);
        if(!cond) return 0;
        nxc_loop_stmt_ast_set_cond(ast,cond);

        if(nxc_expect_current(compiler,nxc_token_semicolon)) return 0;
    }

    token = nxc_do_lex(compiler);
    if(token != nxc_token_right_paren)
    {
        ///set current ast node to myself ...
        compiler->curr_ast_node = ast;

        ///parse $NEXT
        next_stmt = nxc_parse_expression(compiler);
        if(!next_stmt) return 0;
        nxc_loop_stmt_ast_set_next_stmt(ast,next_stmt);

        if(nxc_expect_current(compiler,nxc_token_right_paren)) return 0;
    }

    ///>>>register as current loop node and save old one ...
    old_loop = compiler->curr_loop_node;
    ///>>>set new loop-zone
    ///set curr loop node so that child break/continue will find it's father
    compiler->curr_loop_node = ast;

    ///parse loop body ...
    token = nxc_do_lex(compiler);
    if(token != nxc_token_semicolon)
    {
        ///set current ast node to myself ...
        compiler->curr_ast_node = ast;

        ///pares $Body
        loop_stmt = nxc_parse_statement(compiler);
        if(!loop_stmt) return 0;
        nxc_loop_stmt_ast_set_loop_stmt(ast,loop_stmt);
    }

    ///<<<do restore loop-zone...
    compiler->curr_loop_node = old_loop;

    return ast;
}
/**
 * parse ' while (cond) loop_stmt; '
 * and prefetch next ...
 */
static nxc_ast_t *nxc_parse_while_statement(nxc_compiler_t*const compiler)
{
    int token;
    nxc_ast_t *ast,*cond,*loop_stmt,*old_loop;

    if(nxc_expect(compiler,nxc_token_left_paren)) return 0;///check '('

    ///===alloc xseg space for prolog===///max 2 Instruction
    nxc_compiler_alloc_xseg_space(compiler,2 * sizeof(nxc_instr_t));

    ///create a loop node ...
    ast = nxc_new_ast_node(compiler);
    if(!ast) return 0;
    nxc_ast_set_opcode(ast,nxc_op_while);
    nxc_loop_stmt_ast_init_break_continue_list(ast);

    ///set current ast node to myself ...
    compiler->curr_ast_node = ast;

    ///parse $Condition
    nxc_do_lex(compiler);
    cond = nxc_parse_expression(compiler);
    if(!cond) return 0;
    nxc_loop_stmt_ast_set_cond(ast,cond);

    if(nxc_expect_current(compiler,nxc_token_right_paren)) return 0;///check ')'

    ///>>>register as current loop node and save old one ...
    old_loop = compiler->curr_loop_node;
    ///set curr loop node so that child break/continue will find it's father
    compiler->curr_loop_node = ast;

    token = nxc_do_lex(compiler);
    if(token!=nxc_token_semicolon)
    {
        ///set current ast node to myself ...
        compiler->curr_ast_node = ast;

        ///parse $BODY
        loop_stmt = nxc_parse_statement(compiler);
        if(!loop_stmt) return 0;
        nxc_loop_stmt_ast_set_loop_stmt(ast,loop_stmt);
    }
    else
    {   ///prefetch next ...
        nxc_do_lex(compiler);
    }

    ///<<<restore old loop zone...
    compiler->curr_loop_node = old_loop;

    return ast;
}

/**
 * parse 'do{ $stmt(s) } while($expr);'
 * and prefetch next ...
 */
static nxc_ast_t *nxc_parse_do_statement(nxc_compiler_t*const compiler)
{
    nxc_ast_t *ast,*loop_stmt,*cond,*old_loop;

    if(nxc_expect(compiler,nxc_token_left_brace)) return 0;///check '{'

    ///===alloc xseg space for prolog===///max 1 Instruction
    nxc_compiler_alloc_xseg_space(compiler,1 * sizeof(nxc_instr_t));

    ast = nxc_new_ast_node(compiler);
    if(!ast) return 0;
    nxc_ast_set_opcode(ast,nxc_op_do);
    nxc_loop_stmt_ast_init_break_continue_list(ast);

    ///>>>register as current loop node and save old one ...
    old_loop = compiler->curr_loop_node;
    ///set curr loop node so that child break/continue will find it's father
    compiler->curr_loop_node = ast;

    ///set current ast node to myself ...
    compiler->curr_ast_node = ast;

    ///parse $BODY
    ///!!! this should be a statement block ...
    loop_stmt = nxc_parse_statement_block(compiler);
    if(!loop_stmt) return 0;
    nxc_loop_stmt_ast_set_loop_stmt(ast,loop_stmt);

    ///<<<restore old loop node ...
    compiler->curr_loop_node = old_loop;

    if(nxc_expect_current(compiler,nxc_token_while)) return 0; ///check 'while'
    if(nxc_expect(compiler,nxc_token_left_paren)) return 0; ///check '('

    ///set current ast node to myself ...
    compiler->curr_ast_node = ast;

    ///parse $Condition
    nxc_do_lex(compiler);///prefetch next to do-expression ...
    cond = nxc_parse_expression(compiler);
    if(!cond) return 0;
    nxc_loop_stmt_ast_set_cond(ast,cond);

    if(nxc_expect_current(compiler,nxc_token_right_paren)) return 0;///check ')'
    if(nxc_expect(compiler,nxc_token_semicolon)) return 0;///check ';'
    ///prefetch next ...
    nxc_do_lex(compiler);

    return ast;
}

/**
 * parse 'continue;'
 * and prefetch next ...
 */
static nxc_ast_t *nxc_parse_continue_statement(nxc_compiler_t*const compiler)
{
    nxc_ast_t *ast,*loop_node;
    
    if(nxc_expect(compiler,nxc_token_semicolon)) return 0;///check ';'
    
    ///===alloc xseg space for prolog===///max 1 Instruction
    nxc_compiler_alloc_xseg_space(compiler,1 * sizeof(nxc_instr_t));

    ast = nxc_new_ast_node(compiler);
    if(!ast) return 0;
    nxc_ast_set_opcode(ast,nxc_op_continue);

    ///###@rev:2010-10-9 14:27:28 this need to be fixed for switch!!!
    ///>>>fix father node ...
    nxc_ast_set_father_node(ast,compiler->curr_loop_node);
    ///fix loop node that we ('continue') are in .
    loop_node = compiler->curr_loop_node;
    if(!loop_node)
    {
        nxc_error(compiler,"continue stand OUT of a loop statement!");
        return 0;
    }
    nxc_continue_stmt_ast_set_loop_node(ast,loop_node);
    ///let father loop-node to trace this continue-ast
    nxc_loop_stmt_ast_trace_continue(loop_node,ast);
    ///prefetch next token ...
    nxc_do_lex(compiler);

    return ast;
}

/**
 * parse 'break;'
 * and prefetch next ...
 */
static nxc_ast_t *nxc_parse_break_statement(nxc_compiler_t*const compiler)
{
    nxc_ast_t *ast,*loop_node;
    
    if(nxc_expect(compiler,nxc_token_semicolon)) return 0;

    ///===alloc xseg space for prolog===///max 1 Instruction
    nxc_compiler_alloc_xseg_space(compiler,1 * sizeof(nxc_instr_t));

    ast = nxc_new_ast_node(compiler);
    if(!ast) return 0;
    nxc_ast_set_opcode(ast,nxc_op_break);
    ///>>>fix father node ...
    nxc_ast_set_father_node(ast,compiler->curr_loop_node);
    ///fix loop-node of break-ast
    loop_node = compiler->curr_loop_node;
    if(!loop_node)
    {
        nxc_error(compiler,"continue stand OUT of a loop statement!");
        return 0;
    }
    nxc_break_stmt_ast_set_loop_node(ast,loop_node);
    ///register this continue-node to current loop-node ...
    nxc_loop_stmt_ast_trace_break(loop_node,ast);
    ///prefetch next token ...
    nxc_do_lex(compiler);
    
    return ast;
}

/**
 * parse 'return $expr'
 * and prefetch next ...
 */
static nxc_ast_t *nxc_parse_return_statement(nxc_compiler_t*const compiler)
{
    int token;
    nxc_ast_t *ast,*result;

    ast = nxc_new_ast_node(compiler);
    if(!ast) return 0;
    nxc_ast_set_opcode(ast,nxc_op_return);

    ///===alloc xseg space for prolog===///max 1 Instruction
    nxc_compiler_alloc_xseg_space(compiler,1 * sizeof(nxc_instr_t));

    token = nxc_do_lex(compiler);
    if(token != nxc_token_semicolon)///check ';'
    {
        ///set current ast node to myself ...
        compiler->curr_ast_node = ast;

        ///parse $ReturnValue
        result = nxc_parse_expression(compiler);
        if(!result) return 0;
        nxc_return_stmt_ast_set_result(ast,result);

        if(nxc_expect_current(compiler,nxc_token_semicolon)) return 0;
    }
    nxc_do_lex(compiler);

    return ast;
}

/**
 * parse a statement and generate the ast list ...
 * return 0 means error ...
 * >>>Will prefetch next token<<<
 */
static nxc_ast_t *nxc_parse_statement(nxc_compiler_t*const compiler)
{
    int token;

    token = nxc_cur_tok_type(compiler);
    switch (token)
    {
    case nxc_token_if:            ///'if(expr){}else{}'
        return nxc_parse_if_statement(compiler);
    case nxc_token_for:           ///'for(init;cond;next){}'
        return nxc_parse_for_statement(compiler);
    case nxc_token_while:         ///'while(expr){}'
        return nxc_parse_while_statement(compiler);
    case nxc_token_do:            ///'do {} while(expr)'
        return nxc_parse_do_statement(compiler);
    case nxc_token_continue:      ///'continue'
        return nxc_parse_continue_statement(compiler);
    case nxc_token_break:         ///'break'
        return nxc_parse_break_statement(compiler);
    case nxc_token_return:        ///'return'
        return nxc_parse_return_statement(compiler);
    case nxc_token_left_brace:    ///'{'
        return nxc_parse_statement_block(compiler);
    default:                      ///expression statement : *xxx = yyy;
        return nxc_parse_expression_statement(compiler);
    }

    return 0;
}

/**
 * parse statement block ...
 *    [1].var declaration ...
 *    [2].normal statement ...
 *
 * >>>Will prefetch next token<<<
 */
static nxc_ast_t *nxc_parse_statement_block(nxc_compiler_t*const compiler)
{
    int token;
    nxc_ast_t *ast;
    nxc_ast_t *stmt;

    ast = nxc_new_ast_node(compiler);
    nxc_ast_set_opcode(ast,nxc_op_block);
    nxc_block_stmt_ast_init_stmt_list(ast);

    nxc_do_lex(compiler);
    for (;;)
    {
        ///set current ast node to myself ...
        compiler->curr_ast_node = ast;

        token = nxc_cur_tok_type(compiler);
        switch (token)
        {
        case nxc_token_var:
            ///process variable declaration ...         
            if(nxc_parse_var_declaration(compiler,nxc_sym_local_var)) return 0;
            continue; ///goon next token ...

        case nxc_token_const:
            if(nxc_parse_const_declaration(compiler)) return 0;
            continue;

        case nxc_token_semicolon:
            nxc_do_lex(compiler);
            continue;
        default:
            ///process statement ...
            break;
        }
        ///check '}'
        if(token == nxc_token_right_brace) break; 

        ///>>>re-set myself as curr-ast-node(father node) ...
        compiler->curr_ast_node = ast;

        ///parse sub statements ...
        stmt = nxc_parse_statement(compiler); ///this will prefetch next token..
        if(!stmt) return 0;
        ///trace all sub statements ...(add statement to block)
        nxc_block_stmt_ast_add_stmt(ast,stmt);
    }
    ///prefetch next ...
    nxc_do_lex(compiler);

    return ast;
}

/**###########################################################################*/
/**###########################################################################*/
/**###########################################################################*/

/**
 * trnna parse a variable 
 * @return 0 error  non-zero means var name ... ...
 */
static nxc_str_node_t* __nxc_parse_a_var(nxc_compiler_t *const compiler,
                                         int         *index_type,
                                         int         *index_cnt,                             
                                         char        **type_name)///init const
{
    int token;
    int is_array;
    int right_bracket;
    nxc_token_t *tok;
    nxc_str_node_t *str_node;
    
    ///the current token should be a id ...
    if(nxc_expect_current(compiler,nxc_token_id)) return 0;
    
    *index_cnt  = 0;
    *index_type = 0;
    *type_name  = 0;

    tok      = nxc_cur_tok(compiler);
    str_node = nxc_token_get_str_node(tok);///save str-node for global symbol...
    is_array = 0;
    ///check post fix ...
    token = nxc_do_lex(compiler);
    ///save index-type
    *index_type = token;
    switch (token)
    {
    case nxc_token_left_bracket:
        is_array = 1;
        right_bracket = nxc_token_right_bracket;
        break;
    case nxc_token_left_bracket1:
        is_array = 1;
        right_bracket = nxc_token_right_bracket1;
        break;
    case nxc_token_left_bracket2:///'[[[ $const ]]] : T'
        is_array = 1;
        right_bracket = nxc_token_right_bracket2;
        break;
    default:
        is_array = 0;
        right_bracket = 0;
        *index_type = 0;    ///set default value!!!
        break;
    }

    ///trnna parse array ...
    if (is_array) 
    {
        token = nxc_do_lex(compiler);
        if(token!=right_bracket)
        {
            if(nxc_expect_current(compiler,nxc_token_const_long)) return 0;         
            tok = nxc_cur_tok(compiler);
            *index_cnt =nxc_token_get_long_val(tok);///read number ...
			token = nxc_do_lex(compiler); ///read '*'
			if(token == nxc_token_mul){   ///just a shit hack for convinence...
				if(nxc_expect(compiler,nxc_token_const_long)) return 0;         
				tok = nxc_cur_tok(compiler);
				*index_cnt *=nxc_token_get_long_val(tok);///read number and calc
				nxc_do_lex(compiler);     ///skip number ...
			}
			if(*index_cnt <= 0){
                nxc_error(compiler,"array size can not be '0'");
                return 0;
            }
            if(nxc_expect_current(compiler,right_bracket)) return 0;///check ']'
        }
        else
        {
            *index_cnt = 0;///mark!!! 'no-index' such as 'var kkk[]';
        }
        token = nxc_do_lex(compiler);///prefetch next ...
    }
    else ///non-array ...
    {
        *index_cnt = 0;
    }
    
    ///trnna parse type 
    if(token == nxc_token_colon)
    {
        ///next should be a id
        if(nxc_expect(compiler,nxc_token_id)) return 0;
        ///save the type name ...
        *type_name = nxc_token_get_str_val(nxc_cur_tok(compiler));
        ///prefetch next ...
        token = nxc_do_lex(compiler);
    }

    ///return var name ...
    return str_node;
}

/**
 * HARD-CODED code , damn !!!
 * generate load ast for variable initializer
 * @return 0 means okay ...
 */
static 
int nxc_parse_lvar_initializer(nxc_compiler_t*const compiler,char*var_name)
{
    nxc_ast_t *ast,*left,*right,*father;
    nxc_ast_t *expr;

    ///save father ast first ...
    father = compiler->curr_ast_node;

    ///build a expr-stmt to hold assignment expression ...
    expr = nxc_new_ast_node(compiler);
    if (!expr) return -1;
    nxc_ast_set_opcode(expr,nxc_op_expression);

    ///link to container block ...
    nxc_ast_set_father_node(expr,father);
    ///add expression-statement to block-statement ...
    nxc_block_stmt_ast_add_stmt(father,expr);

    ///create assign ast first and set curr ast to me ...
    ast = nxc_new_ast_node(compiler);
    if (!ast) return -1;
    nxc_ast_set_opcode(ast,nxc_op_assign);

    ///===alloc xseg space for assign operation===
    nxc_compiler_alloc_xseg_space(compiler,sizeof(nxc_instr_t));//st $R,$R

    ///trace this ast ...
    nxc_ast_set_father_node(ast,expr);   ///link to father
    nxc_expr_stmt_ast_set_expr(expr,ast);///tell father to trace me
    
    ///>>>1.parse left id and gen ast ...
    left = __nxc_parse_id_factor(compiler,var_name);
    if (!left) return -1;
    ///trace left
    nxc_ast_set_father_node(left,ast); ///fix left's father to me
    nxc_expr_ast_set_left(ast,left);   ///trace left ast ...

	///>>>2.parse right as assignment_expression !!!
	right = nxc_parse_assignment_expression(compiler);
	if (!right) return -1;
	///trace right
    nxc_ast_set_father_node(right,ast); ///fix right's father to me
    nxc_expr_ast_set_right(ast,right);  ///trace right ast ...

    return 0;
}

/**
 * parse a var and generate symbol for it ...
 * return nonzero means error ...
 * >>>Will prefetch next token<<<
 */
static int nxc_parse_a_var(nxc_compiler_t*const compiler,int sym_type)
{
    nxc_sym_t *sym;
    char *name,*type_name;
    int is_array,index_type,index_cnt,sym_flag;
    int namelen,var_size;
    nxc_str_node_t *str_node;
    char __name[128];
    int __len=0;

    ///1.parse name , index type , initializer .
    str_node = __nxc_parse_a_var(compiler,&index_type,&index_cnt,&type_name);
    if (!str_node) return -1;
    name     = nxc_str_node_get_str(str_node);
    namelen  = nxc_str_node_get_len(str_node);
    switch (sym_type)
    {
    case nxc_sym_local_param:
    case nxc_sym_local_var:
    case nxc_sym_global_var:
        ///using name directly ...
        break;  
    case nxc_sym_member:
        ///generate global name ...
        __len=nxc_gen_member_fullname(__name,compiler->curr_struct_name,name);
        ///create a full-name string ...
        str_node = nxc_lex_create_str(compiler,__name,__len);
        if (!str_node)return -1;
        name     = nxc_str_node_get_str(str_node);
        namelen  = nxc_str_node_get_len(str_node);
        break;
    }

    ///2.check uniqueness ...
    sym = nxc_find_symbol(compiler,name,namelen);
    if(sym){
        nxc_error(compiler,"multi-declaration for symbol '%s'",name);
        return -1;
    }
    ///3.calc variable size ...
    is_array = index_type;   ///set array flag ...
    sym_flag = 0;            ///reset symflag ...
    var_size = 0;
    switch (index_type)
    {
    case 0:                      var_size = 1 * sizeof(long);           break;
    case nxc_token_left_bracket: var_size = index_cnt * sizeof(long);   break;
    case nxc_token_left_bracket1:var_size = index_cnt * sizeof(char);   break;
    case nxc_token_left_bracket2:var_size = index_cnt * sizeof(int);    break;
    }
    ///4.fix local parameter ...
    if (sym_type==nxc_sym_local_param)///convert array parameter to pointer ...
    {
        index_cnt = 0;          ///convert to pointer ...
        var_size = sizeof(long);///convert to pointer ...
        nxc_add_flag(sym_flag,nxc_symflag_lvalue); ///has lvalue ...
    }else{                            ///this is a lvar or gvar or member ...
        if (!index_cnt){ ///hack pointer :zero array treat as pointer ...
            var_size = sizeof(long);
            nxc_add_flag(sym_flag,nxc_symflag_lvalue); ///has lvalue ...
        }else{
            nxc_add_flag(sym_flag,nxc_symflag_array);///mark array,no lvalue...
        }
    }
    ///5.create symbol 
	sym = nxc_new_sym(compiler,str_node,sym_type,var_size,sym_flag);
    if (!sym) return -1;
    
    ///fix variable typename (and store to associated symbol)...
    if (is_array){///(with pointer included ...)
        ///the pointer's type is array , father type keep same with array...
        nxc_sym_set_typename(sym,"__array");    ///set current type ...
        nxc_sym_set_f_typename(sym,type_name);  ///set father type ...
    }else{///var only ...
        nxc_sym_set_typename(sym,type_name);    ///save current type ...
        nxc_sym_set_f_typename(sym,0);          ///no father type ...
    }

	///trnna parse the initializer for local var !!!
	if(nxc_cur_tok_type(compiler) == nxc_token_assign)
	{
		nxc_do_lex(compiler);///skip '='
		if (sym_type == nxc_sym_local_var){
			if (is_array){
				nxc_error(compiler,"array initializer is not supported!!!");
				return -1;
			}
			if (nxc_parse_lvar_initializer(compiler,name)){
				nxc_error(compiler,"failed to parse initializer!!!");
				return -1;
			}
		}
	}
    
    return 0;
}

/**===========================================================================*/
/**===========================================================================*/

/**
 * parse the variable declaration 
 * return non-zero mean error ...
 * >>>Will prefetch next token<<<
 * 'var a,b[100],c[[10]],def;'
 */
static int nxc_parse_var_declaration(nxc_compiler_t*const compiler,int sym_type)
{
    ///loop all var ...
    for (;;)
    {
        ///read first var ..
        nxc_do_lex(compiler);
        ///parse first var and prefetch next ...
        if(nxc_parse_a_var(compiler,sym_type)) return -1;

        //check ';'
        if(nxc_cur_tok_type(compiler) == nxc_token_semicolon) break;
        ///check ','
        if(nxc_expect_current(compiler,nxc_token_comma)) return -1;
    }
    //prefetch next ...
    nxc_do_lex(compiler);

    return 0;
}

/**
 * return non-zero means error ...
 */
static int nxc_parse_func_declaration(nxc_compiler_t*const compiler,int is_api)
{
    int token,param_cnt,namelen;
    nxc_token_t *tok;
    nxc_sym_t *func,*parm_sym;
    nxc_ast_t *stmt;
    char *func_name;
    nxc_str_node_t *str_node;
    char __name[128];
    int  __len;

    if(nxc_expect(compiler,nxc_token_id)) return -1;
    tok = nxc_cur_tok(compiler);
    str_node  = nxc_token_get_str_node(tok);
    func_name = nxc_str_node_get_str(str_node);
    namelen   = nxc_str_node_get_len(str_node);
    if (compiler->curr_struct_name)///we are inside a struct,this is a member...
    {
        ///generate full name ...
        __len = nxc_gen_member_fullname(__name,compiler->curr_struct_name,func_name);
        str_node = nxc_lex_create_str(compiler,__name,__len);
        if (!str_node) return -1;
        func_name = nxc_str_node_get_str(str_node);
        namelen   = nxc_str_node_get_len(str_node);
    }
    ///check uniqueness ...
    func = nxc_find_symbol(compiler,func_name,namelen);
    if(func){
        nxc_error(compiler,"multi-declaration for symbol '%s'",func_name);
        return -1;
    }

    ///precreate a function first ...
    func=nxc_new_sym(compiler,str_node,nxc_sym_global_function,0,0);
    if(!func) return -1;

    ///set current function first ...
    compiler->curr_function = func;
    param_cnt = 0;

    ///create 'this' parametr if this is a member function !!!
    if (compiler->curr_struct_name)
    {
        ///generate a local parameter ...
        parm_sym = nxc_new_sym_ex(compiler,"this",4,nxc_sym_local_param,sizeof(long),nxc_symflag_lvalue);
        if (!parm_sym) return -1;
        ///fix typename
        nxc_sym_set_typename(parm_sym,compiler->curr_struct_name);
        nxc_sym_set_f_typename(parm_sym,0);
        param_cnt++;
    }

    ///trnna parse parameter list ...
    if(nxc_expect(compiler,nxc_token_left_paren)) return -1;///check '('
    token = nxc_do_lex(compiler);                           ///fetch next ...
    if(token != nxc_token_right_paren)                      ///check ')'
    {
        for (;;)
        {
            if(token == nxc_token_var) nxc_do_lex(compiler);    ///skip 'var'
            if(nxc_cur_tok_type(compiler) == nxc_token_ellipsis)///check '...'
            {
                if(!param_cnt){///check if param count >1
                    nxc_error(compiler,"'...' can't be the first parameter");
                    return -1;
                }
                nxc_sym_add_flag(func,nxc_symflag_var_args);    ///fix flag
                if(nxc_expect(compiler,nxc_token_right_paren))return -1;//do ')'
                break;
            }
            ///should be a id ...
            if(nxc_parse_a_var(compiler,nxc_sym_local_param)) return -1;
            param_cnt ++;

            if(nxc_cur_tok_type(compiler)==nxc_token_right_paren) break;//chk')'
            if(nxc_expect_current(compiler,nxc_token_comma))return -1;//check','
            token = nxc_do_lex(compiler);///prefetch next ...
        }
    }
    
    ///save func's param count ...
    nxc_sym_set_param_cnt(func,param_cnt);

    ///check type 
    token = nxc_do_lex(compiler);
    if (token == nxc_token_colon)
    {
        if(nxc_expect(compiler,nxc_token_id)) return -1;

        tok = nxc_cur_tok(compiler);
        nxc_sym_set_f_typename(func,nxc_token_get_str_val(tok));//Set fatherType
        token = nxc_do_lex(compiler); ///prefetch next ...
    }
    if (token == nxc_token_semicolon)
    {
        ///this is a extern function ...
        nxc_sym_add_flag(func,nxc_symflag_extern);            ///add extern-flag
        if (is_api) nxc_sym_add_flag(func,nxc_symflag_fastcall);///fix api-flag

        compiler->curr_function = 0;
        compiler->curr_loop_node = 0;

		///prefetch next
		nxc_do_lex(compiler);
        return 0;
    }

    ///api has no function body!!!
    if(is_api){
        if(nxc_expect_current(compiler,nxc_token_semicolon)) return -1;//chk ';'
    }

    ///chekc '{'----------------------------------------------------------------
    if(nxc_expect_current(compiler,nxc_token_left_brace)) return -1;

    ///===alloc xseg space for prolog===
    nxc_compiler_alloc_xseg_space(compiler,sizeof(nxc_instr_t));///enter instr

    /**
     * parse function body here ...
     */
    stmt = nxc_parse_statement_block(compiler);
    if (!stmt) return -1;
    nxc_sym_set_func_stmt_ast(func,stmt);

    ///===alloc xseg space for prolog===
    nxc_compiler_alloc_xseg_space(compiler,sizeof(nxc_instr_t)); ///return instr

    /**
     * !!! clear current context of parser !!!----------------------------------
     */
    compiler->curr_function = 0;
    compiler->curr_loop_node = 0;

    ///okay ...
    return 0;
}

/**
 * return non-zero means error ...
 */
static int nxc_parse_struct_declaration(nxc_compiler_t*const compiler)
{
    int token;
    nxc_token_t *tok;
	int __len;
    char __name[128];

    if(nxc_expect(compiler,nxc_token_id))return -1;
    tok = nxc_cur_tok(compiler);
    ///save class name and namelen ...
    compiler->curr_struct_name    = nxc_token_get_str_val(tok);
    compiler->curr_struct_namelen = nxc_token_get_len(tok);
    compiler->curr_struct_size    = 0; ///reset struct size ...

	token = nxc_do_lex(compiler);
	if (token == nxc_token_semicolon) ///check ';'
	{
		///reset cotext ...
		compiler->curr_struct_name    = 0;
		compiler->curr_struct_namelen = 0;
		compiler->curr_struct_size    = 0;
		nxc_do_lex(compiler); ///prefetch next token ...
		return 0;
	}

    if(nxc_expect_current(compiler,nxc_token_left_brace)) return -1;///check '{'

    ///fet 'var' or 'func'
    token = nxc_do_lex(compiler);
    for (;;)
    {
        token = nxc_cur_tok_type(compiler);
        if (token == nxc_token_right_brace) break;

        switch (token)
        {
        case nxc_token_var:
            if(nxc_parse_var_declaration(compiler,nxc_sym_member)) return -1;
            break;
        case nxc_token_func:
            if(nxc_parse_func_declaration(compiler,0)) return -1;
            break;
		case nxc_token_api:
			if(nxc_parse_func_declaration(compiler,1)) return -1;
            break;
        default:
            nxc_error(compiler,"only var/func can be declared inside struct!");
            return -1;
        }
    }

    ///generate a const to hold the total-size ...
    __len = nxc_gen_member_fullname(__name,compiler->curr_struct_name,"__SIZE__");
    if(nxc_new_const_ex(compiler,__name,__len,nxc_token_const_long,sizeof(long),
                        (void*)compiler->curr_struct_size) == 0){
        return -1;
    }
    ///generate a local-fake-member to hold size-info ...
    if(nxc_new_sym_ex(compiler,__name,__len,nxc_sym_member,0,0) == 0) return -1;

    ///reset cotext ...
    compiler->curr_struct_name    = 0;
    compiler->curr_struct_namelen = 0;
    compiler->curr_struct_size    = 0;
        
    nxc_do_lex(compiler); ///prefetch next token ...
    return 0;
}

/**
 * return non-zero means error ...
 */
static int nxc_parse_const_declaration(nxc_compiler_t*const compiler)
{
    int token,namelen;
    nxc_token_t *tok;
    nxc_const_node_t *node;
    char *name;
    nxc_str_node_t *str_node;

    for (;;)
    {
        if(nxc_expect(compiler,nxc_token_id)) return -1;
        tok = nxc_cur_tok(compiler);                         ///get token ...
        str_node = nxc_token_get_str_node(tok);
        name = nxc_str_node_get_str(str_node);
        namelen   = nxc_str_node_get_len(str_node);
        if (nxc_expect(compiler,nxc_token_assign)) return -1; ///check  '='
        ///check contant token ...
        token = nxc_do_lex(compiler);
        switch (token)
        {
        case nxc_token_const_long:
        case nxc_token_const_float:
        case nxc_token_const_string:
            break;
        default:
            nxc_error(compiler,"constant is expected!!!");
            return -1;
        }

        ///create a new const node and trace it ...
        tok = nxc_cur_tok(compiler);
        node = nxc_new_const_ex(compiler,
                                name,namelen,
                                nxc_token_get_type(tok),
                                nxc_token_get_len(tok),
                                (void*)nxc_token_get_long_val(tok));
        if(!node) return -1;

        token = nxc_do_lex(compiler);
        if (token == nxc_token_semicolon) break;///check ';'
        if(nxc_expect_current(compiler,nxc_token_comma)) return -1;
    }
    
    nxc_do_lex(compiler);                                  ///pre-fetch next ...

    return 0;
}
/**###########################################################################*/
/**###########################################################################*/
/**###########################################################################*/

/**
 * parse input and generate ast ...
 * return 0 means okay ...
 */
static int nxc_do_parse(nxc_compiler_t *const compiler,char *filename,char *str)
{
    int token;

    if(!str||!str[0]) return 0;
    
    nxc_lex_set_filename(compiler->lexer,filename);///set current    filename ...
    nxc_lex_set_input(compiler->lexer,str);        ///set sourcecode buffer ...
    nxc_do_lex(compiler);                          ///ptr-read a token ...

    /**
     * lex and parse , generate the ast ...
     */
    for (;;)
    {
        ///clear tmp context data of parse between each...
        compiler->curr_ast_node = 0;

        token = nxc_cur_tok_type(compiler);
        switch (token)
        {
        case nxc_token_var:
            ///process variable declaration ...         
            if(nxc_parse_var_declaration(compiler,nxc_sym_global_var)) return-1;
            continue; ///goon next token ...

        case nxc_token_func:
            ///process function declaration ...         
            if(nxc_parse_func_declaration(compiler,0))return -2;//will prefetch
            continue; ///goon next token ...

        case nxc_token_const:
            ///process constant declaration ...         
            if(nxc_parse_const_declaration(compiler)) return -3;///will prefetch
            continue; ///goon next token ...
            
        case nxc_token_api:
            ///process function declaration ...         
            if(nxc_parse_func_declaration(compiler,1))return -4;//will prefetch
            continue; ///goon next token ...

        case nxc_token_struct:
        case nxc_token_class:
            ///process function declaration ...         
            if(nxc_parse_struct_declaration(compiler))return -5;//will prefetch
            continue; ///goon next token ...

        case nxc_token_semicolon:
            token = nxc_do_lex(compiler);
            continue;

        case nxc_token_eof:
        case nxc_token_bad:
            break;
        default:
            nxc_error(compiler,"Syntex error:unexpected token:%s",
                                nxc_get_token_name(token));
            return -3;
        }
        break;
    }

    return 0;
}
/**###########################################################################*/

/**
 * #############################################################################
 * #############################################################################
 * #############################################################################
 */












/**===========================================================================*/
/**========================== AST Translator =================================*/
/**===========================================================================*/

/**
 * allocate a new instructon for curr-function...
 */
___fast nxc_instr_t* nxc_new_instr(nxc_compiler_t*const compiler,int opcode)
{
    nxc_instr_t *instr;

    ///trnna use the precreated instruction ...
    if(compiler->curr_pre_creat_instr)
    {
        instr = compiler->curr_pre_creat_instr;
        compiler->curr_pre_creat_instr = 0;
        nxc_instr_set_opcode(instr,opcode);

        return instr;
    }

    ///alloc instr ...
    instr = (nxc_instr_t *)((char*)compiler->image->text_base + compiler->image->text_size);
    ///update actual text size ...
    compiler->image->text_size += sizeof(nxc_instr_t);
    ///rest space check ...
    if (compiler->image->text_size > compiler->image->text_seg_size){
        nxc_error(compiler,"Insucifficent TEXT Segment!!!\n");
        return 0;
    }
    ///do default init ...
    nxc_instr_do_init(instr);
    nxc_instr_set_opcode(instr,opcode);
    ///update instr-counter of func ...
    nxc_sym_inc_func_instr_cnt(compiler->curr_function);
    
    return instr;
}

/**
 * pre create a nop instr at current position ...
 * if there alread exist a pre-created instruction , just return this one ..
 * return 0 means error ...
 */
___fast nxc_instr_t *nxc_pre_creat_instr(nxc_compiler_t *const compiler)
{
    nxc_instr_t *instr;

    ///trnna return existed one ...
    if(compiler->curr_pre_creat_instr) return compiler->curr_pre_creat_instr;
    instr = nxc_new_instr(compiler,nxc_vmop_nop);
    if(instr){
        ///trace new and return ,,,
        compiler->curr_pre_creat_instr = instr;
        return instr;
    }
    nxc_error(compiler,"not enough memory!!!");
    return instr;
}

/**
 * register allocator and deallocator ...
 * because register is allocate like stack's push pop 
 * so allocate and deallocate is works like stack ...
 * @rev:2010-10-3 register index problem!!!
 * because register are index by a minus offset by ebp
 * so the offset should be act like follow!!!
 * 
 * low------------------------->high
 * +---------+--------+------------+
 * | reg1    |  reg0  |    ebp     |
 * +---------^--------+------------+
 *           |
 *   reg0----+
 * index_of( reg_0 ) = sizeof(long)  (ebp -4/8), NOT 0 !!!
 */
___fast int nxc_alloc_reg(nxc_compiler_t *const compiler)///push expr reg
{
    compiler->expr_sp += sizeof(long);
    ///upate max register area size if necessary ...
    if(compiler->expr_sp > compiler->expr_sp_max)
    {
        compiler->expr_sp_max = compiler->expr_sp;
    }

    return -(compiler->expr_sp);///ebp + offset (so a '-' prefix is required)!!!
}
///pop expr reg
___fast void nxc_free_reg(nxc_compiler_t *const compiler,int reg_index)
{
    compiler->expr_sp -= sizeof(long);
}
///pop all expr reg
___fast void nxc_free_all_reg(nxc_compiler_t *const compiler)
{
    compiler->expr_sp = 0;
}

/**
 * check if specific ast node is lvalue-node ...
 * !!!if opcode=nxc_op_id , then symbol info is required!!!
 * return 0 means failed ...
 */
___fast int nxc_is_lvalue_ast(nxc_compiler_t *const compiler,nxc_ast_t *ast)
{
    switch(ast->opcode)
    {
    case nxc_op_index8:   ///"["
    case nxc_op_index16:  ///"[[["
    case nxc_op_index32:    ///"[["
    case nxc_op_index_l:   ///"[["
        return 1;
    ///------------------factor-----------------
    case nxc_op_id:           ///"$ID"
        //if(nxc_is_flag_set(ast->u.id.sym->sym_flag,nxc_symflag_lvalue))return 1;
        if(nxc_sym_has_flag(nxc_ast_get_sym(ast),nxc_symflag_lvalue)) return 1;
        break;
    }
    //show error here ...
    nxc_ast_error(compiler,ast,"lvalue is required!!!");

    return 0;
}

/**===========================================================================*/
/**===========================================================================*/
/**===========================================================================*/

___fast int __nxc_reg1(nxc_instr_t *instr){return -instr->reg_index1 / sizeof(long);}
___fast int __nxc_reg2(nxc_instr_t *instr){return -instr->u.reg_index2 / sizeof(long);}

___fast void nxc_dump_binary_instr(nxc_compiler_t*const compiler,int opcode,nxc_instr_t *instr)
{
    nxc_printf(compiler,"reg%d = (reg%d %s reg%d)\n",
                __nxc_reg1(instr),
                __nxc_reg1(instr),
                nxc_get_vmop_name(opcode),
                __nxc_reg2(instr));
}

___fast void nxc_dump_binary_instr_i(nxc_compiler_t*const compiler,int opcode,nxc_instr_t *instr)
{
    nxc_printf(compiler,"reg%d = (reg%d %s reg%d)\n",
                __nxc_reg1(instr),
                __nxc_reg1(instr),
                nxc_get_vmop_name(opcode),
                nxc_instr_get_immed_l(instr));
}

___fast void nxc_dump_binary_instr_i1(nxc_compiler_t*const compiler,int opcode,nxc_instr_t *instr)
{
    nxc_printf(compiler,"reg%d = (%d %s reg%d)\n",
                __nxc_reg1(instr),
                nxc_instr_get_immed_l(instr),
                nxc_get_vmop_name(opcode),
                __nxc_reg1(instr));
}

static char *__nxc_float_vmop_name[]={">","<",">=","<=","+","-","*","/","-",};
___fast void nxc_dump_float_instr(nxc_compiler_t*const compiler,int offset,nxc_instr_t *instr)
{
    nxc_printf(compiler,"reg%d = (FReg%d %s FReg%d)\n",
                __nxc_reg1(instr),
                __nxc_reg1(instr),
                __nxc_float_vmop_name[offset],
                __nxc_reg2(instr));
}

___fast void nxc_dump_float_instr_i(nxc_compiler_t*const compiler,int offset,nxc_instr_t *instr)
{
    nxc_printf(compiler,"reg%d = (FReg%d %s %f)\n",
        __nxc_reg1(instr),
        __nxc_reg1(instr),
        __nxc_float_vmop_name[offset],
        nxc_instr_get_immed_f(instr));
}

___fast void nxc_dump_float_instr_i1(nxc_compiler_t*const compiler,int offset,nxc_instr_t *instr)
{
    nxc_printf(compiler,"reg%d = (%f %s FReg%d)\n",
        __nxc_reg1(instr),
        nxc_instr_get_immed_f(instr),
        __nxc_float_vmop_name[offset],
        __nxc_reg1(instr));
}

static char *__nxc_assign_vmop_name[]={"=","+=","-=","*=","/=","%=","|=","&=","^=","<<=",">>=",};
___fast void nxc_dump_assign_instr(nxc_compiler_t*const compiler,int opcode,nxc_instr_t *instr)
{
    char *cast_name;
    int offset;

    if (opcode>=nxc_vmop_st_l)     { cast_name ="long"; offset = opcode-nxc_vmop_st_l;}
    else if (opcode>=nxc_vmop_st32){ cast_name ="int";offset = opcode-nxc_vmop_st32;}
    else if (opcode>=nxc_vmop_st16){ cast_name ="short";offset = opcode-nxc_vmop_st16;}
    else                           { cast_name ="char";offset = opcode-nxc_vmop_st8;}
    nxc_printf(compiler,"*(%s *)[reg%d] %s reg%d\n",
                cast_name,
                __nxc_reg1(instr),
                __nxc_assign_vmop_name[offset],
                __nxc_reg2(instr));
}
___fast void nxc_dump_assign_lvar_instr(nxc_compiler_t*const compiler,int opcode,nxc_instr_t *instr)
{
    int offset;
    
    offset = opcode-nxc_vmop_st_lvar;
    nxc_printf(compiler,"[ebp + %d] %s reg%d\n",
        nxc_instr_get_immed_l(instr),
        __nxc_assign_vmop_name[offset],
        __nxc_reg1(instr));
}
/**
 * dump a instruction ...
 */
void nxc_dump_a_instr(nxc_compiler_t *const compiler,nxc_instr_t *instr)
{
    int opcode;
    char *name;

    name = nxc_get_vmop_name(instr->opcode);
    nxc_printf(compiler,"0x%08X  : ",instr);
    opcode = nxc_instr_get_opcode(instr);
    switch (opcode)
    {
    case nxc_vmop_halt: nxc_printf(compiler,"halt\n");  break;
    case nxc_vmop_nop:  nxc_printf(compiler,"nop\n");   break;
    ///-------------------------------------------------------------------------
    case nxc_vmop_ld8:         ///"*"
        nxc_printf(compiler,"reg%d = *(char*)reg%d\n",__nxc_reg1(instr),__nxc_reg1(instr));
        break;
    case nxc_vmop_ld16:        ///"*"
        nxc_printf(compiler,"reg%d = *(short*)reg%d\n",__nxc_reg1(instr),__nxc_reg1(instr));
        break;
    case nxc_vmop_ld32:        ///"*"
        nxc_printf(compiler,"reg%d = *(int*)reg%d\n",__nxc_reg1(instr),__nxc_reg1(instr));
        break;
    case nxc_vmop_ld_l:        ///"*"
        nxc_printf(compiler,"reg%d = *(long*)reg%d\n",__nxc_reg1(instr),    __nxc_reg1(instr));
        break;
    ///-----------------------------------------
    case nxc_vmop_ldx8:               ///reg=[reg+index]
        nxc_printf(compiler,"reg%d=*(char*)(reg%d+%d)\n",__nxc_reg1(instr),__nxc_reg1(instr),nxc_instr_get_immed32(instr));
        break;
    case nxc_vmop_ldx16:              ///reg=[reg+index*2]
        nxc_printf(compiler,"reg%d=*(short*)(reg%d+%d)\n",__nxc_reg1(instr),__nxc_reg1(instr),nxc_instr_get_immed32(instr));
        break;
    case nxc_vmop_ldx32:              ///reg=[reg+index*4]
        nxc_printf(compiler,"reg%d=*(int*)(reg%d+%d)\n",__nxc_reg1(instr),__nxc_reg1(instr),nxc_instr_get_immed32(instr));
        break;
    case nxc_vmop_ldx_l:              ///reg=[reg+index*4]
        nxc_printf(compiler,"reg%d=*(long*)(reg%d+%d)\n",__nxc_reg1(instr),__nxc_reg1(instr),nxc_instr_get_immed32(instr));
        break;
    ///-------------------------------------------------------------------------
    case nxc_vmop_index8:   ///"[["
        nxc_printf(compiler,"reg%d = *(char *)(reg%d + %reg%d)\n",__nxc_reg1(instr),__nxc_reg1(instr),__nxc_reg2(instr));
        break;
    case nxc_vmop_index16:   ///"[[[["
        nxc_printf(compiler,"reg%d = *(short *)(reg%d + 2 x %reg%d)\n",__nxc_reg1(instr),__nxc_reg1(instr),__nxc_reg2(instr));
        break;
    case nxc_vmop_index32:    ///"[[["
        nxc_printf(compiler,"reg%d = *(int *)(reg%d + 4 x %reg%d)\n",__nxc_reg1(instr),__nxc_reg1(instr),__nxc_reg2(instr));
        break;
    case nxc_vmop_index_l:   ///"["
        nxc_printf(compiler,"reg%d = *(long *)(reg%d + 4 x %reg%d)\n",__nxc_reg1(instr),__nxc_reg1(instr),__nxc_reg2(instr));
        break;
    ///-------------------------------------------------------------------------
    case nxc_vmop_ld_lvar:   ///marco-opcode ...
        nxc_printf(compiler,"reg%d = [ebp + %d]\n",__nxc_reg1(instr),nxc_instr_get_immed32(instr));
        break;
    case nxc_vmop_ld_lvar_addr:
        nxc_printf(compiler,"reg%d = ebp + %d\n",__nxc_reg1(instr),nxc_instr_get_immed32(instr));
        break;
    case nxc_vmop_ld_gvar:
        nxc_printf(compiler,"reg%d = [$data:%d]\n",__nxc_reg1(instr),nxc_instr_get_immed32(instr));
        break;
    case nxc_vmop_ld_gvar_addr:
        nxc_printf(compiler,"reg%d = $data:%d\n",__nxc_reg1(instr),nxc_instr_get_immed32(instr));
        break;
    case nxc_vmop_ldi:             ///func addr : const str : number ...
        nxc_printf(compiler,"reg%d = %d\n",__nxc_reg1(instr),nxc_instr_get_immed32(instr));
        break;
    case nxc_vmop_addi:             ///func addr : const str : number ...
        nxc_printf(compiler,"reg%d += %d\n",__nxc_reg1(instr),nxc_instr_get_immed32(instr));
        break;
    case nxc_vmop_st_lvar:                 ///"="
    case nxc_vmop_add_st_lvar:             ///"+="
    case nxc_vmop_sub_st_lvar:            ///"-="
    case nxc_vmop_mul_st_lvar:             ///"*="
    case nxc_vmop_div_st_lvar:             ///"/="
    case nxc_vmop_mod_st_lvar:             ///"%="
    case nxc_vmop_bor_st_lvar:             ///"|="
    case nxc_vmop_band_st_lvar:            ///"&="
    case nxc_vmop_xor_st_lvar:             ///"^="
    case nxc_vmop_lshift_st_lvar:          ///"<<="
    case nxc_vmop_rshift_st_lvar:          ///">>="
        nxc_dump_assign_lvar_instr(compiler,opcode,instr);
        break;
    ///-------------------------------------------------------------------------
    case nxc_vmop_st8:          ///"="
    case nxc_vmop_add_st8:      ///"+="
    case nxc_vmop_sub_st8:      ///"-="
    case nxc_vmop_mul_st8:      ///"*="
    case nxc_vmop_div_st8:      ///"/="
    case nxc_vmop_mod_st8:      ///"%="
    case nxc_vmop_bor_st8:      ///"|="
    case nxc_vmop_band_st8:     ///"&="
    case nxc_vmop_xor_st8:      ///"^="
    case nxc_vmop_lshift_st8:   ///"<<="
    case nxc_vmop_rshift_st8:   ///">>="
    ///-
    case nxc_vmop_st16:          ///"="
    case nxc_vmop_add_st16:      ///"+="
    case nxc_vmop_sub_st16:      ///"-="
    case nxc_vmop_mul_st16:      ///"*="
    case nxc_vmop_div_st16:      ///"/="
    case nxc_vmop_mod_st16:      ///"%="
    case nxc_vmop_bor_st16:      ///"|="
    case nxc_vmop_band_st16:     ///"&="
    case nxc_vmop_xor_st16:      ///"^="
    case nxc_vmop_lshift_st16:   ///"<<="
    case nxc_vmop_rshift_st16:   ///">>="
    ///-
    case nxc_vmop_st32:          ///"="
    case nxc_vmop_add_st32:      ///"+="
    case nxc_vmop_sub_st32:      ///"-="
    case nxc_vmop_mul_st32:      ///"*="
    case nxc_vmop_div_st32:      ///"/="
    case nxc_vmop_mod_st32:      ///"%="
    case nxc_vmop_bor_st32:      ///"|="
    case nxc_vmop_band_st32:     ///"&="
    case nxc_vmop_xor_st32:      ///"^="
    case nxc_vmop_lshift_st32:   ///"<<="
    case nxc_vmop_rshift_st32:   ///">>="
    ///-
    case nxc_vmop_st_l:          ///"="
    case nxc_vmop_add_st_l:      ///"+="
    case nxc_vmop_sub_st_l:      ///"-="
    case nxc_vmop_mul_st_l:      ///"*="
    case nxc_vmop_div_st_l:      ///"/="
    case nxc_vmop_mod_st_l:      ///"%="
    case nxc_vmop_bor_st_l:      ///"|="
    case nxc_vmop_band_st_l:     ///"&="
    case nxc_vmop_xor_st_l:      ///"^="
    case nxc_vmop_lshift_st_l:   ///"<<="
    case nxc_vmop_rshift_st_l:   ///">>="
        nxc_dump_assign_instr(compiler,opcode,instr);
        break;
    ///-------------------------------------------------------------------------
    case nxc_vmop_inc:          ///"++"
        nxc_printf(compiler,"++ [reg%d]\n",__nxc_reg1(instr));
        break;
    case nxc_vmop_dec:          ///"--"
        nxc_printf(compiler,"-- [reg%d]\n",__nxc_reg1(instr));
        break;
    ///-------------------------------------------------------------------------
    case nxc_vmop_or:        ///"||"
    case nxc_vmop_and:       ///"&&"
    case nxc_vmop_bor:       ///"|"
    case nxc_vmop_band:      ///"&"
    case nxc_vmop_xor:       ///"^"
    case nxc_vmop_eq:        ///"=="
    case nxc_vmop_uneq:      ///"!="
    case nxc_vmop_gt:        ///">"
    case nxc_vmop_lt:        ///"<"
    case nxc_vmop_ge:        ///">="
    case nxc_vmop_le:        ///"<="
    case nxc_vmop_lshift:    ///"<<"
    case nxc_vmop_rshift:    ///">>"
    case nxc_vmop_add:       ///"+"
    case nxc_vmop_sub:       ///"-"
    case nxc_vmop_mul:       ///"*"
    case nxc_vmop_div:       ///"/"
    case nxc_vmop_mod:       ///"%"
        nxc_dump_binary_instr(compiler,opcode,instr);
        break;
    case nxc_vmop_or_i:        ///"||"
    case nxc_vmop_and_i:       ///"&&"
    case nxc_vmop_bor_i:       ///"|"
    case nxc_vmop_band_i:      ///"&"
    case nxc_vmop_xor_i:       ///"^"
    case nxc_vmop_eq_i:        ///"=="
    case nxc_vmop_uneq_i:      ///"!="
    case nxc_vmop_gt_i:        ///">"
    case nxc_vmop_lt_i:        ///"<"
    case nxc_vmop_ge_i:        ///">="
    case nxc_vmop_le_i:        ///"<="
    case nxc_vmop_lshift_i:    ///"<<"
    case nxc_vmop_rshift_i:    ///">>"
    case nxc_vmop_add_i:       ///"+"
    case nxc_vmop_sub_i:       ///"-"
    case nxc_vmop_mul_i:       ///"*"
    case nxc_vmop_div_i:       ///"/"
    case nxc_vmop_mod_i:       ///"%"
        nxc_dump_binary_instr_i(compiler,opcode,instr);
        break;
    case nxc_vmop_or_i1:        ///"||"
    case nxc_vmop_and_i1:       ///"&&"
    case nxc_vmop_bor_i1:       ///"|"
    case nxc_vmop_band_i1:      ///"&"
    case nxc_vmop_xor_i1:       ///"^"
    case nxc_vmop_eq_i1:        ///"=="
    case nxc_vmop_uneq_i1:      ///"!="
    case nxc_vmop_gt_i1:        ///">"
    case nxc_vmop_lt_i1:        ///"<"
    case nxc_vmop_ge_i1:           ///">="
    case nxc_vmop_le_i1:           ///"<="
    case nxc_vmop_lshift_i1:    ///"<<"
    case nxc_vmop_rshift_i1:    ///">>"
    case nxc_vmop_add_i1:       ///"+"
    case nxc_vmop_sub_i1:       ///"-"
    case nxc_vmop_mul_i1:       ///"*"
    case nxc_vmop_div_i1:       ///"/"
    case nxc_vmop_mod_i1:       ///"%"
        nxc_dump_binary_instr_i1(compiler,opcode,instr);
        break;
    case nxc_vmop_f_gt:                 ///":>"
    case nxc_vmop_f_lt:                 ///":<"
    case nxc_vmop_f_ge:                 ///":>="
    case nxc_vmop_f_le:                 ///":<="
    case nxc_vmop_f_add:                ///":+"
    case nxc_vmop_f_sub:                ///":-"
    case nxc_vmop_f_mul:                ///":*"
    case nxc_vmop_f_div:                ///":/"
    case nxc_vmop_f_neg:                ///":-"
        nxc_dump_float_instr(compiler,opcode-nxc_vmop_f_gt,instr);
        break;
    case nxc_vmop_f_gt_i:                 ///":>"
    case nxc_vmop_f_lt_i:                 ///":<"
    case nxc_vmop_f_ge_i:                 ///":>="
    case nxc_vmop_f_le_i:                 ///":<="
    case nxc_vmop_f_add_i:                ///":+"
    case nxc_vmop_f_sub_i:                ///":-"
    case nxc_vmop_f_mul_i:                ///":*"
    case nxc_vmop_f_div_i:                ///":/"
        nxc_dump_float_instr_i(compiler,opcode-nxc_vmop_f_gt_i,instr);
        break;
    case nxc_vmop_f_gt_i1:                 ///":>"
    case nxc_vmop_f_lt_i1:                 ///":<"
    case nxc_vmop_f_ge_i1:                 ///":>="
    case nxc_vmop_f_le_i1:                 ///":<="
    case nxc_vmop_f_add_i1:                ///":+"
    case nxc_vmop_f_sub_i1:                ///":-"
    case nxc_vmop_f_mul_i1:                ///":*"
    case nxc_vmop_f_div_i1:                ///":/"
        nxc_dump_float_instr_i1(compiler,opcode-nxc_vmop_f_gt_i1,instr);
        break;
        ///-------------unary opcode-----------------
    case nxc_vmop_cast:         ///"cast"
        nxc_printf(compiler,"cast is not supported!!!\n");
        break;
    case nxc_vmop_positive:     ///"+"
        nxc_printf(compiler,"<<<><>>>< reg%d\n",__nxc_reg1(instr));
        break;
    case nxc_vmop_negative:     ///"-"
        nxc_printf(compiler,"reg%d = - reg%d\n",__nxc_reg1(instr),__nxc_reg1(instr));
        break;
    case nxc_vmop_compensation: ///"~"
        nxc_printf(compiler,"reg%d = ~ reg%d\n",__nxc_reg1(instr),__nxc_reg1(instr));
        break;
    case nxc_vmop_not:          ///"!"
        nxc_printf(compiler,"reg%d = ! reg%d\n",__nxc_reg1(instr),__nxc_reg1(instr));
        break;
    ///-------------------------------------------------------------------------
    case nxc_vmop_addr_array8:      ///"&["    left + index
        nxc_printf(compiler,"reg%d = reg%d + %reg%d\n",__nxc_reg1(instr),__nxc_reg1(instr),__nxc_reg2(instr));
        break;
    case nxc_vmop_addr_array16:     ///"&[["   left + index * sizeof(short)
        nxc_printf(compiler,"reg%d = reg%d + 2 x %reg%d\n",__nxc_reg1(instr),__nxc_reg1(instr),__nxc_reg2(instr));
        break;
    case nxc_vmop_addr_array32:     ///"&[["   left + index * sizeof(int)
        nxc_printf(compiler,"reg%d = reg%d + 4 x %reg%d\n",__nxc_reg1(instr),__nxc_reg1(instr),__nxc_reg2(instr));
        break;
    case nxc_vmop_addr_array_l:     ///"&[["   left + index * sizeof(short)
        nxc_printf(compiler,"reg%d = reg%d + 4 x %reg%d\n",__nxc_reg1(instr),__nxc_reg1(instr),__nxc_reg2(instr));
        break;
    ///------------------flow-control-----------
    case nxc_vmop_call:                 ///"call    $reg"
        nxc_printf(compiler,"call reg%d\n",__nxc_reg1(instr));
        break;
    case nxc_vmop_postcall:
        nxc_printf(compiler,"postcall reg%d,%d\n",__nxc_reg1(instr),nxc_instr_get_immed32(instr));
        break;
    case nxc_vmop_trap:                 ///"trap    xaddr"
        nxc_printf(compiler,"trap 0x%08X\n",nxc_instr_get_jmp_addr(instr));
        break;
    case nxc_vmop_fast_call:            ///"fasttrap $reg"
        nxc_printf(compiler,"fast_trap reg%d\n",__nxc_reg1(instr));
        break;
    case nxc_vmop_jmp:                  ///"jmp     $imme_addr"
        nxc_printf(compiler,"goto 0x%08X\n",nxc_instr_get_jmp_addr(instr));
        break;
    case nxc_vmop_jz:                   ///"jmp if zero"
        nxc_printf(compiler,"if(reg%d == 0) goto 0x%08X\n",__nxc_reg1(instr),nxc_instr_get_jmp_addr(instr));
        break;
    case nxc_vmop_jnz:                  ///"jmp if non-zero"
        nxc_printf(compiler,"if(reg%d != 0) goto 0x%08X\n",__nxc_reg1(instr),nxc_instr_get_jmp_addr(instr));
        break;
    case nxc_vmop_return_val:           ///"return  $expr"
        nxc_printf(compiler,"$eax = reg%d\n",__nxc_reg1(instr));
        nxc_printf(compiler,"              mov  $esp,$ebp\n");
        nxc_printf(compiler,"              pop  $ebp\n");
        nxc_printf(compiler,"              ret\n");
        break;
    case nxc_vmop_ret:                  ///"ret"
        nxc_printf(compiler,"$eax = 0\n");
        nxc_printf(compiler,"              mov  $esp,$ebp\n");
        nxc_printf(compiler,"              pop  $ebp\n");
        nxc_printf(compiler,"              ret\n");
        break;
    case nxc_vmop_push:                 ///"push     $reg"
        nxc_printf(compiler,"push reg%d\n",__nxc_reg1(instr));
        break;
	case nxc_vmop_pushi:                 ///"push     $reg"
        nxc_printf(compiler,"push %d\n",nxc_instr_get_immed_l(instr));
        break;
    case nxc_vmop_pop:                  ///"pop
        nxc_printf(compiler,"%s\n",name);
        break;
    case nxc_vmop_enter:///enter func ///this will be done automatically ...
        nxc_printf(compiler,"push $ebp\n");
        nxc_printf(compiler,"              mov  $ebp,$esp\n");
        nxc_printf(compiler,"              sub  $esp,%d\n",nxc_instr_get_immed32(instr));
        break;
    case nxc_vmop_leave: ///leave func ///this is done at return statement ...
        nxc_printf(compiler,"%s\n",name);
        break;
    default:
        nxc_printf(compiler,"error---------------\n");
        break;
    }
}

/**
 * dump all instructions of target function ...
 */
int nxc_dump_func_code(nxc_compiler_t *const compiler,nxc_sym_t *func)
{
    int i;
    nxc_instr_t *instr;

    instr = nxc_sym_get_func_addr(func);
    for (i=0;i<nxc_sym_get_func_instr_cnt(func);i++)
    {
        nxc_dump_a_instr(compiler,instr);
        instr++;
    }

    return 0;
}
/**===========================================================================*/
/**===========================================================================*/
/**===========================================================================*/


/**
 * translate const ast node to marco-instruction ...
 * return non-zero means error ...
 */
___fast int nxc_translate_const(nxc_compiler_t *const compiler,nxc_ast_t *ast)
{
    int reg,tok_type;
    nxc_instr_t *instr;
    
    instr = nxc_new_instr(compiler,0);
    if(!instr) return -1;
    reg = nxc_alloc_reg(compiler); ///alloc a reg to hold immediate ...
    nxc_instr_set_opcode(instr,nxc_vmop_ldi);
    nxc_instr_set_reg_index1(instr,reg);///set dst reg ...

    ///associate ast to instr ...
    nxc_ast_set_instr(ast,instr);

    tok_type = nxc_const_ast_get_type(ast);
    switch (tok_type)
    {
    case nxc_token_const_long:
        nxc_ast_set_typename(ast,"___var");
        nxc_instr_set_immed_l(instr,nxc_const_ast_get_long_val(ast));
        break;
    case nxc_token_const_float:
        nxc_ast_set_typename(ast,"float");
        nxc_instr_set_immed_l(instr,nxc_const_ast_get_long_val(ast));
        break;
    case nxc_token_const_string:
        nxc_ast_set_typename(ast,"string");
        ///this is not necessary ...
        nxc_instr_set_immed_l(instr,(long)nxc_const_ast_get_str_val(ast));
        ///trace this instr and fix later ...
        nxc_compiler_trace_load_const(compiler,ast);
        break;
    }
    nxc_ast_set_result_reg(ast,reg);///associate reg with ast node ...

    return 0;
}

/**===========================================================================*/
/**
 * translate id ast node to marco-instruction ...
 * return non-zero means error ...
 */
___fast int nxc_translate_id(nxc_compiler_t*const compiler,
                             nxc_ast_t *ast,
                             int need_lvalue)
{
    int reg;
    nxc_instr_t *instr;
    nxc_sym_t *sym;
    char *name;

    ///[1].resolv symbol first ...
    name = nxc_id_ast_get_name(ast);
    sym = nxc_find_symbol(compiler,name,nxc_strlen(name));
    if(!sym){
        nxc_ast_error(compiler,ast,"unresolved symbol '%s'",name);
        return -1;
    }
    nxc_ast_set_sym(ast,sym);///associate the ast to symbol ...

    ///[2].trnna check lvalue ...
    if(need_lvalue && !nxc_is_lvalue_ast(compiler,ast)){
        nxc_ast_error(compiler,ast,"'%s' has no lvalue",name);
        return -1;
    }

    instr = nxc_new_instr(compiler,0);
    if(!instr) return -1;
    reg = nxc_alloc_reg(compiler);      ///alloc a reg to hold data
    nxc_instr_set_reg_index1(instr,reg);///set dst reg ...

    ///associate instruction with ast node ...
    nxc_ast_set_instr(ast,instr);
    ///associate reg         with ast node ...
    nxc_ast_set_result_reg(ast,reg);

    ///fix and trace sth ...
    switch (nxc_sym_get_type(sym))
    {
    case nxc_sym_global_function:
        ///load symbol addr as immediate number ...
        nxc_instr_set_opcode(instr,nxc_vmop_ldi);
        ///func address is the first instr address ...
        nxc_instr_set_func_addr(instr,nxc_sym_get_func_addr(sym));

        ///trace global & extern symbol anf fix later ...
        if (nxc_sym_has_flag(sym,nxc_symflag_extern)){
            ///trace extern symbol ...
            nxc_compiler_trace_load_x_sym(compiler,ast);
        }else{
            ///trace load-instr and fix address later ...
            nxc_compiler_trace_load_g_sym(compiler,ast);
        }
        break;
    case nxc_sym_global_var:
        ///load global sym addr | val
        if(need_lvalue){
            nxc_instr_set_opcode(instr,nxc_vmop_ld_gvar_addr);
        }
        else{
            ///array are treat separately as sym-addr ...
            if(nxc_sym_is_array(sym)) 
                nxc_instr_set_opcode(instr,nxc_vmop_ld_gvar_addr);
            else
                nxc_instr_set_opcode(instr,nxc_vmop_ld_gvar);
        }

        ///src oprand is var's rel_offset
        nxc_instr_set_var_offset(instr,nxc_sym_get_var_offset(sym));

        ///trace global & extern symbol anf fix later ...
        if (nxc_sym_has_flag(sym,nxc_symflag_extern)){
            ///trace extern symbol ...
            nxc_compiler_trace_load_x_sym(compiler,ast);
        }else{
            ///trace load-instr and fix address later ...
            nxc_compiler_trace_load_g_sym(compiler,ast);
        }
        break;
    case nxc_sym_local_param:
        ///load param sym addr | val
        if(need_lvalue)
            nxc_instr_set_opcode(instr,nxc_vmop_ld_lvar_addr);
        else
            nxc_instr_set_opcode(instr,nxc_vmop_ld_lvar);
        /**
         *   Stack layout ...
         *   low  -------------------->  high
         * --+-------+-------+-------+------+--
         *   |  reg0 |  ebp  |retAddr|param0|
         * --+-------+-------+-------+------+--
         *           ^
         *   EBP-----+
         */
        nxc_instr_set_var_offset(instr,
            nxc_sym_get_var_offset(sym) + sizeof(long) + sizeof(long));
        break;
    case nxc_sym_local_var:
        ///load var sym addr | val
        if(need_lvalue) {
            nxc_instr_set_opcode(instr,nxc_vmop_ld_lvar_addr);
        }
        else{
            ///array are treat separately as sym-addr ...
            if(nxc_sym_is_array(sym))
                nxc_instr_set_opcode(instr,nxc_vmop_ld_lvar_addr);
            else
                nxc_instr_set_opcode(instr,nxc_vmop_ld_lvar);
        }
        /**
         *   Stack layout ...
         *   low  -------------------->  high
         * --+-------+-------+-------+------+--
         *   |  reg0 |  ebp  |retAddr|param0|
         * --+-------+-------+-------+------+--
         *           ^
         *   EBP-----+
         */
        nxc_instr_set_var_offset(instr,nxc_sym_get_var_offset(sym));
        
        ///trace this instruction (trace ast here)...
        nxc_compiler_trace_load_lvar(compiler,ast);
        break;
    case nxc_sym_member:
        ///hack here ::: translate const !!!
        nxc_instr_set_opcode(instr,nxc_vmop_ldi);
        nxc_instr_set_immed_l(instr,nxc_sym_get_var_offset(sym));

        ///!!!!!!!no type inheritation!!!!!!!
        return 0;
        break;
    default:
        nxc_ast_error(compiler,ast,"unknown sym type !!!");
        return -1;
    }

    ///inherit type ...
    ///fix type info ...
    nxc_ast_set_typename(ast,nxc_sym_get_typename(sym));
    nxc_ast_set_f_typename(ast,nxc_sym_get_f_typename(sym));

    return 0;
}

/**
 * translate prefix ast node to marco-instruction ...
 * handle + - & ++ -- ~ ! @ '*' (take care of '*' opcode ...)
 * return non-zero means error ...
*/
___fast int nxc_translate_prefix(nxc_compiler_t *const compiler,
                                 nxc_ast_t *ast,
                                 int ast_opcode,
                                 int calc_lvalue)
{
    int ret,opcode,reg,calc_sub_lvalue;
    nxc_instr_t *instr;
    nxc_ast_t *unary;

    unary = nxc_prefix_ast_get_unary(ast);
    calc_sub_lvalue = 0;
    opcode = 0;
    switch (ast_opcode)
    {
    ///MARK:::currently , '++' operation does not support lvalue!!!fix later
    ///MARK:::currently , '--' operation does not support lvalue!!!fix later
    case nxc_op_inc:          ///"++"   
        if (calc_lvalue){
            nxc_ast_error(compiler,ast,"'++' operation does not support lvalue!!!fix later");
            return -1;
        }
        opcode = nxc_vmop_inc;
        calc_sub_lvalue = 1;
        break;
    case nxc_op_dec:          ///"--"
        if (calc_lvalue){
            nxc_ast_error(compiler,ast,"'--' operation does not support lvalue!!!fix later");
            return -1;
        }
        opcode = nxc_vmop_dec;
        calc_sub_lvalue = 1;
        break;
    case nxc_op_positive:     ///"+"
        opcode = nxc_vmop_positive;
        calc_sub_lvalue = 0;
        break;
    case nxc_op_negative:     ///"-"
        opcode = nxc_vmop_negative;
        calc_sub_lvalue = 0;
        break;
    case nxc_op_compensation: ///"~"
        opcode = nxc_vmop_compensation;
        calc_sub_lvalue = 0;
        break;
    case nxc_op_not:          ///"!"
        opcode = nxc_vmop_not;
        calc_sub_lvalue = 0;
        break;
    case nxc_op_address:      ///"&"
        if (calc_lvalue){
            nxc_ast_error(compiler,ast,"'&' has no lvalue");
            return -1;
        }
        opcode = 0;
        calc_sub_lvalue = 1;
        ///translate right unary for lvalue ...
        ret = nxc_translate(compiler,unary,calc_sub_lvalue);
        if(ret) return ret;
        ///get result register ...
        reg = nxc_ast_get_result_reg(unary);
        ///associate curr-node with result-reg ...
        nxc_ast_set_result_reg(ast,reg);

        return 0;
    case nxc_op_sizeof:       ///"sizeof"
        if (calc_lvalue){
            nxc_ast_error(compiler,ast,"'sizeof' has no lvalue");
            return -1;
        }
        nxc_ast_error(compiler,ast,"'sizeof':not support currently...");
        return -1;
    case nxc_op_cast:         ///"cast"
        if (calc_lvalue){
            nxc_ast_error(compiler,ast,"'cast' has no lvalue");
            return -1;
        }
        nxc_ast_error(compiler,ast,"'cast':not support currently...");
        return -1;
    }

    ///translate right unary first ...
    ret = nxc_translate(compiler,unary,calc_sub_lvalue);
    if(ret) return ret;

    /**
     * generate instr to load right value if necessary ...
     */
    instr = nxc_new_instr(compiler,opcode);
    if(!instr) return -1;
    ///get result register (that hold R-Value) ...
    reg = nxc_ast_get_result_reg(unary);
    ///reuse right unary result reg ...
    nxc_instr_set_reg_index1(instr,reg);

    ///associate curr-node with result-reg ...
    nxc_ast_set_result_reg(ast,reg);

    ///do float hack ...
    {
        if( opcode == nxc_vmop_negative &&
            nxc_ast_is_float_type(unary))
        {
            ///change instruction opcode to float operation ...
            nxc_instr_set_opcode(instr,nxc_vmop_f_neg);

            ///inherit type ...
            nxc_ast_set_typename(ast,nxc_ast_get_typename(unary));
        }
    }
    return 0;
}

/**
 * translate postfix
 * translate array index ast to marco instruction ...
 */
___fast int nxc_translate_index(nxc_compiler_t *const compiler,
                                nxc_ast_t *ast,
                                int opcode,
                                int calc_lvalue)
{
    int ret,base_reg,index_reg;
    nxc_instr_t *instr;
    nxc_ast_t *array_base,*array_index;

    ///translate left array-base for rvalue
    array_base = nxc_array_ast_get_base(ast);
    ret = nxc_translate(compiler,array_base,0);
    if(ret) return ret;
    base_reg = nxc_ast_get_result_reg(array_base);//get base-result-reg

    ///>>>hack for fast access!!!   xxx[$constant]
    array_index = nxc_array_ast_get_index(ast);
    if( nxc_ast_get_opcode(array_index) == nxc_op_const &&
        nxc_const_ast_get_type(array_index) == nxc_token_const_long)
    {
        long index_num;

        index_num = nxc_const_ast_get_long_val(array_index);
        switch (opcode)
        {
        case nxc_op_index8:   index_num <<= nxc_char_shift; break;
        case nxc_op_index16:  index_num <<= nxc_short_shift; break;
        case nxc_op_index32:  index_num <<= nxc_int_shift; break;
        case nxc_op_index_l:  index_num <<= nxc_long_shift; break;
        }

        if(calc_lvalue)
        {
            if (index_num ==0)
            {
                ///return left array_base reg directly!!!
            }
            else ///index != 0
            {
                opcode = nxc_vmop_addi; 
                ///generate a instruction to calc offset
                instr = nxc_new_instr(compiler,0);
                if(!instr) return -1;
                nxc_instr_set_opcode(instr,nxc_vmop_addi);///reg add immediate..
                nxc_instr_set_reg_index1(instr,base_reg); ///reuse base-reg
                nxc_instr_set_immed_l(instr,index_num);   ///immed_offset
            }
        }
        else ///calc rvalue ...
        {
            ///generate a instruction to calc offset
            instr = nxc_new_instr(compiler,opcode);
            if(!instr) return -1;
            if (index_num == 0)
            {
                opcode = nxc_vmop_ld8 + (opcode - nxc_op_index8);
                nxc_instr_set_opcode(instr,opcode);
                nxc_instr_set_reg_index1(instr,base_reg); ///reuse base-reg
            }
            else
            {
                opcode = nxc_vmop_ldx8 + (opcode - nxc_op_index8);
                nxc_instr_set_opcode(instr,opcode);
                nxc_instr_set_reg_index1(instr,base_reg); ///reuse base-reg
                nxc_instr_set_immed_l(instr,index_num);   ///immed_offset
            }
        }
        ///left (=array-base) as result ...
        nxc_ast_set_result_reg(ast,base_reg);///set result_reg = base_reg
        ///inherit father of array-base ...
        nxc_ast_set_typename(ast,nxc_ast_get_f_typename(array_base));
        return 0;
    }

    ///translate right array-index for rvalue
    ret = nxc_translate(compiler,array_index,0);
    if(ret) return ret;
    ///calc final opcode ...
    if (calc_lvalue){
        opcode = nxc_vmop_addr_array8 + (opcode - nxc_op_index8);
    }else{
        opcode = nxc_vmop_index8 + (opcode - nxc_op_index8);
    }
    ///generate a instruction !!!
    instr = nxc_new_instr(compiler,opcode);
    if(!instr) return -1;
    base_reg = nxc_ast_get_result_reg(array_base);
    nxc_instr_set_reg_index1(instr,base_reg);///operand1 is array-base-reg
    index_reg = nxc_ast_get_result_reg(array_index);
    nxc_instr_set_reg_index2(instr,index_reg);///operand2 is array index-reg        
    ///release right result reg ...
    nxc_free_reg(compiler,index_reg); ///release operand 2 reg ...

    ///associate result reg with ast node ...
    nxc_ast_set_result_reg(ast,base_reg); ///store result into operand1

    ///inherit father of array-base ...
    nxc_ast_set_typename(ast,nxc_ast_get_f_typename(array_base));

    return 0;
}

/**
 * translate postfix
 * translate call node to instruction ...
 * Max Instruction count : 3
 */
___fast int nxc_translate_call(nxc_compiler_t *const compiler,nxc_ast_t *ast)
{
    int ret,param_size,param_cnt,func_reg,param_reg,opcode;
    nxc_instr_t *instr;
    nxc_ast_t *func,*param;
    nxc_sym_t *sym;

    /**
     * [1].translate right param first !!!--------------------------------------
     */
    /**
     * inorder aligned the param area size in 8B/16B , we should check total 
     * param size and decide if a extra param is required to keep the alignment
     */
    param_size = 0;
    param_cnt = 0;
    /**
     * [a].we should push params in reverse order ...
     */
    nxc_dlist_for_each_entry_reverse(param,
                                     nxc_call_ast_get_parm_list(ast),
                                     list_head,
                                     nxc_ast_t)
    {
		///const hack here ...
		if ((nxc_ast_get_opcode(param) == nxc_op_const) &&
			((nxc_const_ast_get_type(param) == nxc_token_const_long)||
			 (nxc_const_ast_get_type(param) == nxc_token_const_float))
		   )
		{
			///push param into stack ...
			instr = nxc_new_instr(compiler,nxc_vmop_pushi);
			if(!instr) return -1;
			nxc_instr_set_immed_l(instr,nxc_const_ast_get_long_val(param));
			///calc total param size ...
			param_size += sizeof(long);
			param_cnt  ++;
			continue;
		}
        ///calc rvalue of curr param ...
        ret = nxc_translate(compiler,param,0);
        if(ret) return ret;
        
        ///push param into stack ...
        instr = nxc_new_instr(compiler,nxc_vmop_push);
        if(!instr) return -1;
        param_reg = nxc_ast_get_result_reg(param);
        nxc_instr_set_reg_index1(instr,param_reg);///param result reg

        ///calc total param size ...
        param_size += sizeof(long);
        param_cnt  ++;
        
        ///free result register that held the param result ...
        nxc_free_reg(compiler,param_reg);
    }

    /**
     * [2].translate left function ...------------------------------------------
     */
    func = nxc_call_ast_get_func(ast);
    ///calc func sym addr for rvalue
    ret = nxc_translate(compiler,func,0);
    if(ret) return ret;

    ///>>>get result-register<<<
    func_reg = nxc_ast_get_result_reg(func);

    ///check and get function-symbol for param checking ...
    switch (nxc_ast_get_opcode(func))
    {
    case nxc_op_id:
        sym = nxc_ast_get_sym(func);
        if (nxc_sym_get_type(sym)!= nxc_sym_global_function){
            sym = 0; ///>>>drop symbol, no param checing!!!<<<
        }
        break;
    case nxc_op_member:
        sym = nxc_ast_get_sym(func); ///get associated symbol ...
        switch (nxc_sym_get_type(sym))
        {
        case nxc_sym_global_function:
            ///>>>1.push obj-result ast parameter<<<
            param_cnt ++;                           ///update total param info..
            param_size += sizeof(long);
            ///>>>4.hold func-sym for param-checking<<<

            break;
        case nxc_sym_member:
            ///result-reg = obj-result ...
        default:
            sym = 0; ///>>>drop symbol , no parm count-checking ...<<<
            break;
        }
        break;
    default:
        sym =0;
        break;
    }

    ///trnna check parameter (symbol must be a function)...
    if(sym)
    {
        ///here , the left expr is a function ,check param count ...
        if(nxc_sym_has_flag(sym,nxc_symflag_var_args))
        {
            if(param_cnt < nxc_sym_get_param_cnt(sym)){
                nxc_ast_error(  compiler,
                                ast,
                                "function '%s' require at least %d param.",
                                nxc_sym_get_name(sym),
                                nxc_sym_get_param_cnt(sym));
                return -1;
            }
        }
        else                                    ///fixed param cnt
        {
            if(param_cnt != nxc_sym_get_param_cnt(sym)){
                nxc_ast_error(  compiler,
                                ast,
                                "function '%s' require %d param.",
                                nxc_sym_get_name(sym),
                                nxc_sym_get_param_cnt(sym));
                return -1;
            }
        }
    }

    /**
     * check param size for alignment !!!
     * but current we ignore this!!!
     */
    if(param_size&0x7) ///keep param size tobe 8B aligned!!!
    {
        ///do alignment !!!
    }

    /**
     * [3].generate call instruction after all param is pushed into stack ...
     */
    {
        if (sym && nxc_sym_has_flag(sym,nxc_symflag_fastcall)) {
            opcode = nxc_vmop_fast_call;
        }else{
            opcode = nxc_vmop_call;
        }
        instr = nxc_new_instr(compiler,opcode);
        if(!instr) return -1;
        ///operand1 is register that hold func addr
        nxc_instr_set_reg_index1(instr,func_reg);
        ///operand2 is reserved ...
        ///>>>
    }

    /**
     * [4].postcall:move result to register and pop params
     */
    {
        instr = nxc_new_instr(compiler,nxc_vmop_postcall);
        if(!instr) return -1;
        ///indicate result reg to hold func result (eax)
        nxc_instr_set_reg_index1(instr,func_reg);
        ///indicate size to pop after call (for balance)
        nxc_instr_set_immed32(instr,param_size);
    }

    ///associate result reg with ast node ...
    nxc_ast_set_result_reg(ast,func_reg);

    ///inherit type from function ...
    nxc_ast_set_typename(ast,nxc_ast_get_f_typename(func));
    
    return 0;
}

/**
 * translate member postfix ...
 */
___fast int nxc_translate_member(nxc_compiler_t *const compiler,
                                 nxc_ast_t *ast,
                                 int calc_lvalue)
{
    int ret,obj_reg,func_reg,__len,opcode;
    nxc_instr_t *instr;
    nxc_ast_t *obj;
    nxc_sym_t *sym;
    char __name[128];
    char *type_name,*member_name;

    ///get left obj-ast
    obj = nxc_member_ast_get_obj(ast);
    ///clac rvalue of object ...
    ret  = nxc_translate(compiler,obj,0);
    if(ret) return ret;

    ///get obj's associated reg ...
    obj_reg = nxc_ast_get_result_reg(obj);
    ///get obj's type name ...
    type_name = nxc_ast_get_typename(obj);
    if(!type_name) type_name = "__var"; ///default class is '__var'
    ///get member name (maxlen=60)...
    member_name = nxc_member_ast_get_member_name(ast);
    ///generate real method name (maxlen=60)...
    __len = nxc_gen_member_fullname(__name,type_name,member_name);
    ///find method in global zone !!!
    sym = nxc_find_symbol(compiler,__name,__len);
    if (!sym){
        nxc_ast_error(compiler,ast,"unresolved symbol %s",__name);
        return -1;
    }
    ///destination must be a funcion/member-offset!!!
    if (nxc_sym_get_type(sym)!=nxc_sym_global_function &&
        nxc_sym_get_type(sym)!=nxc_sym_member){
        nxc_ast_error(compiler,ast,"unresolved Object Member '%s'",__name);
        return -1;
    }

    ///check lvalue ...
    if (calc_lvalue) {
        if (!nxc_sym_has_flag(sym,nxc_symflag_lvalue)){
            nxc_ast_error(compiler,ast,"member '%s' has no lvalue",__name);
            return -1;
        }
    }
    ///trnna load symbol into register ...
    if(nxc_sym_get_type(sym) == nxc_sym_member)///----------member--------------
    {
        if (calc_lvalue) ///>>>lvalue<<<
            opcode = nxc_vmop_addi; ///obj_reg = obj_reg + offset
        else{            ///>>>rvalue<<<
            opcode = nxc_vmop_ldx_l;///obj_reg = [obj_reg + offset]
			if(nxc_sym_has_flag(sym,nxc_symflag_array)){///handle array type!!!
				opcode = nxc_vmop_addi;
			}
		}
        ///generate a instruction to calc member-result (obj+offset)
        instr = nxc_new_instr(compiler,0);
        if(!instr) return -1;
        nxc_instr_set_opcode(instr,opcode);        ///set opcode ...
        nxc_instr_set_reg_index1(instr,obj_reg);   ///reuse obj_result_reg
        nxc_instr_set_immed_l(instr,nxc_sym_get_var_offset(sym));//immediate_off

		///######associate 'result-reg ' with current ast######
		nxc_ast_set_result_reg(ast,obj_reg);///left as result ...
		///######associate 'symbol-info' with current ast######
		nxc_ast_set_sym(ast,sym);
    }
    else ///------------------------------------------------method--------------
    {
        ///###########################do nothing here###########################
        ///1.function symbol is associated with member ast
        ///2.return obj-reg directly!!!

		nxc_ast_t *father;
		father = nxc_ast_get_father_node(ast);
		///check if father node is call-ast and 
		///trnna push obj-result ahead of time
		if (father && nxc_ast_get_opcode(father)==nxc_op_call)
		{
			///>>>1.push obj-result ast parameter<<<
            instr = nxc_new_instr(compiler,nxc_vmop_push);
            if(!instr) return -1;
            nxc_instr_set_reg_index1(instr,obj_reg);///param result reg
		}

		///release obj-result-reg
		nxc_free_reg(compiler,obj_reg);

		///>>>2.generate instruction to load method-function<<<
		instr = nxc_new_instr(compiler,nxc_vmop_ldi);///load immediate
		if(!instr) return -1;
		func_reg = nxc_alloc_reg(compiler);///alloc a reg to hold func addr
		nxc_instr_set_reg_index1(instr,func_reg);///func result reg
		nxc_instr_set_immed_l(instr,(long)nxc_sym_get_func_addr(sym));
		
		///######associate 'result-reg ' with current ast######
		nxc_ast_set_result_reg(ast,func_reg);
		
		///>>>3.trace this load instruction ...<<<
		nxc_ast_set_instr(ast,instr); ///associate load-instr with call ast.
		nxc_ast_set_sym(ast,sym);///associate sym with call-ast ...
		///trace global & extern symbol anf fix later ...
		if (nxc_sym_has_flag(sym,nxc_symflag_extern)){
			///trace extern symbol ...
			nxc_compiler_trace_load_x_sym(compiler,ast);
		}else{
			///trace load-instr and fix address later ...
			nxc_compiler_trace_load_g_sym(compiler,ast);
        }
    }

    ///>>>inherit type of member<<<
    nxc_ast_set_typename(ast,nxc_sym_get_typename(sym));
    nxc_ast_set_f_typename(ast,nxc_sym_get_f_typename(sym));
    return 0;
}

/**
 * translate binary ast to marco instruction ...
 */
___fast int nxc_translate_binary(nxc_compiler_t *const compiler,
                                 nxc_ast_t *ast,
                                 int bin_opcode_offset,
                                 int calc_lvalue)
{
    int ret,left_reg,right_reg,xreg,need_inherit;
    nxc_instr_t *instr;
    nxc_ast_t *left,*right,*xast,*const_ast;
    int opcode,l_opcode,f_opcode;
    long long_num;
    float float_num;

    if (calc_lvalue){
        nxc_ast_error(compiler,ast,"'$binary-operator' DOES-NOT support lvalue,fix later");
        return -1;
    }
    ///generate default opcode ...
    opcode = nxc_vmop_or + bin_opcode_offset;
    ///get left ast ...
    left = nxc_expr_ast_get_left(ast);
    right = nxc_expr_ast_get_right(ast);

    ///constant hack ...
    do
    {
        xast = 0;
        l_opcode = 0;
        float_num = 0;
        f_opcode = 0;
        long_num = 0;
        if(nxc_ast_get_opcode(left)==nxc_op_const) ///left is constant
        {
            ///do hack here ...
            const_ast = left;
            xast = right;           
            if(nxc_const_ast_get_type(const_ast)==nxc_token_const_long){
                long_num = nxc_const_ast_get_long_val(const_ast);
                l_opcode = nxc_vmop_or_i1 + bin_opcode_offset;///$long_num op $AST
            }else if (nxc_const_ast_get_type(const_ast)==nxc_token_const_float){
                if(nxc_bin_offset_to_fbin_offset(bin_opcode_offset)>=0){
                    ///$float_num op $AST
                    f_opcode = nxc_vmop_f_gt_i1 + nxc_bin_offset_to_fbin_offset(bin_opcode_offset);
                    float_num = nxc_const_ast_get_float_val(const_ast);
                }else{
                    ///treat as const ...
                    l_opcode = nxc_vmop_or_i1 + bin_opcode_offset;//$long_num op $AST
                    long_num = nxc_const_ast_get_long_val(const_ast);
                }
            }else{
                break;
            }
        }
        else if (nxc_ast_get_opcode(right)==nxc_op_const)//right is constant
        {
            const_ast = right;
            xast = left;
            if(nxc_const_ast_get_type(const_ast)==nxc_token_const_long){
                l_opcode = nxc_vmop_or_i + bin_opcode_offset;//$AST op $long_num
                long_num = nxc_const_ast_get_long_val(const_ast);
            }else if (nxc_const_ast_get_type(const_ast)==nxc_token_const_float){
                if(nxc_bin_offset_to_fbin_offset(bin_opcode_offset)>=0){
                    ///$AST op $float_num
                    f_opcode = nxc_vmop_f_gt_i + nxc_bin_offset_to_fbin_offset(bin_opcode_offset);
                    float_num = nxc_const_ast_get_float_val(const_ast);
                }else{
                    ///treat as const ...
                    l_opcode = nxc_vmop_or_i + bin_opcode_offset;///$AST op $long_num
                    long_num = nxc_const_ast_get_long_val(const_ast);
                }
            }else{
                break;
            }
        }
        else ///both is non-constant
        {
            break;///do normal processing ...
        }

        ///translate $Non-Constant-AST
        ret = nxc_translate(compiler,xast,calc_lvalue);
        if (ret) return ret;

        ///generate binary operate instruction ...
        instr = nxc_new_instr(compiler,0);
        if(!instr) return -1;
        xreg = nxc_ast_get_result_reg(xast);
        nxc_instr_set_reg_index1(instr,xreg); ///operand1 = result reg

        ///process $immediate-operand
        if( nxc_ast_is_float_type(xast) && f_opcode )///both float,one is const
        {
            nxc_instr_set_opcode(instr,f_opcode);  ///fix opcode ...
            nxc_instr_set_immed_f(instr,float_num);///operand2 = float
            ///inherit type ...
            nxc_ast_set_typename(ast,nxc_ast_get_typename(left));
        }
        else ///long operation ...
        {
            nxc_instr_set_opcode(instr,l_opcode); ///fix opcode ...
            nxc_instr_set_immed_l(instr,long_num);///operand2 = long
        }       
        
        ///associate result reg with ast node ...
        nxc_ast_set_result_reg(ast,xreg);///save reg info
        
        return 0;
    }while (0);

    ///calc left rvalue
    ret = nxc_translate(compiler,left,0);
    if(ret) return ret;
    ///calc right rvalue
    ret = nxc_translate(compiler,right,0);
    if(ret) return ret;

    ///always reload ast node after translation ...
    left  = nxc_expr_ast_get_left(ast);
    right = nxc_expr_ast_get_right(ast);
    ///float hack ...
    do
    {
        need_inherit = 0;
        ///map float operation ...
        if (nxc_ast_is_float_type(left) && nxc_ast_is_float_type(right))
        {
            if(nxc_bin_offset_to_fbin_offset(bin_opcode_offset)>=0){
                opcode = nxc_vmop_f_gt + nxc_bin_offset_to_fbin_offset(bin_opcode_offset);
                need_inherit = 1;
            }else{
                opcode = nxc_vmop_or + bin_opcode_offset;
            }
        }
    }while(0);
    ///generate binary operate instruction ...
    instr = nxc_new_instr(compiler,opcode);
    if(!instr) return -1;
    left_reg = nxc_ast_get_result_reg(left);
    right_reg = nxc_ast_get_result_reg(right);
    nxc_instr_set_reg_index1(instr,left_reg); ///operand1 = left result reg
    nxc_instr_set_reg_index2(instr,right_reg);///operand2 = right result reg

    ///associate result reg with ast node ...
    nxc_ast_set_result_reg(ast,left_reg);///save reg info:left as result ...
    
    ///release right result reg ...
    nxc_free_reg(compiler,right_reg);

    ///inherit type if this is a float binary-operation ...
    if (need_inherit)
    {
        nxc_ast_set_typename(ast,nxc_ast_get_typename(left));
    }

    return 0;
}

/**
 * translate binary ast to marco instruction ...
 * @opcode_offset : opcode class , such as = += -= 's offset...
 */
___fast int nxc_translate_assign(nxc_compiler_t *const compiler,
                                 nxc_ast_t *ast,
                                 int opcode_offset,
                                 int calc_lvalue)
{
    int ret,opcode_base,opcode,left_reg,right_reg;
    nxc_instr_t *instr;
    nxc_ast_t *left,*right;

    if(calc_lvalue) {///assign expression is not a lvalue node currently ...
        nxc_ast_error(compiler,ast,"'assign' expr do not suport lvalue,fix later");
        return -1;
    }

    ///get left ast ...
    left = nxc_expr_ast_get_left(ast);
    ///get right ast ...
    right = nxc_expr_ast_get_right(ast);
#if 1
    {///------------------------------------------------------------------------
        ///>>>do hardcode here (optimise store to local var)...
        nxc_sym_t *sym;
        char *__name;
        int __len;
        do 
        {
            if (nxc_ast_get_opcode(left) != nxc_op_id ) break;
            //father = nxc_ast_get_father_node(ast);
            //if (!father) break;
            //if (nxc_ast_get_opcode(father)!=nxc_op_block) break;
            __name = nxc_id_ast_get_name(left);
            __len = nxc_strlen(__name);
            sym = nxc_find_symbol(compiler,__name,__len);
            if(!sym) break;
            if(nxc_sym_get_type(sym)!=nxc_sym_local_param &&
                nxc_sym_get_type(sym)!=nxc_sym_local_var) break;
            ///calc right rvalue
            ret = nxc_translate(compiler,right,0);
            if(ret) return ret;
            ///select vm_opcode here : base + offset 
            opcode = nxc_vmop_st_lvar + opcode_offset;
            ///generate the st-lvar instruction ...
            ///generate binary operate instruction ...
            instr = nxc_new_instr(compiler,opcode);
            if(!instr) return -1;
            right_reg = nxc_ast_get_result_reg(right);
            nxc_instr_set_reg_index1(instr,right_reg); ///rvalue
            nxc_instr_set_var_offset(instr,nxc_sym_get_var_offset(sym)); ///var
            
            ///associate result reg with ast node ...
            nxc_ast_set_result_reg(ast,right_reg); ///left as result ...

            ///associate instr with ast ...
            nxc_ast_set_instr(ast,instr);

            ///trace local var reference and trnna fix later!!!
            nxc_compiler_trace_load_lvar(compiler,ast);

            ///inherit type ...
            nxc_ast_set_typename(ast,nxc_ast_get_typename(left));
            nxc_ast_set_f_typename(ast,nxc_ast_get_f_typename(left));

            return 0;
        } while (0);
    }///------------------------------------------------------------------------
#endif
    ///calc left lvalue
    ret = nxc_translate(compiler,left,1);
    if(ret) return ret;
    ///calc right rvalue
    ret = nxc_translate(compiler,right,0);
    if(ret) return ret;
    ///reload left ...
    left = nxc_expr_ast_get_left(ast);
    ///determine opcode ...
    switch (nxc_ast_get_opcode(left))
    {
    case nxc_op_index8:   ///'[['
        opcode_base = nxc_vmop_st8;
        break;
    case nxc_op_index16:  ///'[[[['
        opcode_base = nxc_vmop_st16;
        break;
    case nxc_op_index32:  ///'[[['
        opcode_base = nxc_vmop_st32;
        break;
    case nxc_op_index_l:  ///'['
        opcode_base = nxc_vmop_st_l;
        break;
    case nxc_op_id:       ///'$id'
        opcode_base = nxc_vmop_st_l;
        break;
	case nxc_op_member:   ///'.' will cause a long operation!!!
		opcode_base = nxc_vmop_st_l;
		break;
    default:
        nxc_ast_error(compiler,ast,"invalid lvalue node");
        return -1;
    }
    ///select vm_opcode here : base + offset 
    opcode = opcode_base + opcode_offset;
    ///generate binary operate instruction ...
    instr = nxc_new_instr(compiler,opcode);
    if(!instr) return -1;
    left_reg = nxc_ast_get_result_reg(left);
    right_reg = nxc_ast_get_result_reg(right);
    nxc_instr_set_reg_index1(instr,left_reg); ///left result reg
    nxc_instr_set_reg_index2(instr,right_reg); ///right result reg
    
    ///associate result reg with ast node ...
    nxc_ast_set_result_reg(ast,left_reg); ///left as result ...
    
    ///release right result reg ...
    nxc_free_reg(compiler,right_reg);

    ///inherit type ...
    nxc_ast_set_typename(ast,nxc_ast_get_typename(left));
    nxc_ast_set_f_typename(ast,nxc_ast_get_f_typename(left));
    
    return 0;
}

/**
 * translate comma ast node to marco instruction...
 * comma expression : (a,b,c)
 *         [,](r1)
 *       //   \\
 *     [,](r1) c(r1)
 *   //   \\
 *  a(r1)  b(r1)
 */
___fast int nxc_translate_comma(nxc_compiler_t *const compiler,
                                nxc_ast_t *ast,
                                int calc_lvalue)
{
    int ret,left_reg,right_reg;
    nxc_ast_t *left,*right;

    if(calc_lvalue) {///comma expression is not a lvalue node currently...
        nxc_ast_error(compiler,ast,"'comma' expr doest not support lvalue,fix later...");
        return -1;
    }
    ///load left ...
    left = nxc_expr_ast_get_left(ast);
    ///calc left rvalue
    ret = nxc_translate(compiler,left,0);
    if(ret) return ret;
    
    ///always re-load left ...
    left = nxc_expr_ast_get_left(ast);
    left_reg = nxc_ast_get_result_reg(left);
    /**
     * release left result register ...
     * this will cause that right expr will using the left reg that released as result_reg!!!
     * this just fit for the 'left-result' ast node ...
     */
    nxc_free_reg(compiler,left_reg);

    ///load left ...
    right = nxc_expr_ast_get_right(ast);
    ///calc right rvalue
    ret = nxc_translate(compiler,right,0);
    if(ret) return ret;
    ///always re-load left ...
    right = nxc_expr_ast_get_right(ast);
    right_reg = nxc_ast_get_result_reg(right);
    ///associate curr node with right result reg ...
    nxc_ast_set_result_reg(ast,right_reg);

    return 0;
}

/**===========================================================================*/

/**
 * translate expression statement node to instruction ...
 */
___fast int nxc_translate_expr_stmt(nxc_compiler_t*const compiler,nxc_ast_t*ast)
{
    int ret;
    nxc_ast_t *expr;

    expr = nxc_expr_stmt_ast_get_expr(ast);
    ret  = nxc_translate(compiler,expr,0);
    if(ret) return ret;

    ///release result regs ...
    nxc_free_reg(compiler,nxc_ast_get_result_reg(expr));

    return ret;
}

/**
 * check if target ast is a empty ast node ...
 */
___fast int nxc_is_empty_stmt(nxc_ast_t *ast)
{
    return !ast || ///no ast
           ( nxc_ast_get_opcode(ast)==nxc_op_block && 
             nxc_block_stmt_ast_is_stmt_list_empty(ast));///empty stmt-block
}

/**
 * translate 'if-node' to instruction ...
 */
___fast int nxc_translate_if_stmt(nxc_compiler_t *const compiler,nxc_ast_t *ast)
{
    int ret,cond_reg;
    nxc_instr_t *jxx_instr,*jmp_instr;
    nxc_instr_t *else_addr,*out_addr;
    nxc_ast_t *cond,*then_stmt,*else_stmt;

    ///load cond ast ...
    cond = nxc_if_stmt_ast_get_cond(ast);
    ///calc condition result for rvalue ...
    ret = nxc_translate(compiler,cond,0);
    if(ret) return ret;
    ///always reload cond ast ...
    cond = nxc_if_stmt_ast_get_cond(ast);
    ///get associated reg of ast ...
    cond_reg = nxc_ast_get_result_reg(cond);
    then_stmt = nxc_if_stmt_ast_get_then_stmt(ast);
    else_stmt = nxc_if_stmt_ast_get_else_stmt(ast);
    ///handle cases ...
    if( nxc_is_empty_stmt(then_stmt) && nxc_is_empty_stmt(else_stmt))
    {
        /**
         * process 'if ($cond) {} else {}
         */
        ///relese result register here ...
        nxc_free_reg(compiler,cond_reg);
        return 0;
    }
    else if(nxc_is_empty_stmt(else_stmt))
    {
        /**
         * process 'if ($cond) { $then_stmt } else {}'
         */
        jxx_instr = nxc_new_instr(compiler,nxc_vmop_jz);
        if(!jxx_instr) return -1;
        nxc_instr_set_reg_index1(jxx_instr,cond_reg);

        ///relese result register here ...
        nxc_free_reg(compiler,cond_reg);

        ///translate $then_stmt-------------------------------------------------
        ret = nxc_translate(compiler,then_stmt,0);
        if(ret) return ret;

        ///pre create a instruction and set jmp ...-----------------------------
        out_addr = nxc_pre_creat_instr(compiler);
        if(!out_addr) return -1;

        ///fix jxx instruction's target address ...!!!using relative address
        nxc_instr_set_jmp_offset_by_addr(jxx_instr,out_addr);
    }
    else if(nxc_is_empty_stmt(then_stmt))
    {
        /**
         * process 'if ($cond) {} else { $else_stmt }'
         */
        jxx_instr = nxc_new_instr(compiler,nxc_vmop_jnz);
        if(!jxx_instr) return -1;
        nxc_instr_set_reg_index1(jxx_instr,cond_reg);

        ///relese result register here ...
        nxc_free_reg(compiler,cond_reg);

        ///translate $else_stmt-------------------------------------------------
        ret = nxc_translate(compiler,else_stmt,0);
        if(ret) return ret;

        ///pre create a instruction and set jmp ...-----------------------------
        out_addr = nxc_pre_creat_instr(compiler);
        if(!out_addr) return -1;

        ///fix jxx instruction's target address ...
        nxc_instr_set_jmp_offset_by_addr(jxx_instr,out_addr);
    }
    else
    {
        /**
         * process 'if ($cond) { $then_stmt } else { $else_stmt }'
         */
        jxx_instr = nxc_new_instr(compiler,nxc_vmop_jz);
        if(!jxx_instr) return -1;
        nxc_instr_set_reg_index1(jxx_instr,cond_reg); ///jxx ($cond_reg) $else
        
        ///relese result register here ...
        nxc_free_reg(compiler,cond_reg);

        ///translate $then_stmt-------------------------------------------------
        ret = nxc_translate(compiler,then_stmt,0);
        if(ret) return ret;

        ///generate a 'jmp $out' instruction ...
        jmp_instr = nxc_new_instr(compiler,nxc_vmop_jmp); ///jmp $OUT
        if(!jmp_instr) return 0;

        ///pre create a instr to get start addr of else_stmt ...----------------
        else_addr = nxc_pre_creat_instr(compiler);
        if(!else_addr) return -1;

        ///fix jxx instruction's target address ...
        ///<<<fix else-addr of 'jxx ($cond_reg) $else'>>>
        nxc_instr_set_jmp_offset_by_addr(jxx_instr,else_addr);

        ///translate $else_stmt-------------------------------------------------
        else_stmt = nxc_if_stmt_ast_get_else_stmt(ast); ///always reload ast ...
        ret = nxc_translate(compiler,else_stmt,0);
        if(ret) return ret;

        ///pre-create a instr to get addr of out_instr ...
        out_addr = nxc_pre_creat_instr(compiler);
        if (!out_addr) return -1;

        ///fix jmp instruction's target address...
        ///fix OUT_address if 'jmp $OUT'
        nxc_instr_set_jmp_offset_by_addr(jmp_instr,out_addr);
    }

    return 0;
}

/**
 * fix all break/continue instruction of specific loop node ...
 */
___fast void nxc_fix_break_continue_instr(nxc_compiler_t *const compiler,
                                          nxc_ast_t   *loop_node,
                                          nxc_instr_t *loop_point,
                                          nxc_instr_t *break_point)
{
    nxc_ast_t *jmp_ast;
    nxc_instr_t *instr;

    nxc_dlist_for_each_entry(jmp_ast,
                            nxc_loop_stmt_ast_get_break_continue_list(loop_node),
                            __trace.trace_node,
                            nxc_ast_t)
    {
        instr = nxc_ast_get_instr(jmp_ast);
        switch (nxc_ast_get_opcode(jmp_ast))
        {
        case nxc_op_break:
            nxc_instr_set_jmp_offset_by_addr(instr,break_point);
            break;
        case nxc_op_continue:
            nxc_instr_set_jmp_offset_by_addr(instr,loop_point);
            break;
        default:
            nxc_error(compiler,"***BUG****");
            return;
        }
    }
}

/**
 * translate if node to instruction ...
 * return non-zero means error ...
 *   +-----------------+
 *   |    init-stmt    |
 *   +-----------------+
 *   |    jmp $cond    +---+
 *   +-----------------+   |
 *   |    next-stmt    |   | <------+
 *   +-----------------+   |        |
 *   |    cond-stmt    |<--+        |
 *   +-----------------+            |
 *   |  jz $out_point  +------+     |
 *   +-----------------+      |     |
 *   |    loop-body    |      |     |
 *   +-----------------+      |     |
 *   | jmp $loop_point +------|-----+
 *   +-----------------+      |
 *   | pre-creat-instr |<-----+
 *   +-----------------+
 */
___fast int nxc_translate_for_stmt(nxc_compiler_t*const compiler,nxc_ast_t *ast)
{
    int ret;
    nxc_instr_t *loop_point = 0;
    nxc_instr_t *break_point = 0;
    nxc_instr_t *jxx_instr = 0;
    nxc_instr_t *jmp_to_loop = 0;
    nxc_instr_t *jmp_to_cond = 0;///jmp instr for init stmt (to skip next-stmt)
    nxc_instr_t *cond_stmt_addr = 0;///start addr of condition statement ...

    nxc_ast_t *init_stmt,*cond,*next_stmt,*loop_stmt;
    int init_reg,cond_reg,next_reg;

    ///get loop_init ast ...
    init_stmt = nxc_loop_stmt_ast_get_init_stmt(ast);
    ///trnna translate init-stmt ...--------------------------------------------
    if(!nxc_is_empty_stmt(init_stmt))
    {
        ///do translate init-stmt ...
        ret = nxc_translate(compiler,init_stmt,0);
        if(ret) return ret;
        ///reload loop_init ast ...
        init_stmt = nxc_loop_stmt_ast_get_init_stmt(ast);
        ///get result of loop-init
        init_reg = nxc_ast_get_result_reg(init_stmt);
        //release result register ...
        nxc_free_reg(compiler,init_reg);
        /**
         * if next-stmt is empty , 
         * we just won't generate the jmp instr after init-stmt ...
         */
        ///get loop_next ast ...
        next_stmt = nxc_loop_stmt_ast_get_next_stmt(ast);
        if(!nxc_is_empty_stmt(next_stmt))
        {
            jmp_to_cond =nxc_new_instr(compiler,nxc_vmop_jmp);
            if(!jmp_to_cond) return -1;
        }
    }

    ///create loop point after init-stmt ...------------------------------------
    loop_point = nxc_pre_creat_instr(compiler);
    if(!loop_point) return -1;

    ///trnna translate next-stmt ...--------------------------------------------
    ///get loop_next ast ...
    next_stmt = nxc_loop_stmt_ast_get_next_stmt(ast);
    if(!nxc_is_empty_stmt(next_stmt))
    {
        ///do translate next-stmt ...
        ret = nxc_translate(compiler,next_stmt,0);
        if(ret) return ret;
        ///reload loop_next ast ...
        next_stmt = nxc_loop_stmt_ast_get_next_stmt(ast);
        ///get associated reg of next-stmt
        next_reg = nxc_ast_get_result_reg(next_stmt);
        ///result result register ...
        nxc_free_reg(compiler,next_reg);
        ///trnna fix jmp_addr for previous 'jmp $cond' instruction ...----------
        if(jmp_to_cond)
        {
            ///get start addr of condition statement or loop-body statement ...
            cond_stmt_addr = nxc_pre_creat_instr(compiler);
            if(!cond_stmt_addr) return -1;
            nxc_instr_set_jmp_offset_by_addr(jmp_to_cond,cond_stmt_addr);
        }
    }

    ///trnna translate the cond-stmt ...----------------------------------------
    ///get loop_condition ast ...
    cond = nxc_loop_stmt_ast_get_cond(ast);
    if(!nxc_is_empty_stmt(cond))
    {
        ///do translate cond-stmt ...
        ret = nxc_translate(compiler,cond,0);
        if(ret) return ret;
        ///reload loop_condition ast ...
        cond = nxc_loop_stmt_ast_get_cond(ast);
        ///generate a jxx instruction ...
        jxx_instr = nxc_new_instr(compiler,nxc_vmop_jz);
        if(!jxx_instr) return -1;
        ///get associated reg of condition statement ...
        cond_reg = nxc_ast_get_result_reg(cond);
        nxc_instr_set_reg_index1(jxx_instr,cond_reg);
        ///release condition result-register ...
        nxc_free_reg(compiler,cond_reg);
    }

    ///trnna translate the loop body ...----------------------------------------
    ///get loop_condition ast ...
    loop_stmt = nxc_loop_stmt_ast_get_loop_stmt(ast);
    if(!nxc_is_empty_stmt(loop_stmt))
    {
        ///do translate next-stmt ...
        ret = nxc_translate(compiler,loop_stmt,0);
        if(ret) return ret;
    }

    ///generate a jmp instruction ...
    jmp_to_loop = nxc_new_instr(compiler,nxc_vmop_jmp);
    if(!jmp_to_loop) return -1;
    nxc_instr_set_jmp_offset_by_addr(jmp_to_loop,loop_point);
    //jmp_to_loop->u.jmp_addr = loop_point;

    ///get break point ...------------------------------------------------------
    break_point = nxc_pre_creat_instr(compiler);
    if(!break_point) return -1;

    ///fix jxx for cond-stmt if necessary ...
    if(jxx_instr) 
        nxc_instr_set_jmp_offset_by_addr(jxx_instr,break_point);
        //jxx_instr->u.jmp_addr = break_point;

    /**
     * here , fix all sub break/continue statement ...
     */
    nxc_fix_break_continue_instr(compiler,ast,loop_point,break_point);

    return 0;
}

/**
 * translate while node to instruction ...
 */
___fast int nxc_translate_while_stmt(nxc_compiler_t*const compiler,nxc_ast_t*ast)
{
    int ret;
    nxc_instr_t *jxx_instr;
    nxc_instr_t *jmp_instr;
    nxc_instr_t *loop_point;
    nxc_instr_t *break_point;

    nxc_ast_t *cond,*loop_stmt;
    int cond_reg;

    ///get loop point first ...
    loop_point = nxc_pre_creat_instr(compiler);
    if(!loop_point) return -1;

    ///get loop_condition ast ...
    cond = nxc_loop_stmt_ast_get_cond(ast);
    ///translate condition ...
    ret = nxc_translate(compiler,cond,0);
    if(ret) return ret;
    
    ///reload loop_condition ast ...
    cond = nxc_loop_stmt_ast_get_cond(ast);
    ///generate a 'jz $break_point' ...
    jxx_instr = nxc_new_instr(compiler,nxc_vmop_jz);
    if(!jxx_instr) return -1;
    cond_reg = nxc_ast_get_result_reg(cond);
    nxc_instr_set_reg_index1(jxx_instr,cond_reg);

    ///release result register ...
    nxc_free_reg(compiler,cond_reg);

    ///load loop_condition ast ...
    loop_stmt = nxc_loop_stmt_ast_get_loop_stmt(ast);
    ///trnna translate loop body ...
    if(!nxc_is_empty_stmt(loop_stmt))
    {
        ret = nxc_translate(compiler,loop_stmt,0);
        if(ret) return ret;
    }

    ///generate a 'jmp $loop_point' instruction ...
    jmp_instr = nxc_new_instr(compiler,nxc_vmop_jmp);
    if(!jmp_instr) return -1;
    nxc_instr_set_jmp_offset_by_addr(jmp_instr,loop_point);

    ///get break point ...
    break_point = nxc_pre_creat_instr(compiler);
    if(!break_point) return -1;

    ///fix 'jxx' instr ...
    nxc_instr_set_jmp_offset_by_addr(jxx_instr,break_point);

    /**
     * here , fix all sub break/continue statement ...
     */
    nxc_fix_break_continue_instr(compiler,ast,loop_point,break_point);

    return 0;
}

/**
 * translate do node to instruction ...
 */
___fast int nxc_translate_do_stmt(nxc_compiler_t *const compiler,nxc_ast_t *ast)
{
    int ret;
    nxc_instr_t *jxx_instr;
    nxc_instr_t *loop_point;
    nxc_instr_t *break_point;

    nxc_ast_t *cond,*loop_stmt;
    int cond_reg;

    ///create and save loop point first ...
    loop_point = nxc_pre_creat_instr(compiler);
    if(!loop_point) return -1;

    ///get loop statement ast ...
    loop_stmt = nxc_loop_stmt_ast_get_loop_stmt(ast);
    ///trnna translate loop body ...
    if(!nxc_is_empty_stmt(loop_stmt))
    {
        ret = nxc_translate(compiler,loop_stmt,0);
        if(ret) return ret;
    }

    ///get condition statement ast ...
    cond = nxc_loop_stmt_ast_get_cond(ast);
    ///translate condition statement ...
    ret = nxc_translate(compiler,cond,0);
    if(ret) return ret;

    ///reload condition statement ast ...
    cond = nxc_loop_stmt_ast_get_cond(ast);

    ///generate a 'jnz $loop_point' instr ...
    jxx_instr = nxc_new_instr(compiler,nxc_vmop_jnz);
    if(!jxx_instr) return -1;
    cond_reg = nxc_ast_get_result_reg(cond);
    nxc_instr_set_reg_index1(jxx_instr,cond_reg);
    nxc_instr_set_jmp_offset_by_addr(jxx_instr,loop_point);

    ///release result register ...
    nxc_free_reg(compiler,cond_reg);

    ///get break point here ...
    break_point = nxc_pre_creat_instr(compiler);
    if(!break_point) return -1;

    /**
     * here , fix all sub break/continue statement ...
     */
    nxc_fix_break_continue_instr(compiler,ast,loop_point,break_point);

    return 0;
}

/**
 * translate break/continue node to instruction ...
 */
___fast int nxc_translate_break_continue(nxc_compiler_t*const compiler,
                                         nxc_ast_t *ast)
{
    nxc_instr_t *instr;

    ///create jmp instr for func ...
    instr = nxc_new_instr(compiler,nxc_vmop_jmp);
    if(!instr) return -1;

    ///associate instr with ast for fixing later ...
    nxc_ast_set_instr(ast,instr);
    //ast->u.break_stmt.instr = instr;

    ///all break&continue statement has been traced to loop node ...

    return 0;
}

/**
 * translate return node to instruction ...
 */
___fast int nxc_translate_return_stmt(nxc_compiler_t*const compiler,
                                      nxc_ast_t*ast)
{
    int ret,result_reg;
    nxc_instr_t *instr;
    nxc_ast_t *result;

    ///get result-ast ...
    result = nxc_return_stmt_ast_get_result(ast);
    if(result)
    {
        ///calc return result for rvalue ...
        ret = nxc_translate(compiler,result,0);
        if(ret) return ret;
        ///create 'ret val' instr for func ...
        instr = nxc_new_instr(compiler,nxc_vmop_return_val);
        if(!instr) return -1;
        ///reload result-ast ...
        result = nxc_return_stmt_ast_get_result(ast);
        ///get associated reg of result ...
        result_reg = nxc_ast_get_result_reg(result);
        nxc_instr_set_reg_index1(instr,result_reg);
        ///release result reg ...
        nxc_free_reg(compiler,result_reg);

        return 0;
    }
    ///create 'ret null' instr for func ...
    instr = nxc_new_instr(compiler,nxc_vmop_ret);
    if(!instr) return -1;

    return 0;
}

/**
 * translate block node to instruction ...
 */
___fast int nxc_translate_stmt_block(nxc_compiler_t *const compiler,
                                     nxc_ast_t *ast)
{
    nxc_ast_t *stmt;

    ///traversal all statement and do check ...
    nxc_dlist_for_each_entry(stmt,
                             nxc_block_stmt_ast_get_stmt_list(ast),
                             list_head,
                             nxc_ast_t)
    {
        if(nxc_translate(compiler,stmt,0)) return -1;
    }

    return 0;
}

/**===========================================================================*/

/**
 * Translate AST to Marco-Instruction ...
 * return non-zero means error ...
 */
static int nxc_translate(nxc_compiler_t *const compiler,
                         nxc_ast_t *ast,
                         int calc_lvalue)
{
    int ret,opcode;

    opcode = nxc_ast_get_opcode(ast);
    switch (opcode)
    {
    ///-----------------------expression(comma expression)----------------------
    case nxc_op_comma:        ///","
        ret = nxc_translate_comma(compiler,ast,calc_lvalue);
        return ret;
    ///---------------------------assignment expression-------------------------
    case nxc_op_assign:       ///"="
    case nxc_op_add_assign:   ///"+="
    case nxc_op_sub_assign:   ///"-="
    case nxc_op_mul_assign:   ///"*="
    case nxc_op_div_assign:   ///"/="
    case nxc_op_mod_assign:   ///"%="
    case nxc_op_bitor_assign: ///"|="
    case nxc_op_bitand_assign:///"&="
    case nxc_op_xor_assign:   ///"^="
    case nxc_op_lshift_assign:///"<<="
    case nxc_op_rshift_assign:///">>="
        ret = nxc_translate_assign(compiler,ast,opcode-nxc_op_assign,calc_lvalue);
        return ret;
    ///----------------------------binary expression----------------------------
    case nxc_op_or:           ///"||"   
    case nxc_op_and:          ///"&&"
    case nxc_op_bit_or:       ///"|"
    case nxc_op_bit_and:      ///"&"
    case nxc_op_xor:          ///"^"
    case nxc_op_equal:        ///"=="
    case nxc_op_unequal:      ///"!="
    case nxc_op_great:        ///">"
    case nxc_op_less:         ///"<"
    case nxc_op_great_equal:  ///">="
    case nxc_op_less_equal:   ///"<="
    case nxc_op_left_shift:   ///"<<"
    case nxc_op_right_shift:  ///">>"
    case nxc_op_add:          ///"+"
    case nxc_op_sub:          ///"-"
    case nxc_op_mul:          ///"*"
    case nxc_op_div:          ///"/"
    case nxc_op_mod:          ///"%"
        ret = nxc_translate_binary(compiler,ast,opcode-nxc_op_or,calc_lvalue);
        return ret; 
    ///----------------------------unary opcode---------------------------------
    case nxc_op_inc:          ///"++"
    case nxc_op_dec:          ///"--"
    case nxc_op_positive:     ///"+"
    case nxc_op_negative:     ///"-"
    case nxc_op_compensation: ///"~"
    case nxc_op_not:          ///"!"
    case nxc_op_address:      ///"&"
    case nxc_op_sizeof:       ///"sizeof"
    case nxc_op_cast:         ///"cast"
        ret=nxc_translate_prefix(compiler,ast,opcode,calc_lvalue);
        return ret;
    ///~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    case nxc_op_index8:   ///"[["
    case nxc_op_index16:  ///"[[[["
    case nxc_op_index32:    ///"[[["
    case nxc_op_index_l:   ///"["
        ret=nxc_translate_index(compiler,ast,opcode,calc_lvalue);
        return ret;
    ///~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    case nxc_op_call:         ///"call"
        if( calc_lvalue ){    ///func call expression is not a lvalue ...
            nxc_ast_error(compiler,ast,"'call' expr has no lvalue!");
            return -1;
        }
        ret = nxc_translate_call(compiler,ast);
        return ret;
    case nxc_op_member:       ///"."
        ret = nxc_translate_member(compiler,ast,calc_lvalue);
        return ret;
    case nxc_op_ptr_right:
        nxc_ast_error(compiler,ast,"not support currently...");
        return -1;
    ///--------------------------------factor-----------------------------------
    case nxc_op_id:           ///"$ID"
        ret = nxc_translate_id(compiler,ast,calc_lvalue);
        return ret;
    case nxc_op_const:        ///"const"
        if(calc_lvalue ) {
            nxc_ast_error(compiler,ast,"'const' has no lvalue!");
            return -1;///[1].const expression is not a lvaue
        }
        ret = nxc_translate_const(compiler,ast);///[2].do translate ...
        return ret;
    ///-------------------------------statement---------------------------------
    case nxc_op_expression:
        ret = nxc_translate_expr_stmt(compiler,ast);
		if(compiler->expr_sp) nxc_ast_error(compiler,ast,"register-leak!!!");
        return ret;
    case nxc_op_if:
        ret = nxc_translate_if_stmt(compiler,ast);
		if(compiler->expr_sp) nxc_ast_error(compiler,ast,"register-leak!!!");
        return ret;
    case nxc_op_for:
        ret = nxc_translate_for_stmt(compiler,ast);
		if(compiler->expr_sp) nxc_ast_error(compiler,ast,"register-leak!!!");
        return ret;
    case nxc_op_while:
        ret = nxc_translate_while_stmt(compiler,ast);
		if(compiler->expr_sp) nxc_ast_error(compiler,ast,"register-leak!!!");
        return ret;
    case nxc_op_do:
        ret = nxc_translate_do_stmt(compiler,ast);
		if(compiler->expr_sp) nxc_ast_error(compiler,ast,"register-leak!!!");
		if(compiler->expr_sp) nxc_ast_error(compiler,ast,"register-leak!!!");
        return ret;
    case nxc_op_break:
    case nxc_op_continue:
        ret = nxc_translate_break_continue(compiler,ast);
		if(compiler->expr_sp) nxc_ast_error(compiler,ast,"register-leak!!!");
        return ret;
    case nxc_op_return:
        ret = nxc_translate_return_stmt(compiler,ast);
		if(compiler->expr_sp) nxc_ast_error(compiler,ast,"register-leak!!!");
        return ret;
    case nxc_op_block:
        ret = nxc_translate_stmt_block(compiler,ast);
		///check expression-stack
		if(compiler->expr_sp) nxc_ast_error(compiler,ast,"register-leak!!!");
        return ret;
    default:
        nxc_ast_error(compiler,ast,"unknown ast opcode:%d",nxc_ast_get_opcode(ast));
        return -1;
    }

    return 0;
}

/**===========================================================================*/
/**===========================================================================*/
/**===========================================================================*/

/**
 * create a 'enter' instruction for local function ...
 * return non-zero means error ...
 */
___fast int nxc_fix_prolog_of_func(nxc_compiler_t*const compiler,nxc_sym_t*func)
{
    nxc_instr_t *instr;

    ///this will add instruction to current function automatically ...
    instr = nxc_new_instr(compiler,nxc_vmop_enter);
    if(!instr) return -1;
    ///set function entry-point ...
    nxc_sym_set_func_addr(func,instr);

    return 0;
}
/**
 * add a 'return ;' ast to 'empty' func or 'no-return' func...
 * return non-zero means error ...
 */
___fast int nxc_fix_return_of_func(nxc_compiler_t*const compiler,nxc_sym_t*func)
{
    nxc_instr_t *instr;
    
    (void)func;
    ///this will add instruction to current function automatically ...
    instr = nxc_new_instr(compiler,nxc_vmop_ret);
    if(!instr) return -1;

    return 0;
}

/**
 * fix 'load $id' & 'enter' instructions for given function ...
 * return non-zero means error ...
 */
___fast int nxc_fix_load_lvar_of_func(nxc_compiler_t*const compiler,nxc_sym_t*func)
{
    nxc_instr_t *instr;
    nxc_ast_t *ast;
    int offset,var_offset;
    int local_stack_size,reg_sect_size,var_sect_size;

    /**
     * [1].fix register area size .
     */
    reg_sect_size = compiler->expr_sp_max;
    var_sect_size = nxc_sym_get_func_lvar_size(func);
    nxc_sym_set_func_lreg_size(func , reg_sect_size); ///save local-register sz

    /**
     * [2].fix prolog of function ...
     */
    ///align the reg-section size ...
    reg_sect_size = nxc_align(reg_sect_size ,sizeof(long)*2);
    ///align the var-section size ...
    var_sect_size = nxc_align(var_sect_size ,sizeof(long)*2);
    ///calc total stack frame size (aligned size)...
    local_stack_size = reg_sect_size + var_sect_size;
    ///get function's 'enter' instruction
    instr = nxc_sym_get_func_addr(func);
    ///fix 'enter' instruction
    nxc_instr_set_immed32(instr,local_stack_size);

    /**
     * [3].fix load local-var instructions of function ...
     */
    ///offset = local variable area start offset
    offset = reg_sect_size;
    nxc_dlist_for_each_entry(ast,&compiler->load_lvar_list,__trace.trace_node,nxc_ast_t)
    {
        ///get 'load $reg,$local_var' instruction ...
        instr = nxc_ast_get_instr(ast);
        /**
         * move all local var to the end of expresiion-stack ...
         *  low------------------------------>high
         * --+--------+--------+-------+--------+--
         *   |  ...   |  var1  |  var0 |  ebp   |
         * --+--------+--------+-------+--------+--
         *              offset/
         *              +----+
         *             /
         * --+--------+--------+-------+--------+--
         *   |  var0  |  reg2  |  reg1 |  ebp   |
         * --+--------+--------+-------+--------+--
         * -var_area->|<-  reg_area  ->|
         */
        var_offset =  nxc_instr_get_var_offset(instr);
        var_offset -= offset;
        nxc_instr_set_var_offset(instr,var_offset);    ///fix local var offset
    }

    return 0;
}

/**
 * check all local function ...
 * return non-zero means error ...
 */
static int nxc_translate_all_local_func(nxc_compiler_t *const compiler)
{
    nxc_sym_t *func;

    ///traversal all func and o check & translate ...
    nxc_dlist_for_each_entry(func,&compiler->global_sym_list,node.gsym_node,nxc_sym_t)
    {
        ///skip global - var ...
        if(nxc_sym_get_type(func) != nxc_sym_global_function )  continue;
        ///skip extern func ...
        if(nxc_sym_has_flag(func,nxc_symflag_extern)) continue;

        ///set curr function first (so that we could locate the local symbol...)
        compiler->curr_function = func;

        ///do initialization ...
        compiler->expr_sp_max = 0;
        compiler->expr_sp = 0;
        nxc_compiler_init_load_lvar_list(compiler);

        ///[1].create prolog instr for each func ...
        if(nxc_fix_prolog_of_func(compiler,func)) return -1;

        ///[2].translate function body...
        if(nxc_translate(compiler,nxc_sym_get_func_stmt_ast(func),0)) return -1;

        ///[3].add a 'return null' ast if func has no return statement at tail..
        if(nxc_fix_return_of_func(compiler,func)) return -1;

        ///fix all load-local-var instruction and prolog of this function ...
        if(nxc_fix_load_lvar_of_func(compiler,func)) return -1;

        if(compiler->enable_debug_info)
        {
            nxc_printf(compiler,
                "----------------------------------------------------------\n");
            nxc_printf(compiler,
                "function:\n\tname         :%s\n",nxc_sym_get_name(func));
            nxc_printf(compiler,
                "\taddress      :0x%08X\n",nxc_sym_get_func_addr(func));
            nxc_printf(compiler,
                "\tstack-size   :%d ([%d])\n",
                nxc_sym_get_func_lvar_size(func),nxc_sym_get_func_lvar_size(func)/4);
            nxc_printf(compiler,
                "\texpr-size    :%d ([%d])\n",
                nxc_sym_get_func_lreg_size(func),nxc_sym_get_func_lreg_size(func)/4);

            nxc_printf(compiler,"-------dump instr--------\n");
            nxc_dump_func_code(compiler,func);
            nxc_printf(compiler,
                "----------------------------------------------------------\n");
        }

        ///here , compiler's expr_sp should be zero!!!
        if(compiler->expr_sp!=0)
        {
            nxc_ast_error(compiler,0,"***EXPR-STACK IS NOT BALANCE***!!!");
            return -1;
        }
        ///-----------------clear translator's tmp-context-data-----------------
        compiler->expr_sp_max = 0;
        compiler->expr_sp = 0;
        compiler->curr_function = 0;
        nxc_compiler_init_load_lvar_list(compiler);//reset 'load-local-var'trace list ...
    }

    return 0;
}

/**
 * alloc data block and apply the data ...
 * @return block head position 
 */
___fast char* __nxc_compiler_apply_dseg_block(nxc_compiler_t*const compiler,
                                              int   dtype,///data type
                                              void *data, ///date ptr
                                              int   dsize)///data size
{
    char* ret;
    int size;

    size = dsize;
    size ++; ///zero-terminator
    size += 8;///reserved header space ...
    size = nxc_align(size,sizeof(long)*2);///keep 8/16 Bytes aligned 
    ret = compiler->image->data_base + compiler->image->data_size;

    ///update total data size ...
    compiler->image->data_size += size;

    if (compiler->image->data_size > compiler->image->data_seg_size){
        nxc_printf(compiler,"FATAL ERROR : insucifficent dseg space");
        return 0;
    }
    nxc_memset(ret,0,size);      ///reset mem ...
    *(int *)ret = dsize;         ///write data size ...
    *(int *)(ret + 4) = dtype;   ///write type info ...
    if (data) nxc_memcpy(ret+8,data,dsize);///write data ...

    return ret;
}

/**
 * generate data segment ...
 * @return 0 means okay ...
 */
static int nxc_gen_data_segment(nxc_compiler_t*const compiler)
{
    int total_dsize;
    int data_reloc_table_size;
    int xsym_reloc_table_size;
    int exp_sym_table_size;
    nxc_str_node_t *str_node;
    char *ptr;
    nxc_sym_t *sym;
    long *l_ptr;
    nxc_ast_t *ast;
    nxc_instr_t *instr;

    ///export symbol table
	///BUG:2011-4-27 23:28:57:recalc the exported sym count!!!
	compiler->global_sym_count = 0;
	nxc_dlist_for_each_entry(sym,&compiler->global_sym_list,node.gsym_node,nxc_sym_t)
    {
		///skip extern symbol ...
		if (nxc_sym_has_flag(sym,nxc_symflag_extern)) continue;
		compiler->global_sym_count++;
    }
    exp_sym_table_size    = compiler->global_sym_count * ( sizeof(long) * 2 );
    nxc_compiler_alloc_dseg_space(compiler,exp_sym_table_size);

    ///extern symbol relocation table
    xsym_reloc_table_size = compiler->load_xsym_count  * ( sizeof(long) * 2 );
    nxc_compiler_alloc_dseg_space(compiler,xsym_reloc_table_size);

    ///data relocation table
    ///   [1].global-sym relocation
    data_reloc_table_size = compiler->load_gsym_count  * ( sizeof(long) * 2 );
    ///   [2].const  relocation
    data_reloc_table_size += compiler->load_const_count * ( sizeof(long) * 2 );
    nxc_compiler_alloc_dseg_space(compiler,data_reloc_table_size);

    if (compiler->enable_debug_info)
    {
        nxc_printf(compiler,"exp_sym_table :%d\n",exp_sym_table_size);
        nxc_printf(compiler,"extern_sym_reloc :%d\n",xsym_reloc_table_size);
        nxc_printf(compiler,"data_reloc :%d\n",data_reloc_table_size);
    }

    ///get total data seg size ...
    total_dsize = compiler->dseg_size;
    
    nxc_compiler_malloc(compiler,10);
    compiler->image->data_base = (char*)__nxc_compiler_malloc(compiler,total_dsize);
    if (!compiler->image->data_base)
    {
        nxc_printf(compiler,"low mem!!!");
        return -1;
    }
    nxc_memset(compiler->image->data_base,0,total_dsize);

    ///set alloc position 
    compiler->image->data_size = 0;               ///reset 'real-size' ...
    compiler->image->data_seg_size = total_dsize; ///save max size ...
    
    ///
    /// build string section ..
    ///
    nxc_dlist_for_each_entry(str_node,&compiler->const_str_list,trace_node,nxc_str_node_t)
    {
        ///alloc space and write data ...
        ptr = __nxc_compiler_apply_dseg_block(compiler,
                                              0,
                                              nxc_str_node_get_str(str_node),
                                              nxc_str_node_get_len(str_node));
        nxc_str_node_set_ext_data(str_node,ptr+8);///save addr allocated
    }

    ///
    /// build `gloal-variable-data` section and fix symbol address...
    ///
    nxc_dlist_for_each_entry(sym,&compiler->global_sym_list,node.gsym_node,nxc_sym_t)
    {
        ///skip non variable ...
        if(nxc_sym_get_type(sym) != nxc_sym_global_var) continue;
        ///skip extern variable ...(not necessary)
        if(nxc_sym_has_flag(sym,nxc_symflag_extern)   ) continue;

        ///alloc space for each global var ...
        ptr = __nxc_compiler_apply_dseg_block(compiler,
                                              0,///default type
                                              0,///no-data
                                              nxc_sym_get_typesize(sym));///size
        ///fix global-variable-address!!!
        nxc_sym_set_var_addr(sym,(void *)(ptr+8)); ///save variable address
    }
    ///~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    ///~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    ///~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /**
     * ###############################Export-Table##############################
     * build exported-symbol-table ...
     *  +---------------+
     *  | sym_name_addr |
     *  +---------------+
     *  | sym_address   |
     *  +---------------+
     */
    ptr = __nxc_compiler_apply_dseg_block(compiler,0,0,exp_sym_table_size);
    l_ptr = (long *)(ptr + 8); ///get chunk start
    ///save table offset ...
    compiler->image->export_table_offset = (long)l_ptr - (long)compiler->image->data_base;
    compiler->image->export_count = compiler->global_sym_count; 
    ///do build table ...
    nxc_dlist_for_each_entry(sym,&compiler->global_sym_list,node.gsym_node,nxc_sym_t)
    {
		///skip extern symbol ...
		if (nxc_sym_has_flag(sym,nxc_symflag_extern)) continue;
		///debug output ...
        if (compiler->enable_debug_info)
            nxc_printf(compiler,"Export SYM:%s %d\n",nxc_sym_get_name(sym),nxc_sym_get_namelen(sym));
        ///write symbol-name address
        l_ptr[0] = (long)nxc_str_node_get_ext_data(nxc_sym_get_str_node(sym));
        ///write symbol address (func and g_var)
        l_ptr[1] = (long)nxc_sym_get_var_addr(sym);
        ///move to next chunk ...
        l_ptr += 2;
    }

    /**
     * ##############################Import-Table###############################
     * build ExternSYM Relocation-Table and fix (`load $Reg , $ExternSym`) ...
     *  +---------------+
     *  | sym_name_addr |
     *  +---------------+
     *  | fix_pos addr  | (operand2's address)
     *  +---------------+
     *  |    ... ...    |  repeat
     */
    ptr = __nxc_compiler_apply_dseg_block(compiler,0,0,xsym_reloc_table_size);
    l_ptr = (long *)(ptr + 8); ///get chunk start
    ///save data-relocation-table offset ...
    compiler->image->import_table_offset = (long)l_ptr - (long)compiler->image->data_base;
    compiler->image->import_count = compiler->load_xsym_count;
    ///do build table ...
    nxc_dlist_for_each_entry(ast,&compiler->load_xsym_list,__trace.trace_node,nxc_ast_t)
    {
        ///get associated symbol ...
        sym = nxc_ast_get_sym(ast);
        if (compiler->enable_debug_info)
            nxc_printf(compiler,"fix instr:load_extern [%s]\n",nxc_sym_get_name(sym));
        ///get associated instruction (load reg,$sym)...
        instr = nxc_ast_get_instr(ast);
        ///write symbol-name address
        l_ptr[0] = (long)nxc_str_node_get_ext_data(nxc_sym_get_str_node(sym));  
        ///write instruction operand2 address ...
        l_ptr[1] = (long)nxc_instr_get_operand2_addr(instr);
        ///move to next 
        l_ptr+=2;
    }

    /**
     * #############################Relocation-Table############################
     * build Data Relocation-Table ...
     *  +---------------+
     *  | fix_pos addr1 | point to instruction's operand2 ...
     *  +---------------+
     *  | fix_pos addr2 |
     *  +---------------+
     *  |    ... ...    |
     */
    ptr = __nxc_compiler_apply_dseg_block(compiler,0,0,data_reloc_table_size);
    l_ptr = (long *)(ptr + 8); ///get chunk start
    ///save data-relocation-table offset ...
    compiler->image->reloc_table_offset = (long)l_ptr - (long)compiler->image->data_base;
    compiler->image->reloc_count = compiler->load_gsym_count + compiler->load_const_count;
    ///do build table ...
    ///
    /// [1].fix `load $reg , $global_symbol`
    ///
    nxc_dlist_for_each_entry(ast,&compiler->load_gsym_list,__trace.trace_node,nxc_ast_t)
    {
        ///get associated symbol ...
        sym = nxc_ast_get_sym(ast);
        if (compiler->enable_debug_info)
            nxc_printf(compiler,"-=-=-=-=fix instr:load [%s]\n",nxc_sym_get_name(sym));
        ///get associated instruction (load $reg , $symbol-address)...
        instr = nxc_ast_get_instr(ast);
        ///write instruction address
        if (nxc_sym_get_type(sym) == nxc_sym_global_function){
            l_ptr[0] = 1;                                   ///TYPE:XSEG-Reloc
        }else{
            l_ptr[0] = 0;                                   ///TYPE:DSEG-Reloc
        }
        l_ptr[1] = (long)nxc_instr_get_operand2_addr(instr);///DATA:OrigRelocPos
        
        ///fix instruction operands ...
        nxc_instr_set_func_addr(instr,(nxc_instr_t *)nxc_sym_get_var_addr(sym));
        ///move to next ...
        l_ptr+=2;
    }
    ///
    /// [2].fix `load $reg , $const_string`
    ///
    nxc_dlist_for_each_entry(ast,&compiler->load_const_list,__trace.trace_node,nxc_ast_t)
    {
        ///get associated symbol ...
        str_node = nxc_const_ast_get_str_node(ast); 
		///debug output ...
        if (compiler->enable_debug_info)
            nxc_printf(compiler,"fix instr:load_const [%s]\n",nxc_str_node_get_str(str_node));
        ///get associated instruction (load $reg,$constant)...
        instr = nxc_ast_get_instr(ast);
        ///write instruction address
        l_ptr[0] = 0;                                     //TYPE:dseg relocation
        l_ptr[1]=(long)nxc_instr_get_operand2_addr(instr);//DATA:OrigRelocPos
        
        ///fix instruction operands ...
        nxc_instr_set_xaddr(instr,nxc_str_node_get_ext_data(str_node));
        ///move to next ...
        l_ptr+=2;
    }

    return 0;
}

/**
 * generate the text segment for compiler ...
 * @return 0 means okay ...
 */
static int nxc_gen_text_segment(nxc_compiler_t*const compiler)
{
    if (compiler->enable_debug_info)
        nxc_printf(compiler,"===total text size : %d\n",compiler->xseg_size);
    compiler->image->text_base = (nxc_instr_t*)__nxc_compiler_malloc(compiler,compiler->xseg_size);
    if (!compiler->image->text_base){
        nxc_printf(compiler,"low memory!!!");
        return -1;
    }
    nxc_memset(compiler->image->text_base,0,compiler->xseg_size);///reset memory
    compiler->image->text_seg_size = compiler->xseg_size; ///save max-size
    compiler->image->text_size     = 0; ///reset text-size counter 

    return 0;
}

///#############################################################################
///#############################################################################
///#############################################################################

/**
 * init a compiler object ...
 * @return 0 means okay ...
 */
int nxc_init_compiler(nxc_compiler_t *compiler,
                      nxc_allocator_t malloc_proc,
                      nxc_deallocator_t free_proc,
                      void*alloc_data,
                      char *parse_buff,
                      int max_parse_buff)
{
    ///zero all ...
    nxc_memset(compiler,0,sizeof(*compiler));

    ///init a private mempool for compiler ...
    nxc_init_mpool(compiler->mem_pool,malloc_proc,free_proc,alloc_data);
    
    ///create string table ...
    nxc_init_builtin_hash_table(compiler->str_table,compiler->mem_pool,128,0,0);
    
    ///lexer will share the mempool ...
    nxc_lex_init(compiler->lexer);
    ///init XXX lexer here ...
    nxc_lex_init_keywords(compiler);
    ///create const-alias table ...
    nxc_init_builtin_hash_table(compiler->const_table,compiler->mem_pool,128,0,0);
    ///create symbol table ...
    nxc_init_builtin_hash_table(compiler->sym_table,compiler->mem_pool,128,0,0);
    
    ///init local function list ...
    nxc_compiler_init_global_sym_list(compiler); ///trace local function 
    nxc_compiler_init_const_str_list(compiler);  ///trace const-string and gsym-name
    nxc_compiler_init_load_gsym_list(compiler);  //trace:load gfunc/gvar instr
    nxc_compiler_init_load_xsym_list(compiler);  //trace:load extern sym instr
    nxc_compiler_init_load_const_list(compiler); //trace:load const string instr
    
    ///init load-local_var instruction trace list ...
    nxc_compiler_init_load_lvar_list(compiler);

    if (!max_parse_buff) max_parse_buff = 8000;///8K at most by default ...
    if (!parse_buff)parse_buff=(char*)nxc_compiler_malloc(compiler,max_parse_buff);
    if (parse_buff){
        nxc_lex_set_parsebuf(compiler->lexer,parse_buff,max_parse_buff);
    }
    else{///set default name-buffer ...
        nxc_lex_set_parsebuf(compiler->lexer,compiler->lexer->def_parse_buff,32);
    }

    return 0;
}

/**
 * finit a compiler object ...
 */
int nxc_fini_compiler(nxc_compiler_t *compiler)
{
    ///save necessary info ...
    nxc_allocator_t malloc_proc;
    nxc_deallocator_t free_proc;
    void*alloc_data;

    malloc_proc = compiler->mem_pool->do_malloc;
    free_proc = compiler->mem_pool->do_free;
    alloc_data = compiler->mem_pool->allocator_data;

    ///clear the memory pool ...
    nxc_clear_mpool(compiler->mem_pool);

    ///if we has the corrupted image , just destroy it ...
    if (compiler->image)
    {
        nxc_script_image_do_destroy(compiler->image,free_proc);
        compiler->image = 0;
    }

    return 0;
}

/**
 * compile the given source string and generate marco-instructions...
 * return 0 means okay ...
 * @return non-zero means error.
 */
nxc_script_image_t* nxc_do_compile(nxc_compiler_t*const compiler,
                                   int count,
                                   char *fname_table[],
                                   char *source_table[])
{
    int i,ret;
    char*fname;
    nxc_script_image_t *image;

    ///1.create image header ...
    image = (nxc_script_image_t *)__nxc_compiler_malloc(compiler,sizeof(nxc_script_image_t));
    if (!image)
    {
        nxc_printf(compiler,"low memory!!!\n");
        return 0;
    }
    nxc_script_image_do_init(image);
    compiler->image = image;

    /**
     * 2.parse all source code and generate the ast ...
     */
    ret = -1;
    for (i=0;i<count;i++)
    {
        fname = fname_table[i];
        if(!fname_table[i]) fname = "N/A";
        if(!source_table[i]) continue; //skip invalid one ...
        nxc_printf(compiler,"\tparse %s ...\n",fname);
        ret = nxc_do_parse(compiler,fname_table[i],source_table[i]);
        if(ret)
        {
            nxc_printf(compiler,"\tparse %s \t\t[ failed ]\n",fname);
            return 0;
        }
        nxc_printf(compiler,"\tparse %s \t\t[ OK ]\n",fname);
        if (compiler->enable_debug_info){
            nxc_printf(compiler,"\t.data size : %d\n",compiler->dseg_size);
            nxc_printf(compiler,"\t.text size : %d\n",compiler->xseg_size);
        }
    }
    if (ret){
        nxc_printf(compiler,"\tno source available!!!\n");
        return 0;
    }

    /**
     * 3.alloc text-segment ...
     */
    nxc_printf(compiler,"\tgenerate .text segment ...\n");
    ret = nxc_gen_text_segment(compiler);
    if (ret)
    {
        nxc_printf(compiler,"\tgenerate .text segment \t\t[failed]\n");
        return 0;
    }
    nxc_printf(compiler,"\tgenerate .text segment \t\t[ OK ]\n");   

    /**
     * 4.check ast and translate into marco-instruction... 
     */
    nxc_printf(compiler,"\ttranslate ast...\n");
    if(nxc_translate_all_local_func(compiler))
    {
        nxc_printf(compiler,"\ttranslate ast \t\t[failed]\n");
        return 0;
    }
    nxc_printf(compiler,"\ttranslate ast \t\t[ OK ]\n");

    /**
     * 5.build the data-segment ...
     */
    nxc_printf(compiler,"\tgenerate .data segment...\n");
    if(nxc_gen_data_segment(compiler))
    {
        nxc_printf(compiler,"\tgenerate .data segment \t\t[failed]\n");
        return 0;
    }
    nxc_printf(compiler,"\tgenerate .data segment \t\t[ OK ]\n");

    ///6.detach and return image,let caller to release it ...
    image = compiler->image;
    compiler->image = 0;

    return image;
}

/**
 * #############################################################################
 * #############################################################################
 * #############################################################################
 */

/**
 * call specific function under given environment ...
 * return value stored in _ctx->eax
 */
long nxc_do_call(nxc_vm_ctx_t*_ctx,nxc_instr_t *xaddr,int argc,void*argv[])
{
    register nxc_instr_t      *eip;
    register long              ebp;
    register long             *esp;
    register long              tmp;
    register long              eax;
    nxc_instr_t                postcall_instr[2];

    ///I/O interface ...
    #define nxc_ld8(ptr)       (long)(*(char* )(ptr))
    #define nxc_ld16(ptr)      (long)(*(short*)(ptr))
    #define nxc_ld32(ptr)      (long)(*(int*  )(ptr))
    #define nxc_ld_l(ptr)      (long)(*(long* )(ptr))
    ///---
    #define nxc_st8(ptr,val)   *(char* )(ptr) = (char )(val)
    #define nxc_st16(ptr,val)  *(short*)(ptr) = (short)(val)
    #define nxc_st32(ptr,val)  *(int*  )(ptr) = (int  )(val)
    #define nxc_st_l(ptr,val)  *(long* )(ptr) = (long )(val)
    ///---wrapper layer---
    #define nxc_regA           (*(long *)(ebp + nxc_instr_get_reg_index1(eip) ))
    #define nxc_regB           (*(long *)(ebp + nxc_instr_get_reg_index2(eip) ))
    #define nxc_f_regA         (*(float *)(ebp + nxc_instr_get_reg_index1(eip)))
    #define nxc_f_regB         (*(float *)(ebp + nxc_instr_get_reg_index2(eip)))
    #define nxc_next_instr()   eip = nxc_instr_get_next(eip)
    #define nxc_push(val)      *--esp = (long)(val)
    #define nxc_pop()          *esp ++
    ///-------------------------------------------------------------------------
    /// initialize global register ...
    esp  = _ctx->esp;               ///stack top
    ebp  = (long)esp;               ///stack base
    eax  = 0;

    ///build a dummy instr ...
    ///the return instruction must be postcall,SEE $nxc_vmop_ret implementation!
    nxc_instr_set_opcode(&postcall_instr[0],nxc_vmop_postcall);
    nxc_instr_set_reg_index1(&postcall_instr[0], -(long)sizeof(long));//alloc a reg!
    nxc_instr_set_immed_l(&postcall_instr[0],argc*sizeof(long)); //balance stack
    nxc_instr_set_opcode(&postcall_instr[1],nxc_vmop_halt);      //Stop Here

    ///push all parameter ...
    for(;argc>0;argc--)
    {
        nxc_push(argv[argc-1]);
    }
    ///push return address
    nxc_push(&postcall_instr[0]);
    ///jmp to target ...
    eip  = xaddr;

    ///skip auto_eip instruction ...
    goto __manual_eip_start_here;
    goto __auto_eip_start_here;
    ///RUN ::: do dispatch instructions ...
    for(;;)
    {
__auto_eip_start_here:
        nxc_next_instr();
__manual_eip_start_here:
        switch (nxc_instr_get_opcode(eip))
        {
        case nxc_vmop_halt:                 ///"halt"
            //_ctx->last_error = 0;
            goto exit_point;
        ///--------------------------assignment opcode--------------------------
        case nxc_vmop_ld8:                  ///load char by ptr ---
            nxc_regA = nxc_ld8( nxc_regA );
            continue;
        case nxc_vmop_ld16:                 ///load short by ptr ---
            nxc_regA = nxc_ld16( nxc_regA );
            continue;
        case nxc_vmop_ld32:                 ///load int by ptr ---
            nxc_regA = nxc_ld32( nxc_regA );
            continue;
        case nxc_vmop_ld_l:                 ///"*" load long by ptr ---
            nxc_regA = nxc_ld_l( nxc_regA );
            continue;
        ///---------------------------------------------------------------------
        case nxc_vmop_ldx8:               ///reg1 = *(char *)(reg1+offset)
            nxc_regA = nxc_ld8(nxc_regA + nxc_instr_get_immed_l(eip));
            continue;
        case nxc_vmop_ldx16:              ///reg1 = *(short *)(reg1+offset)
            nxc_regA = nxc_ld16(nxc_regA + nxc_instr_get_immed_l(eip));
            continue;
        case nxc_vmop_ldx32:              ///reg1 = *(int *)(reg1+offset)
            nxc_regA = nxc_ld32(nxc_regA + nxc_instr_get_immed_l(eip));
            continue;
        case nxc_vmop_ldx_l:              ///reg1 = *(long *)(reg1+offset)
            nxc_regA = nxc_ld_l(nxc_regA + nxc_instr_get_immed_l(eip));
            continue;
        ///---------------------------------------------------------------------
        case nxc_vmop_index8:               ///"[["
            nxc_regA = nxc_ld8(nxc_regA + nxc_regB);
            continue;
        case nxc_vmop_index16:              ///"[[[["
            nxc_regA = nxc_ld16(nxc_regA +(nxc_regB << nxc_short_shift));
            continue;
        case nxc_vmop_index32:              ///"[[["
            nxc_regA = nxc_ld32(nxc_regA +(nxc_regB << nxc_int_shift));
            continue;
        case nxc_vmop_index_l:              ///"["
            nxc_regA = nxc_ld_l(nxc_regA +(nxc_regB << nxc_long_shift));
            continue;
        ///---------------------------------------------------------------------
        case nxc_vmop_ld_lvar:             ///[ebp + var_offset]
            nxc_regA = nxc_ld_l(ebp + nxc_instr_get_immed_l(eip));
            continue;
        case nxc_vmop_ld_lvar_addr:        ///ebp + var_offset
            nxc_regA = ebp + nxc_instr_get_immed_l(eip);
            continue;
        case nxc_vmop_ld_gvar:             ///[dseg + var_offset]
            nxc_regA = nxc_ld_l(nxc_instr_get_immed_l(eip));
            continue;
        case nxc_vmop_ld_gvar_addr:        ///dseg + var_offset
            nxc_regA = nxc_instr_get_immed_l(eip);
            continue;
        case nxc_vmop_ldi:                 ///reg1 = immediate_number
            nxc_regA = nxc_instr_get_immed_l(eip);
            continue;
        case nxc_vmop_addi:                 ///reg1 = reg1 + immediate_number
            nxc_regA += nxc_instr_get_immed_l(eip);
            continue;
        ///---------------------------------------------------------------------
        case nxc_vmop_st8:       ///"="
            tmp = nxc_regB;       ///read regB
            nxc_st8(nxc_regA,tmp);///write mem by regA
            nxc_regA = tmp;       ///write regA with (write)value
            continue;
        case nxc_vmop_add_st8:   ///"+="
            tmp = nxc_ld8(nxc_regA) + nxc_regB;
            nxc_st8(nxc_regA,tmp);
            nxc_regA = tmp;///store rvalue to result reg
            continue;
        case nxc_vmop_sub_st8:   ///"-="
            tmp = nxc_ld8(nxc_regA) - nxc_regB;
            nxc_st8(nxc_regA,tmp);
            nxc_regA = tmp;///store rvalue to result reg
            continue;
        case nxc_vmop_mul_st8:   ///"*="
            tmp = nxc_ld8(nxc_regA) * nxc_regB;
            nxc_st8(nxc_regA,tmp);
            nxc_regA = tmp;///store rvalue to result reg
            continue;
        case nxc_vmop_div_st8:   ///"/="
            tmp = nxc_ld8(nxc_regA) / nxc_regB;
            nxc_st8(nxc_regA,tmp);
            nxc_regA = tmp;///store rvalue to result reg
            continue;
        case nxc_vmop_mod_st8:   ///"%="
            tmp = nxc_ld8(nxc_regA) % nxc_regB;
            nxc_st8(nxc_regA,tmp);
            nxc_regA = tmp;///store rvalue to result reg
            continue;
        case nxc_vmop_bor_st8: ///"|="
            tmp = nxc_ld8(nxc_regA) | nxc_regB;
            nxc_st8(nxc_regA,tmp);
            nxc_regA = tmp;///store rvalue to result reg
            continue;
        case nxc_vmop_band_st8:///"&="
            tmp = nxc_ld8(nxc_regA) & nxc_regB;
            nxc_st8(nxc_regA,tmp);
            nxc_regA = tmp;///store rvalue to result reg
            continue;
        case nxc_vmop_xor_st8:   ///"^="
            tmp = nxc_ld8(nxc_regA) ^ nxc_regB;
            nxc_st8(nxc_regA,tmp);
            nxc_regA = tmp;///store rvalue to result reg
            continue;
        case nxc_vmop_lshift_st8:///"<<="
            tmp = nxc_ld8(nxc_regA) << nxc_regB;
            nxc_st8(nxc_regA,tmp);
            nxc_regA = tmp;///store rvalue to result reg
            continue;
        case nxc_vmop_rshift_st8:///">>="
            tmp = nxc_ld8(nxc_regA) >> nxc_regB;
            nxc_st8(nxc_regA,tmp);
            nxc_regA = tmp;///store rvalue to result reg
            continue;
        ///---------------------------------------------------------------------
        case nxc_vmop_st16:       ///"="
            tmp = nxc_regB;
            nxc_st16(nxc_regA,tmp);
            nxc_regA = tmp;///store rvalue to result reg
            continue;
        case nxc_vmop_add_st16:   ///"+="
            tmp = nxc_ld16(nxc_regA) + nxc_regB;
            nxc_st16(nxc_regA,tmp);
            nxc_regA = tmp;///store rvalue to result reg
            continue;
        case nxc_vmop_sub_st16:   ///"-="
            tmp = nxc_ld16(nxc_regA) - nxc_regB;
            nxc_st16(nxc_regA,tmp);
            nxc_regA = tmp;///store rvalue to result reg
            continue;
        case nxc_vmop_mul_st16:   ///"*="
            tmp = nxc_ld16(nxc_regA) * nxc_regB;
            nxc_st16(nxc_regA,tmp);
            nxc_regA = tmp;///store rvalue to result reg
            continue;
        case nxc_vmop_div_st16:   ///"/="
            tmp = nxc_ld16(nxc_regA) / nxc_regB;
            nxc_st16(nxc_regA,tmp);
            nxc_regA = tmp;///store rvalue to result reg
            continue;
        case nxc_vmop_mod_st16:   ///"%="
            tmp = nxc_ld16(nxc_regA) % nxc_regB;
            nxc_st16(nxc_regA,tmp);
            nxc_regA = tmp;///store rvalue to result reg
            continue;
        case nxc_vmop_bor_st16: ///"|="
            tmp = nxc_ld16(nxc_regA) | nxc_regB;
            nxc_st16(nxc_regA,tmp);
            nxc_regA = tmp;///store rvalue to result reg
            continue;
        case nxc_vmop_band_st16:///"&="
            tmp = nxc_ld16(nxc_regA) & nxc_regB;
            nxc_st16(nxc_regA,tmp);
            nxc_regA = tmp;///store rvalue to result reg
            continue;
        case nxc_vmop_xor_st16:   ///"^="
            tmp = nxc_ld16(nxc_regA) ^ nxc_regB;
            nxc_st16(nxc_regA,tmp);
            nxc_regA = tmp;///store rvalue to result reg
            continue;
        case nxc_vmop_lshift_st16:///"<<="
            tmp = nxc_ld16(nxc_regA) << nxc_regB;
            nxc_st16(nxc_regA,tmp);
            nxc_regA = tmp;///store rvalue to result reg
            continue;
        case nxc_vmop_rshift_st16:///">>="
            tmp = nxc_ld16(nxc_regA) >> nxc_regB;
            nxc_st16(nxc_regA,tmp);
            nxc_regA = tmp;///store rvalue to result reg
            continue;
        ///---------------------------------------------------------------------
        case nxc_vmop_st32:       ///"="
            tmp = nxc_regB;
            nxc_st32(nxc_regA,tmp);
            nxc_regA = tmp;///store rvalue to result reg
            continue;
        case nxc_vmop_add_st32:   ///"+="
            tmp = nxc_ld32(nxc_regA) + nxc_regB;
            nxc_st32(nxc_regA,tmp);
            nxc_regA = tmp;///store rvalue to result reg
            continue;
        case nxc_vmop_sub_st32:   ///"-="
            tmp = nxc_ld32(nxc_regA) - nxc_regB;
            nxc_st32(nxc_regA,tmp);
            nxc_regA = tmp;///store rvalue to result reg
            continue;
        case nxc_vmop_mul_st32:   ///"*="
            tmp = nxc_ld32(nxc_regA) * nxc_regB;
            nxc_st32(nxc_regA,tmp);
            nxc_regA = tmp;///store rvalue to result reg
            continue;
        case nxc_vmop_div_st32:   ///"/="
            tmp = nxc_ld32(nxc_regA) / nxc_regB;
            nxc_st32(nxc_regA,tmp);
            nxc_regA = tmp;///store rvalue to result reg
            continue;
        case nxc_vmop_mod_st32:   ///"%="
            tmp = nxc_ld32(nxc_regA) % nxc_regB;
            nxc_st32(nxc_regA,tmp);
            nxc_regA = tmp;///store rvalue to result reg
            continue;
        case nxc_vmop_bor_st32: ///"|="
            tmp = nxc_ld32(nxc_regA) | nxc_regB;
            nxc_st32(nxc_regA,tmp);
            nxc_regA = tmp;///store rvalue to result reg
            continue;
        case nxc_vmop_band_st32:///"&="
            tmp = nxc_ld32(nxc_regA) & nxc_regB;
            nxc_st32(nxc_regA,tmp);
            nxc_regA = tmp;///store rvalue to result reg
            continue;
        case nxc_vmop_xor_st32:   ///"^="
            tmp = nxc_ld32(nxc_regA) ^ nxc_regB;
            nxc_st32(nxc_regA,tmp);
            nxc_regA = tmp;///store rvalue to result reg
            continue;
        case nxc_vmop_lshift_st32:///"<<="
            tmp = nxc_ld32(nxc_regA) << nxc_regB;
            nxc_st32(nxc_regA,tmp);
            nxc_regA = tmp;///store rvalue to result reg
            continue;
        case nxc_vmop_rshift_st32:///">>="
            tmp = nxc_ld32(nxc_regA) >> nxc_regB;
            nxc_st32(nxc_regA,tmp);
            nxc_regA = tmp;///store rvalue to result reg
            continue;
        ///--------------------------local var acceleration---------------------
        case nxc_vmop_st_lvar:       ///"="
            tmp = nxc_regA;
            nxc_st_l(ebp + nxc_instr_get_immed_l(eip),tmp);
            continue;
        case nxc_vmop_add_st_lvar:   ///"+="
            tmp = nxc_ld_l(ebp + nxc_instr_get_immed_l(eip)); ///load lvar
            tmp += nxc_regA;
            nxc_st_l(ebp + nxc_instr_get_immed_l(eip),tmp);
            nxc_regA = tmp;///store rvalue to result reg
            continue;
        case nxc_vmop_sub_st_lvar:   ///"-="
            tmp = nxc_ld_l(ebp + nxc_instr_get_immed_l(eip)); ///load lvar
            tmp -= nxc_regA;
            nxc_st_l(ebp + nxc_instr_get_immed_l(eip),tmp);
            nxc_regA = tmp;///store rvalue to result reg
            continue;
        case nxc_vmop_mul_st_lvar:   ///"*="
            tmp = nxc_ld_l(ebp + nxc_instr_get_immed_l(eip)); ///load lvar
            tmp *= nxc_regA;
            nxc_st_l(ebp + nxc_instr_get_immed_l(eip),tmp);
            nxc_regA = tmp;///store rvalue to result reg
            continue;
        case nxc_vmop_div_st_lvar:   ///"/="
            tmp = nxc_ld_l(ebp + nxc_instr_get_immed_l(eip)); ///load lvar
            tmp /= nxc_regA;
            nxc_st_l(ebp + nxc_instr_get_immed_l(eip),tmp);
            nxc_regA = tmp;///store rvalue to result reg
            continue;
        case nxc_vmop_mod_st_lvar:   ///"%="
            tmp = nxc_ld_l(ebp + nxc_instr_get_immed_l(eip)); ///load lvar
            tmp %= nxc_regA;
            nxc_st_l(ebp + nxc_instr_get_immed_l(eip),tmp);
            nxc_regA = tmp;///store rvalue to result reg
            continue;
        case nxc_vmop_bor_st_lvar:   ///"|="
            tmp = nxc_ld_l(ebp + nxc_instr_get_immed_l(eip)); ///load lvar
            tmp |= nxc_regA;
            nxc_st_l(ebp + nxc_instr_get_immed_l(eip),tmp);
            nxc_regA = tmp;///store rvalue to result reg
            continue;
        case nxc_vmop_band_st_lvar:   ///"&="
            tmp = nxc_ld_l(ebp + nxc_instr_get_immed_l(eip)); ///load lvar
            tmp &= nxc_regA;
            nxc_st_l(ebp + nxc_instr_get_immed_l(eip),tmp);
            nxc_regA = tmp;///store rvalue to result reg
            continue;
        case nxc_vmop_xor_st_lvar:   ///"^="
            tmp = nxc_ld_l(ebp + nxc_instr_get_immed_l(eip)); ///load lvar
            tmp ^= nxc_regA;
            nxc_st_l(ebp + nxc_instr_get_immed_l(eip),tmp);
            nxc_regA = tmp;///store rvalue to result reg
            continue;
        case nxc_vmop_lshift_st_lvar:   ///"<<="
            tmp = nxc_ld_l(ebp + nxc_instr_get_immed_l(eip)); ///load lvar
            tmp <<= nxc_regA;
            nxc_st_l(ebp + nxc_instr_get_immed_l(eip),tmp);
            nxc_regA = tmp;///store rvalue to result reg
            continue;
        case nxc_vmop_rshift_st_lvar:   ///">>="
            tmp = nxc_ld_l(ebp + nxc_instr_get_immed_l(eip)); ///load lvar
            tmp >>= nxc_regA;
            nxc_st_l(ebp + nxc_instr_get_immed_l(eip),tmp);
            nxc_regA = tmp;///store rvalue to result reg
            continue;
        ///---------------------------------------------------------------------
        case nxc_vmop_st_l:       ///"="
            tmp = nxc_regB;
            nxc_st_l(nxc_regA,tmp);
            nxc_regA = tmp;///store rvalue to result reg
            continue;
        case nxc_vmop_add_st_l:   ///"+="
            tmp = nxc_ld_l(nxc_regA) + nxc_regB;
            nxc_st_l(nxc_regA,tmp);
            nxc_regA = tmp;///store rvalue to result reg
            continue;
        case nxc_vmop_sub_st_l:   ///"-="
            tmp = nxc_ld_l(nxc_regA) - nxc_regB;
            nxc_st_l(nxc_regA,tmp);
            nxc_regA = tmp;///store rvalue to result reg
            continue;
        case nxc_vmop_mul_st_l:   ///"*="
            tmp = nxc_ld_l(nxc_regA) * nxc_regB;
            nxc_st_l(nxc_regA,tmp);
            nxc_regA = tmp;///store rvalue to result reg
            continue;
        case nxc_vmop_div_st_l:   ///"/="
            tmp = nxc_ld_l(nxc_regA) / nxc_regB;
            nxc_st_l(nxc_regA,tmp);
            nxc_regA = tmp;///store rvalue to result reg
            continue;
        case nxc_vmop_mod_st_l:   ///"%="
            tmp = nxc_ld_l(nxc_regA) % nxc_regB;
            nxc_st_l(nxc_regA,tmp);
            nxc_regA = tmp;///store rvalue to result reg
            continue;
        case nxc_vmop_bor_st_l: ///"|="
            tmp = nxc_ld_l(nxc_regA) | nxc_regB;
            nxc_st_l(nxc_regA,tmp);
            nxc_regA = tmp;///store rvalue to result reg
            continue;
        case nxc_vmop_band_st_l:///"&="
            tmp = nxc_ld_l(nxc_regA) & nxc_regB;
            nxc_st_l(nxc_regA,tmp);
            nxc_regA = tmp;///store rvalue to result reg
            continue;
        case nxc_vmop_xor_st_l:   ///"^="
            tmp = nxc_ld_l(nxc_regA) ^ nxc_regB;
            nxc_st_l(nxc_regA,tmp);
            nxc_regA = tmp;///store rvalue to result reg
            continue;
        case nxc_vmop_lshift_st_l:///"<<="
            tmp = nxc_ld_l(nxc_regA) << nxc_regB;
            nxc_st_l(nxc_regA,tmp);
            nxc_regA = tmp;///store rvalue to result reg
            continue;
        case nxc_vmop_rshift_st_l:///">>="
            tmp = nxc_ld_l(nxc_regA) >> nxc_regB;
            nxc_st_l(nxc_regA,tmp);
            nxc_regA = tmp;///store rvalue to result reg
            continue;

        case nxc_vmop_inc:                  ///"++"
            tmp = nxc_ld_l(nxc_regA) + 1;
            nxc_st_l(nxc_regA,tmp);
            nxc_regA = tmp;///store rvalue to result reg
            continue;
        case nxc_vmop_dec:                  ///"--"
            tmp = nxc_ld_l(nxc_regA) + 1;
            nxc_st_l(nxc_regA,tmp);
            nxc_regA = tmp;///store rvalue to result reg
            continue;
        ///----------------------------binary opcode----------------------------
        case nxc_vmop_or:  nxc_regA = ( nxc_regA || nxc_regB ); continue;///"||"
        case nxc_vmop_and: nxc_regA = ( nxc_regA && nxc_regB ); continue;///"&&"
        case nxc_vmop_bor: nxc_regA |= nxc_regB ;               continue;///"|"
        case nxc_vmop_band:nxc_regA &= nxc_regB ;               continue;///"&"
        case nxc_vmop_xor: nxc_regA ^= nxc_regB ;               continue;///"^"
        case nxc_vmop_eq:  nxc_regA = ( nxc_regA == nxc_regB ); continue;///"=="
        case nxc_vmop_uneq:nxc_regA = ( nxc_regA != nxc_regB ); continue;///"!="
        case nxc_vmop_gt:  nxc_regA = ( nxc_regA > nxc_regB );  continue;///">"
        case nxc_vmop_lt:  nxc_regA = ( nxc_regA < nxc_regB );  continue;///"<"
        case nxc_vmop_ge:  nxc_regA = ( nxc_regA >= nxc_regB ); continue;///">="
        case nxc_vmop_le:  nxc_regA = ( nxc_regA <= nxc_regB ); continue;///"<="
        case nxc_vmop_lshift: nxc_regA <<= nxc_regB ;           continue;///"<<"
        case nxc_vmop_rshift: nxc_regA >>= nxc_regB ;           continue;///">>"
        case nxc_vmop_add:    nxc_regA += nxc_regB ;            continue;///"+"
        case nxc_vmop_sub:    nxc_regA -= nxc_regB ;            continue;///"-"
        case nxc_vmop_mul:    nxc_regA *= nxc_regB ;            continue;///"*"
        case nxc_vmop_div:    nxc_regA /= nxc_regB ;            continue;///"/"
        case nxc_vmop_mod:    nxc_regA %= nxc_regB ;            continue;///"%"
        
        ///---------------------------------------------------------------------
        case nxc_vmop_or_i:                   ///"||"
            nxc_regA = ( nxc_regA || nxc_instr_get_immed_l(eip) );
            continue;
        case nxc_vmop_and_i:                  ///"&&"
            nxc_regA = ( nxc_regA && nxc_instr_get_immed_l(eip) );
            continue;
        case nxc_vmop_bor_i:                  ///"|"
            nxc_regA |= nxc_instr_get_immed_l(eip) ;
            continue;
        case nxc_vmop_band_i:                 ///"&"
            nxc_regA &= nxc_instr_get_immed_l(eip) ;
            continue;
        case nxc_vmop_xor_i:                  ///"^"
            nxc_regA ^= nxc_instr_get_immed_l(eip) ;
            continue;
        case nxc_vmop_eq_i:                   ///"=="
            nxc_regA = ( nxc_regA == nxc_instr_get_immed_l(eip) );
            continue;
        case nxc_vmop_uneq_i:                 ///"!="
            nxc_regA = ( nxc_regA != nxc_instr_get_immed_l(eip) );
            continue;
        case nxc_vmop_gt_i:                   ///">"
            nxc_regA = ( nxc_regA > nxc_instr_get_immed_l(eip) );
            continue;
        case nxc_vmop_lt_i:                   ///"<"
            nxc_regA = ( nxc_regA < nxc_instr_get_immed_l(eip) );
            continue;
        case nxc_vmop_ge_i:                   ///">="
            nxc_regA = ( nxc_regA >= nxc_instr_get_immed_l(eip) );
            continue;
        case nxc_vmop_le_i:                   ///"<="
            nxc_regA = ( nxc_regA <= nxc_instr_get_immed_l(eip) );
            continue;
        case nxc_vmop_lshift_i:               ///"<<"
            nxc_regA <<= nxc_instr_get_immed_l(eip) ;
            continue;
        case nxc_vmop_rshift_i:               ///">>"
            nxc_regA >>= nxc_instr_get_immed_l(eip) ;
            continue;
        case nxc_vmop_add_i:                  ///"+"
            nxc_regA += nxc_instr_get_immed_l(eip) ;
            continue;
        case nxc_vmop_sub_i:                  ///"-"
            nxc_regA -= nxc_instr_get_immed_l(eip) ;
            continue;
        case nxc_vmop_mul_i:                  ///"*"
            nxc_regA *= nxc_instr_get_immed_l(eip) ;
            continue;
        case nxc_vmop_div_i:                  ///"/"
            nxc_regA /= nxc_instr_get_immed_l(eip) ;
            continue;
        case nxc_vmop_mod_i:                  ///"%"
            nxc_regA %= nxc_instr_get_immed_l(eip) ;
            continue;

        case nxc_vmop_or_i1:   ///"||"
            nxc_regA = ( nxc_regA || nxc_instr_get_immed_l(eip));
            continue;
        case nxc_vmop_and_i1:   ///"&&"
            nxc_regA = ( nxc_regA && nxc_instr_get_immed_l(eip));
            continue;
        case nxc_vmop_bor_i1:   ///"|"
            nxc_regA |= nxc_instr_get_immed_l(eip) ;
            continue;
        case nxc_vmop_band_i1:  ///"&"
            nxc_regA &= nxc_instr_get_immed_l(eip) ;
            continue;
        case nxc_vmop_xor_i1:   ///"^"
            nxc_regA ^= nxc_instr_get_immed_l(eip) ;
            continue;
        case nxc_vmop_eq_i1:    ///"=="
            nxc_regA = ( nxc_regA == nxc_instr_get_immed_l(eip) ); 
            continue;
        case nxc_vmop_uneq_i1:  ///"!="
            nxc_regA = ( nxc_regA != nxc_instr_get_immed_l(eip) ); 
            continue;
        case nxc_vmop_gt_i1:    ///">"
            nxc_regA = ( nxc_instr_get_immed_l(eip) > nxc_regA );  
            continue;
        case nxc_vmop_lt_i1:    ///"<"
            nxc_regA = ( nxc_instr_get_immed_l(eip) < nxc_regA );  
            continue;
        case nxc_vmop_ge_i1:    ///">="
            nxc_regA = ( nxc_instr_get_immed_l(eip) >= nxc_regA ); 
            continue;
        case nxc_vmop_le_i1:    ///"<="
            nxc_regA = ( nxc_instr_get_immed_l(eip) <= nxc_regA ); 
            continue;
        case nxc_vmop_lshift_i1:///"<<" 
            nxc_regA = ( nxc_instr_get_immed_l(eip) << nxc_regA ); 
            continue;
        case nxc_vmop_rshift_i1:///">>"
            nxc_regA = ( nxc_instr_get_immed_l(eip) >> nxc_regA ); 
            continue;
        case nxc_vmop_add_i1:   ///"+"
            nxc_regA +=  nxc_instr_get_immed_l(eip) ; 
            continue;
        case nxc_vmop_sub_i1:   ///"-"
            nxc_regA = ( nxc_instr_get_immed_l(eip) - nxc_regA ); 
            continue;
        case nxc_vmop_mul_i1:   ///"*"
            nxc_regA *= nxc_instr_get_immed_l(eip); 
            continue;
        case nxc_vmop_div_i1:   ///"/"
            nxc_regA = ( nxc_instr_get_immed_l(eip) / nxc_regA ); 
            continue;
        case nxc_vmop_mod_i1:   ///"%"
            nxc_regA = ( nxc_instr_get_immed_l(eip) % nxc_regA ); 
            continue;
        ///--------------------------float opcode-------------------------------
        case nxc_vmop_f_gt: nxc_regA = (nxc_f_regA > nxc_f_regB );continue;//>
        case nxc_vmop_f_lt: nxc_regA = (nxc_f_regA < nxc_f_regB );continue;//<
        case nxc_vmop_f_ge: nxc_regA = (nxc_f_regA >= nxc_f_regB);continue;//>=
        case nxc_vmop_f_le: nxc_regA = (nxc_f_regA <= nxc_f_regB);continue;//<=
        case nxc_vmop_f_add:nxc_f_regA +=  nxc_f_regB ;continue; ///":+"
        case nxc_vmop_f_sub:nxc_f_regA -=  nxc_f_regB ;continue; ///":-"
        case nxc_vmop_f_mul:nxc_f_regA *=  nxc_f_regB ;continue; ///":*"
        case nxc_vmop_f_div:nxc_f_regA /=  nxc_f_regB ;continue; ///":/"
        case nxc_vmop_f_neg:nxc_f_regA  = -nxc_f_regA ;continue; ///":-"
        ///---------------------------------------------------------------------
        case nxc_vmop_f_gt_i:                 ///":>"
            nxc_regA = ( nxc_f_regA > nxc_instr_get_immed_f(eip) );
            continue;
        case nxc_vmop_f_lt_i:                 ///":<"
            nxc_regA = ( nxc_f_regA < nxc_instr_get_immed_f(eip) );
            continue;
        case nxc_vmop_f_ge_i:                 ///":>="
            nxc_regA = ( nxc_f_regA >= nxc_instr_get_immed_f(eip) );
            continue;
        case nxc_vmop_f_le_i:                 ///":<="
            nxc_regA = ( nxc_f_regA <= nxc_instr_get_immed_f(eip) );
            continue;
        case nxc_vmop_f_add_i:                ///":+"
            nxc_f_regA += nxc_instr_get_immed_f(eip) ;
            continue;
        case nxc_vmop_f_sub_i:                ///":-"
            nxc_f_regA -= nxc_instr_get_immed_f(eip) ;
            continue;
        case nxc_vmop_f_mul_i:                ///":*"
            nxc_f_regA *= nxc_instr_get_immed_f(eip) ;
            continue;
        case nxc_vmop_f_div_i:                ///":/"
            nxc_f_regA /= nxc_instr_get_immed_f(eip) ;
            continue;

        ///---------------------------------------------------------------------
        case nxc_vmop_f_gt_i1:                 ///":>"
            nxc_regA = (nxc_instr_get_immed_f(eip) > nxc_f_regA);
            continue;
        case nxc_vmop_f_lt_i1:                 ///":<"
            nxc_regA = (nxc_instr_get_immed_f(eip) < nxc_f_regA);
            continue;
        case nxc_vmop_f_ge_i1:                 ///":>="
            nxc_regA = (nxc_instr_get_immed_f(eip) >= nxc_f_regA);
            continue;
        case nxc_vmop_f_le_i1:                 ///":<="
            nxc_regA = (nxc_instr_get_immed_f(eip) <= nxc_f_regA);
            continue;
        case nxc_vmop_f_add_i1:                ///":+"
            nxc_f_regA += nxc_instr_get_immed_f(eip) ;
            continue;
        case nxc_vmop_f_sub_i1:                ///":-"
            nxc_f_regA = nxc_instr_get_immed_f(eip) - nxc_f_regA;
            continue;
        case nxc_vmop_f_mul_i1:                ///":*"
            nxc_f_regA *= nxc_instr_get_immed_f(eip) ;
            continue;
        case nxc_vmop_f_div_i1:                ///":/"
            nxc_f_regA = nxc_instr_get_immed_f(eip) / nxc_f_regA;
            continue;
        ///---------------------------unary opcode------------------------------
        case nxc_vmop_cast:                 ///"cast"
            goto exit_point;
        case nxc_vmop_positive:             ///"+"
            continue;
        case nxc_vmop_negative:             ///"-"
            nxc_regA = -nxc_regA ;
            continue;
        case nxc_vmop_compensation:         ///"~"
            nxc_regA = ~nxc_regA ;
            continue;
        case nxc_vmop_not:                  ///"!"
            nxc_regA = !nxc_regA ;
            continue;
        ///---------------------------------------------------------------------
        ///---for lvalue--
        case nxc_vmop_addr_array8:          ///"&["    left + index
            nxc_regA = nxc_regA + nxc_regB;
            continue;
        case nxc_vmop_addr_array16:         ///"&[[[" left + index*sizeof(short)
            nxc_regA = nxc_regA + (nxc_regB << nxc_short_shift);
            continue;
        case nxc_vmop_addr_array32:         ///"&[["   left + index*sizeof(int)
            nxc_regA = nxc_regA + (nxc_regB << nxc_int_shift);
            continue;
        case nxc_vmop_addr_array_l:         ///"&.[" left + index*sizeof(long)
            nxc_regA = nxc_regA + (nxc_regB << nxc_long_shift);
            continue;

        ///---------------------------flow-control------------------------------
        case nxc_vmop_call:
            nxc_push(nxc_instr_get_next(eip));         ///push ret_addr
            eip = (nxc_instr_t *)nxc_regA;         ///jmp target func ...
            goto __manual_eip_start_here;          ///Manual EIP Position
        case nxc_vmop_postcall:
            ///mov $reg,$eax
            nxc_regA = eax;
            ///add $esp,$param_size
            esp = (long *)((long)esp + nxc_instr_get_immed_l(eip));
            continue;
        case nxc_vmop_trap:                 ///"trap    xaddr"
            ///[0].pre calc next eip
            _ctx->eip = (nxc_instr_t *)nxc_pop();  ///pop return_address
            ///[1].store context ...
            _ctx->esp = esp;
            _ctx->ebp = ebp;
            ///[2].call trap handler (of current instruction)...
            //eip->u.trap_func(_ctx);
            eax = (long)nxc_instr_get_trap_func(eip)(_ctx);
            ///[3].update context ...
            esp = _ctx->esp;
            eip = _ctx->eip;///>>>>>>>>>>>>Move to next Instr!!!<<<<<<<<<<<<<<<<
            ///trnna pre-execute POST_CALL!!!
            //if(likely(nxc_instr_get_opcode(eip) == nxc_vmop_postcall))
            {
                ///mov $reg,$eax
                nxc_regA = eax;
                ///add $esp,$param_size
                esp = (long *)((long)esp + nxc_instr_get_immed_l(eip) );
            }
            continue;
        case nxc_vmop_fast_call:            ///"fastcall $reg"
            ///[0].pre calc next eip
            _ctx->eip = nxc_instr_get_next(eip);
            ///[1].store context ...
            _ctx->esp = esp;
            _ctx->ebp = ebp;
            ///[2].call trap handler ...
            ///regA = target function's trap_instruction's addrss ...
            eax = (long)nxc_instr_get_trap_func(((nxc_instr_t *)nxc_regA)) (_ctx);
            
            ///[3].update context ...
            esp = _ctx->esp;
            eip = _ctx->eip;///>>>>>>>>>>>>Move to next Instr!!!<<<<<<<<<<<<<<<<
            ///trnna pre-execute POST_CALL!!!
            //if(likely( nxc_instr_get_opcode(eip) == nxc_vmop_postcall ))
            {
                ///mov $reg,$eax
                nxc_regA = eax;
                ///add $esp,$param_size
                esp = (long *)((long)esp + nxc_instr_get_immed_l(eip)); 
            }
            continue;
        case nxc_vmop_jmp:                  ///"jmp     $imme_addr"
            eip = (nxc_instr_t *)((long)eip + nxc_instr_get_jmp_offset(eip));
            goto __manual_eip_start_here;
        case nxc_vmop_jz:
            if(!nxc_regA)
            {
                eip = (nxc_instr_t *)((long)eip + nxc_instr_get_jmp_offset(eip));
                goto __manual_eip_start_here;
            }
            continue;
        case nxc_vmop_jnz:
            if(nxc_regA)
            {
                eip = (nxc_instr_t *)((long)eip + nxc_instr_get_jmp_offset(eip));
                goto __manual_eip_start_here;
            }
            continue;
        case nxc_vmop_return_val:
            eax = nxc_regA;                ///mov eax,$result
            esp = (long*)ebp;              ///mov esp,ebp
            ebp = nxc_pop();               ///pop ebp
            eip = (nxc_instr_t *)nxc_pop();///pop eip>>>>>Manual st EIP<<<<<
            //!!!trnna pre execute!!!
            //!if(likely(eip->opcode == nxc_vmop_postcall))
            {
                nxc_regA = eax;            ///mov $reg,$eax
                ///add $esp,$param_sz
                esp = (long *)((long)esp + nxc_instr_get_immed_l(eip));
                continue;
            }
            //else{
            //! goto __manual_eip_start_here;
            //!}
        case nxc_vmop_ret:
            ///~~~eax = 0;                 ///mov eax,0
            esp = (long*)ebp;              ///mov esp,ebp
            ebp = nxc_pop();               ///pop ebp
            eip = (nxc_instr_t *)nxc_pop();///pop eip
            ///trnna pre execute POST_CALL!!!
            //if(likely(eip->opcode == nxc_vmop_postcall))
            {
                ///mov $reg,$eax
                ///~~~nxc_regA = 0;
                ///add $esp,$param_sz
                esp = (long *)((long)esp + nxc_instr_get_immed_l(eip));
                continue;
            }
            //else{
            //! goto __manual_eip_start_here;
            //!}
        case nxc_vmop_push:
            nxc_push(nxc_regA);
            continue;
		case nxc_vmop_pushi:
            nxc_push(nxc_instr_get_immed_l(eip));
            continue;
        case nxc_vmop_pop:
            nxc_pop();
            continue;
        case nxc_vmop_enter:
            nxc_push(ebp);///push ebp
            ebp = (long)esp;///mov ebp,esp
            ///sub esp,$lstack_size
            esp = (long *)((long)esp - nxc_instr_get_immed_l(eip));
            continue;
        case nxc_vmop_leave:
            ///unused instruction ...
            continue;
        case nxc_vmop_nop:                  ///"nop"
            continue;
        default:
            _ctx->last_error = -1;
            goto exit_point;
        }
    }
exit_point:
    if(!eip)
    {
        return -1;
    }
    ///store the result ...
    _ctx->eax = eax;

    return eax;
}
