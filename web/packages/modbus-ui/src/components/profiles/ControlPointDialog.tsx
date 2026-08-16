import React, { useState, useEffect, useMemo, useCallback } from 'react';
import {
  Dialog,
  DialogContent,
  DialogHeader,
  DialogTitle,
  DialogDescription,
  DialogFooter,
} from '@/components/ui/dialog';
import { Button } from '@/components/ui/button';
import { Input } from '@/components/ui/input';
import { Label } from '@/components/ui/label';
import {
  Select,
  SelectContent,
  SelectGroup,
  SelectItem,
  SelectLabel,
  SelectTrigger,
  SelectValue,
} from '@/components/ui/select';
import { Switch } from '@/components/ui/switch';
import { Textarea } from '@/components/ui/textarea';
import { SSignalControlPoint, ESignalType } from '../../types';
import { CoilData, RegisterData } from '@/contexts/ModbusContext';
import { T, translate } from '../../i18n';
import { E_FN_CODE } from '@polymech/client-ts';
import { TimeCodeEditor } from '../TimeCodeEditor';
import { ScrollArea } from '../ui/scroll-area';
import { AddressPicker, AddressGroup, AddressOption } from '../modbus/AddressPicker';

// This is defined in SignalPlotEditor and passed down, so we remove the direct import.
const CONTROL_POINT_TIME_SCALE = 1000; 

export type NewControlPointData = Omit<SSignalControlPoint, 'id' | 'state'>;

const signalTypeGroups = [
    { label: 'Buzzer', types: ['BUZZER_FAST_BLINK', 'BUZZER_LONG_BEEP_SHORT_PAUSE', 'BUZZER_OFF', 'BUZZER_SLOW_BLINK', 'BUZZER_SOLID'].sort() },
    { label: 'General', types: ['DISPLAY_MESSAGE', 'NONE', 'PAUSE_PROFILE', 'USER_DEFINED'].sort() },
    { label: 'Hardware I/O', types: ['GPIO_WRITE'].sort() },
    { label: 'Integrations', types: ['IFTTT_WEBHOOK'].sort() },
    { label: 'Modbus', types: ['MB_WRITE_COIL', 'MB_WRITE_HOLDING_REGISTER'].sort() },
    { label: 'PID Control', types: ['START_PIDS', 'STOP_PIDS'].sort() },
    { label: 'System Calls', types: ['CALL_FUNCTION', 'CALL_METHOD', 'CALL_REST'].sort() },
].sort((a, b) => a.label.localeCompare(b.label));

interface ControlPointDialogProps {
  isOpen: boolean;
  onClose: () => void;
  onConfirm: (data: NewControlPointData) => void;
  initialTimeScaled: number;
  plotDuration: number;
  allModbusCoils: CoilData[];
  allModbusRegisters: RegisterData[];
  getActualTimeMs: (cpTime: number, plotDuration: number) => number;
}

const getDefaultCpData = (time: number): NewControlPointData => ({
  time,
  type: ESignalType.MB_WRITE_COIL,
  arg_0: 0,
  arg_1: 1, // Default to ON for a new coil write
  arg_2: undefined,
  name: '',
  description: '',
});

