import { Logger } from 'tslog';

// Create a logger instance
export const logger = new Logger({
  name: 'AppLogger',
  // Optional: Configure minLevel, displayInstanceName, etc.
  minLevel: 2, // 0: silly, 1: trace, 2: debug, 3: info, 4: warn, 5: error, 6: fatal
}); 