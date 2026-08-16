import React, { useState, useEffect, useMemo, useRef } from 'react';
import { LineChart, Line, ResponsiveContainer, Tooltip, YAxis } from 'recharts';
import { Link2Off as WifiOff } from 'lucide-react';
import { T } from '../i18n';
import { useModbus } from '../contexts/ModbusContext';
import { RegisterData } from '@polymech/client-ts';
import { getSlaveIdFromGroup, getControllerStatus } from '../lib/controllerUtils';
import { SP_REGISTER_NAME_SUFFIX, SP_CMD_COMMAND_REGISTER_PREFIX, STOP_CMD_REGISTER_PREFIX, ENABLED_REGISTER_PREFIX, COMMS_WRITE_REGISTER_PREFIX, PV_REGISTER_NAME_SUFFIX } from '../constants';
import { Switch } from '@/components/ui/switch';
import { cn } from '@/lib/utils';
import { getModbusErrorDescription } from '../lib/modbusErrorMap';

export interface CassandraControllerCardProps {
  slaveId?: number;
  name?: string;
  pv?: number | string; // Optional: will be auto-derived from registers if not provided
  isRunning?: boolean; // Optional: will be auto-derived from Status High/Low if not provided
  //hasAlarm?: boolean;
  isAutoTuning?: boolean; // Optional: will be auto-derived from Status High/Low if not provided
  hasHeaterBreak?: boolean; // Optional: will be auto-derived from Status High/Low if not provided
  hasSensorBreak?: boolean; // Optional: will be auto-derived from Status High/Low if not provided
  mode?: 'manual' | 'auto' | 'cascade' | 'program' | 'unknown'; // Optional: will be auto-derived from Status High/Low if not provided
  currentProfile?: string | null;
  isHeating?: boolean; // Optional: will be auto-derived from Status High/Low if not provided
  // Widget instance management
  widgetInstanceId?: string;
  onPropsChange?: (props: Record<string, any>) => void;
}

interface HistoryPoint {
  timestamp: number;
  value: number;
}

const MAX_HISTORY_LENGTH_CASSANDRA = 30; // Number of data points for the PV chart
const PV_ERROR_VALUE_THERMOCOUPLE = 1320;

const STATUS_HIGH_REGISTER_NAME = "Status High";
const STATUS_LOW_REGISTER_NAME = "Status Low";

// Tooltip formatter for the chart
const renderTooltip = ({ active, payload }: any) => {
  if (active && payload && payload.length) {
    return (
      <div className="bg-slate-800/90 dark:bg-black/70 text-white p-1 rounded text-xs border border-slate-300/20 dark:border-white/20">
        {`${payload[0].value.toFixed(typeof payload[0].payload.value === 'number' && !Number.isInteger(payload[0].payload.value) ? 2 : 0)}`}
      </div>
    );
  }
  return null;
};

