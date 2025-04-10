const fs = require("node:fs");
const file = fs.readFileSync("generator/syntax.y").toString();

const [def, body, code] = file.split("%%");
/** @type {Set<string>} */
const nodenames = new Set();

[...def.matchAll(/%token ([A-Z]+)/g)].map((x) => nodenames.add(x[1]));

let bkg = "";

function genList(count) {
  bkgname = bkg.match(/[A-Z][a-zA-z]+/)[0];
  let str = `{ $$ = cmm_node_tree(CMM_TK_${bkgname}, ${count}`;
  for (let i = 1; i <= count; i++) {
    str += ", $";
    str += i;
  }
  str += "); ";
  if (bkg === "Program") {
    str += "cmm_parsed_root = $$; ";
  }
  str += "}";
  return str;
}

const lines = body.split("\n").map((line) => {
  if (line.match(/^[\s]+\|/) || line.match(/^[A-Z][a-zA-z0-9]+/)) {
    const idents = line.matchAll(/[A-Z][A-Za-z0-9]+/g);
    const comma = line.includes(":");
    if (comma) {
      bkg = line.split(":")[0].trim();
    }
    const idents_arr = [...idents];
    const len = idents_arr.length - comma;
    idents_arr.forEach((x) => nodenames.add(x[0]));
    if (len > 0) {
      return [line, genList(len)];
    } else {
      return [line, `{ $$ = cmm_empty_tree(CMM_TK_${bkg}); }`];
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

fs.writeFileSync("Code/syntax.y", [def, ret, code].join("\n%%\n"));

const node_names = [...nodenames];

fs.writeFileSync(
  "Code/syndef.h",
  `
#ifndef LINCA_BYYL_SYNTAX_PREDEFINES
#define LINCA_BYYL_SYNTAX_PREDEFINES

enum CMM_SYNTAX_TOKEN {
${node_names.map((x) => `    CMM_TK_${x},`).join("\n")}
};

char* cmm_token_tostring(enum CMM_SYNTAX_TOKEN token);

#endif
`
);

fs.writeFileSync(
  "Code/syndef.c",
  `
#include "syndef.h"

char* cmm_token_tostring(enum CMM_SYNTAX_TOKEN token) {
    switch (token) {
${node_names.map((x) => `        case CMM_TK_${x}: return "${x}";`).join("\n")}
    }
    return "unexpected";
}
`
);
