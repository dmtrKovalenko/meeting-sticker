const fs = require("fs");
const path = require("path");
const child_process = require("child_process");

const FRAME_SIZE = 24;
const inputFile = process.argv[2];
const name = process.argv[3];

const TMP_DIR = "/tmp/var/gif_to_xbm";

if (fs.existsSync(TMP_DIR)) {
  fs.rmdirSync(TMP_DIR, { recursive: true });
}
fs.mkdirSync(TMP_DIR, { recursive: true });

if (!inputFile) {
  console.log("Please provide an input file");
  process.exit(1);
}

if (!name) {
  console.log("Please provide an output file name");
  process.exit(1);
}

child_process.execSync(
  `ffmpeg -i "${inputFile}" -vf "scale=${FRAME_SIZE}:${FRAME_SIZE}" ${TMP_DIR}/frame%03d.xbm`,
);

const dir = fs.readdirSync(TMP_DIR);
const length = (FRAME_SIZE * FRAME_SIZE) / 8;

let output = `#include <Arduino.h>
#include "animation.cpp"

#define ${name.toUpperCase()}_FRAME_COUNT ${dir.length}
#define ${name.toUpperCase()}_FRAME_SIZE ${FRAME_SIZE}
const byte PROGMEM ${name}_frames[][${length}] = {
`;

dir.forEach(function (file) {
  if (path.extname(file) !== ".xbm") {
    throw new Error(`File ${file} is not an XBM file`);
  }

  let xbmData = fs.readFileSync(path.join(TMP_DIR, file), "utf8");
  let arrayData = xbmData
    .split(/\r?\n/)
    .filter((line) => line.startsWith(" 0x"));

  // Add array to the output string, removing trailing commas
  output += `  {${arrayData.join("").replace(/, +$/g, "")}},\n`;
});

// Complete the array String and write to the file
output += "};\n";
output += `Animation<${length}> ${name}_animation(${length}, ${dir.length}, ${name}_frames);`;

fs.writeFileSync(path.join(__dirname, `src/animations/${name}.cpp`), output);
