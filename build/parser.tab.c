/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 1 "src/parser.y"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <math.h>

/* ---- Type system ---- */
typedef enum {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_CHAR,
    TYPE_STRING,
    TYPE_VOID,
    TYPE_UNKNOWN
} VarType;

static const char *type_name(VarType t) {
    switch (t) {
        case TYPE_INT:    return "int";
        case TYPE_FLOAT:  return "float";
        case TYPE_CHAR:   return "char";
        case TYPE_STRING: return "string";
        case TYPE_VOID:   return "void";
        default:          return "unknown";
    }
}

static const char *c_type_name(VarType t) {
    switch (t) {
        case TYPE_INT:    return "long long";
        case TYPE_FLOAT:  return "double";
        case TYPE_CHAR:   return "char";
        case TYPE_STRING: return "const char*";
        default:          return "long long";
    }
}

/* ---- Forward typedefs ---- */
#ifndef MAMA_TYPES_DEFINED
#define MAMA_TYPES_DEFINED
typedef struct Expr Expr;
typedef struct ArgList ArgList;
typedef struct ParamList ParamList;
#endif

/* ---- Expression node ---- */
struct Expr {
    char *code;
    int is_const;
    long long const_val;
    double const_fval;
    VarType type;
};

/* ---- Symbol table ---- */
typedef struct Symbol {
    char *name;
    VarType type;
    struct Symbol *next;
} Symbol;

/* ---- Function table ---- */
typedef struct FuncDef {
    char *name;
    VarType return_type;
    int param_count;
    char **param_names;
    VarType *param_types;
    struct FuncDef *next;
} FuncDef;

/* ---- Scope stack for local variables ---- */
typedef struct Scope {
    Symbol *locals;
    struct Scope *parent;
} Scope;

static Scope *current_scope = NULL;
static FuncDef *functab = NULL;
static FILE *out_file = NULL;
static int error_count = 0;
static int got_start = 0;

/* Buffer for function forward declarations and definitions */
#define FUNC_BUF_SIZE (64 * 1024)
static char func_defs[FUNC_BUF_SIZE];
static int func_defs_len = 0;
static char func_fwd[FUNC_BUF_SIZE];
static int func_fwd_len = 0;

int yylex(void);
void yyerror(const char *s);
extern int yylineno;
extern FILE *yyin;

/* ---- Helpers ---- */
static char *fmt(const char *pattern, ...) {
    va_list args;
    va_start(args, pattern);
    int needed = vsnprintf(NULL, 0, pattern, args);
    va_end(args);

    char *buf = (char *)malloc((size_t)needed + 1);
    if (!buf) { fprintf(stderr, "Out of memory\n"); exit(1); }

    va_start(args, pattern);
    vsnprintf(buf, (size_t)needed + 1, pattern, args);
    va_end(args);
    return buf;
}

static char *str_dup(const char *s) {
    size_t n = strlen(s) + 1;
    char *out = (char *)malloc(n);
    if (!out) { fprintf(stderr, "Out of memory\n"); exit(1); }
    memcpy(out, s, n);
    return out;
}

static void append_buf(char *buf, int *len, int cap, const char *text) {
    int tlen = (int)strlen(text);
    if (*len + tlen < cap) {
        memcpy(buf + *len, text, (size_t)tlen);
        *len += tlen;
        buf[*len] = '\0';
    }
}

/* ---- Scope management ---- */
static void push_scope(void) {
    Scope *s = (Scope *)malloc(sizeof(Scope));
    s->locals = NULL;
    s->parent = current_scope;
    current_scope = s;
}

static void pop_scope(void) {
    if (!current_scope) return;
    /* Free locals */
    Symbol *cur = current_scope->locals;
    while (cur) {
        Symbol *tmp = cur;
        cur = cur->next;
        free(tmp->name);
        free(tmp);
    }
    Scope *old = current_scope;
    current_scope = current_scope->parent;
    free(old);
}

static int symbol_exists(const char *name) {
    Scope *s = current_scope;
    while (s) {
        Symbol *cur = s->locals;
        while (cur) {
            if (strcmp(cur->name, name) == 0) return 1;
            cur = cur->next;
        }
        s = s->parent;
    }
    return 0;
}

static VarType symbol_type(const char *name) {
    Scope *s = current_scope;
    while (s) {
        Symbol *cur = s->locals;
        while (cur) {
            if (strcmp(cur->name, name) == 0) return cur->type;
            cur = cur->next;
        }
        s = s->parent;
    }
    return TYPE_UNKNOWN;
}

static void add_symbol(const char *name, VarType type) {
    /* Check only current scope for duplicates */
    Symbol *cur = current_scope->locals;
    while (cur) {
        if (strcmp(cur->name, name) == 0) {
            fprintf(stderr, "[Line %d] Duplicate declaration: %s\n", yylineno, name);
            error_count++;
            return;
        }
        cur = cur->next;
    }
    Symbol *n = (Symbol *)malloc(sizeof(Symbol));
    n->name = str_dup(name);
    n->type = type;
    n->next = current_scope->locals;
    current_scope->locals = n;
}

static void must_be_declared(const char *name) {
    if (!symbol_exists(name)) {
        fprintf(stderr, "[Line %d] Mama, ei variable ke ami chini na: %s\n", yylineno, name);
        error_count++;
    }
}

/* ---- Function table ---- */
static void add_func(const char *name, VarType ret, int pc, char **pnames, VarType *ptypes) {
    FuncDef *f = (FuncDef *)malloc(sizeof(FuncDef));
    f->name = str_dup(name);
    f->return_type = ret;
    f->param_count = pc;
    f->param_names = pnames;
    f->param_types = ptypes;
    f->next = functab;
    functab = f;
}

static FuncDef *find_func(const char *name) {
    FuncDef *f = functab;
    while (f) {
        if (strcmp(f->name, name) == 0) return f;
        f = f->next;
    }
    return NULL;
}

/* ---- Type inference and checking ---- */
static VarType infer_literal_type(const char *lit) {
    if (!lit || !lit[0]) return TYPE_INT;
    if (lit[0] == '\'') return TYPE_CHAR;
    if (lit[0] == '"') return TYPE_STRING;
    /* Check for float */
    for (const char *p = lit; *p; p++) {
        if (*p == '.') return TYPE_FLOAT;
    }
    return TYPE_INT;
}

