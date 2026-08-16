import React, { useState, useEffect, useMemo, useRef } from 'react';
import { LineChart, Line, ResponsiveContainer, Tooltip, YAxis } from 'recharts';
import { T } from '../i18n';
import { useModbus } from '../contexts/ModbusContext';
import { RegisterData, CoilData } from '@polymech/client-ts';
import { getSlaveIdFromGroup } from '../lib/controllerUtils';
import { SP_REGISTER_NAME_SUFFIX, SP_CMD_COMMAND_REGISTER_PREFIX, ENABLE_CMD_REGISTER_PREFIX } from '../constants';
import { Switch } from '@/components/ui/switch';
import { cn } from '@/lib/utils';

export interface CassandraControllerCardProps {
  slaveId: number;
  name?: string;
  pv: number | string;
  isRunning?: boolean;
  hasAlarm?: boolean;
  isAutoTuning?: boolean;
  hasHeaterBreak?: boolean;
  hasSensorBreak?: boolean;
  mode?: 'manual' | 'auto' | 'cascade' | 'program' | 'unknown';
  currentProfile?: string | null;
  isHeating?: boolean;
}

interface HistoryPoint {
  timestamp: number;
  value: number;
}

const MAX_HISTORY_LENGTH_CASSANDRA = 30; // Number of data points for the PV chart
const PV_ERROR_VALUE_THERMOCOUPLE = 1320;

// Tooltip formatter for the chart
const renderTooltip = ({ active, payload }: any) => {
  if (active && payload && payload.length) {
    return (
      <div className="bg-black/70 text-white p-1 rounded text-xs border border-white/20">
        {`${payload[0].value.toFixed(typeof payload[0].payload.value === 'number' && !Number.isInteger(payload[0].payload.value) ? 2 : 0)}`}
      </div>
    );
  }
  return null;
};

