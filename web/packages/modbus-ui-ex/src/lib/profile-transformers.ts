import { type ControlPoint, type Profile as ServiceProfile, Profile as UIProfile, Controller, PlotStatus } from '@/types';
import { type RegisterData, type CoilData } from '@polymech/client-ts';
import { getSlaveIdFromGroup, PartitionConfig } from '@/lib/controllerUtils.js';
import { PROFILE_REGISTER_NAMES } from '@/constants';

// Server sends control points as {x: 0-1000 (percentage of duration*10), y: 0-1000 (percentage of maxTemp*10)}
// UI (BezierEditor) expects control points as {x: normalizedTime (0-1), y: normalizedTemp (0-1)}
export const transformServiceControlPointsToUI = (serverPoints: ControlPoint[]): ControlPoint[] => {
    if (!serverPoints || serverPoints.length === 0) {
      return [{ x: 0, y: 0 }, { x: 1, y: 1 }]; // Default ramp if no points
    }
    const percentageDenom = 1000;
    return serverPoints.map(p => ({
      x: (p.x || 0) / percentageDenom, // Normalize x from 0-1000 to 0-1
      y: (p.y || 0) / percentageDenom, // Normalize y from 0-1000 to 0-1
    }));
  };
  
  // UI (BezierEditor) sends control points as {x: normalizedTime (0-1), y: normalizedTemp (0-1)}
  // Server expects control points as {x: 0-1000 (percentage of duration*10), y: 0-1000 (percentage of maxTemp*10)}
  export const transformUIControlPointsToService = (uiPoints: Partial<ControlPoint>[]): ControlPoint[] => {
    const percentageMultiplier = 1000;
    return uiPoints.map(p => ({
      x: Math.round((p.x || 0) * percentageMultiplier),
      y: Math.round((p.y || 0) * percentageMultiplier)
    }));
  };
  
  // Transform ServiceProfile from context to TemperatureProfile for UI
  export const transformServiceProfileToUI = (
    serviceProfile: ServiceProfile, 
    allRegisters: RegisterData[],    
    allCoils: CoilData[], 
    partitionConfig: PartitionConfig[]
  ): UIProfile => {
    const controllerNames: string[] = [];
    if (serviceProfile.targetRegisters && allRegisters && partitionConfig) {
      for (const targetAddr of serviceProfile.targetRegisters) {
        if (targetAddr === 0) continue;
        const targetRegisterEntry = allRegisters.find(reg => reg.address === targetAddr);
        if (targetRegisterEntry) {
          const slaveId = getSlaveIdFromGroup(targetRegisterEntry.group);
          if (slaveId !== null) {
            let foundControllerName: string | undefined = undefined;
            for (const partition of partitionConfig) {
              if (partition.controllers) {
                const controller = partition.controllers.find(c => c.slaveId === slaveId);
                if (controller) {
                  foundControllerName = controller.name || `Controller (Slave ID: ${slaveId})`;
                  break;
                }
              } else if (partition.startSlaveId !== undefined && partition.numControllers !== undefined) {
                if (slaveId >= partition.startSlaveId && slaveId < partition.startSlaveId + partition.numControllers) {
                  foundControllerName = `Controller (Slave ID: ${slaveId})`;
                  break;
                }
              }
            }
            controllerNames.push(foundControllerName || `Unknown Controller (Addr: ${targetAddr})`);
          } else {
            controllerNames.push(`Controller (No Group/SlaveID for Addr: ${targetAddr})`);
          }
        } else {
          controllerNames.push(`Unknown Register (Addr: ${targetAddr})`);
        }
      }
    }
  
    let liveStatus: PlotStatus | undefined = serviceProfile.status; 
    let liveCurrentTemp: number | undefined = serviceProfile.currentTemp;
    let liveElapsed: number | undefined = serviceProfile.elapsed;
    let liveRemaining: number | undefined = serviceProfile.remaining;
  
    let liveEnabled: boolean = serviceProfile.enabled;
  
    // Find Status Register
    const statusRegister = allRegisters.find(
      r => r.group === serviceProfile.name && r.name === PROFILE_REGISTER_NAMES.STATUS
    );
    if (statusRegister && typeof statusRegister.value === 'number' && statusRegister.value in PlotStatus) {
      liveStatus = statusRegister.value as PlotStatus;
    }
  
    // Find Current Temperature Register
    const currentTempRegister = allRegisters.find(
      r => r.group === serviceProfile.name && r.name === PROFILE_REGISTER_NAMES.CURRENT_TEMP
    );
    if (currentTempRegister && typeof currentTempRegister.value === 'number') {
      liveCurrentTemp = currentTempRegister.value;
    }
    
    // Find Elapsed Time Registers (LW and HW)
    const elapsedLwRegister = allRegisters.find(
      r => r.group === serviceProfile.name && r.name === PROFILE_REGISTER_NAMES.ELAPSED_LW
    );
    const elapsedHwRegister = allRegisters.find(
      r => r.group === serviceProfile.name && r.name === PROFILE_REGISTER_NAMES.ELAPSED_HW
    );
    if (elapsedLwRegister && typeof elapsedLwRegister.value === 'number' && 
        elapsedHwRegister && typeof elapsedHwRegister.value === 'number') {
      liveElapsed = (elapsedHwRegister.value << 16) | elapsedLwRegister.value;
    }
  
    // Find Enable Coil
    const enableCoil = allCoils.find(
      c => c.group === serviceProfile.name && c.name === PROFILE_REGISTER_NAMES.ENABLE_CMD
    );
    if (enableCoil !== undefined) { 
      liveEnabled = enableCoil.value; 
    }
  
    return {
      id: serviceProfile.id,
      slot: serviceProfile.slot,
      name: serviceProfile.name || `Profile ${serviceProfile.slot}`,
      description: serviceProfile.description || 'Fetched from Modbus service',
      controlPoints: transformServiceControlPointsToUI(serviceProfile.controlPoints),
      duration: serviceProfile.duration, 
      max: serviceProfile.max,
      targetRegisters: serviceProfile.targetRegisters,
      status: liveStatus,
      currentTemp: liveCurrentTemp,
      elapsed: liveElapsed,
      remaining: liveRemaining,
      associatedControllerNames: controllerNames,
      enabled: liveEnabled,
      signalPlot: serviceProfile.signalPlot,
    };
  };
  
  export const transformControllerConfigsToProfileFormFormat = (partitionConfigs: PartitionConfig[]): Controller[] => {
    const controllers: Controller[] = [];
    let stringIdCounter = 0;
  
    partitionConfigs.forEach(partition => {
      if (partition.controllers) {
        partition.controllers.forEach(cfg => {
          controllers.push({
            id: `form-ctrl-${stringIdCounter++}`,
            name: cfg.name || `Controller ${cfg.slaveId}`,
            slaveId: cfg.slaveId,
            currentTemp: 0,
            targetTemp: 0,
            minTemp: 0,
            maxTemp: 0,
            updateInterval: 1000,
            currentProfile: null,
            isRunning: false,
            lastUpdated: new Date().toISOString(),
            zoneId: partition.name, 
          });
        });
      } else if (partition.startSlaveId !== undefined && partition.numControllers !== undefined) {
        for (let i = 0; i < partition.numControllers; i++) {
          const slaveId = partition.startSlaveId + i;
          controllers.push({
            id: `form-ctrl-${stringIdCounter++}`, 
            name: `Controller ${slaveId}`,
            slaveId: slaveId,
            currentTemp: 0,
            targetTemp: 0,
            minTemp: 0,
            maxTemp: 0,
            updateInterval: 1000,
            currentProfile: null,
            isRunning: false,
            lastUpdated: new Date().toISOString(),
            zoneId: partition.name,
          });
        }
      }
    });
    return controllers;
  }; 