/* Returns the wider of two numeric types for implicit conversion */
static VarType promote(VarType a, VarType b) {
    if (a == TYPE_STRING || b == TYPE_STRING) return TYPE_STRING;
    if (a == TYPE_FLOAT || b == TYPE_FLOAT) return TYPE_FLOAT;
    if (a == TYPE_INT || b == TYPE_INT) return TYPE_INT;
    if (a == TYPE_CHAR && b == TYPE_CHAR) return TYPE_CHAR;
    return TYPE_INT;
}

/* Emit implicit conversion warning */
static void check_implicit_conversion(VarType from, VarType to, int line) {
    if (from == to || from == TYPE_UNKNOWN || to == TYPE_UNKNOWN) return;
    /* char -> int is silent promotion */
    if (from == TYPE_CHAR && to == TYPE_INT) return;
    /* int -> float is silent promotion */
    if (from == TYPE_INT && to == TYPE_FLOAT) return;
    if (from == TYPE_CHAR && to == TYPE_FLOAT) return;
    /* float -> int loses precision */
    if (from == TYPE_FLOAT && to == TYPE_INT) {
        fprintf(stderr, "[Line %d] Warning: implicit conversion from float to int (precision loss)\n", line);
    }
    /* string cannot convert to numeric */
    if (from == TYPE_STRING && (to == TYPE_INT || to == TYPE_FLOAT || to == TYPE_CHAR)) {
        fprintf(stderr, "[Line %d] Type error: cannot convert string to %s\n", line, type_name(to));
        error_count++;
    }
    if ((from == TYPE_INT || from == TYPE_FLOAT || from == TYPE_CHAR) && to == TYPE_STRING) {
        fprintf(stderr, "[Line %d] Type error: cannot convert %s to string\n", line, type_name(from));
        error_count++;
    }
}

/* Wrap expression code in a cast if needed */
static char *cast_code(const char *code, VarType from, VarType to) {
    if (from == to) return str_dup(code);
    if (from == TYPE_CHAR && (to == TYPE_INT || to == TYPE_FLOAT))
        return fmt("((%s)%s)", c_type_name(to), code);
    if (from == TYPE_INT && to == TYPE_FLOAT)
        return fmt("((double)%s)", code);
    if (from == TYPE_FLOAT && to == TYPE_INT)
        return fmt("((long long)%s)", code);
    return str_dup(code);
}

/* ---- Expression constructors ---- */
static Expr *make_expr_code(char *code, VarType type) {
    Expr *e = (Expr *)malloc(sizeof(Expr));
    e->code = code;
    e->is_const = 0;
    e->const_val = 0;
    e->const_fval = 0.0;
    e->type = type;
    return e;
}

static Expr *make_expr_const_int(long long value) {
    Expr *e = (Expr *)malloc(sizeof(Expr));
    e->code = fmt("%lld", value);
    e->is_const = 1;
    e->const_val = value;
    e->const_fval = (double)value;
    e->type = TYPE_INT;
    return e;
}

static Expr *make_expr_const_float(double value) {
    Expr *e = (Expr *)malloc(sizeof(Expr));
    e->code = fmt("%.6f", value);
    e->is_const = 1;
    e->const_val = (long long)value;
    e->const_fval = value;
    e->type = TYPE_FLOAT;
    return e;
}

static Expr *combine_binary(const char *op, Expr *a, Expr *b) {
    VarType result_type = promote(a->type, b->type);

    /* Type check: no arithmetic on strings */
    if (a->type == TYPE_STRING || b->type == TYPE_STRING) {
        fprintf(stderr, "[Line %d] Type error: cannot perform '%s' on string values\n", yylineno, op);
        error_count++;
        return make_expr_const_int(0);
    }

    /* Constant folding */
    if (a->is_const && b->is_const) {
        if (result_type == TYPE_FLOAT || a->type == TYPE_FLOAT || b->type == TYPE_FLOAT) {
            double av = a->const_fval, bv = b->const_fval;
            if (strcmp(op, "+") == 0) return make_expr_const_float(av + bv);
            if (strcmp(op, "-") == 0) return make_expr_const_float(av - bv);
            if (strcmp(op, "*") == 0) return make_expr_const_float(av * bv);
            if (strcmp(op, "/") == 0) {
                if (bv == 0.0) { fprintf(stderr, "[Line %d] Division by zero in constant expression.\n", yylineno); error_count++; return make_expr_const_float(0.0); }
                return make_expr_const_float(av / bv);
            }
            /* Comparisons return int */
            if (strcmp(op, "<") == 0) return make_expr_const_int(av < bv);
            if (strcmp(op, ">") == 0) return make_expr_const_int(av > bv);
            if (strcmp(op, "<=") == 0) return make_expr_const_int(av <= bv);
            if (strcmp(op, ">=") == 0) return make_expr_const_int(av >= bv);
            if (strcmp(op, "==") == 0) return make_expr_const_int(av == bv);
            if (strcmp(op, "!=") == 0) return make_expr_const_int(av != bv);
        } else {
            long long av = a->const_val, bv = b->const_val;
            if (strcmp(op, "+") == 0) return make_expr_const_int(av + bv);
            if (strcmp(op, "-") == 0) return make_expr_const_int(av - bv);
            if (strcmp(op, "*") == 0) return make_expr_const_int(av * bv);
            if (strcmp(op, "/") == 0) {
                if (bv == 0) { fprintf(stderr, "[Line %d] Division by zero in constant expression.\n", yylineno); error_count++; return make_expr_const_int(0); }
                return make_expr_const_int(av / bv);
            }
            if (strcmp(op, "%") == 0) {
                if (bv == 0) { fprintf(stderr, "[Line %d] Modulo by zero in constant expression.\n", yylineno); error_count++; return make_expr_const_int(0); }
                return make_expr_const_int(av % bv);
            }
            if (strcmp(op, "<") == 0) return make_expr_const_int(av < bv);
            if (strcmp(op, ">") == 0) return make_expr_const_int(av > bv);
            if (strcmp(op, "<=") == 0) return make_expr_const_int(av <= bv);
            if (strcmp(op, ">=") == 0) return make_expr_const_int(av >= bv);
            if (strcmp(op, "==") == 0) return make_expr_const_int(av == bv);
            if (strcmp(op, "!=") == 0) return make_expr_const_int(av != bv);
        }
    }

    /* Non-constant: emit with implicit cast if needed */
    char *ac = cast_code(a->code, a->type, result_type);
    char *bc = cast_code(b->code, b->type, result_type);

    /* Comparison operators return int */
    VarType out_type = result_type;
    if (strcmp(op, "<") == 0 || strcmp(op, ">") == 0 ||
        strcmp(op, "<=") == 0 || strcmp(op, ">=") == 0 ||
        strcmp(op, "==") == 0 || strcmp(op, "!=") == 0) {
        out_type = TYPE_INT;
    }

    return make_expr_code(fmt("(%s %s %s)", ac, op, bc), out_type);
}

