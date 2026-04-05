%{
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
%}

%code requires {
#ifndef MAMA_TYPES_DEFINED
#define MAMA_TYPES_DEFINED
typedef struct Expr Expr;
typedef struct ArgList ArgList;
typedef struct ParamList ParamList;
#endif
}

%union {
    char *str;
    Expr *expr;
    ArgList *args;
    ParamList *params;
}

%token START END DECL PRINT INPUT IF ELSE WHILE FOR FUNC RETURN
%token EQ NEQ LEQ GEQ
%token NEWLINE
%token <str> IDENT NUMBER STRING CHARLIT FLOATLIT

%type <expr> expr
%type <str> stmt stmt_list block else_part print_arg assign_core update_core opt_second_update
%type <str> func_def return_stmt
%type <args> arg_list arg_list_ne
%type <params> param_list param_list_ne

%left EQ NEQ '<' '>' LEQ GEQ
%left '+' '-'
%left '*' '/' '%'

%%

program
    : separators START separators stmt_list END separators
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
              fprintf(out_file, "%s", $4 ? $4 : "");
              fprintf(out_file, "    return 0;\n}\n");
              printf("Compilation success: generated C code.\n");
          } else {
              fprintf(stderr, "Compilation failed with %d error(s).\n", error_count);
          }
      }
    ;

separators
    :
    | separators NEWLINE
    ;

stmt_list
    :
    { $$ = str_dup(""); }
    | stmt_list stmt separators
      {
          $$ = fmt("%s%s", $1, $2);
      }
    | stmt_list func_def separators
      {
          /* func_def already appended to func_defs buffer */
          $$ = $1;
      }
    ;

stmt
    : DECL IDENT '=' expr
      {
          VarType vt = $4->type;
          add_symbol($2, vt);
          if (vt == TYPE_FLOAT) {
              $$ = fmt("    double %s = %s;\n", $2, $4->code);
          } else if (vt == TYPE_CHAR) {
              $$ = fmt("    char %s = %s;\n", $2, $4->code);
          } else if (vt == TYPE_STRING) {
              $$ = fmt("    const char* %s = %s;\n", $2, $4->code);
          } else {
              $$ = fmt("    long long %s = %s;\n", $2, $4->code);
          }
      }
    | assign_core
      {
          $$ = fmt("    %s;\n", $1);
      }
    | PRINT '(' print_arg ')'
      {
          $$ = fmt("    %s", $3);
      }
    | INPUT '(' IDENT ')'
      {
          must_be_declared($3);
          VarType vt = symbol_type($3);
          if (vt == TYPE_FLOAT) {
              $$ = fmt("    scanf(\"%%lf\", &%s);\n", $3);
          } else if (vt == TYPE_CHAR) {
              $$ = fmt("    scanf(\" %%c\", &%s);\n", $3);
          } else {
              $$ = fmt("    scanf(\"%%lld\", &%s);\n", $3);
          }
      }
    | IF '(' expr ')' block else_part
      {
          $$ = fmt("    if (%s) %s%s", $3->code, $5, $6);
      }
    | WHILE '(' expr ')' block
      {
          $$ = fmt("    while (%s) %s", $3->code, $5);
      }
    | FOR '(' assign_core ';' expr ';' update_core opt_second_update ')' block
      {
          /* Infinite loop detection for double update:
             If both updates exist and they are constant-only,
             check if they cancel each other out (net change == 0). */
          if ($8 && strcmp($8, "/* no second update */") != 0) {
              /* We embed a runtime safety counter */
              $$ = fmt(
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
                  $3, $5->code, $10, $7, $8
              );
          } else {
              $$ = fmt(
                  "    {\n"
                  "        %s;\n"
                  "        while (%s) {\n"
                  "%s"
                  "            %s;\n"
                  "        }\n"
                  "    }\n",
                  $3, $5->code, $10, $7
              );
          }
      }
    | return_stmt
      {
          $$ = $1;
      }
    | IDENT '(' arg_list ')'
      {
          /* Function call as a statement */
          FuncDef *f = find_func($1);
          if (!f) {
              fprintf(stderr, "[Line %d] Mama, ei function ke ami chini na: %s\n", yylineno, $1);
              error_count++;
              $$ = fmt("    /* unknown func %s */;\n", $1);
          } else {
              if ($3->count != f->param_count) {
                  fprintf(stderr, "[Line %d] Function '%s' expects %d args, got %d\n", yylineno, $1, f->param_count, $3->count);
                  error_count++;
              } else {
                  /* Type check arguments */
                  for (int i = 0; i < $3->count; i++) {
                      check_implicit_conversion($3->exprs[i]->type, f->param_types[i], yylineno);
                  }
              }
              /* Build call */
              char *call = str_dup($1);
              call = fmt("%s(", call);
              for (int i = 0; i < $3->count; i++) {
                  if (i > 0) call = fmt("%s, %s", call, $3->exprs[i]->code);
                  else call = fmt("%s%s", call, $3->exprs[i]->code);
              }
              call = fmt("%s)", call);
              $$ = fmt("    %s;\n", call);
          }
      }
    ;

