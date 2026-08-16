import React, { createContext, useContext, useEffect, useState, useRef, useCallback } from 'react'
import { toast } from 'sonner'

import { type SystemInfo, type ProfileSavePayload } from '@polymech/client-ts'

import
{ 
  type RegisterData, 
  type RegisterUpdatePayload,
  type CoilData,
  type CoilUpdatePayload,
  type LogEntry,
  type WsStatus,
  type DisplayMessagePayload,
  E_FN_CODE
} from "@polymech/client-ts"

import modbusService from '@polymech/client-ts/modbusService'
import modbusApiService from '@polymech/client-ts/modbusApiService'

import modbusServiceMocked from '@polymech/client-ts/modbusServiceMocked'
import modbusApiServiceMocked from '@polymech/client-ts/modbusApiServiceMocked'

import { REST_POLLING_INTERVAL_MS, WS_REGISTER_POLL_INTERVAL_MS } from '@/constants';
import { Profile, ControlPoint } from '@/types';

const POLL_DATA = false

export { type RegisterData, type RegisterUpdatePayload, type CoilData };

// Define the target registers and history duration
const REGISTERS_TO_CHART = [1017, 1033];
const HISTORY_DURATION_MS = 60 * 10 * 1000; // 10 minutes

interface RegisterHistoryPoint {
  timestamp: number;
  value: number;
}

interface ModbusContextType {
  isConnected: boolean;
  apiUrl: string;
  coils: CoilData[];
  registers: RegisterData[];
  profiles: Profile[];
  coilStartAddress: number;
  registerStartAddress: number;
  coilCount: number;
  registerCount: number;
  connecting: boolean;
  systemInfo: SystemInfo | null;
  setApiUrl: (url: string) => void;
  connectToServer: () => Promise<boolean>;
  disconnectFromServer: () => void;
  updateCoil: (address: number, value: boolean) => Promise<void>;
  updateRegister: (address: number, value: number) => Promise<void>;
  writeRegister: (registerId: number, value: number) => Promise<void>;
  refreshData: () => Promise<void>;
  fetchSystemInfo: () => Promise<void>;
  setCoilStartAddress: (address: number) => void;
  setRegisterStartAddress: (address: number) => void;
  setCoilCount: (count: number) => void;
  setRegisterCount: (count: number) => void;
  testRelays: () => Promise<void>;
  wsStatus: WsStatus;
  wsLogEntries: LogEntry[];
  connectWebSocket: () => Promise<boolean>;
  disconnectWebSocket: (intentional?: boolean) => void;
  clearWsLogs: () => void;
  autoRefreshLogs: boolean;
  logRefreshIntervalMs: number;
  setAutoRefreshLogs: (enabled: boolean) => void;
  setLogRefreshIntervalMs: (interval: number) => void;
  requestLogs: () => void;
  autoRefreshRegisters: boolean;
  setAutoRefreshRegisters: (enabled: boolean) => void;
  requestRegisters: () => Promise<void>;
  autoRefreshCoils: boolean;
  setAutoRefreshCoils: (enabled: boolean) => void;
  requestCoils: () => Promise<void>;
  autoRefreshSystemInfo: boolean;
  setAutoRefreshSystemInfo: (enabled: boolean) => void;
  systemInfoRefreshIntervalMs: number;
  setSystemInfoRefreshIntervalMs: (interval: number) => void;
  getProfiles: () => Promise<void>;
  saveProfile: (profileData: ProfileSavePayload) => Promise<any>;
  registerHistory: Record<number, RegisterHistoryPoint[]>;
  displayMessages: DisplayMessagePayload[];
  removeDisplayMessage: (id: string | number) => void;
  clearDisplayMessages: () => void;
  saveMultipleProfiles: (profiles: ProfileSavePayload[]) => Promise<void>;
}
const ModbusContext = createContext<ModbusContextType | undefined>(undefined);

const LOG_LEVEL_MAP: { [key: string]: string } = {
  'F': 'Fatal',
  'E': 'Error',
  'W': 'Warning',
  'I': 'Info',
  'D': 'Debug',
  'T': 'Trace',
  'V': 'Verbose',
};