const ControlPointDialog: React.FC<ControlPointDialogProps> = ({
  isOpen,
  onClose,
  onConfirm,
  initialTimeScaled,
  plotDuration,
  allModbusCoils,
  allModbusRegisters,
  getActualTimeMs,
}) => {
  const [newCp, setNewCp] = useState<NewControlPointData>(getDefaultCpData(initialTimeScaled));

  useEffect(() => {
    if (isOpen) {
      setNewCp(getDefaultCpData(initialTimeScaled));
    }
  }, [isOpen, initialTimeScaled]);

  const groupedSelectableCoils = useMemo((): AddressGroup[] => {
    const coils = allModbusCoils.filter(coil => coil.type === E_FN_CODE.FN_WRITE_COIL);
    const grouped = coils.reduce((acc, coil) => {
        const groupName = coil.group || translate('Uncategorized');
        if (!acc[groupName]) acc[groupName] = [];
        acc[groupName].push({
            value: `coil-${coil.address}`,
            label: `[C] ${groupName}::${coil.name || `Coil ${coil.address}`} (${coil.address})`,
            titleForSeries: `${groupName}::${coil.name || `Coil ${coil.address}`}`,
            source: 'coil' as const,
            group: groupName,
            sortKey: coil.name || `Coil ${coil.address}`,
        });
        return acc;
    }, {} as Record<string, (AddressOption & { sortKey: string })[]>);
    return Object.entries(grouped).map(([label, options]) => ({
        label: `${label} (Coils)`,
        options: options.sort((a, b) => a.sortKey.localeCompare(b.sortKey)),
    })).sort((a, b) => a.label.localeCompare(b.label));
  }, [allModbusCoils]);

  const groupedSelectableRegisters = useMemo((): AddressGroup[] => {
    const registers = allModbusRegisters.filter(reg => reg.type === E_FN_CODE.FN_WRITE_HOLD_REGISTER);
    const grouped = registers.reduce((acc, register) => {
        const groupName = register.group || translate('Uncategorized');
        if (!acc[groupName]) acc[groupName] = [];
        acc[groupName].push({
            value: `register-${register.address}`,
            label: `[R] ${groupName}::${register.name || `Register ${register.address}`} (${register.address})`,
            titleForSeries: `${groupName}::${register.name || `Register ${register.address}`}`,
            source: 'register' as const,
            group: groupName,
            sortKey: register.name || `Register ${register.address}`,
        });
        return acc;
    }, {} as Record<string, (AddressOption & { sortKey: string })[]>);
    return Object.entries(grouped).map(([label, options]) => ({
        label: `${label} (Registers)`,
        options: options.sort((a, b) => a.sortKey.localeCompare(b.sortKey)),
    })).sort((a, b) => a.label.localeCompare(b.label));
  }, [allModbusRegisters]);

  const coilItemLabelMap = useMemo(() => {
    const map = new Map<string, string>();
    groupedSelectableCoils.forEach(group => {
      group.options.forEach(option => {
        map.set(option.value, option.label);
      });
    });
    return map;
  }, [groupedSelectableCoils]);

  const registerItemLabelMap = useMemo(() => {
    const map = new Map<string, string>();
    groupedSelectableRegisters.forEach(group => {
      group.options.forEach(option => {
        map.set(option.value, option.label);
      });
    });
    return map;
  }, [groupedSelectableRegisters]);



  const handlePropertyChange = useCallback((
    propertyName: keyof NewControlPointData,
    rawValue: string | number | boolean
  ) => {
    setNewCp(prev => {
      let value: any = rawValue;
      if (propertyName === 'time' || propertyName === 'arg_0' || propertyName === 'arg_1' || propertyName === 'arg_2' || propertyName === 'type') {
        value = Number(rawValue);
        if (isNaN(value)) value = 0;
      }
      if (propertyName === 'arg_1' && prev.type === ESignalType.MB_WRITE_COIL) {
        value = rawValue ? 1 : 0;
      }

      const updatedCp = { ...prev, [propertyName]: value };

      if (propertyName === 'type') {
        // Reset args when type changes to avoid carry-over
        updatedCp.arg_0 = 0;
        updatedCp.arg_1 = 0;
        updatedCp.arg_2 = undefined;
        if (value === ESignalType.MB_WRITE_COIL) updatedCp.arg_1 = 1;
      }

      return updatedCp;
    });
  }, []);

  const handleTimeChange = (newTotalMilliseconds: number) => {
    if (plotDuration > 0) {
      const newScaledTime = (newTotalMilliseconds / plotDuration) * CONTROL_POINT_TIME_SCALE;
      const clampedScaledTime = Math.round(Math.max(0, Math.min(newScaledTime, CONTROL_POINT_TIME_SCALE)));
      handlePropertyChange('time', clampedScaledTime);
    } else {
      handlePropertyChange('time', 0);
    }
  };

  const handleCoilSelection = (value: string) => {
    // Extract address from value format like "coil-123"
    const addressStr = value.split('-')[1];
    const address = parseInt(addressStr, 10);
    if (isNaN(address)) return;
    handlePropertyChange('arg_0', address);
  };

  const handleRegisterSelection = (value: string) => {
    // Extract address from value format like "register-123"
    const addressStr = value.split('-')[1];
    const address = parseInt(addressStr, 10);
    if (isNaN(address)) return;
    handlePropertyChange('arg_0', address);
  };
  
  const handleConfirm = () => {
    const finalCp = { ...newCp };
    if (!finalCp.name) {
      finalCp.name = `CP @ ${getActualTimeMs(finalCp.time, plotDuration).toFixed(0)}ms`;
    }
    onConfirm(finalCp);
    onClose();
  };
  
  const handleKeyDown = (event: React.KeyboardEvent) => {
    if (event.key === 'Enter' && !event.nativeEvent.isComposing) {
        // Check if focus is on a button, select, or textarea to avoid conflicts
        const target = event.target as HTMLElement;
        if (target.tagName !== 'BUTTON' && target.tagName !== 'TEXTAREA' && !target.closest('[role="listbox"]')) {
            event.preventDefault();
            handleConfirm();
        }
    }
  };

  return (
    <Dialog open={isOpen} onOpenChange={(open) => !open && onClose()}>
      <DialogContent className="sm:max-w-[550px]" onKeyDown={handleKeyDown}>
        <DialogHeader>
          <DialogTitle><T>Create New Control Point</T></DialogTitle>
          <DialogDescription>
            <T>Configure the new control point. Press Enter to confirm or Esc to cancel.</T>
          </DialogDescription>
        </DialogHeader>
        <ScrollArea className="max-h-[60vh] p-6 -mx-4">
          <div className="space-y-4">
            {/* Core Info */}
            <div className="grid grid-cols-1 sm:grid-cols-2 gap-4">
                <div>
                  <Label htmlFor="cp-type"><T>Type:</T></Label>
                  <Select value={String(newCp.type)} onValueChange={(v) => handlePropertyChange('type', v)}>
                    <SelectTrigger id="cp-type" className="mt-1"><SelectValue placeholder={translate("Select type")} /></SelectTrigger>
                    <SelectContent>{signalTypeGroups.map(group => (
                        <SelectGroup key={group.label}><SelectLabel>{group.label}</SelectLabel>
                        {group.types.map(typeName => {
                            const typeValue = ESignalType[typeName as keyof typeof ESignalType];
                            const displayName = typeName.split('_').map(w => w.charAt(0).toUpperCase() + w.substring(1).toLowerCase()).join(' ');
                            return <SelectItem key={typeValue} value={String(typeValue)}>{displayName} ({typeValue})</SelectItem>;
                        })}
                        </SelectGroup>
                    ))}</SelectContent>
                  </Select>
                </div>
                <div>
                  <Label htmlFor="cp-time-h"><T>Time:</T></Label>
                  <TimeCodeEditor
                    totalMilliseconds={getActualTimeMs(newCp.time, plotDuration)}
                    onDurationChange={handleTimeChange}
                    idPrefix="cp-time"
                    disabled={plotDuration <= 0}
                  />
                </div>
            </div>
            
            {/* Details */}
            <div className="space-y-2">
                <div>
                    <Label htmlFor="cp-name"><T>CP Name (Optional):</T></Label>
                    <Input id="cp-name" type="text" value={newCp.name || ''} onChange={(e) => handlePropertyChange('name', e.target.value)} placeholder={translate("e.g., Start Heating")} />
                </div>
                <div>
                    <Label htmlFor="cp-desc"><T>CP Description (Optional):</T></Label>
                    <Textarea id="cp-desc" value={newCp.description || ''} onChange={(e) => handlePropertyChange('description', e.target.value)} rows={2} placeholder={translate("e.g., Turn on coil for pre-heating stage")} />
                </div>
            </div>

            {/* Arguments */}
            <div className="space-y-3 pt-3 border-t">
                <h4 className="text-sm font-semibold text-muted-foreground"><T>Arguments:</T></h4>
                {newCp.type === ESignalType.MB_WRITE_COIL ? (
                    <div className="grid grid-cols-1 sm:grid-cols-2 gap-4 items-start">
                        <div>
                            <Label htmlFor="cp-arg0-coilselect"><T>Coil to Write:</T></Label>
                            <AddressPicker
                                value={`coil-${newCp.arg_0}`}
                                onSelect={handleCoilSelection}
                                groupedItems={groupedSelectableCoils}
                                itemLabelMap={coilItemLabelMap}
                                placeholder="Select Known Coil..."
                                className="w-full justify-between text-xs mt-1"
                                showCoils={true}
                                showRegisters={false}
                                showFavourites={true}
                            />
                        </div>
                        <div className="flex items-center space-x-2 pt-8">
                            <Label htmlFor="cp-arg1-coilval" className="whitespace-nowrap"><T>Value:</T></Label>
                            <Switch id="cp-arg1-coilval" checked={newCp.arg_1 === 1} onCheckedChange={(c) => handlePropertyChange('arg_1', c)} />
                            <span className="text-muted-foreground select-none">{newCp.arg_1 === 1 ? <T>ON</T> : <T>OFF</T>}</span>
                        </div>
                    </div>
                ) : newCp.type === ESignalType.MB_WRITE_HOLDING_REGISTER ? (
                    <div className="grid grid-cols-1 sm:grid-cols-2 gap-4">
                        <div>
                            <Label htmlFor="cp-arg0-regselect"><T>Register to Write:</T></Label>
                            <AddressPicker
                                value={`register-${newCp.arg_0}`}
                                onSelect={handleRegisterSelection}
                                groupedItems={groupedSelectableRegisters}
                                itemLabelMap={registerItemLabelMap}
                                placeholder="Select Known Register..."
                                className="w-full justify-between text-xs mt-1"
                                showCoils={false}
                                showRegisters={true}
                                showFavourites={true}
                            />
                        </div>
                        <div>
                            <Label htmlFor="cp-arg1-regval"><T>Value (arg_1):</T></Label>
                            <Input id="cp-arg1-regval" type="number" value={newCp.arg_1} onChange={(e) => handlePropertyChange('arg_1', e.target.value)} />
                        </div>
                    </div>
                ) : (
                    <p className="text-xs text-muted-foreground italic"><T>Arguments for this type are not yet configurable through this dialog.</T></p>
                )}
            </div>
          </div>
        </ScrollArea>
        <DialogFooter>
          <Button variant="ghost" onClick={onClose}><T>Cancel</T></Button>
          <Button onClick={handleConfirm}><T>Create Control Point</T></Button>
        </DialogFooter>
      </DialogContent>
    </Dialog>
  );
};

export default ControlPointDialog; 