// src/lib/controllerUtils.ts
import { RegisterData } from '@polymech/client-ts';

export interface ControllerConfig {
  slaveid: number;
  name: string; 
  enabled: boolean;
}

export interface PartitionConfig {
  name: string;
  controllers: ControllerConfig[];
  startslaveid?: number;
  numcontrollers?: number;
}

export interface ControllerStatus {
  isRunning: boolean;
  // hasAlarm: boolean;
  isAutoTuning: boolean;
  hasHeaterBreak: boolean;
  hasSensorBreak: boolean;
  mode: 'manual' | 'auto' | 'cascade' | 'program' | 'unknown';
  isHeating: boolean;
}

// Configuration for partitions, aligned with client/src/types.ts
export function getPartitionConfig(): PartitionConfig[] {
  return [
    {
      name: "Cassandra Bank Alpha",
      controllers: [
        { slaveid: 10, name: "Karina", enabled: true }, 
        { slaveid: 11, name: "Castor", enabled: true },
        { slaveid: 12, name: "Cetus", enabled: true },
        { slaveid: 13, name: "Corona", enabled: true },
      ]
    },
    {
      name: "Cassandra Bank Beta",
      controllers: [
        { slaveid: 14, name: "Coma B", enabled: true },
        { slaveid: 15, name: "Corvus", enabled: true },
        { slaveid: 16, name: "Crater", enabled: true },
        { slaveid: 17, name: "Crux", enabled: true },
      ],
      startslaveid: 14, 
      numcontrollers: 4,
    }
  ];
}

export function getSlaveIdFromGroup(groupName: string | undefined): number | null {
    if (!groupName) return null;

    // Try the format "slave_1", "slave_2", etc.
    let match = groupName.match(/slave_(\d+)/);
    if (match) {
        return parseInt(match[1], 10);
    }

    // Fallback to the format "some_group[1]", "another_group[2]"
    match = groupName.match(/\[(\d+)\]$/);
    if (match) {
        return parseInt(match[1], 10);
    }

    return null;
}

export function findRegisterForProfile(
    allRegisters: any[], 
    profileName: string, 
    profileSlot: number, 
    registerName: string
): any | undefined {
    
  const matchingRegisters = allRegisters.filter(
        (reg) => reg.group === profileName && reg.name.includes(registerName)
    );

    if (matchingRegisters.length === 1) {
        return matchingRegisters[0];
    }

    if (matchingRegisters.length > 1) {
        const slotSpecificId = `profile-${profileSlot}-${registerName.toLowerCase().replace(/ /g, '-')}`;
        const found = matchingRegisters.find((reg) => reg.id === slotSpecificId);
        if(found) return found;
    }
    
    // Fallback for cases where the id might not be set as expected yet,
    // but we need to ensure we don't just grab the first one for the wrong slot.
    // This part is tricky without a guaranteed unique identifier on the register itself.
    // The most robust solution is ensuring the backend provides one.
    // For now, if we can't find a slot-specific one, we return undefined
    // to prevent acting on the wrong profile.
    return undefined;
}

export function findCoilForProfile(
    allCoils: any[], 
    profileName: string, 
    profileSlot: number, 
    coilName: string
): any | undefined {
    
    const matchingCoils = allCoils.filter(
        (coil) => coil.group === profileName && coil.name.includes(coilName)
    );

    if (matchingCoils.length === 1) {
        return matchingCoils[0];
    }

    if (matchingCoils.length > 1) {
        const slotSpecificId = `profile-${profileSlot}-${coilName.toLowerCase().replace(/ /g, '-')}`;
        const found = matchingCoils.find((coil) => coil.id === slotSpecificId);
        if(found) return found;
    }
    
    return undefined;
}

// Status bit definitions
export const STATUS_BITS = {
  // Status High bits (bits 16-31)
  RUN_STOP: 24,        // Bit 24: Run/Stop status
  ALARM: 25,           // Bit 25: Alarm status
  AT: 26,              // Bit 26: Auto-tuning status
  HEATER_BREAK: 27,    // Bit 27: Heater break alarm
  SENSOR_BREAK: 28,    // Bit 28: Sensor break alarm
  // Status Low bits (bits 0-15)
  MANUAL: 0,           // Bit 0: Manual mode
  AUTO: 1,             // Bit 1: Auto mode
  CASCADE: 2,          // Bit 2: Cascade mode
  PROGRAM: 3,          // Bit 3: Program mode
  Control_OutputOpenOutput: 8, // Bit 8: Control Output Open (Heating ON)
} as const;

// Helper function to check status bits
export const checkStatusBit = (high: number, low: number, bit: number): boolean => {
  if (bit <= 15) {
    return (low & (1 << bit)) !== 0;
  } else {
    return (high & (1 << (bit - 16))) !== 0;
  }
};

// Helper function to get controller status
export const getControllerStatus = (statusHigh: RegisterData | undefined, statusLow: RegisterData | undefined): ControllerStatus => {
  if (!statusHigh || !statusLow || typeof statusHigh.value !== 'number' || typeof statusLow.value !== 'number') {
    return {
      isRunning: false,
      // hasAlarm: false,
      isAutoTuning: false,
      hasHeaterBreak: false,
      hasSensorBreak: false,
      mode: 'unknown',
      isHeating: false,
    };
  }
  const high = statusHigh.value;
  const low = statusLow.value;
  
  // Determine operation mode
  let mode: ControllerStatus['mode'] = 'unknown';
  if (checkStatusBit(high, low, STATUS_BITS.MANUAL)) mode = 'manual';
  else if (checkStatusBit(high, low, STATUS_BITS.AUTO)) mode = 'auto';
  else if (checkStatusBit(high, low, STATUS_BITS.CASCADE)) mode = 'cascade';
  else if (checkStatusBit(high, low, STATUS_BITS.PROGRAM)) mode = 'program';

  return {
    isRunning: !checkStatusBit(high, low, STATUS_BITS.RUN_STOP), // RunStop is inverted
    // hasAlarm: checkStatusBit(high, low, STATUS_BITS.ALARM),
    isAutoTuning: checkStatusBit(high, low, STATUS_BITS.AT),
    hasHeaterBreak: checkStatusBit(high, low, STATUS_BITS.HEATER_BREAK),
    hasSensorBreak: checkStatusBit(high, low, STATUS_BITS.SENSOR_BREAK),
    mode,
    isHeating: checkStatusBit(high, low, STATUS_BITS.Control_OutputOpenOutput),
  };
}; 