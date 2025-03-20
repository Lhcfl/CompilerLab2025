const DEFINATIONS = {
  FLOAT: /[0-9]+\.[0-9]*/,
  INT: /[0-9]+/,
  SEMI: [";"],
  COMMA: [","],
  ASSIGNOP: ["="],
  RELOP: [">", "<", ">=", "<=", "==", "!="],
  PLUS: ["+"],
  MINUS: ["-"],
  STAR: ["*"],
  DIV: ["/"],
  AND: ["&&"],
  OR: ["||"],
  DOT: ["."],
  NOT: ["!"],
  TYPE: ["int", "float"],
  LP: ["("],
  RP: [")"],
  LB: ["["],
  RB: ["]"],
  LC: ["{"],
  RC: ["}"],
  STRUCT: ["struct"],
  RETURN: ["return"],
  IF: ["if"],
  ELSE: ["else"],
  WHILE: ["while"],
  ID: /[a-zA-Z_][a-zA-Z0-9_]*/,
};

const fs = require("node:fs");
const { isRegExp } = require("node:util/types");
const file = fs.readFileSync("generator/lexical.l").toString();
const [def, body, code] = file.split("%%");

function apply(name) {
  const send = [];
  if (name == "FLOAT") {
    send.push("cmm_send_yylval_float(atof(yytext));");
  } else if (name == "INT") {
    send.push("cmm_send_yylval_int(atoi(yytext));");
  } else if (name == "ID") {
    send.push("cmm_send_yylval_ident(yytext);");
  } else if (name == "TYPE") {
    send.push("cmm_send_yylval_type(yytext);");
  } else {
    send.push(`cmm_send_yylval_token("${name}");`);
  }
  return [...send, `cmm_send_yylval_loc(yylineno, 1);`, `return ${name};`];
}

const handled = Object.entries(DEFINATIONS).map(([kind, rule]) => {
  if (isRegExp(rule)) {
    return [rule.source, apply(kind)];
  } else {
    return [rule.map((x) => JSON.stringify(x)).join("|"), apply(kind)];
  }
});

const handled_max_len =
  handled.map((x) => x[0]).reduce((a, b) => Math.max(a, b.length), 0) + 1;

function fill_space(str, len) {
  while (str.length < len) {
    str += " ";
  }
  return str;
}

const ret = handled
  .map(([rule, body]) => {
    return [
      fill_space(rule, handled_max_len) + "{",
      ...body.map((x) => fill_space("", handled_max_len + 4) + x),
      fill_space("", handled_max_len) + "}",
    ];
  })
  .flat()
  .join("\n");

fs.writeFileSync(
  "Code/lexical.l",
  [def, [ret, body].join("\n\n"), code].join("\n%%\n")
);
