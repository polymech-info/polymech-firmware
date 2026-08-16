import { useCallback, useRef } from 'react';
import {
  type SystemInfo,
  type LogEntry,
  type ComponentInfo,
  type Settings,
} from '@polymech/client-ts';
import modbusApiService from '@polymech/client-ts/modbusApiService';
import logger from '@/Logger';

export interface WSSystemHooks {
  fetchSystemInfo: () => Promise<void>;
  addWsLogEntry: (level: string, message: string, timestamp?: number, id?: number, name?: string) => void;
  clearWsLogs: () => void;
  getComponents: () => Promise<void>;
  getSettings: () => Promise<void>;
}

export const useWSSystem = (
  setSystemInfo: React.Dispatch<React.SetStateAction<SystemInfo | null>>,
  setWsLogEntries: React.Dispatch<React.SetStateAction<LogEntry[]>>,
  setComponents: React.Dispatch<React.SetStateAction<ComponentInfo[]>>,
  setSettings: React.Dispatch<React.SetStateAction<Settings | null>>
): WSSystemHooks => {
  const messageIdCounter = useRef<number>(0);

  const addWsLogEntry = useCallback((
    level: string, 
    message: string, 
    timestamp: number = Date.now(),
    id?: number,
    name?: string
  ) => {
    setWsLogEntries(prev => {
      const MAX_LOG_ENTRIES = 1000;
      const newLog: LogEntry = { 
        logId: messageIdCounter.current++, 
        timestamp, 
        level, 
        message,
        id,
        name
      };
      const updatedLogs = [...prev, newLog];
      if (updatedLogs.length > MAX_LOG_ENTRIES) {
        return updatedLogs.slice(updatedLogs.length - MAX_LOG_ENTRIES);
      }
      return updatedLogs;
    });
  }, [setWsLogEntries]);

  const clearWsLogs = useCallback(() => {
    setWsLogEntries([]);
    addWsLogEntry('Info', 'Client-side logs cleared.', Date.now());
  }, [setWsLogEntries, addWsLogEntry]);

  const fetchSystemInfo = useCallback(async (): Promise<void> => {
    try {
      const info = await modbusApiService.getSystemInfo();
      setSystemInfo(info);
    } catch (error) {
        logger.logError(error, 'Error fetching system info during poll');
    }
  }, [setSystemInfo]);

  const getComponents = useCallback(async (): Promise<void> => {
    try {
      const componentData = await modbusApiService.getComponents();
      setComponents(componentData);
    } catch (error) {
      logger.logError(error, 'Error fetching components');
    }
  }, [setComponents]);

  const getSettings = useCallback(async (): Promise<void> => {
    try {
      const settingsData = await modbusApiService.getSettings();
      setSettings(settingsData);
    } catch (error) {
      logger.logError(error, 'Error fetching cassandra settings');
    }
  }, [setSettings]);

  return {
    fetchSystemInfo,
    addWsLogEntry,
    clearWsLogs,
    getComponents,
    getSettings,
  };
}; 