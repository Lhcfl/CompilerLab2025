const fs = require("node:fs");
const file = fs.readFileSync("Code/syntax.txt").toString();

const [def, body, code] = file.split("%%");

let bkg = "";

function genList(count) {
  let str = `{ $$ = cmm_node_tree("${bkg}", ${count}`;
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
  if (line.match(/^[\s]+\|/) || line.match(/^[A-Z][a-zA-z]+/)) {
    const idents = line.matchAll(/[A-Z][A-Za-z]+/g);
    const comma = line.includes(":");
    if (comma) {
      bkg = line.split(":")[0].trim();
    }
    const len = [...idents].length - comma;
    if (len > 0) {
      return [line, genList(len)];
    } else {
      return [line, `{ $$ = cmm_empty_tree("${bkg}"); }`];
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

fs.writeFileSync("Code/syntax.y", [def, body, code].join("\n%%\n"));