const CassandraControllerCard: React.FC<CassandraControllerCardProps> = ({ 
  slaveId, 
  name, 
  pv, 
  isRunning: isRunningAsProp = true,
  hasAlarm = false,
  isAutoTuning = false,
  hasHeaterBreak = false,
  hasSensorBreak = false,
  mode = 'unknown',
  currentProfile = null,
  isHeating = false
}) => {
  const { registers, coils, updateRegister, updateCoil } = useModbus();
  const [derivedSp, setDerivedSp] = useState<number | string>('N/A');
  const [pvHistory, setPvHistory] = useState<HistoryPoint[]>([]);
  const [spHistory, setSpHistory] = useState<HistoryPoint[]>([]);
  const [wasRunning, setWasRunning] = useState(isRunningAsProp);
  const [showStartupEffect, setShowStartupEffect] = useState(false);
  const startupTimeoutRef = useRef<NodeJS.Timeout>();
  const [isEditingSp, setIsEditingSp] = useState<boolean>(false);
  const [spInputValue, setSpInputValue] = useState<string>("");
  const spInputRef = useRef<HTMLInputElement>(null);
  const [isTogglingEnable, setIsTogglingEnable] = useState<boolean>(false);

  const isPvError = pv === PV_ERROR_VALUE_THERMOCOUPLE;
  const canEditSp = true;

  const enableDisableCoil = useMemo(() => {
    if (!coils) return null;
    return coils.find(coil => 
      getSlaveIdFromGroup(coil.group) === slaveId && 
      coil.name.startsWith(ENABLE_CMD_REGISTER_PREFIX)
    );
  }, [coils, slaveId, ENABLE_CMD_REGISTER_PREFIX]);

  // If enableDisableCoil.value is true (e.g., 1), it means the controller is commanded to STOP.
  // So, isCommandedEnabled should be true (for UI, meaning "it IS enabled") when coil.value is false (e.g., 0).
  const isCommandedEnabled = enableDisableCoil ? !enableDisableCoil.value : false;

  const displayAsRunning = useMemo(() => {
    return enableDisableCoil ? isCommandedEnabled : isRunningAsProp;
  }, [enableDisableCoil, isCommandedEnabled, isRunningAsProp]);

  const handleEnableToggle = async (newVisualSwitchState: boolean) => {
    if (!enableDisableCoil) return;
    const newCoilValue = !newVisualSwitchState;
    setIsTogglingEnable(true);
    try {
      await updateCoil(enableDisableCoil.address, newCoilValue);
    } catch (error) {
      console.error(`Failed to toggle enable state for controller ${slaveId}:`, error);
    } finally {
      setIsTogglingEnable(false);
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
      console.log(`SP update command sent for slave ID ${slaveId} to address ${commandRegister.address} with value ${numericSp}`);
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

  const getStatusBadge = () => {
    if (!displayAsRunning) return <span className="text-xs text-muted-foreground"><T>Stopped</T></span>;
    if (isHeating) return <span className="text-xs text-red-500 font-semibold"><T>Heating</T></span>;
    if (hasAlarm) return <span className="text-xs text-red-500"><T>Alarm</T></span>;
    if (isAutoTuning) return <span className="text-xs text-yellow-500"><T>Auto-tuning</T></span>;
    if (hasHeaterBreak) return <span className="text-xs text-red-500"><T>Heater Break</T></span>;
    if (hasSensorBreak) return <span className="text-xs text-red-500"><T>Sensor Break</T></span>;
    return <span className="text-xs text-muted-foreground"><T>{mode}</T></span>;
  };

  const getCardStyle = () => {
    const baseStyle = "grid grid-cols-3 items-center gap-x-2 gap-y-1 p-3 border rounded-lg bg-card shadow hover:shadow-md transition-all duration-500";
    const errorStyle = isPvError ? 'border-red-500' : '';
    const runningStyle = !displayAsRunning ? 'opacity-50 grayscale' : 'opacity-100';
    const alarmStyle = hasAlarm ? 'border-red-500' : '';
    const autoTuningStyle = isAutoTuning ? 'border-yellow-500' : '';
    const startupStyle = showStartupEffect ? 'animate-pulse ring-2 ring-green-500 ring-opacity-50' : '';
    
    return `${baseStyle} ${errorStyle} ${runningStyle} ${alarmStyle} ${autoTuningStyle} ${startupStyle}`;
  };

  return (
    <div className={getCardStyle()}>
      <div className="font-medium col-span-3 text-sm mb-1 pb-1 border-b flex justify-between items-center">
        <span className="transition-colors duration-500">
          {name ? <T>{name}</T> : <T>Controller {slaveId}</T>} (ID: {slaveId})
        </span>
        <div className="grid grid-cols-3 items-center gap-x-2 gap-y-1 justify-items-end">
          <div className="justify-self-end">
            <div 
              title={isHeating ? "Heating" : "Not Heating"}
              className={cn(
                "w-4 h-4 rounded-full",
                isHeating 
                  ? "bg-red-600 animate-pulse shadow-[0_0_10px_3px_rgba(239,68,68,0.7)]"
                  : "bg-gray-400 opacity-50"
              )}
            ></div>
          </div>
          {enableDisableCoil && (
            <div className="flex items-center space-x-1.5">
              <span className={cn(
                "text-xs font-mono",
                isCommandedEnabled ? "text-green-400" : "text-muted-foreground"
              )}>
                {}
              </span>
              <Switch
                id={`enable-controller-${slaveId}`}
                checked={isCommandedEnabled}
                onCheckedChange={handleEnableToggle}
                disabled={isTogglingEnable}
                className={cn(isCommandedEnabled && "bg-primary")}
                aria-label="Toggle Enable Controller"
              />
            </div>
          )}          
          {getStatusBadge()}
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
              <Line type="monotone" dataKey="value" stroke="#8884d8" strokeWidth={1.5} dot={false} isAnimationActive={false}/>
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
            className="text-sm p-1 border rounded w-full bg-background text-foreground font-mono disabled:opacity-50"
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
              <Line type="monotone" dataKey="value" stroke="#82ca9d" strokeWidth={1.5} dot={false} isAnimationActive={false}/>
            </LineChart>
          </ResponsiveContainer>
        )}
      </div>
    </div>
  );
};

export default CassandraControllerCard; 