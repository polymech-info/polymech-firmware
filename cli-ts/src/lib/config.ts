import fs from 'fs/promises';

interface ConfigPatch {
  features?: { [key: string]: boolean };
  settings?: { [key: string]: string | number };
}

export async function patchConfig(dst: string, patch: ConfigPatch) {
  const newLines = [
    '#ifndef CONFIG_USER_H',
    '#define CONFIG_USER_H',
    ''
  ];

  if (patch.features) {
    for (const [name, enabled] of Object.entries(patch.features)) {
      if (enabled) {
        newLines.push(`#define ${name}`);
      } else {
        newLines.push(`#undef ${name}`);
      }
    }
  }

  if (patch.settings && Object.keys(patch.settings).length > 0) {
    newLines.push('');
    for (const [name, value] of Object.entries(patch.settings)) {
      const valueFormatted = typeof value === 'string' ? `"${value}"` : String(value);
      // Undefine the original before defining the new one to avoid compiler warnings.
      newLines.push(`#undef ${name}`);
      newLines.push(`#define ${name} ${valueFormatted}`);
    }
  }

  newLines.push('');
  newLines.push('#endif // CONFIG_USER_H');

  await fs.writeFile(dst, newLines.join('\n'));
} 