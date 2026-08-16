import { useCallback } from 'react';
import modbusService from '@polymech/client-ts/modbusService';
import modbusApiService from '@polymech/client-ts/modbusApiService';
import {
  type SystemInfo,
  type WsStatus,
  type DisplayMessagePayload,
  type CoilData,
} from '@polymech/client-ts';
import { useWSModbus } from './WS_Modbus';
import logger from '@/Logger';
// No feature imports here; keep connection concerns isolated

const POLL_DATA = false;

export interface WSSocketHooks {
  connectToServer: (url?: string) => Promise<boolean>;
  disconnectFromServer: () => void;
  abortConnectionAttempt: () => void;
  connectWebSocket: (url: string) => Promise<boolean>;
  disconnectWebSocket: (intentional?: boolean) => void;
  handleWsStatusChange: (status: WsStatus) => void;
  handleWsLogMessage: (logEntries: any[]) => void;
}

export const useWSSocket = (
  apiUrl: string,
  setApiUrl: (url: string) => void,
  setIsConnected: (value: boolean) => void,
  setConnecting: (value: boolean) => void,
  setConnectionAborted: (value: boolean) => void,
  setWsStatus: (status: WsStatus) => void,
  setCoils: (coils: CoilData[]) => void,
  setSystemInfo: (info: SystemInfo | null) => void,
  wsModbus: ReturnType<typeof useWSModbus>,
  getProfiles: () => Promise<void>,
  getComponents: () => Promise<void>,
  getSettings: () => Promise<void>,
  addWsLogEntry: (level: string, message: string, timestamp?: number, id?: number, name?: string) => void,
  handleDisplayMessage: (payload: DisplayMessagePayload) => void,
): WSSocketHooks => {

  const handleSystemInfo = useCallback((info: SystemInfo) => {
    setSystemInfo(info);
  }, [setSystemInfo]);

  const handleWsLogMessage = useCallback((logEntries: any[]) => {
    if (!Array.isArray(logEntries)) {
      logger.warn("handleWsLogMessage received non-array data, this may indicate an issue.", logEntries);
      return;
    }
    for (const logData of logEntries) {
      const level = logData.level || 'Info';
      const message = typeof logData === 'string' ? logData : logData.message || JSON.stringify(logData);
      const timestamp = logData.timestamp || Date.now();
      addWsLogEntry(level, message, timestamp, logData.id, logData.name);
    }
  }, [addWsLogEntry]);

  const handleWsStatusChange = useCallback((status: WsStatus) => {
    logger.info(`Context: WS Status Changed -> ${status}`);
    setWsStatus(status);
    setIsConnected(status === 'CONNECTED');

    if (status === 'CONNECTED') {
       logger.success('WebSocket Connected');
    } else {
       if (status === 'ERROR') {
           logger.error('WebSocket Connection Error');
       } else if (status === 'RECONNECTING') {
           logger.warn('WebSocket Reconnecting...');
       }
    }
  }, [setWsStatus, setIsConnected]);

  const disconnectWebSocket = useCallback((intentional: boolean = true) => {
    addWsLogEntry('Info', `Requesting WebSocket disconnection (intentional: ${intentional})...`);
    modbusService.disconnect(intentional);
  }, [addWsLogEntry]);

  const connectWebSocket = useCallback(async (urlToConnect: string): Promise<boolean> => {
    if (!urlToConnect) {
      logger.error('API URL not set');
      return false;
    }
    let wsUrl: string;
    try {
      const url = new URL(urlToConnect);
      wsUrl = `${url.protocol === 'https:' ? 'wss:' : 'ws:'}//${url.host}/ws`;
    } catch (error) {
      logger.logError(error, 'Invalid API URL');
      return false;
    }
    try {
      const newConnectionEstablished = await modbusService.connect(
        wsUrl,
        handleWsStatusChange,
        handleWsLogMessage,
        wsModbus.handleRegisterData,
        wsModbus.handleRegisterUpdate,
        wsModbus.handleCoilData,
        wsModbus.handleCoilUpdate,
        handleDisplayMessage,
        handleSystemInfo
      );
      return newConnectionEstablished;
    } catch (error) { 
      logger.logError(error, '[ModbusContext] modbusService.connect() failed');
      return false;
    }
  }, [handleWsStatusChange, handleWsLogMessage, wsModbus, handleDisplayMessage, handleSystemInfo]);

  const abortConnectionAttempt = useCallback(() => {
    setConnectionAborted(true);
    setConnecting(false);
    disconnectWebSocket(true);
    logger.info('Connection attempt cancelled.');
  }, [setConnectionAborted, setConnecting, disconnectWebSocket]);

  const connectToServer = async (urlToUse?: string): Promise<boolean> => {
    const targetUrl = urlToUse || apiUrl;

    logger.info(`Connecting to server with URL: ${targetUrl}`);
    modbusApiService.setBaseUrl(targetUrl);

    if (urlToUse && urlToUse !== apiUrl) {
      setApiUrl(targetUrl);
    }

    setConnectionAborted(false);
    setConnecting(true);
    try {
      const didConnect = await connectWebSocket(targetUrl);

      if (didConnect) {
        logger.info('[ModbusContext] New WebSocket connection successful via connectWebSocket().');
        logger.success('WebSocket connection established.');        
        try {
          await wsModbus.requestCoils();
          await wsModbus.requestRegisters();          
          await getComponents();
          await getSettings();
          if(!POLL_DATA) {
            //return
          }

        } catch (fetchError: any) {
            logger.logError(fetchError, '[ModbusContext] Error during initial data fetches within connectToServer');
        }
      } else {
        if (modbusService.getConnectionStatus() === 'CONNECTED') {
          logger.info('[ModbusContext] WebSocket is already connected. Skipping re-initialization.');
        } else {
          logger.error('[ModbusContext] Failed to establish WebSocket connection.');
        }
      }
      
      const isNowConnected = modbusService.getConnectionStatus() === 'CONNECTED';
      
      setConnecting(false);
      return isNowConnected;

    } catch (error: any) {
      logger.logError(error, '[ModbusContext] Error in connectToServer');
      setIsConnected(false); 
      setConnecting(false);
      return false;
    }
  };

  const disconnectFromServer = (): void => {
    setConnectionAborted(true);
    setIsConnected(false);
    setSystemInfo(null);
    setCoils([]);
    disconnectWebSocket(true);
    logger.info('Disconnected from Modbus API');
  };

  return {
    connectToServer,
    disconnectFromServer,
    abortConnectionAttempt,
    connectWebSocket,
    disconnectWebSocket,
    handleWsStatusChange,
    handleWsLogMessage,
  };
}; 