static long long char_to_int(const char *lit) {
    size_t len = strlen(lit);
    if (len >= 3 && lit[0] == '\'' && lit[len - 1] == '\'') {
        if (len == 3) return (unsigned char)lit[1];
        if (len == 4 && lit[1] == '\\') {
            switch (lit[2]) {
                case 'n': return '\n';
                case 't': return '\t';
                case '\\': return '\\';
                case '\'': return '\'';
                case '0': return '\0';
                default: return (unsigned char)lit[2];
            }
        }
    }
    return 0;
}

/* ---- Argument list helpers ---- */
typedef struct ArgList {
    int count;
    int cap;
    Expr **exprs;
} ArgList;

static ArgList *new_arglist(void) {
    ArgList *al = (ArgList *)malloc(sizeof(ArgList));
    al->count = 0;
    al->cap = 4;
    al->exprs = (Expr **)malloc(sizeof(Expr *) * (size_t)al->cap);
    return al;
}

static void arglist_push(ArgList *al, Expr *e) {
    if (al->count >= al->cap) {
        al->cap *= 2;
        al->exprs = (Expr **)realloc(al->exprs, sizeof(Expr *) * (size_t)al->cap);
    }
    al->exprs[al->count++] = e;
}

/* ---- Parameter list helpers ---- */
typedef struct ParamList {
    int count;
    int cap;
    char **names;
} ParamList;

static ParamList *new_paramlist(void) {
    ParamList *pl = (ParamList *)malloc(sizeof(ParamList));
    pl->count = 0;
    pl->cap = 4;
    pl->names = (char **)malloc(sizeof(char *) * (size_t)pl->cap);
    return pl;
}

static void paramlist_push(ParamList *pl, char *name) {
    if (pl->count >= pl->cap) {
        pl->cap *= 2;
        pl->names = (char **)realloc(pl->names, sizeof(char *) * (size_t)pl->cap);
    }
    pl->names[pl->count++] = name;
}

/* Track if we are inside a function definition */
static int in_function = 0;
static VarType current_func_return_type = TYPE_VOID;