const CassandraControllerCard: React.FC<CassandraControllerCardProps> = ({
  slaveId = 1,
  name = 'Controller 1',
  pv: pvProp, // Rename to pvProp to distinguish from derived value
  isRunning: isRunningProp,
  isAutoTuning: isAutoTuningProp,
  hasHeaterBreak: hasHeaterBreakProp,
  hasSensorBreak: hasSensorBreakProp,
  mode: modeProp,
  currentProfile = null,
  isHeating: isHeatingProp
}) => {
  const { registers, coils, updateRegister, updateCoil } = useModbus();
  const [derivedSp, setDerivedSp] = useState<number | string>('N/A');
  const [derivedPv, setDerivedPv] = useState<number | string>('N/A');
  const [derivedStatus, setDerivedStatus] = useState({
    isRunning: false,
    isAutoTuning: false,
    hasHeaterBreak: false,
    hasSensorBreak: false,
    mode: 'unknown' as 'manual' | 'auto' | 'cascade' | 'program' | 'unknown',
    isHeating: false,
  });
  const [pvHistory, setPvHistory] = useState<HistoryPoint[]>([]);
  const [spHistory, setSpHistory] = useState<HistoryPoint[]>([]);
  const [showStartupEffect, setShowStartupEffect] = useState(false);
  const startupTimeoutRef = useRef<NodeJS.Timeout>();
  const [isEditingSp, setIsEditingSp] = useState<boolean>(false);
  const [spInputValue, setSpInputValue] = useState<string>("");
  const spInputRef = useRef<HTMLInputElement>(null);
  const [isTogglingEnable, setIsTogglingEnable] = useState<boolean>(false);
  const [isTogglingMasterEnable, setIsTogglingMasterEnable] = useState<boolean>(false);
  const [isTogglingCommsWrite, setIsTogglingCommsWrite] = useState<boolean>(false);
  const [registerError, setRegisterError] = useState<string | null>(null);
  const [registerErrorCode, setRegisterErrorCode] = useState<number | null>(null);
  const [heatingHistory, setHeatingHistory] = useState<{ timestamp: number; isHeating: boolean }[]>([]);

  // Use derived values if props are not provided, otherwise use props
  const pv = pvProp !== undefined ? pvProp : derivedPv;
  const isRunning = isRunningProp !== undefined ? isRunningProp : derivedStatus.isRunning;
  const isAutoTuning = isAutoTuningProp !== undefined ? isAutoTuningProp : derivedStatus.isAutoTuning;
  const hasHeaterBreak = hasHeaterBreakProp !== undefined ? hasHeaterBreakProp : derivedStatus.hasHeaterBreak;
  const hasSensorBreak = hasSensorBreakProp !== undefined ? hasSensorBreakProp : derivedStatus.hasSensorBreak;
  const mode = modeProp !== undefined ? modeProp : derivedStatus.mode;
  const isHeating = isHeatingProp !== undefined ? isHeatingProp : derivedStatus.isHeating;

  const [wasRunning, setWasRunning] = useState(isRunning);

  const isPvError = pv === PV_ERROR_VALUE_THERMOCOUPLE;
  const hasAnyError = !!registerError || hasHeaterBreak || hasSensorBreak || isPvError;
  const canEditSp = true;

  const { runStopCoil, masterEnableCoil, commsWriteCoil } = useMemo(() => {
    let runStop: any = null;
    let masterEnable: any = null;
    let commsWrite: any = null;

    if (coils) {
      for (const coil of coils) {
        if (getSlaveIdFromGroup(coil.group) !== slaveId) continue;

        if (coil.name.startsWith(STOP_CMD_REGISTER_PREFIX)) runStop = coil;
        else if (coil.name.startsWith(ENABLED_REGISTER_PREFIX)) masterEnable = coil;
        else if (coil.name.startsWith(COMMS_WRITE_REGISTER_PREFIX)) commsWrite = coil;
      }
    }

    return {
      runStopCoil: runStop,
      masterEnableCoil: masterEnable,
      commsWriteCoil: commsWrite
    };
  }, [coils, slaveId, STOP_CMD_REGISTER_PREFIX, ENABLED_REGISTER_PREFIX, COMMS_WRITE_REGISTER_PREFIX]);

  // If enableDisableCoil.value is true (e.g., 1), it means the controller is commanded to STOP.
  // So, isCommandedEnabled should be true (for UI, meaning "it IS enabled") when coil.value is false (e.g., 0).
  const isCommandedToRun = runStopCoil ? !runStopCoil.value : false;
  const isMasterEnabled = masterEnableCoil ? masterEnableCoil.value : false;
  const isCommsWriteEnabled = commsWriteCoil ? commsWriteCoil.value : false;

  // console.log(`Controller Slave : ${slaveId} : isCommandedToRun : ${isCommandedToRun} - SP : ${derivedSp} - PV : ${pv} - isMasterEnabled : ${isMasterEnabled} - isCommsWriteEnabled : ${isCommsWriteEnabled}`);

  const displayAsRunning = useMemo(() => {
    return runStopCoil ? isCommandedToRun : isRunning;
  }, [runStopCoil, isCommandedToRun, isRunning]);

  const handleRunStopToggle = async (newVisualSwitchState: boolean) => {
    if (!runStopCoil) return;
    const newCoilValue = !newVisualSwitchState;
    setIsTogglingEnable(true);
    try {
      await updateCoil(runStopCoil.address, newCoilValue);
    } catch (error) {
      console.error(`Failed to toggle enable state for controller ${slaveId}:`, error);
    } finally {
      setIsTogglingEnable(false);
    }
  };

  const handleMasterEnableToggle = async (newEnabledState: boolean) => {
    if (!masterEnableCoil) return;
    setIsTogglingMasterEnable(true);
    try {
      await updateCoil(masterEnableCoil.address, newEnabledState);
    } catch (error) {
      console.error(`Failed to toggle master enable for controller ${slaveId}:`, error);
    } finally {
      setIsTogglingMasterEnable(false);
    }
  };

  const handleCommsWriteToggle = async (newEnabledState: boolean) => {
    if (!commsWriteCoil) return;
    setIsTogglingCommsWrite(true);
    try {
      await updateCoil(commsWriteCoil.address, newEnabledState);
    } catch (error) {
      console.error(`Failed to toggle comms write for controller ${slaveId}:`, error);
    } finally {
      setIsTogglingCommsWrite(false);
    }
  };

  // Handle running state changes
  useEffect(() => {
    if (displayAsRunning && !wasRunning) {
      // Controller just started running
      setShowStartupEffect(false);
      startupTimeoutRef.current = setTimeout(() => {
        setShowStartupEffect(false);
      }, 100); // Effect lasts 1 second (changed from 2000)
    }
    setWasRunning(displayAsRunning);
    return () => {
      if (startupTimeoutRef.current) {
        clearTimeout(startupTimeoutRef.current);
      }
    };
  }, [displayAsRunning, wasRunning]);

  // Effect to derive SP from registers
  useEffect(() => {
    if (registers && registers.length > 0) {
      const spRegister = registers.find((reg: RegisterData) =>
        getSlaveIdFromGroup(reg.group) === slaveId && reg.name.endsWith(SP_REGISTER_NAME_SUFFIX)
      );
      if (spRegister && typeof spRegister.value === 'number') {
        setDerivedSp(spRegister.value);
      } else {
        setDerivedSp('N/A');
      }
    } else {
      // setDerivedSp('N/A'); // Keep previous value or set to N/A if registers are cleared
    }
  }, [registers, slaveId]);

  // Effect to derive PV from registers (if not provided as prop)
  useEffect(() => {
    // Only derive if not provided as prop
    if (pvProp === undefined && registers && registers.length > 0) {
      const pvRegister = registers.find((reg: RegisterData) =>
        getSlaveIdFromGroup(reg.group) === slaveId && reg.name.endsWith(PV_REGISTER_NAME_SUFFIX)
      );
      if (pvRegister && typeof pvRegister.value === 'number') {
        setDerivedPv(pvRegister.value);
      } else {
        setDerivedPv('N/A');
      }
    }
  }, [registers, slaveId, pvProp]);

  // Effect to derive controller status from registers (if not provided as props)
  useEffect(() => {
    // Only derive if at least one status prop is not provided
    const needsDerivation = isRunningProp === undefined || isAutoTuningProp === undefined ||
      hasHeaterBreakProp === undefined || hasSensorBreakProp === undefined ||
      modeProp === undefined || isHeatingProp === undefined;

    if (needsDerivation && registers && registers.length > 0) {
      const statusHighRegister = registers.find((reg: RegisterData) =>
        getSlaveIdFromGroup(reg.group) === slaveId && reg.name === STATUS_HIGH_REGISTER_NAME
      );
      const statusLowRegister = registers.find((reg: RegisterData) =>
        getSlaveIdFromGroup(reg.group) === slaveId && reg.name === STATUS_LOW_REGISTER_NAME
      );

      const status = getControllerStatus(statusHighRegister, statusLowRegister);
      setDerivedStatus(status);
    }
  }, [registers, slaveId, isRunningProp, isAutoTuningProp, hasHeaterBreakProp, hasSensorBreakProp, modeProp, isHeatingProp]);

  useEffect(() => {
    const now = Date.now();
    // Update PV history only if it's a valid number and not the specific error code
    if (typeof pv === 'number' && !isNaN(pv) && pv !== PV_ERROR_VALUE_THERMOCOUPLE) {
      setPvHistory(prevHistory => {
        const newHistory = [...prevHistory, { timestamp: now, value: pv }];
        return newHistory.length > MAX_HISTORY_LENGTH_CASSANDRA
          ? newHistory.slice(newHistory.length - MAX_HISTORY_LENGTH_CASSANDRA)
          : newHistory;
      });
    } else if (pv === PV_ERROR_VALUE_THERMOCOUPLE) {
      // Optionally, clear history or add a specific marker if needed when error occurs
      // For now, we just don't add the error value to the history meant for 0-300 range.
    }

    // Updated to use derivedSp
    if (typeof derivedSp === 'number' && !isNaN(derivedSp)) {
      setSpHistory(prevHistory => {
        const newHistory = [...prevHistory, { timestamp: now, value: derivedSp }];
        return newHistory.length > MAX_HISTORY_LENGTH_CASSANDRA
          ? newHistory.slice(newHistory.length - MAX_HISTORY_LENGTH_CASSANDRA)
          : newHistory;
      });
    }
  }, [pv, derivedSp]);

  // Track heating history for oscillation calculation
  useEffect(() => {
    if (typeof isHeating !== 'boolean') return;
    const now = Date.now();
    setHeatingHistory(prev => {
      // Keep last 60 seconds of history
      const newHistory = [...prev, { timestamp: now, isHeating }];
      const cutoff = now - 60000;
      return newHistory.filter(pt => pt.timestamp >= cutoff);
    });
  }, [isHeating]);

  const oscillationsPerMin = useMemo(() => {
    if (heatingHistory.length < 2) return 0;

    let transitions = 0;
    for (let i = 1; i < heatingHistory.length; i++) {
      // Count False -> True transitions (turn on events)
      if (!heatingHistory[i - 1].isHeating && heatingHistory[i].isHeating) {
        transitions++;
      }
    }

    // Extrapolate if less than a minute of data? 
    // Or just show actual count in window. 
    // User asked for "x oscilations per min".
    // If we have < 1 min data, maybe just show current count or project it?
    // Let's stick to simple count within the sliding window (which grows to 1 min).
    // If window is very short, rate might be misleading if extrapolated.

    return transitions;
  }, [heatingHistory]);

  const heatingRate = useMemo(() => {
    if (pvHistory.length < 2) return null;
    const latest = pvHistory[pvHistory.length - 1];
    const oldest = pvHistory[0];
    const timeSpanMs = latest.timestamp - oldest.timestamp;

    // We want at least a few seconds of data to show a rate, ideally approaching 1 min
    // user asked for "degc / 1 min"
    if (timeSpanMs < 5000) return null; // Wait for at least 5s of data

    const deltaV = latest.value - oldest.value;
    const ratePerMinute = (deltaV / timeSpanMs) * 60000;

    return ratePerMinute.toFixed(1);
  }, [pvHistory]);

  // Auto-focus SP input when editing starts
  useEffect(() => {
    if (isEditingSp && spInputRef.current) {
      spInputRef.current.focus();
      const currentSpString = (typeof derivedSp === 'number' ? derivedSp.toFixed(1) : String(derivedSp === 'N/A' ? '' : derivedSp));
      spInputRef.current.value = currentSpString;
      setSpInputValue(currentSpString);
      spInputRef.current.select();
    }
  }, [isEditingSp, derivedSp]);

  const handleSpSubmit = async () => {

    if (!isEditingSp || !canEditSp) return;

    const numericSp = parseFloat(spInputValue);
    if (isNaN(numericSp)) {
      console.error("Invalid SP value entered.");
      setIsEditingSp(false);
      return;
    }

    if (!registers || registers.length === 0) {
      console.error("Registers not available for SP update.");
      setIsEditingSp(false);
      return;
    }

    const commandRegister = registers.find(
      (reg: RegisterData) =>
        getSlaveIdFromGroup(reg.group) === slaveId &&
        reg.name.startsWith(SP_CMD_COMMAND_REGISTER_PREFIX)
    );

    if (!commandRegister) {
      console.error(`SP command register with prefix '${SP_CMD_COMMAND_REGISTER_PREFIX}' not found for slave ID ${slaveId}.`);
      setIsEditingSp(false);
      return;
    }

    try {
      await updateRegister(commandRegister.address, numericSp);
      // derivedSp will update via WebSocket push, no need to set it directly here
    } catch (error) {
      console.error("Failed to update SP:", error);
      // Optionally, revert spInputValue or show error to user before exiting edit mode
    } finally {
      setIsEditingSp(false);
    }
  };

  const handleSpInputKeyDown = (event: React.KeyboardEvent<HTMLInputElement>) => {
    if (event.key === 'Enter') {
      handleSpSubmit();
    } else if (event.key === 'Escape') {
      setIsEditingSp(false);
    }
  };

  const startSpEdit = () => {
    if (!canEditSp) return;
    const currentSpString = (typeof derivedSp === 'number' ? derivedSp.toFixed(1) : String(derivedSp === 'N/A' ? '' : derivedSp));
    setSpInputValue(currentSpString);
    setIsEditingSp(true);
  };

  useEffect(() => {
    if (registers && registers.length > 0) {
      // Find the first register for this slave with a non-zero error
      const errorRegister = registers.find(reg =>
        getSlaveIdFromGroup(reg.group) === slaveId && reg.error && reg.error !== 0
      );

      if (errorRegister && typeof errorRegister.error === 'number') {
        setRegisterError(getModbusErrorDescription(errorRegister.error));
        setRegisterErrorCode(errorRegister.error);
      } else {
        setRegisterError(null);
        setRegisterErrorCode(null);
      }
    } else {
      setRegisterError(null);
      setRegisterErrorCode(null);
    }
  }, [registers, slaveId]);

  const getStatusBadge = () => {
    if (registerErrorCode === 224) return null;
    if (registerError) return <span className="text-xs text-red-500 font-semibold"><T>{registerError}</T></span>;
    if (!displayAsRunning) return <span className="text-xs text-slate-500 dark:text-slate-400"><T>Stopped</T></span>;
    if (isHeating) return <span className="text-xs text-red-500 font-semibold"><T>Heating</T></span>;
    //if (hasAlarm) return <span className="text-xs text-red-500"><T>Alarm</T></span>;
    if (isAutoTuning) return <span className="text-xs text-amber-500"><T>Auto-tuning</T></span>;
    if (hasHeaterBreak) return <span className="text-xs text-red-500"><T>Heater Break</T></span>;
    if (hasSensorBreak) return <span className="text-xs text-red-500"><T>Sensor Break</T></span>;
    return <span className="text-xs text-slate-500 dark:text-slate-400"><T>{mode}</T></span>;
  };

  const getCardStyle = () => {
    const baseStyle = "grid grid-cols-3 items-center gap-x-2 gap-y-1 p-3 glass-card hover:shadow-xl transition-all duration-500";
    const errorStyle = isPvError ? 'border-red-400/60 ring-1 ring-red-400/30' : '';
    const runningStyle = !displayAsRunning ? '' : 'opacity-100';
    //const alarmStyle = hasAlarm ? 'border-red-500' : '';
    const autoTuningStyle = isAutoTuning ? 'border-amber-400/60 ring-1 ring-amber-400/30' : '';
    const startupStyle = showStartupEffect ? 'animate-pulse ring-2 ring-emerald-400/50' : '';

    return cn(baseStyle, errorStyle, runningStyle, autoTuningStyle, startupStyle);
  };

  const getCardOpacity = () => {
    return !isMasterEnabled ? 'opacity-50' : 'opacity-100';
  }

  return (
    <div className={cn(getCardStyle(), getCardOpacity())}>
      <div className="font-medium col-span-3 text-sm mb-1 pb-1 border-b border-slate-300/30 dark:border-white/10 flex justify-between items-center">
        <div className="flex items-center gap-x-2">
          {masterEnableCoil && (
            <div className="flex flex-col items-center gap-0.5">
              <Switch
                id={`master-enable-controller-${slaveId}`}
                checked={isMasterEnabled}
                onCheckedChange={handleMasterEnableToggle}
                disabled={isTogglingMasterEnable}
                className={cn(isMasterEnabled && "data-[state=checked]:bg-emerald-500", "scale-75 origin-bottom")}
                aria-label="Toggle Controller Enabled"
                title={isMasterEnabled ? 'Controller Enabled' : 'Controller Disabled'}
              />
              <span className="text-[9px] uppercase font-bold text-slate-500 dark:text-slate-400 leading-none"><T>Enbl</T></span>
            </div>
          )}
          {runStopCoil && (
            <div className="flex flex-col items-center gap-0.5">
              <Switch
                id={`run-stop-controller-${slaveId}`}
                checked={isCommandedToRun}
                onCheckedChange={handleRunStopToggle}
                disabled={isTogglingEnable}
                className={cn(isCommandedToRun && "data-[state=checked]:bg-blue-500", "scale-75 origin-bottom")}
                aria-label="Toggle Run/Stop"
                title={isCommandedToRun ? 'Controller Running' : 'Controller Stopped'}
              />
              <span className="text-[9px] uppercase font-bold text-slate-500 dark:text-slate-400 leading-none"><T>Run</T></span>
            </div>
          )}
          {commsWriteCoil && (
            <div className="flex flex-col items-center gap-0.5">
              <Switch
                id={`comms-write-controller-${slaveId}`}
                checked={isCommsWriteEnabled}
                onCheckedChange={handleCommsWriteToggle}
                disabled={isTogglingCommsWrite}
                className={cn(isCommsWriteEnabled && "data-[state=checked]:bg-sky-500", "scale-75 origin-bottom")}
                aria-label="Toggle Comms Write"
                title={isCommsWriteEnabled ? 'Comms Write Enabled' : 'Comms Write Disabled'}
              />
              <span className="text-[9px] uppercase font-bold text-slate-500 dark:text-slate-400 leading-none"><T>Comms</T></span>
            </div>
          )}
          <span className="transition-colors duration-500 text-slate-700 dark:text-white">
            {name ? <T>{name}</T> : <T>Controller {slaveId}</T>} (ID: {slaveId})
          </span>
        </div>
        <div className="justify-self-end flex items-center gap-2">
          {registerErrorCode === 224 && <WifiOff className="w-4 h-4 text-red-500" />}
          <div
            title={isHeating ? "Heating" : "Not Heating"}
            className={cn(
              "w-4 h-4 rounded-full",
              isHeating
                ? `bg-red-600 ${!hasAnyError ? "animate-pulse shadow-[0_0_10px_3px_rgba(239,68,68,0.7)]" : ""}`
                : "bg-gray-400 opacity-50"
            )}
          ></div>
        </div>
      </div>

      {/* New Row for Profile Name */}
      {currentProfile && (
        <div className="col-span-3 text-right mb-1 mt-0.5">
          <span className="text-xs text-purple-700 bg-purple-100 px-1.5 py-0.5 rounded-full font-medium whitespace-nowrap">
            <T>Profile</T>: {currentProfile}
          </span>
        </div>
      )}

      {/* PV Row */}
      <div className="text-xs col-span-1 font-semibold self-center"><T>PV</T>:</div>
      <div className={`text-sm col-span-1 font-mono justify-self-start self-center ${isPvError ? 'text-red-500 font-bold' : ''}`}>
        {isPvError ? <T>T/C Err</T> : (pv === 'N/A' ? <T>N/A</T> : (typeof pv === 'number' ? pv.toFixed(1) : pv))}
      </div>
      <div className="col-span-1 h-8 w-full self-center"> {/* Container for PV chart */}
        {!isPvError && typeof pv === 'number' && pvHistory.length > 1 && (
          <ResponsiveContainer width="100%" height="100%">
            <LineChart data={pvHistory} margin={{ top: 2, right: 0, left: 0, bottom: 2 }}>
              <Tooltip content={renderTooltip} cursor={{ stroke: 'rgba(128, 128, 128, 0.3)', strokeWidth: 1 }} />
              <YAxis domain={[0, 300]} hide />
              <Line type="monotone" dataKey="value" stroke="#8884d8" strokeWidth={1.5} dot={false} isAnimationActive={false} />
            </LineChart>
          </ResponsiveContainer>
        )}
      </div>

      {/* SP Row - Now with Inline Editing */}
      <div className="text-xs col-span-1 font-semibold self-center"><T>SP</T>:</div>
      {isEditingSp ? (
        <div className="col-span-1 flex items-center">
          <input
            ref={spInputRef}
            type="number"
            value={spInputValue}
            onChange={(e) => setSpInputValue(e.target.value)}
            onBlur={handleSpSubmit}
            onKeyDown={handleSpInputKeyDown}
            disabled={!canEditSp}
            className="text-sm p-1 glass-input rounded w-full font-mono disabled:opacity-50"
            style={{ minWidth: '60px' }}
          />
        </div>
      ) : (
        <div
          className={`text-sm col-span-1 font-mono justify-self-start self-center p-1 ${canEditSp ? 'cursor-pointer hover:bg-muted/50 rounded underline decoration-dashed decoration-blue-500' : ''}`}
          onClick={canEditSp ? startSpEdit : undefined}
          title={canEditSp ? "Click to edit SP" : (currentProfile ? "SP controlled by profile" : "Controller stopped or SP not editable")}
        >
          {derivedSp === 'N/A' ? <T>N/A</T> : (typeof derivedSp === 'number' ? derivedSp.toFixed(1) : derivedSp)}
        </div>
      )}
      <div className="col-span-1 h-8 w-full self-center"> {/* Container for SP chart */}
        {typeof derivedSp === 'number' && spHistory.length > 1 && (
          <ResponsiveContainer width="100%" height="100%">
            <LineChart data={spHistory} margin={{ top: 2, right: 0, left: 0, bottom: 2 }}>
              <Tooltip content={renderTooltip} cursor={{ stroke: 'rgba(128, 128, 128, 0.3)', strokeWidth: 1 }} />
              <YAxis domain={[0, 300]} hide />
              <Line type="monotone" dataKey="value" stroke="#06b6d4" strokeWidth={1.5} dot={false} isAnimationActive={false} />
            </LineChart>
          </ResponsiveContainer>
        )}
      </div>

      {/* Status Badge Row */}
      <div className="col-span-3 text-center mt-2 pt-2 border-t border-slate-300/30 dark:border-white/10 flex flex-col items-center">
        <div className="flex flex-col items-center mb-1">
          {heatingRate !== null && (
            <span className="text-[10px] text-slate-400 mt-0.5">
              {Number(heatingRate) > 0 ? '+' : ''}{heatingRate} °C / 1 min
            </span>
          )}
          <span className="text-[10px] text-slate-400 leading-none">
            {oscillationsPerMin} osc / min
          </span>
        </div>
        {getStatusBadge()}
      </div>
    </div>
  );
};

export default CassandraControllerCard; 