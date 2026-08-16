import React, { createContext, useContext, useEffect, useState, useCallback, useRef } from 'react';

import {
  type SystemInfo,
  type ProfileSavePayload,
  type PressureProfileSavePayload,
  type ComponentInfo,
  type Settings,
} from '@polymech/client-ts';

import {
  type RegisterData,
  type CoilData,
  type LogEntry,
  type WsStatus,
  type DisplayMessagePayload,
} from "@polymech/client-ts"

import modbusApiService from '@polymech/client-ts/modbusApiService';
import { useWSModbus } from './WS_Modbus';
import { useWSSocket } from './WS_Socket';
import { useWSProfiles } from './WS_Profiles';
import { useWSSystem } from './WS_System';
import { Profile, PlotStatus } from '@/types';
import logger from '@/Logger';
import { compileTimeFlags, isProfilesEnabled } from '../features';
import { translate } from '../i18n';
import * as Tone from 'tone';
import { PROFILE_REGISTER_NAMES } from '@/constants';

export { type RegisterData, type CoilData };

interface ModbusContextType {
  // Connection State
  isConnected: boolean;
  connecting: boolean;
  connectionAborted: boolean;
  apiUrl: string;

  // WebSocket Properties
  wsStatus: WsStatus;
  wsLogEntries: LogEntry[];

  // Feature Flags
  featureFlags: typeof compileTimeFlags;
  serverSettings: Record<string, any>;

  // Data Collections
  coils: CoilData[];
  registers: RegisterData[];
  profiles: Profile[];
  pressureProfiles: any[]; // Array to store pressure profiles
  components: ComponentInfo[];
  systemInfo: SystemInfo | null;
  settings: Settings | null;
  registerHistory: Record<string, any[]>;
  displayMessages: DisplayMessagePayload[];

  // Configuration Properties
  coilStartAddress: number;
  coilCount: number;
  registerStartAddress: number;
  registerCount: number;

  // Connection Management
  setApiUrl: (url: string) => void;
  connectToServer: (url?: string) => Promise<boolean>;
  disconnectFromServer: () => void;
  abortConnectionAttempt: () => void;

  // WebSocket Management
  connectWebSocket: (url: string) => Promise<boolean>;
  disconnectWebSocket: (intentional?: boolean) => void;
  clearWsLogs: () => void;

  // Configuration Setters
  setCoilStartAddress: (address: number) => void;
  setRegisterStartAddress: (address: number) => void;
  setCoilCount: (count: number) => void;
  setRegisterCount: (count: number) => void;

  // Data Operations - Coils
  updateCoil: (address: number, value: boolean) => Promise<void>;
  updateMultipleCoils: (updates: { address: number, value: boolean }[]) => Promise<void>;
  requestCoils: () => Promise<void>;

  // Data Operations - Registers
  updateRegister: (address: number, value: number) => Promise<void>;
  updateMultipleRegisters: (updates: { address: number, value: number }[]) => Promise<void>;
  writeRegister: (registerId: number, value: number) => Promise<void>;
  requestRegisters: () => Promise<void>;

  // Data Operations - System & Components
  fetchSystemInfo: () => Promise<void>;
  refreshData: () => Promise<void>;
  getComponents: () => Promise<void>;
  getSettings: () => Promise<void>;
  getFeatureFlags: () => Promise<void>;

  // Profile Management
  getProfiles: () => Promise<void>;
  saveProfile: (profileData: ProfileSavePayload) => Promise<any>;
  uploadProfiles: (profilesData: ProfileSavePayload[]) => Promise<any>; // Add this
  getPressureProfiles: () => Promise<void>;
  savePressureProfile: (profileData: PressureProfileSavePayload) => Promise<any>;

  // Display Message Management
  removeDisplayMessage: (id: string | number) => void;
  clearDisplayMessages: () => void;
}
const ModbusContext = createContext<ModbusContextType | undefined>(undefined);

interface ModbusProviderProps {
  children: React.ReactNode;
}

