import path from 'path';
import fs from 'fs/promises';
import { patchConfig } from '../lib/config.js';

export const command = 'patch-config';
export const describe = 'Create a configuration overlay file from a JSON patch.';

export const builder = {
  patch: {
    describe: 'Path to the JSON patch file',
    type: 'string',
    demandOption: true,
  },
  dst: {
    default: 'src/config-user.h',
    describe: 'Path to the destination configuration file',
    type: 'string',
  }
};

export const handler = async (argv: { patch: string, src: string, dst: string }) => {
  try {
    const patchPath = path.resolve(argv.patch);
    //const srcPath = path.resolve(argv.src);
    const dstPath = path.resolve(argv.dst);

    const patchContent = await fs.readFile(patchPath, 'utf-8');
    const patch = JSON.parse(patchContent);

    await patchConfig(dstPath, patch);
    console.log(`Successfully created overlay file at ${dstPath}`);
  } catch (error) {
    if (error instanceof Error) {
        console.error(`Error applying patch: ${error.message}`);
    } else {
        console.error('An unknown error occurred while applying the patch.');
    }
  }
}; 