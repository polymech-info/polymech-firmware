import React, { useState, useEffect, useRef } from 'react';
import { useModbus } from '@/contexts/ModbusContext'; // Assuming you'll need this for data and actions
import modbusApiService, { PlungerSettingsResponse, PlungerSettingsUpdatePayload } from '@polymech/client-ts/modbusApiService'; // Added for Plunger Settings fetch
import PotentiometerControls from './PotentiometerControls'; // Import the new component
import JoystickControls from './JoystickControls'; // Import the new component
import VFDControls from './VFDControls'; // Import the new component
import PlungerControlCard from './PlungerControlCard'; // Import the new component

// Enums based on your provided C++ code
const E_POSITION = {
    CENTER: 0,
    UP: 1,
    DOWN: 2,
    LEFT: 3,
    RIGHT: 4,
    UNKNOWN: 5
} as const;
type PositionKeys = keyof typeof E_POSITION;
const positionToString = (value: number): PositionKeys => {
    for (const key in E_POSITION) {
        if (E_POSITION[key as PositionKeys] === value) return key as PositionKeys;
    }
    return 'UNKNOWN';
};

const E_MODE = {
    LOCAL: 0,
    REMOTE: 1
} as const;
type ModeKeys = keyof typeof E_MODE;
const modeToString = (value: number): ModeKeys => {
    return value === E_MODE.REMOTE ? 'REMOTE' : 'LOCAL';
};

// Updated VFD Commands and Directions based on E_SAKO_DIRECTION
const SAKO_MAIN_COMMANDS = { 
    // These might not be needed if 7670 handles run/stop with direction.
    // Or, there might be a generic START (1?) and specific STOPs.
    STOP_DECEL: 6,        // From E_SAKO_DIRECTION
    STOP_FREE_RUN: 5,   // From E_SAKO_DIRECTION
    FAULT_RESET: 7,       // From E_SAKO_DIRECTION
} as const;

// VFD Direction/Action Commands for register 7670 (SAKO: Direction Cmd)
const SAKO_DIRECTION_ACTIONS = {
    FWD: 1,               // E_SAKO_DIR_FWD
    REV: 2,               // E_SAKO_DIR_REV
    // Using STOP_DECEL as the primary stop for this register, can add FREE_STOP if needed.
    STOP: 6,              // E_SAKO_DIR_DECELERATION_STOP 
} as const;

const PLUNGER_COMMANDS = { // For Plunger Command register 671
    NONE: 0,
    HOME: 1,
    PLUNGE: 2,
    STOP: 3,
    INFO: 4,
    FILL: 5,
    REPLAY: 6
} as const;

// Updated PLUNGER_STATES to match C++ enum
const PLUNGER_STATES = {
    IDLE: 0,
    HOMING_MANUAL: 1,
    HOMING_AUTO: 2,
    PLUNGING_MANUAL: 3,
    PLUNGING_AUTO: 4,
    STOPPING: 5,
    JAMMED: 6,
    RESETTING_JAM: 7,
    RECORD: 8,
    REPLAY: 9,
    FILLING: 10,
    POST_FLOW: 11,
    // UNKNOWN_STATE will be handled by the toString function if no match
} as const;

const plungerStateToString = (value: number): string => {
    // Iterate over PLUNGER_STATES to find the matching key for the given value
    for (const key of Object.keys(PLUNGER_STATES)) {
        if (PLUNGER_STATES[key as keyof typeof PLUNGER_STATES] === value) {
            return key.replace(/_/g, ' '); // Replace underscores with spaces for better readability
        }
    }
    return `UNKNOWN_STATE (${value})`; // Fallback for unknown values
};

