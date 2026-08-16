export interface ParsedRegister {
  mainName: string;
  enumValues: { val: number; label: string }[];
}

export function parseRegisterName(name: string): ParsedRegister | null {
  const startIndex = name.indexOf('(');
  const endIndex = name.lastIndexOf(')');

  if (startIndex === -1 || endIndex === -1 || startIndex > endIndex) {
    return null;
  }

  const mainName = name.substring(0, startIndex);
  const enumString = name.substring(startIndex + 1, endIndex);

  if (!enumString) {
     return null;
  }

  try {
    const enumValues = enumString.split(',').map(pair => {
      const separatorIndex = pair.indexOf(':');
      if (separatorIndex === -1) {
          throw new Error("Invalid enum pair format");
      }
      const valStr = pair.substring(0, separatorIndex).trim();
      const label = pair.substring(separatorIndex + 1).trim();
      const val = parseInt(valStr, 10);
      if(isNaN(val)) {
          throw new Error("Invalid value in enum pair");
      }
      return { val, label };
    });

    if (enumValues.length === 0) return null;

    return { mainName, enumValues };
  } catch (error) {
    // console.error('Error parsing enum register name:', name, error);
    return null;
  }
} 