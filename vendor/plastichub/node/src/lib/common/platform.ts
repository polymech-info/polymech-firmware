import { platform, arch } from 'os';

export const os = () => {
  if (platform() === 'win32') {
    return 'windows';
  } else if (platform() === 'darwin') {
    return 'osx';
  } else if (arch() === 'arm') {
    return 'arm';
  }
  return 'linux';
}