// Custom neumorphic-inspired button style using Tailwind utilities
const neumorphicBase = "px-4 py-2 rounded-lg font-semibold transition-all duration-150 ease-in-out focus:outline-none focus:ring-2 focus:ring-offset-2 flex items-center justify-center";
const neumorphicLight = `bg-slate-100 text-slate-700 hover:bg-slate-200 shadow-[3px_3px_7px_#bec8e4,-3px_-3px_7px_#ffffff] active:shadow-[inset_3px_3px_7px_#bec8e4,inset_-3px_-3px_7px_#ffffff]`;
const neumorphicDark = `dark:bg-slate-800 dark:text-slate-300 dark:hover:bg-slate-700 dark:shadow-[3px_3px_7px_#2c3e50,-3px_-3px_7px_#4a6572] dark:active:shadow-[inset_3px_3px_7px_#2c3e50,inset_-3px_-3px_7px_#4a6572]`;
const neumorphicButtonClass = `${neumorphicBase} ${neumorphicLight} ${neumorphicDark}`;

// Updated style for active/selected D-pad button with orange transparent tint
const neumorphicActiveLight = `bg-orange-400/30 text-orange-700 shadow-[inset_3px_3px_7px_#c87600,inset_-3px_-3px_7px_#ffe8cc]`; // Adjusted shadow for orange
const neumorphicActiveDark = `dark:bg-orange-600/30 dark:text-orange-300 dark:shadow-[inset_3px_3px_7px_#8a5300,inset_-3px_-3px_7px_#ffc966]`; // Adjusted shadow for orange
const neumorphicButtonActiveClass = `${neumorphicBase} ${neumorphicActiveLight} ${neumorphicActiveDark}`;

const neumorphicDestructiveBase = "px-4 py-2 rounded-lg font-semibold transition-all duration-150 ease-in-out focus:outline-none focus:ring-2 focus:ring-offset-2 flex items-center justify-center";
const neumorphicDestructiveLight = `bg-red-500 text-white hover:bg-red-600 shadow-[3px_3px_7px_#c82333,-3px_-3px_7px_#ff7878] active:shadow-[inset_3px_3px_7px_#c82333,inset_-3px_-3px_7px_#ff7878]`;
const neumorphicDestructiveDark = `dark:bg-red-600 dark:text-white dark:hover:bg-red-700 dark:shadow-[3px_3px_7px_#a71d2a,-3px_-3px_7px_#e85a5a] dark:active:shadow-[inset_3px_3px_7px_#a71d2a,inset_-3px_-3px_7px_#e85a5a]`;
const neumorphicDestructiveButtonClass = `${neumorphicDestructiveBase} ${neumorphicDestructiveLight} ${neumorphicDestructiveDark}`;

// Larger button style for Plunger HMI main actions
const largeNeumorphicButtonClass = `${neumorphicButtonClass} py-3 px-6 text-lg`;
const largeNeumorphicDestructiveButtonClass = `${neumorphicDestructiveButtonClass} py-3 px-6 text-lg`;

// Control feature flags
const ENABLE_JOYSTICK_CONTROLS = false; // Set to true to enable Joystick card
const ENABLE_POTENTIOMETER_CONTROLS = true; // Example for future use
const ENABLE_VFD_CONTROLS = true;         // Example for future use

interface PlungerHMIDisplayProps {
  isDashboardView?: boolean;
}

