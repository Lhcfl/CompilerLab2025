const input = `
Program: ExtDefList
    ;

ExtDefList: /* empty */
    | ExtDef ExtDefList
    ;

ExtDef: Specifier ExtDecList SEMI
    | Specifier SEMI
    | Specifier FunDec CompSt
    ;

ExtDecList: VarDec
    | VarDec COMMA ExtDecList
    ;

/* Specifiers */ 

Specifier: TYPE
    | StructSpecifier
    ;

StructSpecifier: STRUCT OptTag LC DefList RC
    | STRUCT Tag
    ;

OptTag: /* empty */ 
    | ID
    ;

Tag: ID
    ;

/* Declarators */

VarDec: ID
    | VarDec LB INT RB
    ;

FunDec: ID LP VarList RP
    | ID LP RP
    ;

VarList: ParamDec COMMA VarList
    | ParamDec
    ;

ParamDec: Specifier VarDec
    ;

/* Statements */
CompSt: LC DefList StmtList RC
    ;

StmtList: /* empty */
    | Stmt StmtList
    ;

Stmt: Exp SEMI
    | CompSt
    | RETURN Exp SEMI
    | IF LP Exp RP Stmt
    | IF LP Exp RP Stmt ELSE Stmt
    | WHILE LP Exp RP Stmt
    ;

/** Local Definations */
DefList: /* empty */
    | Def DefList
    ;

Def: Specifier DecList SEMI
    ;

DecList: Dec
    | Dec COMMA DecList
    ;

Dec: VarDec
    | VarDec ASSIGNOP Exp
    ;

/** expressions */

Exp: Exp ASSIGNOP Exp
    | Exp AND Exp
    | Exp OR Exp
    | Exp RELOP Exp
    | Exp PLUS Exp
    | Exp MINUS Exp
    | Exp STAR Exp
    | Exp DIV Exp
    | LP Exp RP
    | MINUS Exp
    | NOT Exp
    | ID LP Args RP
    | ID LP RP
    | Exp LB Exp RB
    | Exp DOT ID
    | ID                            
    | INT                          
    | FLOAT           
    ;

Args: Exp COMMA Args               
    | Exp                     
    ;
`;

function genList(count) {
  let str = "{ $$ = cmm_node_tree(" + count;
  for (let i = 1; i <= count; i++) {
    str += ", $";
    str += i;
  }
  str += "); }";
  return str;
}

const lines = input.split("\n").map((line) => {
  if (line.match(/^[\s]+\|/) || line.match(/^[A-Z][a-zA-z]+/)) {
    const idents = line.matchAll(/[A-Z][A-Za-z]+/g);
    const comma = line.includes(":");
    const len = [...idents].length - comma;
    if (len > 0) {
      return [line, genList(len)];
    } else {
      return [line, "{ $$ = cmm_empty_tree(); }"];
    }
  } else {
    return [line, null];
  }
});

const line_space =
  lines.map((x) => x[0]).reduce((a, b) => Math.max(a, b.length), 0) + 3;

const ret = lines
  .map(([pre, next]) => {
    if (!next) {
      return pre;
    }
    let str = pre;
    while (str.length < line_space) {
      str += " ";
    }
    return str + next;
  })
  .join("\n");

console.log(ret);