return_stmt
    : RETURN expr
      {
          if (!in_function) {
              fprintf(stderr, "[Line %d] 'ferot_de_mama' used outside of function!\n", yylineno);
              error_count++;
          } else {
              check_implicit_conversion($2->type, current_func_return_type, yylineno);
          }
          $$ = fmt("    return %s;\n", $2->code);
      }
    | RETURN
      {
          if (!in_function) {
              fprintf(stderr, "[Line %d] 'ferot_de_mama' used outside of function!\n", yylineno);
              error_count++;
          }
          $$ = str_dup("    return;\n");
      }
    ;

func_def
    : FUNC IDENT '(' param_list ')'
      {
          /* Mid-rule action: register function and push scope before parsing body */
          int pc = $4->count;
          char **pnames_copy = (char **)malloc(sizeof(char *) * (size_t)(pc > 0 ? pc : 1));
          VarType *ptypes = (VarType *)malloc(sizeof(VarType) * (size_t)(pc > 0 ? pc : 1));
          for (int i = 0; i < pc; i++) {
              pnames_copy[i] = str_dup($4->names[i]);
              ptypes[i] = TYPE_INT;
          }

          if (find_func($2)) {
              fprintf(stderr, "[Line %d] Duplicate function definition: %s\n", yylineno, $2);
              error_count++;
          } else {
              add_func($2, TYPE_INT, pc, pnames_copy, ptypes);
          }

          in_function = 1;
          current_func_return_type = TYPE_INT;
          push_scope();
          for (int i = 0; i < $4->count; i++) {
              add_symbol($4->names[i], TYPE_INT);
          }
      }
      block
      {
          int pc = $4->count;
          char **pnames = $4->names;

          /* Build C function signature */
          char *sig = fmt("long long %s(", $2);
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
          char *def = fmt("%s {\n%s}\n\n", sig, $7);
          append_buf(func_defs, &func_defs_len, FUNC_BUF_SIZE, def);

          pop_scope();
          in_function = 0;
          $$ = str_dup("");
      }
    ;

param_list
    :
      { $$ = new_paramlist(); }
    | param_list_ne
      { $$ = $1; }
    ;

param_list_ne
    : IDENT
      {
          $$ = new_paramlist();
          paramlist_push($$, str_dup($1));
          /* Add param to current scope for body parsing — handled via func_def scope push */
      }
    | param_list_ne ',' IDENT
      {
          paramlist_push($1, str_dup($3));
          $$ = $1;
      }
    ;

arg_list
    :
      { $$ = new_arglist(); }
    | arg_list_ne
      { $$ = $1; }
    ;

arg_list_ne
    : expr
      {
          $$ = new_arglist();
          arglist_push($$, $1);
      }
    | arg_list_ne ',' expr
      {
          arglist_push($1, $3);
          $$ = $1;
      }
    ;

assign_core
    : IDENT '=' expr
      {
          must_be_declared($1);
          VarType vt = symbol_type($1);
          if (vt != TYPE_UNKNOWN) {
              check_implicit_conversion($3->type, vt, yylineno);
          }
          $$ = fmt("%s = %s", $1, cast_code($3->code, $3->type, vt));
      }
    ;

