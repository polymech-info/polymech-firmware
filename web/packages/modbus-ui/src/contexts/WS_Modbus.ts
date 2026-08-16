import { useState, useEffect, useCallback, useRef } from 'react';
import modbusApiService from '@polymech/client-ts/modbusApiService';
import modbusService from '@polymech/client-ts/modbusService';
import { CoilData, RegisterData } from '@/contexts/ModbusContext';
import { LogEntry, CoilUpdatePayload, RegisterUpdatePayload, E_FN_CODE } from '@polymech/client-ts';
import logger from '@/Logger';
import pMap from 'p-map';

export function useWSModbus(
  setCoils: React.Dispatch<React.SetStateAction<CoilData[]>>,
  setRegisters: React.Dispatch<React.SetStateAction<RegisterData[]>>,
  coils: CoilData[],
  registers: RegisterData[],
  isConnected: boolean,
  addWsLogEntry: (entry: LogEntry) => void,
  refreshData: () => Promise<void>
) {
  // Ref to hold the callback to avoid stale state in WebSocket event handlers
  const handleCoilUpdateCallback = useRef<(update: CoilUpdatePayload) => void>();
  const handleRegisterUpdateCallback = useRef<(update: RegisterUpdatePayload) => void>();

  useEffect(() => {
    // Keep the callback in the ref updated with the latest `coils` state
    handleCoilUpdateCallback.current = (update: CoilUpdatePayload) => {
      setCoils(prevCoils => {
        const newCoils = [...prevCoils];
        let updated = false;

        if (update.fc === E_FN_CODE.FN_WRITE_MULT_COILS && Array.isArray(update.values)) {
          update.values.forEach((value, i) => {
            const address = update.address + i;
            const index = newCoils.findIndex(c => c.address === address);
            if (index !== -1 && newCoils[index].value !== value) {
              newCoils[index] = { ...newCoils[index], value };
              updated = true;
            }
          });
        } else if (typeof update.value === 'boolean') {
          const index = newCoils.findIndex(c => c.address === update.address);
          if (index !== -1 && newCoils[index].value !== update.value) {
            newCoils[index] = { ...newCoils[index], value: update.value };
            updated = true;
          }
        }

        return updated ? newCoils : prevCoils;
      });
    };
  }, [coils, setCoils]);

  useEffect(() => {
    handleRegisterUpdateCallback.current = (update: RegisterUpdatePayload) => {
      setRegisters(prevRegisters => {
        const newRegisters = [...prevRegisters];
        let updated = false;

        // Handle multiple register updates (FC 16)
        if (update.fc === E_FN_CODE.FN_WRITE_MULT_REGISTERS && Array.isArray(update.values)) {
          update.values.forEach((value, i) => {
            const address = update.address + i;
            const index = newRegisters.findIndex(reg => reg.address === address);
            if (index !== -1) {
              if (newRegisters[index].value !== value) {
                newRegisters[index] = { ...newRegisters[index], value };
                updated = true;
              }
            }
          });
        }
        // Handle single register updates (FC 3, 6)
        else if (typeof update.value === 'number') {
          const index = prevRegisters.findIndex(reg => reg.address === update.address);
          if (index !== -1) {
            if (newRegisters[index].value !== update.value) {
              newRegisters[index] = { ...newRegisters[index], value: update.value };
              updated = true;
            }
          } else {
            console.warn(`Register update received for unknown address: ${update.address}`, update);
          }
        }

        return updated ? newRegisters : prevRegisters;
      });
    };
  }, [registers, setRegisters]);

  const requestRegisters = useCallback(async () => {
    if (!isConnected) return;
    try {
      // Fetch all pages of registers via WebSocket.
      // The service handles pagination and will call `handleRegisterData` when complete.
      await modbusService.getRegisters();
    } catch (error) {
      logger.logError(error, 'Register fetch error');
    }
  }, [isConnected]);

  const requestCoils = useCallback(async () => {
    if (!isConnected) return;
    try {
      // Fetch all pages of coils
      const data = await modbusApiService.getCoils(0, 500);
      const mappedCoils: CoilData[] = data.map(c => ({
        ...c,
        name: c.name ?? `Coil ${c.address}`,
        id: c.id ?? c.address,
        group: c.group ?? 'Default',
        type: c.type ?? 0,
        flags: c.flags ?? 0,
      }));
      setCoils(mappedCoils);
    } catch (error) {
      logger.logError(error, 'Coil fetch error');
    }
  }, [isConnected, setCoils]);

  const handleRegisterData = useCallback((registerData: RegisterData[]) => {
    const mappedRegisters = registerData.map(r => ({
      ...r,
      component: r.component ?? 'Unknown',
    }));
    setRegisters(mappedRegisters);
  }, [setRegisters]);

  const handleCoilData = useCallback((coilData: CoilData[]) => {
    setCoils(coilData);
  }, [setCoils]);

  const handleRegisterUpdate = useCallback((update: RegisterUpdatePayload) => {
    if (handleRegisterUpdateCallback.current) {
      handleRegisterUpdateCallback.current(update);
    }
  }, []);

  // This is the function passed to the WebSocket handler. It calls the function in the ref.
  const handleCoilUpdate = useCallback((update: CoilUpdatePayload) => {
    if (handleCoilUpdateCallback.current) {
      handleCoilUpdateCallback.current(update);
    }
  }, []);

  const handleMultipleCoilUpdates = useCallback((updates: CoilData[]) => {
    setCoils(prevCoils => {
      const newCoils = [...prevCoils];
      let updated = false;
      updates.forEach(update => {
        const index = newCoils.findIndex(c => c.address === update.address);
        if (index !== -1) {
          newCoils[index] = { ...newCoils[index], value: update.value, name: update.name, group: update.group };
          updated = true;
        }
      });
      return updated ? newCoils : prevCoils;
    });
  }, [setCoils]);

  const updateCoil = useCallback(async (address: number, value: boolean) => {
    // Optimistic UI update
    setCoils(prevCoils =>
      prevCoils.map(c => (c.address === address ? { ...c, value } : c))
    );

    if (!isConnected) {
      logger.warn("Cannot update coil: not connected. UI updated optimistically.");
      return;
    }
    try {
      await modbusService.writeCoil(address, value);
      //logger.success(`Successfully sent update for coil ${address} to server.`);
    } catch (error) {
      logger.logError(error, `Failed to update coil ${address}. Reverting optimistic update.`);
      // Revert UI on failure
      setCoils(prevCoils =>
        prevCoils.map(c => (c.address === address ? { ...c, value: !value } : c))
      );
    }
  }, [isConnected, setCoils]);

  const updateMultipleCoils = useCallback(async (updates: { address: number, value: boolean }[]) => {
    if (!isConnected) {
      logger.warn("Cannot update multiple coils: not connected.");
      return;
    }
    if (updates.length === 0) return;

    try {
      await pMap(updates, async (update) => {
        await modbusService.writeCoil(update.address, update.value);
        await new Promise(resolve => setTimeout(resolve, 10));
      }, { concurrency: 1 });
    } catch (error) {
      logger.logError(error, "Failed to update one or more coils.");
    }
  }, [isConnected]);

  const updateRegister = useCallback(async (address: number, value: number) => {
    if (!isConnected) {
      logger.warn("Cannot update register: not connected.");
      return;
    }
    try {
      await modbusService.writeRegister(address, value);
    } catch (error) {
      logger.logError(error, `Failed to update register ${address}`);
    }
  }, [isConnected]);

  const updateMultipleRegisters = useCallback(async (updates: { address: number, value: number }[]) => {
    if (!isConnected) {
      logger.warn("Cannot update multiple registers: not connected.");
      return;
    }
    try {
      console.log('Updating multiple registers', updates);
      await pMap(updates, async (u) => {
        await modbusService.writeRegister(u.address, u.value);
        await new Promise(resolve => setTimeout(resolve, 120));
      }, { concurrency: 1 });
    } catch (error) {
      logger.logError(error, "Failed to update one or more registers in batch.");
    }
  }, [isConnected]);

  const writeRegister = useCallback(async (registerId: number, value: number) => {
    if (!isConnected) {
      logger.warn("Cannot write register: not connected.");
      return;
    }
    try {
      await modbusService.writeRegister(registerId, value);
    } catch (error) {
      logger.logError(error, `Failed to write register ${registerId}`);
    }
  }, [isConnected]);

  return {
    handleRegisterData,
    handleCoilData,
    handleRegisterUpdate,
    handleCoilUpdate,
    handleMultipleCoilUpdates,
    updateCoil,
    updateMultipleCoils,
    updateRegister,
    updateMultipleRegisters,
    writeRegister,
    requestRegisters,
    requestCoils,
  };
} 