import type { Argv } from "yargs";
import path from "path";
import fs from "fs/promises";
import { log } from "../lib/logger.js";

export const command = "build-proto";
export const desc = "Copies .proto files to the client app for dynamic loading.";

const REPO_ROOT = path.join(path.resolve("./"));

// --- Configuration ---
const CLIENT_PROTO_DEST_DIR = path.resolve(REPO_ROOT, "../web/packages/modbus-ui/src/proto");
const PROTO_SRC_DIR = path.join(REPO_ROOT, "proto");

console.log("REPO_ROOT", REPO_ROOT);
console.log("CLIENT_PROTO_DEST_DIR", CLIENT_PROTO_DEST_DIR);
const cmd = `pbjs -t static-module -w commonjs -o ${CLIENT_PROTO_DEST_DIR}/bundle.js ${PROTO_SRC_DIR}/bundle.proto`;
export const handler = async (): Promise<void> => {
  try {
    log.info(`Converting .proto files from '${PROTO_SRC_DIR}' to '${CLIENT_PROTO_DEST_DIR}'`);

    await fs.mkdir(CLIENT_PROTO_DEST_DIR, { recursive: true });

    const protoFiles = (await fs.readdir(PROTO_SRC_DIR)).filter(f => f.endsWith(".proto"));
    if (protoFiles.length === 0) {
      log.warn("No .proto files found to copy.");
      return;
    }

    for (const file of protoFiles) {
      const srcPath = path.join(PROTO_SRC_DIR, file);
      const destPath = path.join(CLIENT_PROTO_DEST_DIR, file);
      console.log("srcPath", srcPath);
      console.log("destPath", destPath);
    }

    log.info("\nSuccessfully copied all .proto files to the client!");
  } catch (error) {
    log.error("\nFailed to convert .proto files.", error);
    process.exit(1);
  }
}; 