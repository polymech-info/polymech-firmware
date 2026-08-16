import React, { useState, useEffect, useRef } from 'react';
import { useModbus } from '@/contexts/ModbusContext'; // Assuming you'll need this for data and actions
import modbusApiService, { PlungerSettingsResponse } from '@polymech/client-ts/modbusApiService'; // Added for Plunger Settings fetch
import PotentiometerControls from './PotentiometerControls'; // Import the new component
import JoystickControls from './JoystickControls'; // Import the new component
import VFDControls from './VFDControls'; // Import the new component
import PlungerControlCard from './PlungerControlCard'; // Import the new component
import { T } from '../i18n';

import CustomWidgets from './CustomWidgets';
import CollapsibleSection from './CollapsibleSection';
import { DELTA_VFD_REGISTER_NAMES, DELTA_VFD_GROUP, PLUNGER_REGISTER_NAMES, PLUNGER_GROUP } from '@/constants';

// Enums based on your provided C++ code
const E_POSITION = {
    CENTER: 0,
    UP: 1,
    DOWN: 2,
    LEFT: 3,
    RIGHT: 4,
    UNKNOWN: 5
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
  const pot1ValueReg = registers.find(r => r.address === 29);
  const pot1ModeReg = registers.find(r => r.address === 30);
  const pot1RemoteSettingReg = registers.find(r => r.address === 31);
  
  const pot2ValueReg = registers.find(r => r.address === 33);
  const pot2ModeReg = registers.find(r => r.address === 34);
  const pot2RemoteSettingReg = registers.find(r => r.address === 35);
  
  const joystickPositionReg = registers.find(r => r.address === 49 && r.group === 'Joystick');
  const joystickModeReg = registers.find(r => r.address === 50 && r.group === 'Joystick'); 
  const joystickOverrideReg = registers.find(r => r.address === 51 && r.group === 'Joystick'); 
  const joystickModeRegAddr = joystickModeReg?.address;
  const joystickOverrideRegAddr = joystickOverrideReg?.address;
  const joystickPositionCmdRegAddr = joystickPositionReg?.address;

  const vfdRunFreqReg = registers.find(r => r.group?.startsWith(DELTA_VFD_GROUP) && r.name === DELTA_VFD_REGISTER_NAMES.RUNNING_FREQUENCY);
  const vfdSetFreqDisplayReg = registers.find(r => r.group?.startsWith(DELTA_VFD_GROUP) && r.name === DELTA_VFD_REGISTER_NAMES.SET_FREQUENCY);
  const vfdCurrentReg = registers.find(r => r.group?.startsWith(DELTA_VFD_GROUP) && r.name === DELTA_VFD_REGISTER_NAMES.OUTPUT_CURRENT);
  const vfdPowerReg = registers.find(r => r.group?.startsWith(DELTA_VFD_GROUP) && r.name === DELTA_VFD_REGISTER_NAMES.OUTPUT_POWER_KW);
  const vfdTorqueReg = registers.find(r => r.group?.startsWith(DELTA_VFD_GROUP) && r.name === DELTA_VFD_REGISTER_NAMES.OUTPUT_TORQUE_PERCENT);
  const vfdFaultReg = registers.find(r => r.group?.startsWith(DELTA_VFD_GROUP) && r.name === DELTA_VFD_REGISTER_NAMES.FAULT_CODE);
  const vfdRunningReg = registers.find(r => r.group?.startsWith(DELTA_VFD_GROUP) && r.name === DELTA_VFD_REGISTER_NAMES.IS_RUNNING);
  const vfdSetFreqCmdReg = registers.find(r => r.group?.startsWith(DELTA_VFD_GROUP) && r.name === DELTA_VFD_REGISTER_NAMES.CMD_FREQ);
  const vfdDirectionCmdReg = registers.find(r => r.group?.startsWith(DELTA_VFD_GROUP) && r.name === DELTA_VFD_REGISTER_NAMES.CMD_DIRECTION);
  const vfdMainCommandReg = registers.find(r => r.group?.startsWith(DELTA_VFD_GROUP) && r.name === DELTA_VFD_REGISTER_NAMES.CMD_COMMAND);
  const VFD_RUN_FREQ_DISPLAY_SCALE = 100  // Delta VFD RUNNING_FREQUENCY register returns centiHz (5000 = 50 Hz) 
  const VFD_CURRENT_DISPLAY_RAW = true;  

  const plungerStateReg = registers.find(r => r.group === PLUNGER_GROUP && r.name.startsWith(PLUNGER_REGISTER_NAMES.STATE));
  const plungerCommandReg = registers.find(r => r.group === PLUNGER_GROUP && r.name.startsWith(PLUNGER_REGISTER_NAMES.COMMAND));
  const plungerCommandRegAddr = plungerCommandReg?.address;


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

  const vfdCurrentDisplayValue = vfdCurrentReg ? (VFD_CURRENT_DISPLAY_RAW ? vfdCurrentReg.value : (vfdCurrentReg.value / 1)) : null;
  const vfdTorqueDisplayValue = vfdTorqueReg?.value;
  const maxOpTimeSeconds = plungerSettings ? (plungerSettings.defaultMaxOperationDurationMs / 1000).toFixed(1) + ' s' : 'N/A';

  return (
    <div className="space-y-3 md:space-y-6" id="plunger-hmi-display">
      <CollapsibleSection
        title={<T>Custom Widgets</T>}
        storageKey="hmi-custom-widgets-collapsible"
        initiallyOpen={false}
        className="glass-panel"
        headerClassName="flex justify-between items-center p-3 rounded-t-lg"
        contentClassName="p-3 glass-card rounded-b-lg"
        titleClassName="text-lg font-semibold glass-text"
        buttonClassName="text-slate-600 dark:text-white/80 hover:text-slate-800 dark:hover:text-white"
      >
        <CustomWidgets />
      </CollapsibleSection>

      <CollapsibleSection
        title={<T>Plunger Control</T>}
        storageKey="hmi-plunger-control-collapsible"
        className="glass-panel"
        headerClassName="flex justify-between items-center p-3 rounded-t-lg"
        contentClassName="p-3 glass-card rounded-b-lg"
        titleClassName="text-lg font-semibold glass-text"
        buttonClassName="text-slate-600 dark:text-white/80 hover:text-slate-800 dark:hover:text-white"
      >
        <PlungerControlCard 
          plungerStateReg={plungerStateReg}
          plungerCommandRegAddr={plungerCommandRegAddr}
          isDashboardView={isDashboardView}
          onPopout={handlePopout}
          vfdCurrentDisplayValue={vfdCurrentDisplayValue}
          VFD_CURRENT_DISPLAY_RAW={VFD_CURRENT_DISPLAY_RAW}
          peakVfdCurrent={peakVfdCurrent}
          vfdTorqueDisplayValue={vfdTorqueDisplayValue ?? null}
          peakVfdTorque={peakVfdTorque}
          maxOpTimeSeconds={maxOpTimeSeconds}
          plungerSettings={plungerSettings}
          plungerSettingsLoading={plungerSettingsLoading}
          vfdRunFreqReg={vfdRunFreqReg}
          VFD_RUN_FREQ_DISPLAY_SCALE={VFD_RUN_FREQ_DISPLAY_SCALE}
        />
      </CollapsibleSection>

      {ENABLE_POTENTIOMETER_CONTROLS && (
        <CollapsibleSection
          title={<T>Potentiometer Controls</T>}
          storageKey="hmi-potentiometer-collapsible"
          className="glass-panel"
          headerClassName="flex justify-between items-center p-3 rounded-t-lg"
          contentClassName="p-3 glass-card rounded-b-lg"
          titleClassName="text-lg font-semibold glass-text"
          buttonClassName="text-slate-600 dark:text-white/80 hover:text-slate-800 dark:hover:text-white"
        >
          <PotentiometerControls 
            pot1ValueReg={pot1ValueReg}
            pot1ModeReg={pot1ModeReg}
            pot1RemoteSettingReg={pot1RemoteSettingReg}
            pot2ValueReg={pot2ValueReg}
            pot2ModeReg={pot2ModeReg}
            pot2RemoteSettingReg={pot2RemoteSettingReg}
          />
        </CollapsibleSection>
      )}

      {ENABLE_JOYSTICK_CONTROLS && (
        <CollapsibleSection
          title={<T>Joystick Controls</T>}
          storageKey="hmi-joystick-collapsible"
          className="glass-panel"
          headerClassName="flex justify-between items-center p-3 rounded-t-lg"
          contentClassName="p-3 glass-card rounded-b-lg"
          titleClassName="text-lg font-semibold glass-text"
          buttonClassName="text-slate-600 dark:text-white/80 hover:text-slate-800 dark:hover:text-white"
        >
          <JoystickControls 
            joystickPositionReg={joystickPositionReg}
            joystickModeReg={joystickModeReg} 
            joystickOverrideReg={joystickOverrideReg} 
            joystickModeRegAddr={joystickModeRegAddr}
            joystickOverrideRegAddr={joystickOverrideRegAddr}
            joystickPositionCmdRegAddr={joystickPositionCmdRegAddr}
          />
        </CollapsibleSection>
      )}

      {ENABLE_VFD_CONTROLS && (
        <CollapsibleSection
          title={<T>VFD Control</T>}
          storageKey="hmi-vfd-collapsible"
          className="glass-panel"
          headerClassName="flex justify-between items-center p-3 rounded-t-lg"
          contentClassName="p-3 glass-card rounded-b-lg"
          titleClassName="text-lg font-semibold glass-text"
          buttonClassName="text-slate-600 dark:text-white/80 hover:text-slate-800 dark:hover:text-white"
        >
          <VFDControls />
        </CollapsibleSection>
      )}
    </div>
  );
};

export default PlungerHMIDisplay; 