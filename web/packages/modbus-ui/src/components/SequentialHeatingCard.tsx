import { useModbus } from '@/contexts/ModbusContext';
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card';
import { Input } from '@/components/ui/input';
import { Label } from '@/components/ui/label';
import { T } from '../i18n';
import { cn } from '@/lib/utils';
import { CoilSwitch } from '@/components/CoilSwitch';
import { useState, useEffect, KeyboardEvent } from 'react';
import {
  Popover,
  PopoverContent,
  PopoverTrigger,
} from "@/components/ui/popover";
import { AlertCircle, HelpCircle } from 'lucide-react';
import {
  Select,
  SelectContent,
  SelectItem,
  SelectTrigger,
  SelectValue,
} from "@/components/ui/select";

import {
  HEATING_TIME,
  HEATING_TIME_S,
  REGISTER_NAMES,
  REGISTER_GROUPS,
  STATUS,
  MAX_VALUES,
  MIN_VALUES
} from '@/constants';

interface SequentialHeatingCardProps {

}

interface InputError {
  field: string;
  message: string;
  serverError?: string;
}

const SequentialHeatingCard: React.FC<SequentialHeatingCardProps> = ({ }) => {
  const { registers, coils, updateRegister } = useModbus();
  const [localMaxTime, setLocalMaxTime] = useState<string>('0');
  const [localMaxSim, setLocalMaxSim] = useState<string>('0');
  const [localOffset, setLocalOffset] = useState<string>('0');
  const [localStartIndex, setLocalStartIndex] = useState<string>('0');
  const [localEndIndex, setLocalEndIndex] = useState<string>('0');
  const [localMode, setLocalMode] = useState<string>('0');
  const [error, setError] = useState<InputError | null>(null);
  const [isUpdating, setIsUpdating] = useState(false);

  // Find the enable coil
  const enableCoil = coils.find(coil =>
    coil.group === REGISTER_GROUPS.AMPERAGE_BUDGET &&
    coil.name === REGISTER_NAMES.ENABLE
  );

  // Find the relevant registers
  const infoRegister = registers.find(reg =>
    reg.group === REGISTER_GROUPS.AMPERAGE_BUDGET &&
    reg.name === REGISTER_NAMES.INFO
  );

  const maxTimeRegister = registers.find(reg =>
    reg.group === REGISTER_GROUPS.AMPERAGE_BUDGET &&
    reg.name === REGISTER_NAMES.MAX_TIME
  );

  const maxSimRegister = registers.find(reg =>
    reg.group === REGISTER_GROUPS.AMPERAGE_BUDGET &&
    reg.name === REGISTER_NAMES.MAX_SIM
  );

  const offsetRegister = registers.find(reg =>
    reg.group === REGISTER_GROUPS.AMPERAGE_BUDGET &&
    reg.name === REGISTER_NAMES.OFFSET
  );

  const startIndexRegister = registers.find(reg =>
    reg.group === REGISTER_GROUPS.AMPERAGE_BUDGET &&
    reg.name === REGISTER_NAMES.START
  );

  const endIndexRegister = registers.find(reg =>
    reg.group === REGISTER_GROUPS.AMPERAGE_BUDGET &&
    reg.name === REGISTER_NAMES.END
  );

  const modeRegister = registers.find(reg =>
    reg.group === REGISTER_GROUPS.AMPERAGE_BUDGET &&
    reg.name.startsWith(REGISTER_NAMES.MODE)
  );

  // Update local state when register values change
  useEffect(() => {
    if (maxTimeRegister?.value !== undefined) setLocalMaxTime(maxTimeRegister.value.toString());
    if (maxSimRegister?.value !== undefined) setLocalMaxSim(maxSimRegister.value.toString());
    if (offsetRegister?.value !== undefined) setLocalOffset(offsetRegister.value.toString());
    if (startIndexRegister?.value !== undefined) setLocalStartIndex(startIndexRegister.value.toString());
    if (endIndexRegister?.value !== undefined) setLocalEndIndex(endIndexRegister.value.toString());
    if (modeRegister?.value !== undefined) setLocalMode(modeRegister.value.toString());
  }, [
    maxTimeRegister?.value,
    maxSimRegister?.value,
    offsetRegister?.value,
    startIndexRegister?.value,
    endIndexRegister?.value,
    modeRegister?.value
  ]);

  const validateMaxTime = (value: number): boolean => {
    if (isNaN(value)) return false;
    if (value < HEATING_TIME_S.MIN_S) return false;
    if (value > HEATING_TIME_S.MAX_S) return false;
    return true;
  };

  const validateMaxSim = (value: number): boolean => {
    if (isNaN(value)) return false;
    if (value < MIN_VALUES.SIMULTANEOUS) return false;
    if (value > MAX_VALUES.SIMULTANEOUS) return false;
    return true;
  };

  const validateOffset = (value: number): boolean => {
    if (isNaN(value)) return false;
    if (value < MIN_VALUES.OFFSET) return false;
    if (value > MAX_VALUES.OFFSET) return false;
    return true;
  };

  const validateStartIndex = (value: number): boolean => {
    if (isNaN(value)) return false;
    // Add specific validation for startIndex if necessary, e.g., range
    // For now, basic NaN check
    return true;
  };

  const validateEndIndex = (value: number): boolean => {
    if (isNaN(value)) return false;
    // Add specific validation for endIndex if necessary, e.g., range, greater than startIndex
    // For now, basic NaN check
    const startIndex = parseInt(localStartIndex);
    if (!isNaN(startIndex) && value < startIndex) return false; // End index must be >= start index
    return true;
  };

  const handleUpdate = async (
    value: string,
    register: typeof maxTimeRegister,
    validate: ((value: number) => boolean) | null,
    fieldName: string
  ) => {
    const numValue = parseInt(value);

    // Only validate if a validation function is provided
    if (validate && !validate(numValue)) {
      setError({ field: fieldName, message: `Invalid value for ${fieldName}` });
      return;
    }

    if (!register) {
      setError({ field: fieldName, message: `Register not found for ${fieldName}` });
      return;
    }

    try {
      setIsUpdating(true);
      setError(null);
      await updateRegister(register.address, numValue);
    } catch (err) {
      const serverError = err instanceof Error ? err.message : 'Unknown error';
      setError({
        field: fieldName,
        message: `Failed to update ${fieldName}`,
        serverError
      });
      // Revert to previous register value
      if (register.value !== undefined) {
        switch (fieldName) {
          case 'Max Time': setLocalMaxTime(register.value.toString()); break;
          case 'Max Simultaneous': setLocalMaxSim(register.value.toString()); break;
          case 'Window Offset': setLocalOffset(register.value.toString()); break;
          case 'Start Index': setLocalStartIndex(register.value.toString()); break;
          case 'End Index': setLocalEndIndex(register.value.toString()); break;
          case 'Mode': setLocalMode(register.value.toString()); break;
        }
      }
    } finally {
      setIsUpdating(false);
    }
  };

  const handleMaxTimeChange = () => handleUpdate(localMaxTime, maxTimeRegister, validateMaxTime, 'Max Time');
  const handleMaxSimChange = () => handleUpdate(localMaxSim, maxSimRegister, validateMaxSim, 'Max Simultaneous');
  const handleOffsetChange = () => handleUpdate(localOffset, offsetRegister, validateOffset, 'Window Offset');
  const handleStartIndexChange = () => handleUpdate(localStartIndex, startIndexRegister, validateStartIndex, 'Start Index');
  const handleEndIndexChange = () => handleUpdate(localEndIndex, endIndexRegister, validateEndIndex, 'End Index');
  const handleModeChange = (value: string) => {
    handleUpdate(value, modeRegister, null, 'Mode');
  };

  const handleKeyDown = (e: KeyboardEvent<HTMLInputElement>, handler: () => Promise<void>) => {
    if (e.key === 'Enter') {
      e.currentTarget.blur();
      handler();
    }
  };

  const formatTime = (ms: number): string => {
    if (ms >= 3600000) { // 1 hour or more
      const hours = Math.floor(ms / 3600000);
      const minutes = Math.floor((ms % 3600000) / 60000);
      return `${hours}h ${minutes}m`;
    } else if (ms >= 60000) { // 1 minute or more
      const minutes = Math.floor(ms / 60000);
      const seconds = Math.floor((ms % 60000) / 1000);
      return `${minutes}m ${seconds}s`;
    } else {
      return `${ms}ms`;
    }
  };

  const isEnabled = enableCoil?.value;
  const currentInfo = infoRegister?.value || 0;

  const getInputClassName = (fieldName: string) => cn(
    "w-full min-w-[200px] glass-input",
    error?.field === fieldName && "border-red-500 focus-visible:ring-red-500"
  );

  return (
    <Card id="sequential-heating-card" className={cn(
      "glass-card",
      isEnabled ? "border-emerald-400/50 ring-1 ring-emerald-400/30" : ""
    )}>
      <CardHeader className="pb-3 pt-4">
        <div className="flex justify-between items-center">
          <div className="flex items-center gap-2">
            <CardTitle className="text-lg text-slate-700 dark:text-white"><T>Sequential Heating</T></CardTitle>
            <Popover>
              <PopoverTrigger asChild>
                <button className="text-slate-400 hover:text-slate-600 dark:hover:text-slate-200 transition-colors">
                  <HelpCircle className="h-4 w-4" />
                  <span className="sr-only">Help</span>
                </button>
              </PopoverTrigger>
              <PopoverContent className="w-80 p-4 space-y-2 bg-white/95 dark:bg-slate-900/95 backdrop-blur-md border border-slate-200 dark:border-slate-800">
                <h4 className="font-semibold text-sm mb-2">Parameter Guide</h4>
                <div className="space-y-3 text-xs text-slate-600 dark:text-slate-300">
                  <div>
                    <span className="font-medium block text-slate-900 dark:text-slate-100">Heating Time</span>
                    Maximum duration a specific device can stay ON before forced switch-off.
                  </div>
                  <div>
                    <span className="font-medium block text-slate-900 dark:text-slate-100">Max Simultaneous</span>
                    Maximum number of devices allowed to heat at the same time.
                  </div>
                  <div>
                    <span className="font-medium block text-slate-900 dark:text-slate-100">Window Offset</span>
                    Number of steps the heating "window" slides after a batch completes.
                  </div>
                  <div>
                    <span className="font-medium block text-slate-900 dark:text-slate-100">Mode</span>
                    <ul className="list-disc list-inside mt-1 space-y-0.5">
                      <li><b>Cycle All:</b> Strict round-robin.</li>
                      <li><b>Cycle by SP:</b> Round-robin but checks Setpoint.</li>
                      <li><b>Heat Any:</b> Any device needing heat (up to Max Sim).</li>
                      <li><b>Heat Most Urgent:</b> Prioritizes devices with largest temperature gap.</li>
                    </ul>
                  </div>
                  <div>
                    <span className="font-medium block text-slate-900 dark:text-slate-100">Start/End Index</span>
                    Defines the range of device IDs managed by this logic.
                  </div>
                </div>
              </PopoverContent>
            </Popover>
          </div>
          {enableCoil && (
            <CoilSwitch
              address={enableCoil.address}
              value={isEnabled ?? false}
              aria-label="Enable Sequential Heating"
            />
          )}
        </div>
      </CardHeader>
      <CardContent className="space-y-4">
        {error && (
          <div className="flex flex-col gap-1 p-2 bg-red-500/10 border border-red-500/20 rounded-md">
            <div className="flex items-center gap-2 text-red-500 text-sm">
              <AlertCircle className="h-4 w-4" />
              <span><T>{error.message}</T></span>
            </div>
            {error.serverError && (
              <div className="text-xs text-red-400/80 pl-6">
                <T>{error.serverError}</T>
              </div>
            )}
          </div>
        )}
        <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
          <div className="space-y-2">
            <Label className="text-sm text-slate-600 dark:text-slate-300">
              <T>Heating Time</T> (<T>in seconds</T>)
            </Label>
            <div className="flex flex-col gap-1">
              <Input
                type="number"
                value={localMaxTime}
                onChange={(e) => {
                  setLocalMaxTime(e.target.value);
                  if (error?.field === 'Max Time') setError(null);
                }}
                onBlur={handleMaxTimeChange}
                onKeyDown={(e) => handleKeyDown(e, handleMaxTimeChange)}
                disabled={isUpdating}
                min={HEATING_TIME_S.MIN_S}
                max={HEATING_TIME_S.MAX_S}
                step={HEATING_TIME_S.STEP_S}
                className={getInputClassName('Max Time')}
                aria-label="Heating Time Input"
              />
              <div className="text-xs text-slate-500 dark:text-slate-400">
                {formatTime((parseInt(localMaxTime) || 0) * 1000)}
              </div>
            </div>
          </div>
          <div className="space-y-2">
            <Label className="text-sm text-slate-600 dark:text-slate-300"><T>Max Simultaneous</T></Label>
            <Input
              type="number"
              value={localMaxSim}
              onChange={(e) => {
                setLocalMaxSim(e.target.value);
                if (error?.field === 'Max Simultaneous') setError(null);
              }}
              onBlur={handleMaxSimChange}
              onKeyDown={(e) => handleKeyDown(e, handleMaxSimChange)}
              disabled={isUpdating}
              min={MIN_VALUES.SIMULTANEOUS}
              max={MAX_VALUES.SIMULTANEOUS}
              step={1}
              className={getInputClassName('Max Simultaneous')}
              aria-label="Max Simultaneous Input"
            />
          </div>
          <div className="space-y-2">
            <Label className="text-sm text-slate-600 dark:text-slate-300"><T>Window Offset</T></Label>
            <Input
              type="number"
              value={localOffset}
              onChange={(e) => {
                setLocalOffset(e.target.value);
                if (error?.field === 'Window Offset') setError(null);
              }}
              onBlur={handleOffsetChange}
              onKeyDown={(e) => handleKeyDown(e, handleOffsetChange)}
              disabled={isUpdating}
              min={MIN_VALUES.OFFSET}
              max={MAX_VALUES.OFFSET}
              step={1}
              className={getInputClassName('Window Offset')}
              aria-label="Window Offset Input"
            />
          </div>
          <div className="space-y-2">
            <Label className="text-sm text-slate-600 dark:text-slate-300"><T>Mode</T></Label>
            <Select
              value={localMode}
              onValueChange={handleModeChange}
              disabled={isUpdating}
            >
              <SelectTrigger className={getInputClassName('Mode')} aria-label="Amperage Budget Mode">
                <SelectValue placeholder="Select a mode" />
              </SelectTrigger>
              <SelectContent className="glass-panel border-0">
                <SelectItem value="0">Cycle All</SelectItem>
                <SelectItem value="1">Cycle by Setpoint</SelectItem>
                <SelectItem value="2">Heat Any by Setpoint</SelectItem>
                <SelectItem value="3">Heat most urgent</SelectItem>
              </SelectContent>
            </Select>
          </div>
        </div>
        <div className="grid grid-cols-1 md:grid-cols-2 gap-6 mt-4">
          <div className="space-y-2">
            <Label className="text-sm text-slate-600 dark:text-slate-300"><T>Start Index</T></Label>
            <Input
              type="number"
              value={localStartIndex}
              onChange={(e) => {
                setLocalStartIndex(e.target.value);
                if (error?.field === 'Start Index') setError(null);
              }}
              onBlur={handleStartIndexChange}
              onKeyDown={(e) => handleKeyDown(e, handleStartIndexChange)}
              disabled={isUpdating}
              // Add min/max/step if applicable from constants
              className={getInputClassName('Start Index')}
              aria-label="Start Index Input"
            />
          </div>
          <div className="space-y-2">
            <Label className="text-sm text-slate-600 dark:text-slate-300"><T>End Index</T></Label>
            <Input
              type="number"
              value={localEndIndex}
              onChange={(e) => {
                setLocalEndIndex(e.target.value);
                if (error?.field === 'End Index') setError(null);
              }}
              onBlur={handleEndIndexChange}
              onKeyDown={(e) => handleKeyDown(e, handleEndIndexChange)}
              disabled={isUpdating}
              // Add min/max/step if applicable from constants
              className={getInputClassName('End Index')}
              aria-label="End Index Input"
            />
          </div>
        </div>
      </CardContent>
    </Card>
  );
};

export default SequentialHeatingCard; 