const getLogLevelFromLine = (logLine: string): string => {
  const prefix = logLine.substring(0, 1);
  return LOG_LEVEL_MAP[prefix] || 'Unknown';
};

export const ModbusProvider: React.FC<{ children: React.ReactNode }> = ({ children }) => {
  // State declarations
  const [isConnected, setIsConnected] = useState(false);
  const [connecting, setConnecting] = useState(false);
  const [apiUrl, setApiUrl] = useState(() => {
    const savedUrl = localStorage.getItem('apiUrl');
    return savedUrl || 'http://192.168.1.250';
  });
  const [coils, setCoils] = useState<CoilData[]>([]);
  const [registers, setRegisters] = useState<RegisterData[]>([]);
  const [profiles, setProfiles] = useState<Profile[]>([]);
  const [coilStartAddress, setCoilStartAddress] = useState(0);
  const [registerStartAddress, setRegisterStartAddress] = useState(0);
  const [coilCount, setCoilCount] = useState(100);
  const [registerCount, setRegisterCount] = useState(200);
  const [systemInfo, setSystemInfo] = useState<SystemInfo | null>(null);
  const [wsStatus, setWsStatus] = useState<WsStatus>('DISCONNECTED');
  const [wsLogEntries, setWsLogEntries] = useState<LogEntry[]>([]);
  const [autoRefreshLogs, setAutoRefreshLogs] = useState<boolean>(false);
  const [logRefreshIntervalMs, setLogRefreshIntervalMs] = useState<number>(5000);
  const [autoRefreshRegisters, setAutoRefreshRegisters] = useState<boolean>(true);
  const [autoRefreshCoils, setAutoRefreshCoils] = useState<boolean>(true);
  const [autoRefreshSystemInfo, setAutoRefreshSystemInfo] = useState<boolean>(true);
  const [systemInfoRefreshIntervalMs, setSystemInfoRefreshIntervalMs] = useState<number>(3500);
  const [registerHistory, setRegisterHistory] = useState<Record<number, RegisterHistoryPoint[]>>({});
  const [displayMessages, setDisplayMessages] = useState<DisplayMessagePayload[]>([
    { id: 'initial-welcome', message: 'Welcome to Cassandra', timestamp: Date.now() }
  ]);

  // Refs
  const messageIdCounter = useRef<number>(0);
  const logRefreshTimerRef = useRef<NodeJS.Timeout | null>(null);
  const registerRefreshTimerRef = useRef<NodeJS.Timeout | null>(null);
  const coilRefreshTimerRef = useRef<NodeJS.Timeout | null>(null);
  const systemInfoRefreshTimerRef = useRef<NodeJS.Timeout | null>(null);

  // Coil-related handlers and callbacks
  const addWsLogEntry = useCallback((level: string, message: string, timestamp: number = Date.now()) => {
    setWsLogEntries(prev => {
      const MAX_LOG_ENTRIES = 1000;
      const newLog = { id: messageIdCounter.current++, timestamp, level, message };
      const updatedLogs = [...prev, newLog];
      if (updatedLogs.length > MAX_LOG_ENTRIES) {
        return updatedLogs.slice(updatedLogs.length - MAX_LOG_ENTRIES);
      }
      return updatedLogs;
    });
  }, []);

  const stopCoilRefreshTimer = useCallback(() => {
    if (coilRefreshTimerRef.current) {
      clearInterval(coilRefreshTimerRef.current);
      coilRefreshTimerRef.current = null;
    }
  }, []);

  const requestCoils = useCallback(async (): Promise<void> => {
    if (modbusService.getConnectionStatus() === 'CONNECTED') {
      try {
        await modbusService.requestCoils();
      } catch (wsError) {
        console.error('Error requesting full coil list:', wsError);
        toast.error('Error fetching coils: ' + (wsError instanceof Error ? wsError.message : String(wsError)));
      }
    } else {
      console.warn('Skipping coil request: WebSocket not connected.');
    }
  }, []);

  const handleCoilData = useCallback((allCoils: CoilData[]) => {
    setCoils(allCoils);
  }, []);

  const handleCoilUpdate = useCallback((update: CoilUpdatePayload) => {
    setCoils(prevCoils => {
      const index = prevCoils.findIndex(coil => coil.address === update.address);
      if (index === -1) {
        console.warn(`Coil update received for unknown address: ${update.address}`);
        return prevCoils;
      }
      const newCoils = [...prevCoils];
      newCoils[index] = { ...newCoils[index], value: update.value };
      return newCoils;
    });
  }, []);

  const removeDisplayMessage = useCallback((id: string | number) => {
    setDisplayMessages(prevMessages => prevMessages.filter(msg => msg.id !== id));
  }, []);

  const clearDisplayMessages = useCallback(() => {
    setDisplayMessages([]);
  }, []);

  const handleDisplayMessage = useCallback((payload: DisplayMessagePayload) => {
    setDisplayMessages(prevMessages => {
      const index = prevMessages.findIndex(msg => msg.id === payload.id);
      if (index !== -1) {
        const newMessages = [...prevMessages];
        newMessages[index] = payload;
        return newMessages;
      } else {
        return [...prevMessages, payload];
      }
    });
  }, []);

  const startCoilRefreshTimer = useCallback(() => {
    stopCoilRefreshTimer();
    if (modbusService.getConnectionStatus() === 'CONNECTED') {
      requestCoils();
      coilRefreshTimerRef.current = setInterval(requestCoils, WS_REGISTER_POLL_INTERVAL_MS);
    } else {
      addWsLogEntry('Warning', 'Cannot start coil polling: WebSocket not connected.', Date.now());
    }
  }, [stopCoilRefreshTimer, requestCoils, addWsLogEntry]);

  useEffect(() => {
    modbusApiService.setBaseUrl(apiUrl);
    localStorage.setItem('apiUrl', apiUrl);
  }, [apiUrl]);

  const getProfiles = useCallback(async (): Promise<void> => {
    try {
      const response = await modbusApiService.getProfiles();
      const mappedProfiles: Profile[] = response.profiles.map((p: any) => ({
        ...p,
        enabled: p.enabled !== undefined ? p.enabled : false, 
        elapsed: p.elapsed !== undefined ? p.elapsed : 0,      
        remaining: p.remaining !== undefined ? p.remaining : 0,  
        slot: p.slot !== undefined ? p.slot : 0, 
        name: p.name || "Unnamed Profile",
        duration: p.duration !== undefined ? p.duration : 0,
        status: p.status !== undefined ? p.status : 0, 
        currentTemp: p.currentTemp !== undefined ? p.currentTemp : 0,
        max: p.max !== undefined ? p.max : 0,
        controlPoints: (p.controlPoints || []).map((cp: any) => ({ x: cp.x || 0, y: cp.y || 0 })) as ControlPoint[],
        targetRegisters: p.targetRegisters || [],
      }));
      setProfiles(mappedProfiles);
    } catch (error) {
      console.error('Error fetching profiles:', error);
      toast.error('Error fetching profiles: ' + (error instanceof Error ? error.message : String(error)));
    }
  }, [apiUrl]); 

  const handleWsLogMessage = useCallback((logData: any) => {
    const level = logData.level || 'Info';
    const message = typeof logData === 'string' ? logData : logData.message || JSON.stringify(logData);
    const timestamp = logData.timestamp || Date.now();
    addWsLogEntry(level, message, timestamp);
  }, [addWsLogEntry]);
  
  const handleRegisterData = useCallback((allRegisters: RegisterData[]) => {
    
    setRegisters(allRegisters); // Re-enable setting registers from WebSocket
  }, []);

  const handleRegisterUpdate = useCallback((update: RegisterUpdatePayload) => {
     setRegisters(prevRegisters => {
         // Handle read register update (FN_READ_HOLD_REGISTER) - ignore count field
         if (update.fc === E_FN_CODE.FN_READ_HOLD_REGISTER && typeof update.value === 'number') {
             const index = prevRegisters.findIndex(reg => reg.address === update.address);
             if (index === -1) {
                 return prevRegisters;
             }
             const newRegisters = [...prevRegisters];
             newRegisters[index] = { ...newRegisters[index], value: update.value }; 
             return newRegisters;
         }
         // Handle single register update (FN_WRITE_HOLD_REGISTER)
         else if (update.fc === E_FN_CODE.FN_WRITE_HOLD_REGISTER && typeof update.value === 'number') {
             const index = prevRegisters.findIndex(reg => reg.address === update.address);
             if (index === -1) {
                 return prevRegisters;
             }
             const newRegisters = [...prevRegisters];
             newRegisters[index] = { ...newRegisters[index], value: update.value }; 
             return newRegisters;
         }
         // Handle multiple register update (FN_WRITE_MULT_REGISTERS)
         else if (update.fc === E_FN_CODE.FN_WRITE_MULT_REGISTERS) {
             const updateData = update as any;
             
             // Validate required fields for multiple register update
             if (!Array.isArray(updateData.values) || typeof updateData.count !== 'number') {
                 console.warn('Invalid multiple register update data:', update);
                 return prevRegisters;
             }
             
             // Validate count matches values array length
             if (updateData.count > updateData.values.length) {
                 return prevRegisters;
             }
             
             const newRegisters = [...prevRegisters];
             let updated = false;
             
             for (let i = 0; i < updateData.values.length && i < updateData.count; i++) {
                 const registerAddress = update.address + i;
                 const registerValue = updateData.values[i];
                 if (typeof registerValue !== 'number') {
                     continue;
                 }                 
                 const index = newRegisters.findIndex(reg => reg.address === registerAddress);
                 if (index !== -1) {
                     newRegisters[index] = { ...newRegisters[index], value: registerValue };
                     updated = true;
                 } else {
                     console.warn(`Register update received for unknown address: ${registerAddress}`);
                 }
             }
             return updated ? newRegisters : prevRegisters;
         }
         else {
             console.warn('Invalid register update FC or missing data:', update);
         }
         
         return prevRegisters;
     });

     /*
     if (REGISTERS_TO_CHART.includes(update.address)) {
         setRegisterHistory(prevHistory => {
             const now = Date.now();
             const sixtyMinutesAgo = now - HISTORY_DURATION_MS;
             const currentHistory = prevHistory[update.address] || [];
             const newHistory = [
                 ...currentHistory.filter(point => point.timestamp >= sixtyMinutesAgo),
                 { timestamp: now, value: update.value }
             ];
             return {
                 ...prevHistory,
                 [update.address]: newHistory
             };
         });
     }
     */
  }, []);

  const requestLogs = useCallback(() => {
      modbusService.requestLogs();
  }, []);

  const stopLogRefreshTimer = useCallback(() => {
      if (logRefreshTimerRef.current) {
          clearInterval(logRefreshTimerRef.current);
          logRefreshTimerRef.current = null;
      }
  }, []);

  const startLogRefreshTimer = useCallback(() => {
      stopLogRefreshTimer();
      if (modbusService.getConnectionStatus() === 'CONNECTED') {
          requestLogs();
          logRefreshTimerRef.current = setInterval(requestLogs, logRefreshIntervalMs);
      } else {
           addWsLogEntry('Warning', 'Cannot start log refresh: WebSocket not connected.', Date.now());
      }
  }, [stopLogRefreshTimer, requestLogs, logRefreshIntervalMs, addWsLogEntry]);

  const requestRegisters = useCallback(async (): Promise<void> => {
    if (modbusService.getConnectionStatus() === 'CONNECTED') {
        try {
            await modbusService.getRegisters();
        } catch (wsError) {
             console.error('Error requesting full register list via requestRegisters:', wsError);
             toast.error('Error fetching registers: ' + (wsError instanceof Error ? wsError.message : String(wsError)));
        }
    } else {
        console.warn('Skipping register request in requestRegisters: WebSocket not connected.');
    }
  }, []); // Dependencies: if modbusService could change, add it. For now, empty.

  const stopRegisterRefreshTimer = useCallback(() => {
    if (registerRefreshTimerRef.current) {
        clearInterval(registerRefreshTimerRef.current);
        registerRefreshTimerRef.current = null;
    }
  }, []);
  
  const startRegisterRefreshTimer = useCallback(() => {
    stopRegisterRefreshTimer(); 
    if (isConnected && modbusService.getConnectionStatus() === 'CONNECTED') { // Check both context and service status
        requestRegisters(); // Initial fetch for the polling cycle
        registerRefreshTimerRef.current = setInterval(requestRegisters, WS_REGISTER_POLL_INTERVAL_MS);
    } else {
        addWsLogEntry('Warning', 'Cannot start register polling: WebSocket not connected.', Date.now());
    }
  }, [isConnected, requestRegisters, stopRegisterRefreshTimer, addWsLogEntry]);

  // New SystemInfo specific functions
  const fetchSystemInfo = useCallback(async (): Promise<void> => {
    if (modbusService.getConnectionStatus() !== 'CONNECTED' && !isConnected) { // Check both ws and context state
        console.warn('Skipping system info fetch: Not connected.');
        return;
    }
    try {
      const info = await modbusApiService.getSystemInfo();
      setSystemInfo(info);
    } catch (error) {
      console.error('Error fetching system info during poll:', error);
      toast.error('Error fetching system info: ' + (error instanceof Error ? error.message : String(error)));
    }
  }, [isConnected]); // Depends on isConnected from context to re-evaluate if connection state changes

  const stopSystemInfoRefreshTimer = useCallback(() => {
    if (systemInfoRefreshTimerRef.current) {
      clearInterval(systemInfoRefreshTimerRef.current);
      systemInfoRefreshTimerRef.current = null;
    }
  }, []);

  const startSystemInfoRefreshTimer = useCallback(() => {
    stopSystemInfoRefreshTimer();
    if (isConnected) { // Use context's isConnected state
      fetchSystemInfo(); // Initial fetch
      systemInfoRefreshTimerRef.current = setInterval(fetchSystemInfo, systemInfoRefreshIntervalMs);
    } else {
      addWsLogEntry('Warning', 'Cannot start system info polling: Not connected.', Date.now());
    }
  }, [isConnected, fetchSystemInfo, systemInfoRefreshIntervalMs, stopSystemInfoRefreshTimer, addWsLogEntry]);
 
  const handleWsStatusChange = useCallback((status: WsStatus) => {
    console.log('Context: WS Status Changed ->', status);
    setWsStatus(status);
    setIsConnected(status === 'CONNECTED');

    if (status === 'CONNECTED') {
       toast.success('WebSocket Connected');
    } else {
       stopLogRefreshTimer();
       stopRegisterRefreshTimer(); // Ensure this is called on disconnect/error
       stopCoilRefreshTimer();
       stopSystemInfoRefreshTimer();
       if (status === 'ERROR') {
           toast.error('WebSocket Connection Error');
       } else if (status === 'RECONNECTING') {
           toast.warning('WebSocket Reconnecting...');
       }
    }
  }, [stopLogRefreshTimer, stopRegisterRefreshTimer, stopCoilRefreshTimer, stopSystemInfoRefreshTimer]); 

  const connectWebSocket = useCallback(async (): Promise<boolean> => {
    if (!apiUrl) {
      console.error('API URL not set');
      return false;
    }
    let wsUrl: string;
    try {
      const url = new URL(apiUrl);
      wsUrl = `${url.protocol === 'https:' ? 'wss:' : 'ws:'}//${url.host}/ws`;
    } catch (error) {
      console.error('Invalid API URL:', error);
      return false;
    }
    try {
      await modbusService.connect(
        wsUrl,
        handleWsStatusChange,
        handleWsLogMessage,
        handleRegisterData,
        handleRegisterUpdate,
        handleCoilData,
        handleCoilUpdate,
        handleDisplayMessage
      );
      return true;
    } catch (error) { 
      console.error('[ModbusContext] modbusService.connect() failed:', error);
      return false;
    }
  }, [apiUrl, handleWsStatusChange, handleWsLogMessage, handleRegisterData, handleRegisterUpdate, handleCoilData, handleCoilUpdate, handleDisplayMessage]);

  const connectToServer = async (): Promise<boolean> => {
    setConnecting(true);
    try {
      const didConnect = await connectWebSocket();

      if (didConnect) {
        console.log('[ModbusContext] WebSocket connection successful via connectWebSocket().');
        toast.success('WebSocket connection established.');        
        try {
          await requestCoils();
          
          await requestRegisters();          
          await getProfiles();
          await fetchSystemInfo();
          if(!POLL_DATA) {
            //return
          }
          //if (autoRefreshLogs) startLogRefreshTimer();
          //if (autoRefreshRegisters) startRegisterRefreshTimer();
          //if (autoRefreshCoils) startCoilRefreshTimer();
          if (autoRefreshSystemInfo) startSystemInfoRefreshTimer(); // Start the new system info timer

        } catch (fetchError: any) {
          console.error('[ModbusContext] Error during initial data fetches within connectToServer:', fetchError);
          toast.error('Error fetching initial data: ' + (fetchError.message || String(fetchError)));
        }

        setConnecting(false);
        return true;
      } else {
        console.log('[ModbusContext] WebSocket connection failed via connectWebSocket().');
        toast.error('Failed to establish WebSocket connection.');
        setConnecting(false);
        return false;
      }
    } catch (error: any) {
      console.error('[ModbusContext] Error in connectToServer:', error);
      toast.error('Connection error: ' + (error instanceof Error ? error.message : String(error)));
      setIsConnected(false); 
      setConnecting(false);
      return false;
    }
  };

  const disconnectFromServer = (): void => {
    setIsConnected(false);
    setSystemInfo(null);
    setCoils([]);
    // setRegisters([]); // Registers disabled by user
    disconnectWebSocket(true);
    toast.info('Disconnected from Modbus API');
  };

  const refreshData = useCallback(async (): Promise<void> => {
    // This function now primarily fetches coils and profiles.
    // SystemInfo is handled by its own polling mechanism.
    // Registers are currently disabled by the user.
    if (modbusService.getConnectionStatus() !== 'CONNECTED') {
      toast.warning("Cannot refresh data: WebSocket not connected.");
      return;
    }
    try {
      console.log('refreshData');
      const rawCoilData = await modbusApiService.getCoils(coilStartAddress, coilCount);
      // const rawRegisterData = await modbusApiService.getRegisters(registerStartAddress, registerCount); // User disabled
      const mappedCoilData = rawCoilData.map((coil: any) => {
        let numericId = coil.address;
        if (coil.id !== undefined) {
          const parsedId = parseInt(String(coil.id), 10);
          if (!isNaN(parsedId)) {
            numericId = parsedId;
          }
        }
        return {
          address: coil.address,
          value: coil.value,
          name: coil.name || `Coil ${coil.address}`,
          id: numericId, 
          group: coil.group || 'Default',
          type: typeof coil.type === 'number' ? coil.type : 0, 
          access: typeof coil.access === 'number' ? coil.access : 0,
          flags: typeof coil.flags === 'number' ? coil.flags : 0,
        };
      });
      setCoils(mappedCoilData as CoilData[]); 

      /* Registers are disabled by user
      const mappedRegisterData = rawRegisterData.map((reg: any) => ({ 
        address: reg.address,
        value: reg.value,
        name: reg.name || `Register ${reg.address}`,
        id: String(reg.id), 
        group: reg.group || 'Default',
        type: typeof reg.type === 'number' ? reg.type : 0, 
        access: typeof reg.access === 'number' ? reg.access : 0,
        flags: typeof reg.flags === 'number' ? reg.flags : 0,
        description: reg.description || '', 
        component: reg.group || String(reg.id) || 'DefaultComponent'
      }));
      setRegisters(mappedRegisterData as RegisterData[]); 
      */
      // setSystemInfo(info); // This is now handled by fetchSystemInfo directly
      await getProfiles();

    } catch (error) {
      console.error('Data refresh error (coils/profiles):', error);
      toast.error('Data refresh error: ' + (error instanceof Error ? error.message : String(error)));
    }
  }, [coilStartAddress, coilCount, /*registerStartAddress, registerCount,*/ getProfiles]); // Removed register deps 

  const testRelays = async (): Promise<void> => {
    if (modbusService.getConnectionStatus() !== 'CONNECTED') {
        toast.warning("Cannot test relays: WebSocket not connected. This action might still use REST if available.");
    }
    try {
      const result = await modbusApiService.testRelays();
      if (result.success) {
        toast.success(result.message || 'Relay test successful');
      } else {
        toast.error(result.message || 'Relay test failed');
      }
    } catch (error) {
      console.error('Relay test error:', error);
      toast.error('Relay test error: ' + (error instanceof Error ? error.message : String(error)));
    }
  };

  const updateCoil = async (address: number, value: boolean): Promise<void> => {
    if (modbusService.getConnectionStatus() !== 'CONNECTED') {
      toast.error('Cannot update coil: WebSocket not connected.');
      return;
    }
    try {
      setCoils(prevCoils => {
        return prevCoils.map(coil => 
          coil.address === address ? { ...coil, value } : coil
        );
      });
      await modbusService.writeCoil(address, value);
      toast.success(`Coil ${address} update requested.`);
    } catch (error) {
      setCoils(prevCoils => {
        return prevCoils.map(coil => 
          coil.address === address ? { ...coil, value: !value } : coil
        );
      });
      console.error(`Error updating coil ${address}:`, error);
      toast.error(`Error updating coil ${address}: ` + (error instanceof Error ? error.message : String(error)));
    }
  };

  const updateRegister = async (address: number, value: number): Promise<void> => {
     if (modbusService.getConnectionStatus() !== 'CONNECTED') { // Registers disabled
        toast.error('Cannot update register: WebSocket not connected.');
        return;
     }
     try {
       await modbusService.writeRegister(address, value); 
      setRegisters(prevRegisters => {
        return prevRegisters.map(register => 
          register.address === address ? { ...register, value } : register
        );
      });
      toast.success(`Register ${address} update requested.`);
    } catch (error) {
      console.error(`Error updating register ${address}:`, error);
      toast.error(`Error updating register ${address}: ` + (error instanceof Error ? error.message : String(error)));
     }
  };

  const writeRegister = useCallback(async (registerId: number, value: number) => {
    const register = registers.find(r => r.id === String(registerId)); // Registers disabled
    if (!register) {
      toast.error('Register not found');
      return;
    }
    try {
      await modbusService.writeRegister(register.address, value);
      await refreshData();
    } catch (error) {
      toast.error('Failed to write register');
      console.error('Error writing register:', error);
    }
  }, [/*registers,*/ refreshData]);

  const saveProfile = useCallback(async (profileData: ProfileSavePayload): Promise<any> => {
    try {
      const response = await modbusApiService.saveProfile(profileData);
      toast.success(`Profile "${profileData.name}" saved successfully to server.`);
      await getProfiles();
      return response; 
    } catch (error) {
      console.error('Error saving profile via context:', error);
      toast.error('Error saving profile: ' + (error instanceof Error ? error.message : String(error)));
      throw error; 
    }
  }, [getProfiles]);

  const saveMultipleProfiles = useCallback(async (profiles: ProfileSavePayload[]) => {
    if (!modbusService) {
      toast.error("Modbus service not initialized.");
      return;
    }
    try {
      await (modbusService as any).saveMultipleProfiles(profiles);
      await getProfiles(); // Refresh profiles after saving
    } catch (error) {
      console.error("Error saving multiple profiles:", error);
      throw error;
    }
  }, [modbusService, getProfiles]);

  useEffect(() => {
    return () => {
        console.log('[ModbusContext] Unmount cleanup: Disconnecting WebSocket.');
        modbusService.disconnect(true);
        stopLogRefreshTimer();
        stopRegisterRefreshTimer();
        stopCoilRefreshTimer();
        stopSystemInfoRefreshTimer(); // Stop new timer on unmount
    };
  }, []); 

  useEffect(() => {
       if (autoRefreshLogs && wsStatus === 'CONNECTED' && !connecting) {
           startLogRefreshTimer();
       } else {
           stopLogRefreshTimer();
       }
       return stopLogRefreshTimer;
  }, [autoRefreshLogs, wsStatus, logRefreshIntervalMs, startLogRefreshTimer, stopLogRefreshTimer, connecting]);

  useEffect(() => {
    if (autoRefreshRegisters && isConnected && !connecting) {
        console.log('[ModbusContext] useEffect triggering startRegisterRefreshTimer.');
        startRegisterRefreshTimer();
    } else {
        console.log('[ModbusContext] useEffect triggering stopRegisterRefreshTimer.');
        stopRegisterRefreshTimer();
    }
    return stopRegisterRefreshTimer;
  }, [autoRefreshRegisters, isConnected, connecting, startRegisterRefreshTimer, stopRegisterRefreshTimer]);

  useEffect(() => {
    if (autoRefreshCoils && wsStatus === 'CONNECTED' && !connecting) {
        startCoilRefreshTimer(); 
    } else {
        stopCoilRefreshTimer();
    }
    return stopCoilRefreshTimer;
  }, [autoRefreshCoils, wsStatus, startCoilRefreshTimer, stopCoilRefreshTimer, connecting]);

  // New useEffect for SystemInfo polling
  useEffect(() => {
    if (autoRefreshSystemInfo && isConnected && !connecting) { // Use isConnected from context
      startSystemInfoRefreshTimer();
    } else {
      stopSystemInfoRefreshTimer();
    }
    return stopSystemInfoRefreshTimer;
  }, [autoRefreshSystemInfo, isConnected, connecting, startSystemInfoRefreshTimer, stopSystemInfoRefreshTimer]);

  const clearWsLogs = useCallback(() => {
    setWsLogEntries([]);
    addWsLogEntry('Info', 'Client-side logs cleared.', Date.now());
  }, [addWsLogEntry]);

  const disconnectWebSocket = useCallback((intentional: boolean = true) => {
    addWsLogEntry('Info', `Requesting WebSocket disconnection (intentional: ${intentional})...`);
    modbusService.disconnect(intentional);
    // Stop all timers when WebSocket is intentionally disconnected
    stopLogRefreshTimer();
    stopRegisterRefreshTimer();
    stopCoilRefreshTimer();
    stopSystemInfoRefreshTimer();
  }, [addWsLogEntry, stopLogRefreshTimer, stopRegisterRefreshTimer, stopCoilRefreshTimer, stopSystemInfoRefreshTimer]);

  return (
    <ModbusContext.Provider
      value={{
        isConnected,
        apiUrl,
        coils,
        registers,
        profiles,
        coilStartAddress,
        registerStartAddress,
        coilCount,
        registerCount,
        connecting,
        systemInfo,
        setApiUrl,
        connectToServer,
        disconnectFromServer,
        updateCoil,
        updateRegister,
        writeRegister,
        refreshData,
        fetchSystemInfo, // Expose new function
        setCoilStartAddress,
        setCoilCount,
        setRegisterStartAddress,
        setRegisterCount,
        testRelays,
        wsStatus,
        wsLogEntries,
        connectWebSocket,
        disconnectWebSocket,
        clearWsLogs,
        autoRefreshLogs,
        logRefreshIntervalMs,
        setAutoRefreshLogs,
        setLogRefreshIntervalMs,
        requestLogs,
        autoRefreshRegisters,
        setAutoRefreshRegisters,
        requestRegisters,
        autoRefreshCoils,
        setAutoRefreshCoils,
        requestCoils,
        autoRefreshSystemInfo, // Expose new state
        setAutoRefreshSystemInfo, // Expose new setter
        systemInfoRefreshIntervalMs, // Expose new state
        setSystemInfoRefreshIntervalMs, // Expose new setter
        registerHistory,
        getProfiles,
        saveProfile,
        saveMultipleProfiles,
        displayMessages,
        removeDisplayMessage,
        clearDisplayMessages,
      }}
    >
      {children}
    </ModbusContext.Provider>
  );
};

export const useModbus = (): ModbusContextType => {
  const context = useContext(ModbusContext);
  if (!context) {
    throw new Error('useModbus must be used within a ModbusProvider');
  }
  return context;
};
