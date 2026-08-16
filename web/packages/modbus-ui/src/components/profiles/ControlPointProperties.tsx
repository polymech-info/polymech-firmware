import React, { useMemo } from 'react';
import { SSignalControlPoint, ESignalState, ESignalType } from '../../types'; // Adjust path as needed
import { Input } from '@/components/ui/input';
import { Label } from '@/components/ui/label';
import { Select, SelectContent, SelectGroup, SelectItem, SelectLabel, SelectTrigger, SelectValue } from '@/components/ui/select';
import { Switch } from '@/components/ui/switch';
import { Textarea } from '@/components/ui/textarea';
import { Button } from '@/components/ui/button'; // Import Button
import { PlayCircleIcon } from 'lucide-react'; // Import an icon
import { CoilData, RegisterData, } from '@/contexts/ModbusContext'; // Import CoilData and RegisterData
import { T, translate } from '../../i18n';
import { E_FN_CODE } from '@polymech/client-ts';
import { getControlPointTypeNames } from '../../lib/i18n';
import { TimeCodeEditor } from '../TimeCodeEditor';
import { AddressPicker, AddressGroup, AddressOption } from '../modbus/AddressPicker';
import { parseRegisterName } from '@/lib/modbusUtils';
import FlagDisplay from '../FlagDisplay';

interface ControlPointPropertiesProps {
  plotSlot: number;
  selectedCpData: SSignalControlPoint | null;
  isFirstPlotAndPlaying: boolean;
  plotDuration: number;
  getActualTimeMs: (cpTime: number, plotDuration: number) => number;
  onControlPointPropertyChange: (
    propertyName: keyof Omit<SSignalControlPoint, 'id' | 'user' | 'state'>,
    rawValue: string | number | boolean
  ) => void;
  CONTROL_POINT_TIME_SCALE: number;
  allModbusCoils: CoilData[]; // Add allModbusCoils to props
  allModbusRegisters: RegisterData[]; // Add allModbusRegisters to props
  onExecuteControlPoint: (controlPoint: SSignalControlPoint) => Promise<void>; // New prop
}