const PlungerHMIDisplay: React.FC<PlungerHMIDisplayProps> = ({ isDashboardView = false }) => {
  const { registers, connecting } = useModbus(); // updateRegister is no longer directly used here
  const [plungerSettings, setPlungerSettings] = useState<PlungerSettingsResponse | null>(null);
  const [plungerSettingsLoading, setPlungerSettingsLoading] = useState<boolean>(true);

  const peakCurrentTimerRef = useRef<NodeJS.Timeout | null>(null);
  const peakTorqueTimerRef = useRef<NodeJS.Timeout | null>(null);
  const [peakVfdCurrent, setPeakVfdCurrent] = useState<number | null>(null);
  const [peakVfdTorque, setPeakVfdTorque] = useState<number | null>(null);

  useEffect(() => {
    const fetchPlungerSettings = async () => {
      try {
        setPlungerSettingsLoading(true);
        const settings = await modbusApiService.getPlungerSettings();
        setPlungerSettings(settings);
      } catch (err) {
        console.error("Failed to fetch plunger settings for HMI display:", err);
      } finally {
        setPlungerSettingsLoading(false);
      }
    };
    fetchPlungerSettings();
  }, []);

  // Find all necessary registers here in the parent HMI component
  const pot1ValueReg = registers.find(r => r.address === 18 && parseInt(String(r.id), 10) === 350);
  const pot1ModeReg = registers.find(r => r.address === 19 && parseInt(String(r.id), 10) === 350);
  const pot1RemoteSettingReg = registers.find(r => r.address === 20 && parseInt(String(r.id), 10) === 350);
  const pot2ValueReg = registers.find(r => r.address === 22 && parseInt(String(r.id), 10) === 351);
  const pot2ModeReg = registers.find(r => r.address === 23 && parseInt(String(r.id), 10) === 351);
  const pot2RemoteSettingReg = registers.find(r => r.address === 24 && parseInt(String(r.id), 10) === 351);
  
  const joystickPositionReg = registers.find(r => r.address === 80 && r.group === 'Joystick - 4P');
  const joystickModeReg = registers.find(r => r.address === 81 && r.group === 'Joystick - 4P'); 
  const joystickOverrideReg = registers.find(r => r.address === 82 && r.group === 'Joystick - 4P'); 
  const joystickModeRegAddr = joystickModeReg?.address;
  const joystickOverrideRegAddr = joystickOverrideReg?.address;
  const joystickPositionCmdRegAddr = joystickPositionReg?.address;

  const vfdRunFreqReg = registers.find(r => r.address === 7661 && r.group === 'SAKO_VFD[10]');
  const vfdSetFreqDisplayReg = registers.find(r => r.address === 7662 && r.group === 'SAKO_VFD[10]');
  const vfdCurrentReg = registers.find(r => r.address === 7663 && r.group === 'SAKO_VFD[10]');
  const vfdPowerReg = registers.find(r => r.address === 7664 && r.group === 'SAKO_VFD[10]');
  const vfdTorqueReg = registers.find(r => r.address === 7665 && r.group === 'SAKO_VFD[10]');
  const vfdFaultReg = registers.find(r => r.address === 7666 && r.group === 'SAKO_VFD[10]');
  const vfdRunningReg = registers.find(r => r.address === 7667 && r.group === 'SAKO_VFD[10]'); 
  const vfdSetFreqCmdRegAddr = 7669;
  const vfdDirectionCmdRegAddr = 7670;
  const vfdMainCommandRegAddr = 7671; 
  const VFD_RUN_FREQ_DISPLAY_SCALE = 100; 
  const VFD_SET_FREQ_DISPLAY_RAW = true; 
  const VFD_CURRENT_DISPLAY_RAW = true;  

  const plungerStateReg = registers.find(r => r.address === 670 && r.group === 'Plunger');
  const plungerCommandRegAddr = 671;

  useEffect(() => {
    const currentValue = vfdCurrentReg?.value;
    if (currentValue !== undefined) {
      const actualCurrent = VFD_CURRENT_DISPLAY_RAW ? currentValue : currentValue / 10; 
      if (peakVfdCurrent === null || actualCurrent > peakVfdCurrent) {
        setPeakVfdCurrent(actualCurrent);
      }
      if (peakCurrentTimerRef.current) {
        clearTimeout(peakCurrentTimerRef.current);
      }
      peakCurrentTimerRef.current = setTimeout(() => {
        setPeakVfdCurrent(null); 
      }, 10000);
    }
    return () => {
      if (peakCurrentTimerRef.current) {
        clearTimeout(peakCurrentTimerRef.current);
      }
    };
  }, [vfdCurrentReg?.value, VFD_CURRENT_DISPLAY_RAW]);

  useEffect(() => {
    const currentTorque = vfdTorqueReg?.value;
    if (currentTorque !== undefined) {
      if (peakVfdTorque === null || currentTorque > peakVfdTorque) {
        setPeakVfdTorque(currentTorque);
      }
      if (peakTorqueTimerRef.current) {
        clearTimeout(peakTorqueTimerRef.current);
      }
      peakTorqueTimerRef.current = setTimeout(() => {
        setPeakVfdTorque(null); 
      }, 10000);
    }
    return () => {
      if (peakTorqueTimerRef.current) {
        clearTimeout(peakTorqueTimerRef.current);
      }
    };
  }, [vfdTorqueReg?.value]); 
  
  const handlePopout = () => {
    const currentBase = window.location.origin + window.location.pathname.replace(/index.html$/, '');
    const popoutUrl = `${currentBase}#/app/plunger`;
    window.open(popoutUrl, '_blank', 'noopener,noreferrer');
  };

  if ((connecting || plungerSettingsLoading) && registers.length === 0 && !plungerSettings) { // Adjusted loading condition slightly
    return <p className="p-4">Loading HMI data...</p>; 
  }

  const vfdCurrentDisplayValue = vfdCurrentReg ? (VFD_CURRENT_DISPLAY_RAW ? vfdCurrentReg.value : (vfdCurrentReg.value / 10)) : null;
  const vfdTorqueDisplayValue = vfdTorqueReg?.value;
  const maxOpTimeSeconds = plungerSettings ? (plungerSettings.defaultMaxOperationDurationMs / 1000).toFixed(1) : 'N/A';

  return (
    <div className="space-y-8 p-2 bg-slate-50 dark:bg-slate-900">
      <PlungerControlCard 
        plungerStateReg={plungerStateReg}
        plungerCommandRegAddr={plungerCommandRegAddr}
        isDashboardView={isDashboardView}
        onPopout={handlePopout}
        vfdCurrentDisplayValue={vfdCurrentDisplayValue}
        VFD_CURRENT_DISPLAY_RAW={VFD_CURRENT_DISPLAY_RAW}
        peakVfdCurrent={peakVfdCurrent}
        vfdTorqueDisplayValue={vfdTorqueDisplayValue}
        peakVfdTorque={peakVfdTorque}
        maxOpTimeSeconds={maxOpTimeSeconds}
        plungerSettings={plungerSettings}
        plungerSettingsLoading={plungerSettingsLoading}
        vfdRunFreqReg={vfdRunFreqReg}
        VFD_RUN_FREQ_DISPLAY_SCALE={VFD_RUN_FREQ_DISPLAY_SCALE}
      />

      {ENABLE_POTENTIOMETER_CONTROLS && (
        <PotentiometerControls 
          pot1ValueReg={pot1ValueReg}
          pot1ModeReg={pot1ModeReg}
          pot1RemoteSettingReg={pot1RemoteSettingReg}
          pot2ValueReg={pot2ValueReg}
          pot2ModeReg={pot2ModeReg}
          pot2RemoteSettingReg={pot2RemoteSettingReg}
        />
      )}

      {ENABLE_JOYSTICK_CONTROLS && (
        <JoystickControls 
          joystickPositionReg={joystickPositionReg}
          joystickModeReg={joystickModeReg} 
          joystickOverrideReg={joystickOverrideReg} 
          joystickModeRegAddr={joystickModeRegAddr}
          joystickOverrideRegAddr={joystickOverrideRegAddr}
          joystickPositionCmdRegAddr={joystickPositionCmdRegAddr}
        />
      )}

      {ENABLE_VFD_CONTROLS && (
        <VFDControls 
          vfdRunFreqReg={vfdRunFreqReg}
          vfdSetFreqDisplayReg={vfdSetFreqDisplayReg}
          vfdCurrentReg={vfdCurrentReg}
          vfdPowerReg={vfdPowerReg} 
          vfdTorqueReg={vfdTorqueReg}
          vfdFaultReg={vfdFaultReg}   
          vfdRunningReg={vfdRunningReg}
          vfdSetFreqCmdRegAddr={vfdSetFreqCmdRegAddr}
          vfdDirectionCmdRegAddr={vfdDirectionCmdRegAddr}
          vfdMainCommandRegAddr={vfdMainCommandRegAddr}
          VFD_RUN_FREQ_DISPLAY_SCALE={VFD_RUN_FREQ_DISPLAY_SCALE}
          VFD_SET_FREQ_DISPLAY_RAW={VFD_SET_FREQ_DISPLAY_RAW}
          VFD_CURRENT_DISPLAY_RAW={VFD_CURRENT_DISPLAY_RAW}
        />
      )}
    </div>
  );
};

export default PlungerHMIDisplay; 