export const ModbusProvider: React.FC<ModbusProviderProps> = ({ children }) => {
  // State declarations
  const [isConnected, setIsConnected] = useState(false);
  const [connecting, setConnecting] = useState(false);
  const [connectionAborted, setConnectionAborted] = useState(false);
  const [apiUrl, setApiUrl] = useState(() => {
    // Prioritize current window location
    if (typeof window !== 'undefined' && window.location.host && !window.location.host.includes('localhost')) {
      return `${window.location.protocol}//${window.location.host}`;
    }
    // Fallback to localStorage if window.location is not available
    const savedUrl = localStorage.getItem('apiUrl');
    return savedUrl || 'http://192.168.239.250/';
  });

  // Ensure the API base URL is configured before any API calls (e.g., getFeatureFlags)
  useEffect(() => {
    modbusApiService.setBaseUrl(apiUrl);
    localStorage.setItem('apiUrl', apiUrl);
  }, [apiUrl]);

  const [notifiedFinishedProfiles, setNotifiedFinishedProfiles] = useState<Set<string>>(new Set());

  const onProfileStateChanged = useCallback((profile: Profile, status: PlotStatus) => {
    if (!profile.name) return;

    const baseUrl = apiUrl.replace('/api', '');

    const playSound = (file: string) => {
      new Tone.Player(`${baseUrl}/assets/${file}`).toDestination().autostart = true;
    };

    switch (status) {
      case PlotStatus.INITIALIZING:
        playSound('4.mp3');
        break;

      case PlotStatus.FINISHED:
        if (notifiedFinishedProfiles.has(profile.name)) {
          return;
        }

        const notificationMessage = translate('Profile "{profileName}" has finished.').replace('{profileName}', profile.name);

        const showNotification = () => {
          new Notification(translate("Profile Finished"), {
            body: notificationMessage,
          });
          playSound('1.mp3');
          setNotifiedFinishedProfiles(prev => new Set(prev).add(profile.name!));
        };

        if (Notification.permission === "granted") {
          showNotification();
        } else {
          Notification.requestPermission().then(permission => {
            if (permission === "granted") {
              showNotification();
            }
          });
        }
        break;

      case PlotStatus.RUNNING:
        if (notifiedFinishedProfiles.has(profile.name)) {
          playSound('4.mp3');
          setNotifiedFinishedProfiles(prev => {
            const newSet = new Set(prev);
            newSet.delete(profile.name!);
            return newSet;
          });
        }
        break;

      case PlotStatus.PAUSED:
        playSound('2.mp3');
        break;

      case PlotStatus.STOPPED:
        playSound('3.mp3');
        break;
    }
  }, [notifiedFinishedProfiles, apiUrl]);

  const [featureFlags, setFeatureFlags] = useState(compileTimeFlags);
  const [serverSettings, setServerSettings] = useState<Record<string, any>>({});

  const getFeatureFlags = useCallback(async () => {
    try {
      const serverFeatures = await modbusApiService.getFeatureFlags();
      if (serverFeatures && serverFeatures.features) {
        setFeatureFlags(prevFlags => {
          const newFlags = { ...prevFlags, ...serverFeatures.features };
          // Ensure compile-time flags are still respected.
          for (const key in newFlags) {
            if (Object.prototype.hasOwnProperty.call(compileTimeFlags, key)) {
              if ((compileTimeFlags as any)[key] === false) {
                (newFlags as any)[key] = false;
              }
            }
          }
          return newFlags;
        });
      }
      if (serverFeatures && serverFeatures.settings) {
        setServerSettings(serverFeatures.settings);
      }
    } catch (error) {
      logger.logError(error, "Could not fetch or merge server feature flags");
    }
  }, []);

  useEffect(() => {
    getFeatureFlags();
  }, [getFeatureFlags]);

  const [coils, setCoils] = useState<CoilData[]>([]);
  const [registers, setRegisters] = useState<RegisterData[]>([]);
  const [profiles, setProfiles] = useState<Profile[]>([]);
  const [pressureProfiles, setPressureProfiles] = useState<any[]>([]);
  const [components, setComponents] = useState<ComponentInfo[]>([]);
  const [settings, setSettings] = useState<Settings | null>(null);
  const [coilStartAddress, setCoilStartAddress] = useState(0);
  const [registerStartAddress, setRegisterStartAddress] = useState(0);
  const [coilCount, setCoilCount] = useState(100);
  const [registerCount, setRegisterCount] = useState(200);
  const [systemInfo, setSystemInfo] = useState<SystemInfo | null>(null);
  const [wsStatus, setWsStatus] = useState<WsStatus>('DISCONNECTED');
  const [wsLogEntries, setWsLogEntries] = useState<LogEntry[]>([]);
  const [registerHistory] = useState<Record<string, any[]>>({});
  const [displayMessages, setDisplayMessages] = useState<DisplayMessagePayload[]>([

  ]);

  const {
    getProfiles,
    saveProfile,
    uploadProfiles, // Add this
    getPressureProfiles,
    savePressureProfile,
  } = useWSProfiles(apiUrl, setProfiles, setPressureProfiles);

  const {
    fetchSystemInfo,
    addWsLogEntry,
    clearWsLogs,
    getComponents,
    getSettings,
  } = useWSSystem(
    setSystemInfo,
    setWsLogEntries,
    setComponents,
    setSettings
  );

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

  const refreshData = useCallback(async (): Promise<void> => {
    if (!isConnected) {
      logger.warn("Cannot refresh data: WebSocket not connected.");
      return;
    }
    try {

      // Fetch Coils
      const rawCoilData = await modbusApiService.getCoils(coilStartAddress, coilCount);
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

      try {
        // Fetch Registers
        const registerData = await modbusApiService.getRegisters(registerStartAddress, registerCount);
        const mappedRegisters = registerData.map(r => ({
          ...r,
          component: 'Unknown', // Provide a default value for the missing property
        }));
        setRegisters(mappedRegisters);
      } catch (error) {
        console.error(error, 'Data refresh error (registers)', error);
      }

      try {
        if (isProfilesEnabled(featureFlags)) {
          await getProfiles();
          await getPressureProfiles();
        }
      } catch (error) {
        console.error(error, 'Data refresh error (profiles)', error);
      }

    } catch (error) {
      console.error(error, 'Data refresh error (coils/profiles/registers)', error);
    }
  }, [isConnected, coilStartAddress, coilCount, registerStartAddress, registerCount, getProfiles, getPressureProfiles, featureFlags, fetchSystemInfo]);

  const adaptedAddWsLogEntry = useCallback((entry: LogEntry) => {
    addWsLogEntry(entry.level, entry.message, entry.timestamp, entry.id, entry.name);
  }, [addWsLogEntry]);

  const wsModbus = useWSModbus(
    setCoils,
    setRegisters,
    coils,
    registers,
    isConnected,
    adaptedAddWsLogEntry,
    refreshData
  );

  const wsSocket = useWSSocket(
    apiUrl,
    setApiUrl,
    setIsConnected,
    setConnecting,
    setConnectionAborted,
    setWsStatus,
    setCoils,
    setSystemInfo,
    wsModbus,
    getProfiles,
    getComponents,
    getSettings,
    addWsLogEntry,
    handleDisplayMessage
  );

  const {
    requestCoils,
    requestRegisters,
    updateCoil,
    updateMultipleCoils,
    updateRegister,
    updateMultipleRegisters,
    writeRegister,
  } = wsModbus;

  const {
    connectToServer,
    disconnectFromServer,
    abortConnectionAttempt,
    disconnectWebSocket,
    connectWebSocket
  } = wsSocket;

  const removeDisplayMessage = useCallback((id: string | number) => {
    setDisplayMessages(prevMessages => prevMessages.filter(msg => msg.id !== id));
  }, []);

  const clearDisplayMessages = useCallback(() => {
    setDisplayMessages([]);
  }, []);



  useEffect(() => {
    if (isConnected && !connecting) {
      refreshData();
    }
  }, [isConnected, connecting, refreshData]);

  const prevRegistersRef = useRef<RegisterData[]>([]);

  useEffect(() => {
    if (onProfileStateChanged && registers && registers.length > 0 && profiles && profiles.length > 0) {
      profiles.forEach(profile => {
        if (!profile.name) return;

        const statusRegister = registers.find(
          r => r.group === profile.name && r.name.startsWith(PROFILE_REGISTER_NAMES.STATUS));

        if (statusRegister && statusRegister.value in PlotStatus) {
          const prevStatusRegister = prevRegistersRef.current.find(
            r => r.address === statusRegister.address
          );

          const currentStatus = statusRegister.value as PlotStatus;
          const previousStatus = prevStatusRegister ? prevStatusRegister.value as PlotStatus : undefined;

          if (currentStatus !== previousStatus) {
            onProfileStateChanged(profile, currentStatus);
          }
        }
      });
    }
    prevRegistersRef.current = registers;
  }, [registers, profiles, onProfileStateChanged]);

  return (
    <ModbusContext.Provider
      value={{
        isConnected,
        apiUrl,
        coils,
        registers,
        profiles,
        pressureProfiles,
        components,
        featureFlags,
        serverSettings,
        coilStartAddress,
        registerStartAddress,
        coilCount,
        registerCount,
        connecting,
        connectionAborted,
        systemInfo,
        setApiUrl,
        connectToServer,
        disconnectFromServer,
        abortConnectionAttempt,
        updateCoil,
        updateMultipleCoils,
        updateRegister,
        updateMultipleRegisters,
        writeRegister,
        refreshData,
        fetchSystemInfo,
        setCoilStartAddress,
        setCoilCount,
        setRegisterStartAddress,
        setRegisterCount,
        wsStatus,
        wsLogEntries,
        connectWebSocket: () => connectWebSocket(apiUrl),
        disconnectWebSocket,
        clearWsLogs,
        requestRegisters,
        requestCoils,
        registerHistory,
        getComponents,
        saveProfile,
        uploadProfiles, // Add this
        getProfiles,
        displayMessages,
        removeDisplayMessage,
        clearDisplayMessages,
        settings,
        getPressureProfiles,
        savePressureProfile,
        getSettings,
        getFeatureFlags
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