#line 517 "build/parser.tab.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "parser.tab.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_START = 3,                      /* START  */
  YYSYMBOL_END = 4,                        /* END  */
  YYSYMBOL_DECL = 5,                       /* DECL  */
  YYSYMBOL_PRINT = 6,                      /* PRINT  */
  YYSYMBOL_INPUT = 7,                      /* INPUT  */
  YYSYMBOL_IF = 8,                         /* IF  */
  YYSYMBOL_ELSE = 9,                       /* ELSE  */
  YYSYMBOL_WHILE = 10,                     /* WHILE  */
  YYSYMBOL_FOR = 11,                       /* FOR  */
  YYSYMBOL_FUNC = 12,                      /* FUNC  */
  YYSYMBOL_RETURN = 13,                    /* RETURN  */
  YYSYMBOL_EQ = 14,                        /* EQ  */
  YYSYMBOL_NEQ = 15,                       /* NEQ  */
  YYSYMBOL_LEQ = 16,                       /* LEQ  */
  YYSYMBOL_GEQ = 17,                       /* GEQ  */
  YYSYMBOL_NEWLINE = 18,                   /* NEWLINE  */
  YYSYMBOL_IDENT = 19,                     /* IDENT  */
  YYSYMBOL_NUMBER = 20,                    /* NUMBER  */
  YYSYMBOL_STRING = 21,                    /* STRING  */
  YYSYMBOL_CHARLIT = 22,                   /* CHARLIT  */
  YYSYMBOL_FLOATLIT = 23,                  /* FLOATLIT  */
  YYSYMBOL_24_ = 24,                       /* '<'  */
  YYSYMBOL_25_ = 25,                       /* '>'  */
  YYSYMBOL_26_ = 26,                       /* '+'  */
  YYSYMBOL_27_ = 27,                       /* '-'  */
  YYSYMBOL_28_ = 28,                       /* '*'  */
  YYSYMBOL_29_ = 29,                       /* '/'  */
  YYSYMBOL_30_ = 30,                       /* '%'  */
  YYSYMBOL_31_ = 31,                       /* '='  */
  YYSYMBOL_32_ = 32,                       /* '('  */
  YYSYMBOL_33_ = 33,                       /* ')'  */
  YYSYMBOL_34_ = 34,                       /* ';'  */
  YYSYMBOL_35_ = 35,                       /* ','  */
  YYSYMBOL_36_ = 36,                       /* '{'  */
  YYSYMBOL_37_ = 37,                       /* '}'  */
  YYSYMBOL_YYACCEPT = 38,                  /* $accept  */
  YYSYMBOL_program = 39,                   /* program  */
  YYSYMBOL_separators = 40,                /* separators  */
  YYSYMBOL_stmt_list = 41,                 /* stmt_list  */
  YYSYMBOL_stmt = 42,                      /* stmt  */
  YYSYMBOL_return_stmt = 43,               /* return_stmt  */
  YYSYMBOL_func_def = 44,                  /* func_def  */
  YYSYMBOL_45_1 = 45,                      /* $@1  */
  YYSYMBOL_param_list = 46,                /* param_list  */
  YYSYMBOL_param_list_ne = 47,             /* param_list_ne  */
  YYSYMBOL_arg_list = 48,                  /* arg_list  */
  YYSYMBOL_arg_list_ne = 49,               /* arg_list_ne  */
  YYSYMBOL_assign_core = 50,               /* assign_core  */
  YYSYMBOL_update_core = 51,               /* update_core  */
  YYSYMBOL_opt_second_update = 52,         /* opt_second_update  */
  YYSYMBOL_print_arg = 53,                 /* print_arg  */
  YYSYMBOL_block = 54,                     /* block  */
  YYSYMBOL_else_part = 55,                 /* else_part  */
  YYSYMBOL_expr = 56                       /* expr  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   181

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  38
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  19
/* YYNRULES -- Number of rules.  */
#define YYNRULES  56
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  123

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   278


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    30,     2,     2,
      32,    33,    28,    26,    35,    27,     2,    29,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    34,
      24,    31,    25,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    36,     2,    37,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   481,   481,   507,   508,   513,   514,   518,   526,   540,
     544,   548,   560,   564,   568,   605,   609,   641,   651,   663,
     662,   717,   718,   723,   729,   738,   739,   744,   749,   757,
     769,   774,   779,   788,   789,   794,   809,   817,   820,   827,
     831,   835,   845,   849,   854,   881,   885,   886,   887,   888,
     889,   890,   891,   892,   893,   894,   895
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "START", "END", "DECL",
  "PRINT", "INPUT", "IF", "ELSE", "WHILE", "FOR", "FUNC", "RETURN", "EQ",
  "NEQ", "LEQ", "GEQ", "NEWLINE", "IDENT", "NUMBER", "STRING", "CHARLIT",
  "FLOATLIT", "'<'", "'>'", "'+'", "'-'", "'*'", "'/'", "'%'", "'='",
  "'('", "')'", "';'", "','", "'{'", "'}'", "$accept", "program",
  "separators", "stmt_list", "stmt", "return_stmt", "func_def", "$@1",
  "param_list", "param_list_ne", "arg_list", "arg_list_ne", "assign_core",
  "update_core", "opt_second_update", "print_arg", "block", "else_part",
  "expr", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-70)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     -70,    13,     0,   -70,   -70,   -70,    -3,   162,   -70,    -2,
     -13,   -11,     1,     3,     4,     8,    62,   -24,   -70,   -70,
     -70,   -70,    -3,    -1,    62,     9,    62,    62,    10,    45,
      47,   -70,   -70,   -70,   -70,    62,   135,    62,    62,    -3,
      -3,    62,    53,   135,    55,    75,    95,    64,    42,    74,
      62,   115,    62,    62,    62,    62,    62,    62,    62,    62,
      62,    62,    62,   135,    80,    72,   135,   135,   -70,   -70,
      78,    78,    62,   -70,    82,    81,    84,   -70,    37,    37,
      37,    37,    37,    37,    -5,    -5,   -70,   -70,   -70,   -70,
      62,   -70,    23,   -70,    44,   -70,    99,   -70,   135,    -3,
      78,   -70,   107,    78,   -70,    43,   -70,   -21,    92,   -70,
     -70,    62,    62,    62,   107,   100,    -3,   135,   135,   135,
     -70,    78,   -70
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       3,     0,     0,     1,     3,     4,     5,     0,     3,     0,
       0,     0,     0,     0,     0,     0,    18,     0,     3,    15,
       3,     9,     2,     0,     0,     0,     0,     0,     0,     0,
      43,    39,    42,    41,    40,     0,    17,     0,    25,     6,
       7,     0,     0,    35,     0,     0,     0,     0,     0,    21,
      25,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    29,     0,    26,    27,     8,    10,    11,
       0,     0,     0,    23,     0,    22,     0,    45,    55,    56,
      53,    54,    51,    52,    46,    47,    48,    49,    50,    16,
       0,     3,    37,    13,     0,    19,     0,    44,    28,     5,
       0,    12,     0,     0,    24,     0,    38,     0,    33,    20,
       3,     0,     0,     0,     0,     0,    36,    30,    31,    32,
      34,     0,    14
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -70,   -70,    -4,    35,   -70,   -70,   -70,   -70,   -70,   -70,
      85,   -70,   108,    24,   -70,   -70,   -69,   -70,   -15
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
       0,     1,     2,     7,    18,    19,    20,   103,    74,    75,
      64,    65,    21,   108,   115,    42,    92,   101,    66
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int8 yytable[] =
{
       6,    36,    93,     4,    22,   111,   112,    37,    38,    43,
     113,    45,    46,     3,    39,     5,    40,    23,     5,    24,
      51,    25,    63,    60,    61,    62,    67,    29,    44,    47,
      41,   106,   100,    26,   109,    27,    28,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,     9,    10,
      11,    12,   122,    13,    14,    15,    16,    94,    52,    53,
      54,    55,    17,    58,    59,    60,    61,    62,    56,    57,
      58,    59,    60,    61,    62,    98,    72,    49,   102,    50,
     110,    30,    31,    32,    33,    34,    68,    99,    69,    52,
      53,    54,    55,    73,    35,    37,   117,   118,   119,    56,
      57,    58,    59,    60,    61,    62,   116,    90,    70,    52,
      53,    54,    55,    89,    91,    95,    96,    97,   104,    56,
      57,    58,    59,    60,    61,    62,   107,   114,    71,    52,
      53,    54,    55,   121,   105,    76,    48,     0,   120,    56,
      57,    58,    59,    60,    61,    62,     0,     0,    77,    52,
      53,    54,    55,     0,     0,     0,     0,     0,     0,    56,
      57,    58,    59,    60,    61,    62,     8,     9,    10,    11,
      12,     0,    13,    14,    15,    16,     0,     0,     0,     0,
       0,    17
};

