import React, { useState, useEffect, useRef } from 'react';
import { useModbus } from '@/contexts/ModbusContext';
import modbusApiService, { PlungerSettingsResponse } from '@polymech/client-ts/modbusApiService';
import PlungerControlCard from './PlungerControlCard';
import { DELTA_VFD_REGISTER_NAMES, DELTA_VFD_GROUP, PLUNGER_REGISTER_NAMES, PLUNGER_GROUP } from '@/constants';

const PlungerControlWidget: React.FC = () => {
  const { registers, connecting } = useModbus();
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
        console.error("Failed to fetch plunger settings for widget:", err);
      } finally {
        setPlungerSettingsLoading(false);
      }
    };
    fetchPlungerSettings();
  }, []);

  // Find all necessary registers
  const vfdRunFreqReg = registers.find(r => r.group?.startsWith(DELTA_VFD_GROUP) && r.name === DELTA_VFD_REGISTER_NAMES.RUNNING_FREQUENCY);
  const vfdCurrentReg = registers.find(r => r.group?.startsWith(DELTA_VFD_GROUP) && r.name === DELTA_VFD_REGISTER_NAMES.OUTPUT_CURRENT);
  const vfdTorqueReg = registers.find(r => r.group?.startsWith(DELTA_VFD_GROUP) && r.name === DELTA_VFD_REGISTER_NAMES.OUTPUT_TORQUE_PERCENT);
  
  const plungerStateReg = registers.find(r => r.group === PLUNGER_GROUP && r.name.startsWith(PLUNGER_REGISTER_NAMES.STATE));
  const plungerCommandReg = registers.find(r => r.group === PLUNGER_GROUP && r.name.startsWith(PLUNGER_REGISTER_NAMES.COMMAND));
  const plungerCommandRegAddr = plungerCommandReg?.address;

  const VFD_RUN_FREQ_DISPLAY_SCALE = 100;
  const VFD_CURRENT_DISPLAY_RAW = true;

  // Peak current tracking
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

  // Peak torque tracking
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

  if ((connecting || plungerSettingsLoading) && registers.length === 0 && !plungerSettings) {
    return <p className="p-4">Loading plunger control data...</p>; 
  }

  const vfdCurrentDisplayValue = vfdCurrentReg ? (VFD_CURRENT_DISPLAY_RAW ? vfdCurrentReg.value : (vfdCurrentReg.value / 1)) : null;
  const vfdTorqueDisplayValue = vfdTorqueReg?.value;
  const maxOpTimeSeconds = plungerSettings ? (plungerSettings.defaultMaxOperationDurationMs / 1000).toFixed(1) + ' s' : 'N/A';

  return (
    <PlungerControlCard 
      plungerStateReg={plungerStateReg}
      plungerCommandRegAddr={plungerCommandRegAddr}
      isDashboardView={false}
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
  );
};

export default PlungerControlWidget;




