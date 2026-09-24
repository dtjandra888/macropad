import { readdir, readFile, writeFile } from "node:fs/promises";
import { join, relative, extname } from "node:path";

const DIST_DIR = "./dist";

// Change this to wherever your firmware expects fsdata.c.
// using dedicated fs directory for now
const OUTPUT_FILE = process.argv[2] ?? "./fs/fsdata.c";

interface FileEntry {
  path: string;
  data: Buffer;
  symbol: string;
  contentType: string;
}

const MIME_TYPES: Record<string, string> = {
  ".html": "text/html",
  ".css": "text/css",
  ".js": "application/javascript",
  ".json": "application/json",
  ".svg": "image/svg+xml",
  ".png": "image/png",
  ".jpg": "image/jpeg",
  ".jpeg": "image/jpeg",
  ".gif": "image/gif",
  ".ico": "image/x-icon",
  ".webp": "image/webp",
  ".woff": "font/woff",
  ".woff2": "font/woff2",
  ".ttf": "font/ttf",
};

async function collectFiles(dir: string): Promise<FileEntry[]> {
  const entries: FileEntry[] = [];

  async function walk(currentDir: string) {
    const items = await readdir(currentDir, { withFileTypes: true });

    for (const item of items) {
      const fullPath = join(currentDir, item.name);

      if (item.isDirectory()) {
        await walk(fullPath);
        continue;
      }

      if (!item.isFile()) {
        continue;
      }

      const relativePath = relative(DIST_DIR, fullPath)
        .split("\\")
        .join("/");

      const urlPath = "/" + relativePath;

      const symbol = "data_" + relativePath
        .replace(/[^a-zA-Z0-9_]/g, "_");

      const contentType =
        MIME_TYPES[extname(item.name).toLowerCase()] ??
        "application/octet-stream";

      entries.push({
        path: urlPath,
        data: await readFile(fullPath),
        symbol,
        contentType,
      });
    }
  }

  await walk(dir);

  return entries;
}

function escapeByte(byte: number): string {
  return `0x${byte.toString(16).padStart(2, "0")}`;
}

function generateDataArray(file: FileEntry): string {
  const pathBytes = Buffer.from(file.path + "\0", "utf8");

  const bytes = Buffer.concat([
    pathBytes,
    file.data,
  ]);

  const lines: string[] = [];

  for (let i = 0; i < bytes.length; i += 12) {
    const chunk = bytes.subarray(i, i + 12);

    lines.push(
      "    " +
      Array.from(chunk, escapeByte).join(", ") +
      ","
    );
  }

  return [
    `static const unsigned char ${file.symbol}[] = {`,
    `    /* ${file.path} */`,
    ...lines,
    `};`,
    "",
  ].join("\n");
}

function generateFileNode(
  file: FileEntry,
  nextSymbol: string | null,
): string {
  const pathLength = Buffer.byteLength(file.path, "utf8") + 1;

  const next = nextSymbol === null
    ? "NULL"
    : nextSymbol;

  return [
    `const struct fsdata_file ${file.symbol.replace("data_", "file_")}[] = {`,
    `    {${next}, ${file.symbol}, ${file.symbol} + ${pathLength},`,
    `     sizeof(${file.symbol}) - ${pathLength},`,
    `    0}`,
    `};`,
    "",
  ].join("\n");
}

async function main() {
  const files = await collectFiles(DIST_DIR);

  if (files.length === 0) {
    throw new Error(`No files found in ${DIST_DIR}`);
  }

  // Make index.html the root of the filesystem.
  files.sort((a, b) => {
    if (a.path === "/index.html") return -1;
    if (b.path === "/index.html") return 1;
    return a.path.localeCompare(b.path);
  });

  const output: string[] = [];

  output.push(
    `#include "fsdata.h"`,
    `#include "lwip/apps/httpd.h"`,
    `#include "lwip/opt.h"`,
    ``,
  );

  // Generate byte arrays.
  for (const file of files) {
    output.push(generateDataArray(file));
  }

  // Generate filesystem linked list.
  // reversed because the variables have dependencies
  // on each other
  for (let i = files.length - 1; i >= 0; i--) {
    const file = files[i];
    const next = files[i + 1];

    const fileSymbol = file.symbol.replace("data_", "file_");
    const nextSymbol = next
      ? next.symbol.replace("data_", "file_")
      : null;

    output.push(generateFileNode(file, nextSymbol));
  }

  const rootSymbol = files[0].symbol.replace("data_", "file_");

  output.push(
    `const struct fsdata_file *FS_ROOT = ${rootSymbol};`,
    ``,
    `#define FS_NUMFILES ${files.length}`,
    ``,
  );

  await writeFile(
    OUTPUT_FILE,
    output.join("\n"),
    "utf8",
  );

  console.log(`Generated ${OUTPUT_FILE}`);
  console.log(`Embedded ${files.length} files:`);

  for (const file of files) {
    console.log(`  ${file.path} (${file.data.length} bytes)`);
  }
}

await main();