const ControlPointProperties: React.FC<ControlPointPropertiesProps> = ({
  plotSlot,
  selectedCpData,

  plotDuration,
  getActualTimeMs,
  onControlPointPropertyChange,
  CONTROL_POINT_TIME_SCALE,
  allModbusCoils, // Destructure new prop
  allModbusRegisters, // Destructure new prop
  onExecuteControlPoint, // Destructure new prop
}) => {

  // --- PERFORMANCE OPTIMIZATION ---
  const controlPointTypeNames = useMemo(() => getControlPointTypeNames(translate), []);

  const signalTypeGroups = useMemo(() => {
    return [
      {
        label: translate('Buzzer'),
        types: [
          'BUZZER_FAST_BLINK',
          'BUZZER_LONG_BEEP_SHORT_PAUSE',
          'BUZZER_OFF',
          'BUZZER_SLOW_BLINK',
          'BUZZER_SOLID',
        ].sort(),
      },
      {
        label: translate('General'),
        types: ['DISPLAY_MESSAGE', 'NONE', 'PAUSE_PROFILE', 'USER_DEFINED'].sort(),
      },
      {
        label: translate('Hardware I/O'),
        types: ['GPIO_WRITE'].sort(),
      },
      {
        label: translate('Integrations'),
        types: ['IFTTT_WEBHOOK'].sort(),
      },
      {
        label: translate('Modbus'),
        types: ['MB_WRITE_COIL', 'MB_WRITE_HOLDING_REGISTER'].sort(),
      },
      {
        label: translate('PID Control'),
        types: ['START_PIDS', 'STOP_PIDS'].sort(),
      },
      {
        label: translate('System Calls'),
        types: ['CALL_FUNCTION', 'CALL_METHOD', 'CALL_REST'].sort(),
      },
    ].sort((a, b) => a.label.localeCompare(b.label));
  }, []);

  // Memoize and group the lists of selectable coils and registers for AddressPicker.
  const groupedAddressItems = useMemo((): AddressGroup[] => {
    const coils = allModbusCoils.filter(coil => coil.type === E_FN_CODE.FN_WRITE_COIL);
    const registers = allModbusRegisters.filter(register => register.type === E_FN_CODE.FN_WRITE_HOLD_REGISTER);

    const createOption = (item: CoilData | RegisterData, source: 'register' | 'coil'): AddressOption => {
      const prefix = source === 'register' ? '[R]' : '[C]';
      const name = item.name || `${source === 'register' ? 'Register' : 'Coil'} ${item.address}`;
      const group = item.group || translate('Uncategorized');
      return {
        value: String(item.address),
        label: `${prefix} ${group}::${name} (${translate("Addr:")} ${item.address})`,
        titleForSeries: `${group}::${name}`,
        source: source,
        group,
      };
    };

    const allItems = [
      ...registers.map(r => createOption(r, 'register')),
      ...coils.map(c => createOption(c, 'coil'))
    ];

    const grouped = allItems.reduce((acc, item) => {
      const groupName = `${item.group} (${item.source === 'register' ? 'Registers' : 'Coils'})`;
      if (!acc[groupName]) acc[groupName] = [];
      acc[groupName].push(item);
      return acc;
    }, {} as Record<string, AddressOption[]>);

    return Object.entries(grouped)
      .map(([label, options]) => ({
        label,
        options: options.sort((a, b) => a.label.localeCompare(b.label)),
      }))
      .sort((a, b) => a.label.localeCompare(b.label));
  }, [allModbusCoils, allModbusRegisters]);

  // Create itemLabelMap for AddressPicker
  const itemLabelMap = useMemo(() => {
    const labelMap = new Map<string, string>();
    groupedAddressItems.forEach(group => {
      group.options.forEach(option => {
        labelMap.set(`${option.source}-${option.value}`, option.label);
      });
    });
    return labelMap;
  }, [groupedAddressItems]);

  const selectedRegister = useMemo(() => {
    if (selectedCpData?.type === ESignalType.MB_WRITE_HOLDING_REGISTER && selectedCpData.arg_0) {
      return allModbusRegisters.find(r => r.address === selectedCpData.arg_0);
    }
    return undefined;
  }, [selectedCpData, allModbusRegisters]);

  const parsedRegister = useMemo(() => {
    if (selectedRegister?.name) {
      return parseRegisterName(selectedRegister.name);
    }
    return null;
  }, [selectedRegister]);

  const isFlagsRegister = useMemo(() => {
      return selectedRegister?.name.toLowerCase().includes('flags');
  }, [selectedRegister]);


  if (!selectedCpData) {
    return <p className="text-xs text-muted-foreground italic"><T>Select a control point to see its properties.</T></p>;
  }



  const handleTypeChange = (value: string) => {
    const type = parseInt(value, 10);
    if (!isNaN(type)) {
      onControlPointPropertyChange('type', type);
      const name = controlPointTypeNames[type] || '';
      onControlPointPropertyChange('name', name);
    }
  };

  const handleRunActionClick = () => {
    if (selectedCpData) {
      onExecuteControlPoint(selectedCpData);
    }
  };

  const handleTimeChange = (newTotalMilliseconds: number) => {
    if (plotDuration > 0) {
        const newScaledTime = (newTotalMilliseconds / plotDuration) * CONTROL_POINT_TIME_SCALE;
        const clampedScaledTime = Math.round(Math.max(0, Math.min(newScaledTime, CONTROL_POINT_TIME_SCALE)));
        onControlPointPropertyChange('time', clampedScaledTime);
    } else {
        onControlPointPropertyChange('time', 0);
    }
  };

  return (
    <div className="space-y-4 text-xs p-3 border rounded-md bg-background" id={`cp-properties-${selectedCpData.id}`}>
      {/* Group 1: Core Info & Action */}
      <div className="mb-4">
        <div className="flex justify-between items-center mb-2">
          <span className="text-xs text-muted-foreground font-medium"><T>ID:</T> {selectedCpData.id}</span>
          <Button
            size="sm"
            variant="outline"
            onClick={handleRunActionClick}
            disabled={!selectedCpData}
            className="h-7"
            title={translate("Run this control point action now")}
          >
            <PlayCircleIcon className="h-4 w-4 mr-1" />
            <T>Run Action</T>
          </Button>
        </div>
      </div>

      {/* Group 2: Timing & Type */}
      <div className="space-y-3 mb-4">
        <div>
          <Label htmlFor={`cp-time-${plotSlot}-h`} className="text-xs"><T>Time:</T></Label>
          <div className="mt-0.5">
            <TimeCodeEditor
              totalMilliseconds={getActualTimeMs(selectedCpData.time, plotDuration)}
              onDurationChange={handleTimeChange}
              idPrefix={`cp-time-${plotSlot}`}
              disabled={plotDuration <= 0}
            />
          </div>
          {plotDuration <= 0 && <p className="text-muted-foreground text-xs mt-1"><T>Set plot duration to enable time editing.</T></p>}
          <p className="text-muted-foreground text-xs mt-0.5"><T>Scale:</T> {selectedCpData.time} / {CONTROL_POINT_TIME_SCALE}</p>
        </div>
        <div>
          <Label htmlFor={`cp-state-${plotSlot}`} className="text-xs"><T>State:</T></Label>
          <Input type="text" value={`${ESignalState[selectedCpData.state]} (${selectedCpData.state})`} readOnly className="h-7 mt-0.5 bg-muted/50" />
        </div>
        <div>
          <Label htmlFor={`cp-type-${plotSlot}`} className="text-xs"><T>Type:</T></Label>
          <Select value={String(selectedCpData.type)} onValueChange={handleTypeChange}>
            <SelectTrigger id={`cp-type-${plotSlot}`} className="h-7 mt-0.5 text-xs"><SelectValue placeholder={translate("Select type")} /></SelectTrigger>
            <SelectContent>
              {signalTypeGroups.map(group => (
                <SelectGroup key={group.label}>
                  <SelectLabel className="text-xs pl-2 py-1.5 font-semibold text-muted-foreground">{group.label}</SelectLabel>
                  {group.types.map(typeName => {
                    const typeValue = ESignalType[typeName as keyof typeof ESignalType];
                    const displayName = controlPointTypeNames[typeValue] || typeName;
                    return (
                      <SelectItem key={typeValue} value={String(typeValue)} className="text-xs">
                        {displayName} ({typeValue})
                      </SelectItem>
                    );
                  })}
                </SelectGroup>
              ))}
            </SelectContent>
          </Select>
        </div>
      </div>

      {/* Group 3: Details */}
      <div className="space-y-3 mb-4">
        <div>
          <Label htmlFor={`cp-name-${plotSlot}`} className="text-xs"><T>CP Name (Optional):</T></Label>
          <Input id={`cp-name-${plotSlot}`} type="text" value={selectedCpData.name || ''} onChange={(e) => onControlPointPropertyChange('name', e.target.value)} className="h-7 mt-0.5" placeholder={translate("Enter CP name")} />
        </div>
        <div>
          <Label htmlFor={`cp-desc-${plotSlot}`} className="text-xs"><T>CP Description (Optional):</T></Label>
          <Textarea id={`cp-desc-${plotSlot}`} value={selectedCpData.description || ''} onChange={(e) => onControlPointPropertyChange('description', e.target.value)} className="text-xs mt-0.5" rows={2} placeholder={translate("Enter CP description")} />
        </div>
      </div>

      {/* Group 4: Arguments (Conditional) */}
      <div className="space-y-3">
        <h4 className="text-xs font-semibold text-muted-foreground border-b pb-1 mb-2"><T>Arguments:</T></h4>
        {selectedCpData.type === ESignalType.MB_WRITE_COIL ? (
          <>
            <div className="mb-2">
              <Label htmlFor={`cp-arg0-coiladdr-${plotSlot}`} className="text-xs"><T>Coil Address (arg_0):</T></Label>
              <div className="mt-0.5">
                <AddressPicker
                  value={(selectedCpData.arg_0 ?? 0) > 0 ? `coil-${selectedCpData.arg_0}` : ''}
                  onSelect={(value) => {
                    const address = parseInt(value, 10) || 0;
                    onControlPointPropertyChange('arg_0', address);
                  }}
                  groupedItems={groupedAddressItems}
                  itemLabelMap={itemLabelMap}
                  placeholder="Select Known Coil..."
                  className="h-7 text-xs w-full"
                  showCoils={true}
                  showRegisters={false}
                  showFavourites={true}
                />
              </div>
            </div>
            <div className="flex items-center space-x-2 pt-1">
              <Label htmlFor={`cp-arg1-coilval-${plotSlot}`} className="text-xs whitespace-nowrap mr-2"><T>Coil Value (arg_1):</T></Label>
              <Switch id={`cp-arg1-coilval-${plotSlot}`} checked={selectedCpData.arg_1 === 1} onCheckedChange={(checked) => onControlPointPropertyChange('arg_1', checked)} />
              <span className="text-muted-foreground text-xs select-none">{selectedCpData.arg_1 === 1 ? <T>ON</T> : <T>OFF</T>}</span>
            </div>
          </>
        ) : selectedCpData.type === ESignalType.MB_WRITE_HOLDING_REGISTER ? (
          <>
            <div className="mb-2">
              <Label htmlFor={`cp-arg0-regaddr-${plotSlot}`} className="text-xs"><T>Register Address (arg_0):</T></Label>
              <div className="mt-0.5">
                <AddressPicker
                  value={(selectedCpData.arg_0 ?? 0) > 0 ? `register-${selectedCpData.arg_0}` : ''}
                  onSelect={(value) => {
                    const address = parseInt(value, 10) || 0;
                    onControlPointPropertyChange('arg_0', address);
                  }}
                  groupedItems={groupedAddressItems}
                  itemLabelMap={itemLabelMap}
                  placeholder="Select Known Register..."
                  className="h-7 text-xs w-full"
                  showCoils={false}
                  showRegisters={true}
                  showFavourites={true}
                />
              </div>
            </div>
            <div className="mb-2">
              <Label htmlFor={`cp-arg1-regval-${plotSlot}`} className="text-xs"><T>Register Value (arg_1):</T></Label>
              {parsedRegister && isFlagsRegister ? (
                <div className="space-y-2 mt-2 p-3 border rounded-md">
                   <FlagDisplay
                      parsedFlags={parsedRegister}
                      currentValue={selectedCpData.arg_1}
                      onValueChange={(value) => onControlPointPropertyChange('arg_1', value)}
                    />
                </div>
              ) : parsedRegister && !isFlagsRegister ? (
                <div className="space-y-2 mt-0.5">
                  <Select
                    value={String(selectedCpData.arg_1)}
                    onValueChange={(value) => onControlPointPropertyChange('arg_1', value)}
                  >
                    <SelectTrigger id={`cp-arg1-enumval-${plotSlot}`} className="h-7 text-xs">
                      <SelectValue placeholder={translate("Select from known values...")} />
                    </SelectTrigger>
                    <SelectContent>
                      {parsedRegister.enumValues.map(({ val, label }) => (
                        <SelectItem key={val} value={String(val)} className="text-xs">
                          {label} ({val})
                        </SelectItem>
                      ))}
                    </SelectContent>
                  </Select>
                </div>
              ) : null}
              <Input
                id={`cp-arg1-regval-${plotSlot}`}
                type="number"
                value={selectedCpData.arg_1}
                onChange={(e) => onControlPointPropertyChange('arg_1', e.target.value)}
                className="h-7 mt-0.5"
              />
            </div>
          </>
        ) : selectedCpData.type === ESignalType.CALL_METHOD ? (
          <>
            <div className="mb-2">
              <Label htmlFor={`cp-arg0-methodid-${plotSlot}`} className="text-xs"><T>Method ID (arg_0):</T></Label>
              <Input id={`cp-arg0-methodid-${plotSlot}`} type="number" value={selectedCpData.arg_0} onChange={(e) => onControlPointPropertyChange('arg_0', e.target.value)} className="h-7 mt-0.5" />
            </div>
            <div className="mb-2">
              <Label htmlFor={`cp-arg1-methodarg-${plotSlot}`} className="text-xs"><T>Argument 1 (arg_1):</T></Label>
              <Input id={`cp-arg1-methodarg-${plotSlot}`} type="number" value={selectedCpData.arg_1} onChange={(e) => onControlPointPropertyChange('arg_1', e.target.value)} className="h-7 mt-0.5" />
            </div>
          </>
        ) : selectedCpData.type === ESignalType.PAUSE_PROFILE ? (
          <div className="mb-2">
            <Label htmlFor={`cp-arg0-pauseduration-${plotSlot}`} className="text-xs"><T>Pause Profile:</T></Label>
          </div>
        ) : selectedCpData.type === ESignalType.START_PIDS ? (
          <div className="mb-2">
            <p className="text-xs text-muted-foreground"><T>This action starts all PID controllers associated with the parent temperature profile.</T></p>
          </div>
        ) : selectedCpData.type === ESignalType.STOP_PIDS ? (
          <div className="mb-2">
            <p className="text-xs text-muted-foreground"><T>This action stops all PID controllers associated with the parent temperature profile.</T></p>
          </div>
        ) : selectedCpData.type === ESignalType.IFTTT_WEBHOOK ? (
          <div className="mb-2">
            <p className="text-xs text-muted-foreground">
              <T>The IFTTT notification message should be entered in the "CP Description" field above.</T>
            </p>
          </div>
        ) : selectedCpData.type >= ESignalType.BUZZER_OFF && selectedCpData.type <= ESignalType.BUZZER_LONG_BEEP_SHORT_PAUSE ? (
          <div className="mb-2">
            <Label htmlFor={`cp-arg0-buzz-duration-${plotSlot}`} className="text-xs"><T>Duration (ms, arg_0):</T></Label>
            <Input
              id={`cp-arg0-buzz-duration-${plotSlot}`}
              type="number"
              value={selectedCpData.arg_0}
              onChange={(e) => onControlPointPropertyChange('arg_0', e.target.value)}
              className="h-7 mt-0.5"
              placeholder={translate("0 for indefinite")}
            />
             <p className="text-xs text-muted-foreground mt-1">
              <T>Set the duration in milliseconds for the buzzer to play (max 5000ms). Use 0 for the default 5-second duration.</T>
            </p>
          </div>
        ) : (
          <>
            <div className="mb-2">
              <Label htmlFor={`cp-arg0-generic-${plotSlot}`} className="text-xs"><T>Argument 0:</T></Label>
              <Input id={`cp-arg0-generic-${plotSlot}`} type="number" value={selectedCpData.arg_0} onChange={(e) => onControlPointPropertyChange('arg_0', e.target.value)} className="h-7 mt-0.5" />
            </div>
            <div className="mb-2">
              <Label htmlFor={`cp-arg1-generic-${plotSlot}`} className="text-xs"><T>Argument 1:</T></Label>
              <Input id={`cp-arg1-generic-${plotSlot}`} type="number" value={selectedCpData.arg_1} onChange={(e) => onControlPointPropertyChange('arg_1', e.target.value)} className="h-7 mt-0.5" />
            </div>
          </>
        )}
        <div className="mt-2"> {/* Ensure arg_2 has margin too */}
          <Label htmlFor={`cp-arg2-${plotSlot}`} className="text-xs"><T>Argument 2 (Optional):</T></Label>
          <Input id={`cp-arg2-${plotSlot}`} type="number" value={selectedCpData.arg_2 === undefined ? '' : selectedCpData.arg_2} onChange={(e) => onControlPointPropertyChange('arg_2', e.target.value)} placeholder={translate("N/A")} className="h-7 mt-0.5" />
        </div>
      </div>
    </div>
  );
};

export default ControlPointProperties;
