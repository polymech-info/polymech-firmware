import { toast } from 'sonner';
import React from 'react';

export type LogLevel = 'info' | 'warn' | 'error' | 'success' | 'debug';

export interface LogEntry {
  level: LogLevel;
  message: string;
  timestamp: number;
  data?: any;
}

export interface LoggerConfig {
  enableConsole: boolean;
  enableToaster: boolean;
  deduplicationWindow: number; // ms
}

class Logger {
  private static instance: Logger;
  private config: LoggerConfig;
  private recentLogs: Map<string, number> = new Map();

  private constructor() {
    this.config = {
      enableConsole: true,
      enableToaster: true,
      deduplicationWindow: 5000, // 5 seconds
    };
  }

  public static getInstance(): Logger {
    if (!Logger.instance) {
      Logger.instance = new Logger();
    }
    return Logger.instance;
  }

  public configure(config: Partial<LoggerConfig>): void {
    this.config = { ...this.config, ...config };
  }

  private isDuplicate(message: string): boolean {
    const now = Date.now();
    const lastLogTime = this.recentLogs.get(message);
    
    if (lastLogTime && (now - lastLogTime) < this.config.deduplicationWindow) {
      return true;
    }
    
    this.recentLogs.set(message, now);
    
    // Clean up old entries
    for (const [msg, timestamp] of this.recentLogs.entries()) {
      if (now - timestamp > this.config.deduplicationWindow) {
        this.recentLogs.delete(msg);
      }
    }
    
    return false;
  }

  private logToConsole(level: LogLevel, message: string, data?: any): void {
    if (!this.config.enableConsole) return;

    const timestamp = new Date().toISOString();
    const prefix = `[${timestamp}] [${level.toUpperCase()}]`;
    
    switch (level) {
      case 'error':
        console.error(prefix, message, data || '');
        break;
      case 'warn':
        console.warn(prefix, message, data || '');
        break;
      case 'debug':
        console.debug(prefix, message, data || '');
        break;
      case 'info':
      case 'success':
      default:
        console.log(prefix, message, data || '');
        break;
    }
  }

  private logToToaster(level: LogLevel, message: string): void {
    if (!this.config.enableToaster) return;

    const toastId = toast(
      React.createElement(
        'div',
        {
          onClick: () => toast.dismiss(toastId),
          style: { cursor: 'pointer', width: '100%' },
        },
        message
      )
    );

    switch (level) {
      case 'error':
        toast.error(message, { id: toastId });
        break;
      case 'warn':
        toast.warning(message, { id: toastId });
        break;
      case 'success':
        toast.success(message, { id: toastId });
        break;
      case 'info':
        toast.info(message, { id: toastId });
        break;
      case 'debug':
        // Don't show debug messages in toaster by default
        toast.dismiss(toastId);
        break;
    }
  }

  public log(level: LogLevel, message: string, data?: any): void {
    if (this.isDuplicate(message)) {
      return;
    }

    this.logToConsole(level, message, data);
    this.logToToaster(level, message);
  }

  public info(message: string, data?: any): void {
    this.log('info', message, data);
  }

  public warn(message: string, data?: any): void {
    this.log('warn', message, data);
  }

  public error(message: string, data?: any): void {
    this.log('error', message, data);
  }

  public success(message: string, data?: any): void {
    this.log('success', message, data);
  }

  public debug(message: string, data?: any): void {
    this.log('debug', message, data);
  }

  // Convenience methods for common patterns
  public logError(error: unknown, context?: string): void {
    const message = context 
      ? `${context}: ${error instanceof Error ? error.message : String(error)}`
      : error instanceof Error ? error.message : String(error);
    
    this.error(message, error instanceof Error ? error.stack : undefined);
  }

  public logApiError(operation: string, error: unknown): void {
    this.logError(error, `Error during ${operation}`);
  }

  public logWebSocketError(operation: string, error: unknown): void {
    this.logError(error, `WebSocket error during ${operation}`);
  }

  // Method to temporarily disable toaster (useful for batch operations)
  public withoutToaster<T>(fn: () => T): T {
    const originalToasterState = this.config.enableToaster;
    this.config.enableToaster = false;
    try {
      return fn();
    } finally {
      this.config.enableToaster = originalToasterState;
    }
  }
}

// Export singleton instance
export const logger = Logger.getInstance();
export default logger; 