update_core
    : IDENT '+' expr
      {
          must_be_declared($1);
          $$ = fmt("%s = %s + %s", $1, $1, $3->code);
      }
    | IDENT '-' expr
      {
          must_be_declared($1);
          $$ = fmt("%s = %s - %s", $1, $1, $3->code);
      }
    | IDENT '=' expr
      {
          must_be_declared($1);
          $$ = fmt("%s = %s", $1, $3->code);
      }
    ;

opt_second_update
    :
    { $$ = str_dup("/* no second update */"); }
    | ',' update_core
      { $$ = fmt("%s;", $2); }
    ;

print_arg
    : expr
      {
          if ($1->type == TYPE_STRING) {
              $$ = fmt("printf(\"%%s\\n\", %s);\n", $1->code);
          } else if ($1->type == TYPE_FLOAT) {
              $$ = fmt("printf(\"%%f\\n\", (double)(%s));\n", $1->code);
          } else if ($1->type == TYPE_CHAR) {
              $$ = fmt("printf(\"%%c\\n\", (char)(%s));\n", $1->code);
          } else {
              $$ = fmt("printf(\"%%lld\\n\", (long long)(%s));\n", $1->code);
          }
      }
    ;

block
    : '{' separators stmt_list '}' separators
      {
          $$ = fmt("{\n%s    }\n", $3);
      }
    ;

else_part
    :
      {
          $$ = str_dup("");
      }
    | ELSE block
      {
          $$ = fmt("    else %s", $2);
      }
    ;

expr
    : NUMBER
      {
          $$ = make_expr_const_int(strtoll($1, NULL, 10));
      }
    | FLOATLIT
      {
          $$ = make_expr_const_float(strtod($1, NULL));
      }
    | CHARLIT
      {
          Expr *e = (Expr *)malloc(sizeof(Expr));
          e->code = str_dup($1);
          e->is_const = 1;
          e->const_val = char_to_int($1);
          e->const_fval = (double)e->const_val;
          e->type = TYPE_CHAR;
          $$ = e;
      }
    | STRING
      {
          $$ = make_expr_code(str_dup($1), TYPE_STRING);
      }
    | IDENT
      {
          must_be_declared($1);
          $$ = make_expr_code(str_dup($1), symbol_type($1));
      }
    | IDENT '(' arg_list ')'
      {
          /* Function call in expression */
          FuncDef *f = find_func($1);
          if (!f) {
              fprintf(stderr, "[Line %d] Mama, ei function ke ami chini na: %s\n", yylineno, $1);
              error_count++;
              $$ = make_expr_const_int(0);
          } else {
              if ($3->count != f->param_count) {
                  fprintf(stderr, "[Line %d] Function '%s' expects %d args, got %d\n", yylineno, $1, f->param_count, $3->count);
                  error_count++;
              } else {
                  for (int i = 0; i < $3->count; i++) {
                      check_implicit_conversion($3->exprs[i]->type, f->param_types[i], yylineno);
                  }
              }
              char *call = str_dup($1);
              call = fmt("%s(", call);
              for (int i = 0; i < $3->count; i++) {
                  if (i > 0) call = fmt("%s, %s", call, $3->exprs[i]->code);
                  else call = fmt("%s%s", call, $3->exprs[i]->code);
              }
              call = fmt("%s)", call);
              $$ = make_expr_code(call, f->return_type);
          }
      }
    | '(' expr ')'
      {
          $$ = $2;
      }
    | expr '+' expr   { $$ = combine_binary("+", $1, $3); }
    | expr '-' expr   { $$ = combine_binary("-", $1, $3); }
    | expr '*' expr   { $$ = combine_binary("*", $1, $3); }
    | expr '/' expr   { $$ = combine_binary("/", $1, $3); }
    | expr '%' expr   { $$ = combine_binary("%", $1, $3); }
    | expr '<' expr   { $$ = combine_binary("<", $1, $3); }
    | expr '>' expr   { $$ = combine_binary(">", $1, $3); }
    | expr LEQ expr   { $$ = combine_binary("<=", $1, $3); }
    | expr GEQ expr   { $$ = combine_binary(">=", $1, $3); }
    | expr EQ expr    { $$ = combine_binary("==", $1, $3); }
    | expr NEQ expr   { $$ = combine_binary("!=", $1, $3); }
    ;

%%

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