static const yytype_int8 yycheck[] =
{
       4,    16,    71,     3,     8,    26,    27,    31,    32,    24,
      31,    26,    27,     0,    18,    18,    20,    19,    18,    32,
      35,    32,    37,    28,    29,    30,    41,    19,    19,    19,
      31,   100,     9,    32,   103,    32,    32,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,     5,     6,
       7,     8,   121,    10,    11,    12,    13,    72,    14,    15,
      16,    17,    19,    26,    27,    28,    29,    30,    24,    25,
      26,    27,    28,    29,    30,    90,    34,    32,    34,    32,
      37,    19,    20,    21,    22,    23,    33,    91,    33,    14,
      15,    16,    17,    19,    32,    31,   111,   112,   113,    24,
      25,    26,    27,    28,    29,    30,   110,    35,    33,    14,
      15,    16,    17,    33,    36,    33,    35,    33,    19,    24,
      25,    26,    27,    28,    29,    30,    19,    35,    33,    14,
      15,    16,    17,    33,    99,    50,    28,    -1,   114,    24,
      25,    26,    27,    28,    29,    30,    -1,    -1,    33,    14,
      15,    16,    17,    -1,    -1,    -1,    -1,    -1,    -1,    24,
      25,    26,    27,    28,    29,    30,     4,     5,     6,     7,
       8,    -1,    10,    11,    12,    13,    -1,    -1,    -1,    -1,
      -1,    19
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,    39,    40,     0,     3,    18,    40,    41,     4,     5,
       6,     7,     8,    10,    11,    12,    13,    19,    42,    43,
      44,    50,    40,    19,    32,    32,    32,    32,    32,    19,
      19,    20,    21,    22,    23,    32,    56,    31,    32,    40,
      40,    31,    53,    56,    19,    56,    56,    19,    50,    32,
      32,    56,    14,    15,    16,    17,    24,    25,    26,    27,
      28,    29,    30,    56,    48,    49,    56,    56,    33,    33,
      33,    33,    34,    19,    46,    47,    48,    33,    56,    56,
      56,    56,    56,    56,    56,    56,    56,    56,    56,    33,
      35,    36,    54,    54,    56,    33,    35,    33,    56,    40,
       9,    55,    34,    45,    19,    41,    54,    19,    51,    54,
      37,    26,    27,    31,    35,    52,    40,    56,    56,    56,
      51,    33,    54
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    38,    39,    40,    40,    41,    41,    41,    42,    42,
      42,    42,    42,    42,    42,    42,    42,    43,    43,    45,
      44,    46,    46,    47,    47,    48,    48,    49,    49,    50,
      51,    51,    51,    52,    52,    53,    54,    55,    55,    56,
      56,    56,    56,    56,    56,    56,    56,    56,    56,    56,
      56,    56,    56,    56,    56,    56,    56
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     6,     0,     2,     0,     3,     3,     4,     1,
       4,     4,     6,     5,    10,     1,     4,     2,     1,     0,
       7,     0,     1,     1,     3,     0,     1,     1,     3,     3,
       3,     3,     3,     0,     2,     1,     5,     0,     2,     1,
       1,     1,     1,     1,     4,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* program: separators START separators stmt_list END separators  */
#line 482 "src/parser.y"
      {
          got_start = 1;
          if (error_count == 0) {
              fprintf(out_file, "#include <stdio.h>\n");
              fprintf(out_file, "#include <stdlib.h>\n\n");
              /* Forward declarations */
              if (func_fwd_len > 0) {
                  fprintf(out_file, "/* Forward declarations */\n");
                  fprintf(out_file, "%s\n", func_fwd);
              }
              /* Function definitions */
              if (func_defs_len > 0) {
                  fprintf(out_file, "%s\n", func_defs);
              }
              fprintf(out_file, "int main(void) {\n");
              fprintf(out_file, "%s", (yyvsp[-2].str) ? (yyvsp[-2].str) : "");
              fprintf(out_file, "    return 0;\n}\n");
              printf("Compilation success: generated C code.\n");
          } else {
              fprintf(stderr, "Compilation failed with %d error(s).\n", error_count);
          }
      }
#line 1665 "build/parser.tab.c"
    break;

  case 5: /* stmt_list: %empty  */
#line 513 "src/parser.y"
    { (yyval.str) = str_dup(""); }
#line 1671 "build/parser.tab.c"
    break;

  case 6: /* stmt_list: stmt_list stmt separators  */
#line 515 "src/parser.y"
      {
          (yyval.str) = fmt("%s%s", (yyvsp[-2].str), (yyvsp[-1].str));
      }
#line 1679 "build/parser.tab.c"
    break;

  case 7: /* stmt_list: stmt_list func_def separators  */
#line 519 "src/parser.y"
      {
          /* func_def already appended to func_defs buffer */
          (yyval.str) = (yyvsp[-2].str);
      }
#line 1688 "build/parser.tab.c"
    break;

  case 8: /* stmt: DECL IDENT '=' expr  */
#line 527 "src/parser.y"
      {
          VarType vt = (yyvsp[0].expr)->type;
          add_symbol((yyvsp[-2].str), vt);
          if (vt == TYPE_FLOAT) {
              (yyval.str) = fmt("    double %s = %s;\n", (yyvsp[-2].str), (yyvsp[0].expr)->code);
          } else if (vt == TYPE_CHAR) {
              (yyval.str) = fmt("    char %s = %s;\n", (yyvsp[-2].str), (yyvsp[0].expr)->code);
          } else if (vt == TYPE_STRING) {
              (yyval.str) = fmt("    const char* %s = %s;\n", (yyvsp[-2].str), (yyvsp[0].expr)->code);
          } else {
              (yyval.str) = fmt("    long long %s = %s;\n", (yyvsp[-2].str), (yyvsp[0].expr)->code);
          }
      }
#line 1706 "build/parser.tab.c"
    break;

  case 9: /* stmt: assign_core  */
#line 541 "src/parser.y"
      {
          (yyval.str) = fmt("    %s;\n", (yyvsp[0].str));
      }
#line 1714 "build/parser.tab.c"
    break;

  case 10: /* stmt: PRINT '(' print_arg ')'  */
#line 545 "src/parser.y"
      {
          (yyval.str) = fmt("    %s", (yyvsp[-1].str));
      }
#line 1722 "build/parser.tab.c"
    break;

  case 11: /* stmt: INPUT '(' IDENT ')'  */
#line 549 "src/parser.y"
      {
          must_be_declared((yyvsp[-1].str));
          VarType vt = symbol_type((yyvsp[-1].str));
          if (vt == TYPE_FLOAT) {
              (yyval.str) = fmt("    scanf(\"%%lf\", &%s);\n", (yyvsp[-1].str));
          } else if (vt == TYPE_CHAR) {
              (yyval.str) = fmt("    scanf(\" %%c\", &%s);\n", (yyvsp[-1].str));
          } else {
              (yyval.str) = fmt("    scanf(\"%%lld\", &%s);\n", (yyvsp[-1].str));
          }
      }
#line 1738 "build/parser.tab.c"
    break;

  case 12: /* stmt: IF '(' expr ')' block else_part  */
#line 561 "src/parser.y"
      {
          (yyval.str) = fmt("    if (%s) %s%s", (yyvsp[-3].expr)->code, (yyvsp[-1].str), (yyvsp[0].str));
      }
#line 1746 "build/parser.tab.c"
    break;

  case 13: /* stmt: WHILE '(' expr ')' block  */
#line 565 "src/parser.y"
      {
          (yyval.str) = fmt("    while (%s) %s", (yyvsp[-2].expr)->code, (yyvsp[0].str));
      }
#line 1754 "build/parser.tab.c"
    break;

  case 14: /* stmt: FOR '(' assign_core ';' expr ';' update_core opt_second_update ')' block  */
#line 569 "src/parser.y"
      {
          /* Infinite loop detection for double update:
             If both updates exist and they are constant-only,
             check if they cancel each other out (net change == 0). */
          if ((yyvsp[-2].str) && strcmp((yyvsp[-2].str), "/* no second update */") != 0) {
              /* We embed a runtime safety counter */
              (yyval.str) = fmt(
                  "    {\n"
                  "        int __mama_toggle = 0;\n"
                  "        long long __mama_safety = 0;\n"
                  "        %s;\n"
                  "        while (%s) {\n"
                  "%s"
                  "            if (__mama_toggle == 0) { %s; } else { %s }\n"
                  "            __mama_toggle = 1 - __mama_toggle;\n"
                  "            if (++__mama_safety > 1000000LL) {\n"
                  "                printf(\"Mama, infinite loop dhora porche! Loop force-stop.\\n\");\n"
                  "                break;\n"
                  "            }\n"
                  "        }\n"
                  "    }\n",
                  (yyvsp[-7].str), (yyvsp[-5].expr)->code, (yyvsp[0].str), (yyvsp[-3].str), (yyvsp[-2].str)
              );
          } else {
              (yyval.str) = fmt(
                  "    {\n"
                  "        %s;\n"
                  "        while (%s) {\n"
                  "%s"
                  "            %s;\n"
                  "        }\n"
                  "    }\n",
                  (yyvsp[-7].str), (yyvsp[-5].expr)->code, (yyvsp[0].str), (yyvsp[-3].str)
              );
          }
      }
#line 1795 "build/parser.tab.c"
    break;

  case 15: /* stmt: return_stmt  */
#line 606 "src/parser.y"
      {
          (yyval.str) = (yyvsp[0].str);
      }
#line 1803 "build/parser.tab.c"
    break;

  case 16: /* stmt: IDENT '(' arg_list ')'  */
#line 610 "src/parser.y"
      {
          /* Function call as a statement */
          FuncDef *f = find_func((yyvsp[-3].str));
          if (!f) {
              fprintf(stderr, "[Line %d] Mama, ei function ke ami chini na: %s\n", yylineno, (yyvsp[-3].str));
              error_count++;
              (yyval.str) = fmt("    /* unknown func %s */;\n", (yyvsp[-3].str));
          } else {
              if ((yyvsp[-1].args)->count != f->param_count) {
                  fprintf(stderr, "[Line %d] Function '%s' expects %d args, got %d\n", yylineno, (yyvsp[-3].str), f->param_count, (yyvsp[-1].args)->count);
                  error_count++;
              } else {
                  /* Type check arguments */
                  for (int i = 0; i < (yyvsp[-1].args)->count; i++) {
                      check_implicit_conversion((yyvsp[-1].args)->exprs[i]->type, f->param_types[i], yylineno);
                  }
              }
              /* Build call */
              char *call = str_dup((yyvsp[-3].str));
              call = fmt("%s(", call);
              for (int i = 0; i < (yyvsp[-1].args)->count; i++) {
                  if (i > 0) call = fmt("%s, %s", call, (yyvsp[-1].args)->exprs[i]->code);
                  else call = fmt("%s%s", call, (yyvsp[-1].args)->exprs[i]->code);
              }
              call = fmt("%s)", call);
              (yyval.str) = fmt("    %s;\n", call);
          }
      }
#line 1836 "build/parser.tab.c"
    break;

  case 17: /* return_stmt: RETURN expr  */
#line 642 "src/parser.y"
      {
          if (!in_function) {
              fprintf(stderr, "[Line %d] 'ferot_de_mama' used outside of function!\n", yylineno);
              error_count++;
          } else {
              check_implicit_conversion((yyvsp[0].expr)->type, current_func_return_type, yylineno);
          }
          (yyval.str) = fmt("    return %s;\n", (yyvsp[0].expr)->code);
      }
#line 1850 "build/parser.tab.c"
    break;

  case 18: /* return_stmt: RETURN  */
#line 652 "src/parser.y"
      {
          if (!in_function) {
              fprintf(stderr, "[Line %d] 'ferot_de_mama' used outside of function!\n", yylineno);
              error_count++;
          }
          (yyval.str) = str_dup("    return;\n");
      }
#line 1862 "build/parser.tab.c"
    break;

  case 19: /* $@1: %empty  */
#line 663 "src/parser.y"
      {
          /* Mid-rule action: register function and push scope before parsing body */
          int pc = (yyvsp[-1].params)->count;
          char **pnames_copy = (char **)malloc(sizeof(char *) * (size_t)(pc > 0 ? pc : 1));
          VarType *ptypes = (VarType *)malloc(sizeof(VarType) * (size_t)(pc > 0 ? pc : 1));
          for (int i = 0; i < pc; i++) {
              pnames_copy[i] = str_dup((yyvsp[-1].params)->names[i]);
              ptypes[i] = TYPE_INT;
          }

          if (find_func((yyvsp[-3].str))) {
              fprintf(stderr, "[Line %d] Duplicate function definition: %s\n", yylineno, (yyvsp[-3].str));
              error_count++;
          } else {
              add_func((yyvsp[-3].str), TYPE_INT, pc, pnames_copy, ptypes);
          }

          in_function = 1;
          current_func_return_type = TYPE_INT;
          push_scope();
          for (int i = 0; i < (yyvsp[-1].params)->count; i++) {
              add_symbol((yyvsp[-1].params)->names[i], TYPE_INT);
          }
      }
#line 1891 "build/parser.tab.c"
    break;

  case 20: /* func_def: FUNC IDENT '(' param_list ')' $@1 block  */
#line 688 "src/parser.y"
      {
          int pc = (yyvsp[-3].params)->count;
          char **pnames = (yyvsp[-3].params)->names;

          /* Build C function signature */
          char *sig = fmt("long long %s(", (yyvsp[-5].str));
          for (int i = 0; i < pc; i++) {
              if (i > 0) sig = fmt("%s, long long %s", sig, pnames[i]);
              else sig = fmt("%slong long %s", sig, pnames[i]);
          }
          if (pc == 0) sig = fmt("%svoid", sig);
          sig = fmt("%s)", sig);

          /* Forward declaration */
          char *fwd = fmt("%s;\n", sig);
          append_buf(func_fwd, &func_fwd_len, FUNC_BUF_SIZE, fwd);

          /* Full definition — $7 is the block (after mid-rule action at $6) */
          char *def = fmt("%s {\n%s}\n\n", sig, (yyvsp[0].str));
          append_buf(func_defs, &func_defs_len, FUNC_BUF_SIZE, def);

          pop_scope();
          in_function = 0;
          (yyval.str) = str_dup("");
      }
#line 1921 "build/parser.tab.c"
    break;

  case 21: /* param_list: %empty  */
#line 717 "src/parser.y"
      { (yyval.params) = new_paramlist(); }
#line 1927 "build/parser.tab.c"
    break;

  case 22: /* param_list: param_list_ne  */
#line 719 "src/parser.y"
      { (yyval.params) = (yyvsp[0].params); }
#line 1933 "build/parser.tab.c"
    break;

  case 23: /* param_list_ne: IDENT  */
#line 724 "src/parser.y"
      {
          (yyval.params) = new_paramlist();
          paramlist_push((yyval.params), str_dup((yyvsp[0].str)));
          /* Add param to current scope for body parsing — handled via func_def scope push */
      }
#line 1943 "build/parser.tab.c"
    break;

  case 24: /* param_list_ne: param_list_ne ',' IDENT  */
#line 730 "src/parser.y"
      {
          paramlist_push((yyvsp[-2].params), str_dup((yyvsp[0].str)));
          (yyval.params) = (yyvsp[-2].params);
      }
#line 1952 "build/parser.tab.c"
    break;

  case 25: /* arg_list: %empty  */
#line 738 "src/parser.y"
      { (yyval.args) = new_arglist(); }
#line 1958 "build/parser.tab.c"
    break;

  case 26: /* arg_list: arg_list_ne  */
#line 740 "src/parser.y"
      { (yyval.args) = (yyvsp[0].args); }
#line 1964 "build/parser.tab.c"
    break;

  case 27: /* arg_list_ne: expr  */
#line 745 "src/parser.y"
      {
          (yyval.args) = new_arglist();
          arglist_push((yyval.args), (yyvsp[0].expr));
      }
#line 1973 "build/parser.tab.c"
    break;

  case 28: /* arg_list_ne: arg_list_ne ',' expr  */
#line 750 "src/parser.y"
      {
          arglist_push((yyvsp[-2].args), (yyvsp[0].expr));
          (yyval.args) = (yyvsp[-2].args);
      }
#line 1982 "build/parser.tab.c"
    break;

  case 29: /* assign_core: IDENT '=' expr  */
#line 758 "src/parser.y"
      {
          must_be_declared((yyvsp[-2].str));
          VarType vt = symbol_type((yyvsp[-2].str));
          if (vt != TYPE_UNKNOWN) {
              check_implicit_conversion((yyvsp[0].expr)->type, vt, yylineno);
          }
          (yyval.str) = fmt("%s = %s", (yyvsp[-2].str), cast_code((yyvsp[0].expr)->code, (yyvsp[0].expr)->type, vt));
      }
#line 1995 "build/parser.tab.c"
    break;

  case 30: /* update_core: IDENT '+' expr  */
#line 770 "src/parser.y"
      {
          must_be_declared((yyvsp[-2].str));
          (yyval.str) = fmt("%s = %s + %s", (yyvsp[-2].str), (yyvsp[-2].str), (yyvsp[0].expr)->code);
      }
#line 2004 "build/parser.tab.c"
    break;

  case 31: /* update_core: IDENT '-' expr  */
#line 775 "src/parser.y"
      {
          must_be_declared((yyvsp[-2].str));
          (yyval.str) = fmt("%s = %s - %s", (yyvsp[-2].str), (yyvsp[-2].str), (yyvsp[0].expr)->code);
      }
#line 2013 "build/parser.tab.c"
    break;

  case 32: /* update_core: IDENT '=' expr  */
#line 780 "src/parser.y"
      {
          must_be_declared((yyvsp[-2].str));
          (yyval.str) = fmt("%s = %s", (yyvsp[-2].str), (yyvsp[0].expr)->code);
      }
#line 2022 "build/parser.tab.c"
    break;

  case 33: /* opt_second_update: %empty  */
#line 788 "src/parser.y"
    { (yyval.str) = str_dup("/* no second update */"); }
#line 2028 "build/parser.tab.c"
    break;

  case 34: /* opt_second_update: ',' update_core  */
#line 790 "src/parser.y"
      { (yyval.str) = fmt("%s;", (yyvsp[0].str)); }
#line 2034 "build/parser.tab.c"
    break;

  case 35: /* print_arg: expr  */
#line 795 "src/parser.y"
      {
          if ((yyvsp[0].expr)->type == TYPE_STRING) {
              (yyval.str) = fmt("printf(\"%%s\\n\", %s);\n", (yyvsp[0].expr)->code);
          } else if ((yyvsp[0].expr)->type == TYPE_FLOAT) {
              (yyval.str) = fmt("printf(\"%%f\\n\", (double)(%s));\n", (yyvsp[0].expr)->code);
          } else if ((yyvsp[0].expr)->type == TYPE_CHAR) {
              (yyval.str) = fmt("printf(\"%%c\\n\", (char)(%s));\n", (yyvsp[0].expr)->code);
          } else {
              (yyval.str) = fmt("printf(\"%%lld\\n\", (long long)(%s));\n", (yyvsp[0].expr)->code);
          }
      }
#line 2050 "build/parser.tab.c"
    break;

  case 36: /* block: '{' separators stmt_list '}' separators  */
#line 810 "src/parser.y"
      {
          (yyval.str) = fmt("{\n%s    }\n", (yyvsp[-2].str));
      }
#line 2058 "build/parser.tab.c"
    break;

  case 37: /* else_part: %empty  */
#line 817 "src/parser.y"
      {
          (yyval.str) = str_dup("");
      }
#line 2066 "build/parser.tab.c"
    break;

  case 38: /* else_part: ELSE block  */
#line 821 "src/parser.y"
      {
          (yyval.str) = fmt("    else %s", (yyvsp[0].str));
      }
#line 2074 "build/parser.tab.c"
    break;

  case 39: /* expr: NUMBER  */
#line 828 "src/parser.y"
      {
          (yyval.expr) = make_expr_const_int(strtoll((yyvsp[0].str), NULL, 10));
      }
#line 2082 "build/parser.tab.c"
    break;

  case 40: /* expr: FLOATLIT  */
#line 832 "src/parser.y"
      {
          (yyval.expr) = make_expr_const_float(strtod((yyvsp[0].str), NULL));
      }
#line 2090 "build/parser.tab.c"
    break;

  case 41: /* expr: CHARLIT  */
#line 836 "src/parser.y"
      {
          Expr *e = (Expr *)malloc(sizeof(Expr));
          e->code = str_dup((yyvsp[0].str));
          e->is_const = 1;
          e->const_val = char_to_int((yyvsp[0].str));
          e->const_fval = (double)e->const_val;
          e->type = TYPE_CHAR;
          (yyval.expr) = e;
      }
#line 2104 "build/parser.tab.c"
    break;

  case 42: /* expr: STRING  */
#line 846 "src/parser.y"
      {
          (yyval.expr) = make_expr_code(str_dup((yyvsp[0].str)), TYPE_STRING);
      }
#line 2112 "build/parser.tab.c"
    break;

  case 43: /* expr: IDENT  */
#line 850 "src/parser.y"
      {
          must_be_declared((yyvsp[0].str));
          (yyval.expr) = make_expr_code(str_dup((yyvsp[0].str)), symbol_type((yyvsp[0].str)));
      }
#line 2121 "build/parser.tab.c"
    break;

  case 44: /* expr: IDENT '(' arg_list ')'  */
#line 855 "src/parser.y"
      {
          /* Function call in expression */
          FuncDef *f = find_func((yyvsp[-3].str));
          if (!f) {
              fprintf(stderr, "[Line %d] Mama, ei function ke ami chini na: %s\n", yylineno, (yyvsp[-3].str));
              error_count++;
              (yyval.expr) = make_expr_const_int(0);
          } else {
              if ((yyvsp[-1].args)->count != f->param_count) {
                  fprintf(stderr, "[Line %d] Function '%s' expects %d args, got %d\n", yylineno, (yyvsp[-3].str), f->param_count, (yyvsp[-1].args)->count);
                  error_count++;
              } else {
                  for (int i = 0; i < (yyvsp[-1].args)->count; i++) {
                      check_implicit_conversion((yyvsp[-1].args)->exprs[i]->type, f->param_types[i], yylineno);
                  }
              }
              char *call = str_dup((yyvsp[-3].str));
              call = fmt("%s(", call);
              for (int i = 0; i < (yyvsp[-1].args)->count; i++) {
                  if (i > 0) call = fmt("%s, %s", call, (yyvsp[-1].args)->exprs[i]->code);
                  else call = fmt("%s%s", call, (yyvsp[-1].args)->exprs[i]->code);
              }
              call = fmt("%s)", call);
              (yyval.expr) = make_expr_code(call, f->return_type);
          }
      }
#line 2152 "build/parser.tab.c"
    break;

  case 45: /* expr: '(' expr ')'  */
#line 882 "src/parser.y"
      {
          (yyval.expr) = (yyvsp[-1].expr);
      }
#line 2160 "build/parser.tab.c"
    break;

  case 46: /* expr: expr '+' expr  */
#line 885 "src/parser.y"
                      { (yyval.expr) = combine_binary("+", (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 2166 "build/parser.tab.c"
    break;

  case 47: /* expr: expr '-' expr  */
#line 886 "src/parser.y"
                      { (yyval.expr) = combine_binary("-", (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 2172 "build/parser.tab.c"
    break;

  case 48: /* expr: expr '*' expr  */
#line 887 "src/parser.y"
                      { (yyval.expr) = combine_binary("*", (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 2178 "build/parser.tab.c"
    break;

  case 49: /* expr: expr '/' expr  */
#line 888 "src/parser.y"
                      { (yyval.expr) = combine_binary("/", (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 2184 "build/parser.tab.c"
    break;

  case 50: /* expr: expr '%' expr  */
#line 889 "src/parser.y"
                      { (yyval.expr) = combine_binary("%", (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 2190 "build/parser.tab.c"
    break;

  case 51: /* expr: expr '<' expr  */
#line 890 "src/parser.y"
                      { (yyval.expr) = combine_binary("<", (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 2196 "build/parser.tab.c"
    break;

  case 52: /* expr: expr '>' expr  */
#line 891 "src/parser.y"
                      { (yyval.expr) = combine_binary(">", (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 2202 "build/parser.tab.c"
    break;

  case 53: /* expr: expr LEQ expr  */
#line 892 "src/parser.y"
                      { (yyval.expr) = combine_binary("<=", (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 2208 "build/parser.tab.c"
    break;

  case 54: /* expr: expr GEQ expr  */
#line 893 "src/parser.y"
                      { (yyval.expr) = combine_binary(">=", (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 2214 "build/parser.tab.c"
    break;

  case 55: /* expr: expr EQ expr  */
#line 894 "src/parser.y"
                      { (yyval.expr) = combine_binary("==", (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 2220 "build/parser.tab.c"
    break;

  case 56: /* expr: expr NEQ expr  */
#line 895 "src/parser.y"
                      { (yyval.expr) = combine_binary("!=", (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 2226 "build/parser.tab.c"
    break;


#line 2230 "build/parser.tab.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (YY_("syntax error"));
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 898 "src/parser.y"


void yyerror(const char *s) {
    if (!got_start && error_count == 0) {
        fprintf(stderr, "[Line %d] 'Edike ay mama' likhte bhule geso!!\n", yylineno);
    } else {
        fprintf(stderr, "[Line %d] Syntax error: %s\n", yylineno, s);
    }
    error_count++;
}

int main(int argc, char **argv) {
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: %s <input.mama> [output.c]\n", argv[0]);
        return 1;
    }

    const char *input_path = argv[1];
    const char *output_path = (argc == 3) ? argv[2] : "generated.c";

    yyin = fopen(input_path, "r");
    if (!yyin) {
        fprintf(stderr, "Could not open input: %s\n", input_path);
        return 1;
    }

    out_file = fopen(output_path, "w");
    if (!out_file) {
        fclose(yyin);
        fprintf(stderr, "Could not open output: %s\n", output_path);
        return 1;
    }

    /* Initialize buffers */
    func_defs[0] = '\0';
    func_fwd[0] = '\0';

    /* Initialize global scope */
    push_scope();

    int parse_result = yyparse();

    pop_scope();

    fclose(yyin);
    fclose(out_file);

    if (parse_result != 0 || error_count > 0) {
        remove(output_path);
        return 1;
    }

    return 0;
}
