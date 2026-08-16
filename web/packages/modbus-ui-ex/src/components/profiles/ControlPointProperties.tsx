import React, { useState, useEffect, useMemo } from 'react';
import { SSignalControlPoint, ESignalState, ESignalType } from '../../types'; // Adjust path as needed
import { Input } from '@/components/ui/input';
import { Label } from '@/components/ui/label';
import { Select, SelectContent, SelectItem, SelectTrigger, SelectValue } from '@/components/ui/select';
import { Switch } from '@/components/ui/switch';
import { Textarea } from '@/components/ui/textarea';
import { Button } from '@/components/ui/button'; // Import Button
import { PlayCircleIcon } from 'lucide-react'; // Import an icon
import { CoilData, RegisterData, useModbus } from '@/contexts/ModbusContext'; // Import CoilData and RegisterData
import { T, translate } from '../../i18n';
import modbusApiService from '@polymech/client-ts/modbusApiService';
import { RegisteredMethod } from '@polymech/client-ts';

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
  isFirstPlotAndPlaying,
  plotDuration,
  getActualTimeMs,
  onControlPointPropertyChange,
  CONTROL_POINT_TIME_SCALE,
  allModbusCoils, // Destructure new prop
  allModbusRegisters, // Destructure new prop
  onExecuteControlPoint, // Destructure new prop
}) => {
  const [registeredMethods, setRegisteredMethods] = useState<RegisteredMethod[]>([]);
  const [isLoadingMethods, setIsLoadingMethods] = useState(false);

  const getCoilDisplayName = (coil: CoilData): string => {
    const addressStr = `(${translate("Addr:")} ${coil.address})`;
    const groupName = coil.group || translate('N/A');
    const coilName = coil.name || `${translate("Coil")} ${coil.address}`;

    if (coil.group && coil.name) {
      return `${coil.group}::${coil.name} ${addressStr}`;
    }
    if (coil.name) {
      return `${coil.name} ${addressStr}`;
    }
    // If only group exists, or neither (covered by coilName default)
    return `${groupName}::${coilName} ${addressStr}`;
  };

  const getRegisterDisplayName = (register: RegisterData): string => {
    const addressStr = `(${translate("Addr:")} ${register.address})`;
    const groupName = register.group || translate('N/A');
    const registerName = register.name || `${translate("Register")} ${register.address}`;

    if (register.group && register.name) {
      return `${register.group}::${register.name} ${addressStr}`;
    }
    if (register.name) {
      return `${register.name} ${addressStr}`;
    }
    // If only group exists, or neither (covered by registerName default)
    return `${groupName}::${registerName} ${addressStr}`;
  };

  // --- PERFORMANCE OPTIMIZATION ---
  // Memoize the lists of selectable coils and registers.
  // This prevents the expensive .filter() and .map() operations on the entire
  // Modbus register/coil list on every single render, which was the cause of the UI freezing.
  const selectableCoils = useMemo(() => {
    return allModbusCoils
      .filter(coil => coil.type === 1 || coil.type === 5)
      .map(coil => ({
        value: String(coil.address),
        label: getCoilDisplayName(coil)
      }));
  }, [allModbusCoils]);

  const selectableRegisters = useMemo(() => {
    return allModbusRegisters
      .filter(reg => reg.access === 2 || reg.access === 3)
      .map(register => ({
        value: String(register.address),
        label: getRegisterDisplayName(register)
      }));
  }, [allModbusRegisters]);

  useEffect(() => {
    const fetchMethods = async () => {
      setIsLoadingMethods(true);
      try {
        const methods = await modbusApiService.getRegisteredMethods();
        const uniqueMethods = methods.filter((method, index, self) =>
          index === self.findIndex((m) => m.id === method.id)
        );
        setRegisteredMethods(uniqueMethods);
      } catch (error) {
        console.error("Failed to fetch registered methods:", error);
      } finally {
        setIsLoadingMethods(false);
      }
    };
    fetchMethods();
  }, []);

  if (!selectedCpData) {
    return <p className="text-xs text-muted-foreground italic"><T>Select a control point to see its properties.</T></p>;
  }

  const handleCoilSelectionChange = (selectedValue: string) => {
    const address = parseInt(selectedValue, 10);
    if (!isNaN(address)) {
      onControlPointPropertyChange('arg_0', address);
    }
  };

  const handleRegisterSelectionChange = (selectedValue: string) => {
    const address = parseInt(selectedValue, 10);
    if (!isNaN(address)) {
      onControlPointPropertyChange('arg_0', address);
    }
  };

  const handleRunActionClick = () => {
    if (selectedCpData) {
      onExecuteControlPoint(selectedCpData);
    }
  };

  return (
    <div className="space-y-4 text-xs p-3 border rounded-md bg-background">
      {/* Group 1: Core Info & Action */}
      <div className="mb-4">
        <div className="flex justify-between items-center mb-2">
          <span className="text-xs text-muted-foreground font-medium"><T>ID:</T> {selectedCpData.id}</span>
          <Button 
            size="sm" 
            variant="outline"
            onClick={handleRunActionClick} 
            disabled={isFirstPlotAndPlaying || !selectedCpData}
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
          <Label htmlFor={`cp-time-${plotSlot}`} className="text-xs"><T>Time (0-1000 scale):</T></Label>
          <Input id={`cp-time-${plotSlot}`} type="number" value={selectedCpData.time} onChange={(e) => onControlPointPropertyChange('time', e.target.value)} max={CONTROL_POINT_TIME_SCALE} min={0} className="h-7 mt-0.5" disabled={isFirstPlotAndPlaying} />
          <p className="text-muted-foreground text-xs mt-0.5"><T>Actual:</T> {getActualTimeMs(selectedCpData.time, plotDuration).toFixed(0)}ms</p>
        </div>
        <div>
          <Label htmlFor={`cp-state-${plotSlot}`} className="text-xs"><T>State:</T></Label>
          <Input type="text" value={`${ESignalState[selectedCpData.state]} (${selectedCpData.state})`} readOnly className="h-7 mt-0.5 bg-muted/50" />
        </div>
        <div>
          <Label htmlFor={`cp-type-${plotSlot}`} className="text-xs"><T>Type:</T></Label>
          <Select value={String(selectedCpData.type)} onValueChange={(value) => onControlPointPropertyChange('type', value)} disabled={isFirstPlotAndPlaying}>
            <SelectTrigger id={`cp-type-${plotSlot}`} className="h-7 mt-0.5 text-xs"><SelectValue placeholder={translate("Select type")} /></SelectTrigger>
            <SelectContent>{Object.entries(ESignalType).filter(([key, value]) => !isNaN(Number(value))).map(([key, value]) => (<SelectItem key={value} value={String(value)} className="text-xs">{key} ({value})</SelectItem>))}</SelectContent>
          </Select>
        </div>
      </div>

      {/* Group 3: Details */}
      <div className="space-y-3 mb-4">
        <div>
          <Label htmlFor={`cp-name-${plotSlot}`} className="text-xs"><T>CP Name (Optional):</T></Label>
          <Input id={`cp-name-${plotSlot}`} type="text" value={selectedCpData.name || ''} onChange={(e) => onControlPointPropertyChange('name', e.target.value)} className="h-7 mt-0.5" disabled={isFirstPlotAndPlaying} placeholder={translate("Enter CP name")} />
        </div>
        <div>
          <Label htmlFor={`cp-desc-${plotSlot}`} className="text-xs"><T>CP Description (Optional):</T></Label>
          <Textarea id={`cp-desc-${plotSlot}`} value={selectedCpData.description || ''} onChange={(e) => onControlPointPropertyChange('description', e.target.value)} className="text-xs mt-0.5" rows={2} disabled={isFirstPlotAndPlaying} placeholder={translate("Enter CP description")} />
        </div>
      </div>

      {/* Group 4: Arguments (Conditional) */}
      <div className="space-y-3">
        <h4 className="text-xs font-semibold text-muted-foreground border-b pb-1 mb-2"><T>Arguments:</T></h4>
        {selectedCpData.type === ESignalType.MB_WRITE_COIL ? (
          <>
            <div className="mb-2">
              <Label htmlFor={`cp-arg0-coiladdr-${plotSlot}`} className="text-xs"><T>Coil Address (arg_0):</T></Label>
              <Input id={`cp-arg0-coiladdr-${plotSlot}`} type="number" value={selectedCpData.arg_0} onChange={(e) => onControlPointPropertyChange('arg_0', e.target.value)} className="h-7 mt-0.5" disabled={isFirstPlotAndPlaying} />
              <Select onValueChange={handleCoilSelectionChange} disabled={isFirstPlotAndPlaying} value={String(selectedCpData.arg_0)}>
                <SelectTrigger id={`cp-arg0-coilselect-${plotSlot}`} className="h-7 mt-1 text-xs"><SelectValue placeholder={translate("Or Select Known Coil...")} /></SelectTrigger>
                <SelectContent>{selectableCoils.map(coil => (<SelectItem key={coil.value} value={coil.value} className="text-xs">{coil.label}</SelectItem>))}</SelectContent>
              </Select>
            </div>
            <div className="flex items-center space-x-2 pt-1">
              <Label htmlFor={`cp-arg1-coilval-${plotSlot}`} className="text-xs whitespace-nowrap mr-2"><T>Coil Value (arg_1):</T></Label>
              <Switch id={`cp-arg1-coilval-${plotSlot}`} checked={selectedCpData.arg_1 === 1} onCheckedChange={(checked) => onControlPointPropertyChange('arg_1', checked)} disabled={isFirstPlotAndPlaying} />
              <span className="text-muted-foreground text-xs select-none">{selectedCpData.arg_1 === 1 ? <T>ON</T> : <T>OFF</T>}</span>
            </div>
          </>
        ) : selectedCpData.type === ESignalType.MB_WRITE_HOLDING_REGISTER ? (
          <>
            <div className="mb-2">
              <Label htmlFor={`cp-arg0-regaddr-${plotSlot}`} className="text-xs"><T>Register Address (arg_0):</T></Label>
              <Input id={`cp-arg0-regaddr-${plotSlot}`} type="number" value={selectedCpData.arg_0} onChange={(e) => onControlPointPropertyChange('arg_0', e.target.value)} className="h-7 mt-0.5" disabled={isFirstPlotAndPlaying} />
              <Select onValueChange={handleRegisterSelectionChange} disabled={isFirstPlotAndPlaying} value={String(selectedCpData.arg_0)}>
                <SelectTrigger id={`cp-arg0-regselect-${plotSlot}`} className="h-7 mt-1 text-xs"><SelectValue placeholder={translate("Or Select Known Register...")} /></SelectTrigger>
                <SelectContent>{selectableRegisters.map(register => (<SelectItem key={register.value} value={register.value} className="text-xs">{register.label}</SelectItem>))}</SelectContent>
              </Select>
            </div>
            <div className="mb-2">
              <Label htmlFor={`cp-arg1-regval-${plotSlot}`} className="text-xs"><T>Register Value (arg_1):</T></Label>
              <Input id={`cp-arg1-regval-${plotSlot}`} type="number" value={selectedCpData.arg_1} onChange={(e) => onControlPointPropertyChange('arg_1', e.target.value)} className="h-7 mt-0.5" disabled={isFirstPlotAndPlaying} />
            </div>
          </>
        ) : selectedCpData.type === ESignalType.CALL_METHOD ? (
          <>
            <div className="mb-2">
              <Label htmlFor={`cp-arg0-methodselect-${plotSlot}`} className="text-xs"><T>Method (ID in arg_0):</T></Label>
              <Select
                onValueChange={(value) => onControlPointPropertyChange('arg_0', value)}
                disabled={isFirstPlotAndPlaying || isLoadingMethods}
                value={String(selectedCpData.arg_0)}
              >
                <SelectTrigger id={`cp-arg0-methodselect-${plotSlot}`} className="h-7 mt-1 text-xs">
                  <SelectValue placeholder={isLoadingMethods ? "Loading methods..." : "Select Method..."} />
                </SelectTrigger>
                <SelectContent>
                  {registeredMethods.map(method => (
                    <SelectItem key={method.id} value={String(method.id)} className="text-xs">
                      {`${method.component}::${method.method} (${method.id})`}
                    </SelectItem>
                  ))}
                </SelectContent>
              </Select>
            </div>
            <div className="mb-2">
              <Label htmlFor={`cp-arg1-methodarg-${plotSlot}`} className="text-xs"><T>Argument 1 (arg_1):</T></Label>
              <Input id={`cp-arg1-methodarg-${plotSlot}`} type="number" value={selectedCpData.arg_1} onChange={(e) => onControlPointPropertyChange('arg_1', e.target.value)} className="h-7 mt-0.5" disabled={isFirstPlotAndPlaying} />
            </div>
          </>
        ) : selectedCpData.type === ESignalType.PAUSE_PROFILE ? (
          <div className="mb-2">
            <Label htmlFor={`cp-arg0-pauseduration-${plotSlot}`} className="text-xs"><T>Pause Profile:</T></Label>            
          </div>
        ) : (
          <>
            <div className="mb-2">
              <Label htmlFor={`cp-arg0-generic-${plotSlot}`} className="text-xs"><T>Argument 0:</T></Label>
              <Input id={`cp-arg0-generic-${plotSlot}`} type="number" value={selectedCpData.arg_0} onChange={(e) => onControlPointPropertyChange('arg_0', e.target.value)} className="h-7 mt-0.5" disabled={isFirstPlotAndPlaying} />
            </div>
            <div className="mb-2">
              <Label htmlFor={`cp-arg1-generic-${plotSlot}`} className="text-xs"><T>Argument 1:</T></Label>
              <Input id={`cp-arg1-generic-${plotSlot}`} type="number" value={selectedCpData.arg_1} onChange={(e) => onControlPointPropertyChange('arg_1', e.target.value)} className="h-7 mt-0.5" disabled={isFirstPlotAndPlaying} />
            </div>
          </>
        )}
        <div className="mt-2"> {/* Ensure arg_2 has margin too */}
          <Label htmlFor={`cp-arg2-${plotSlot}`} className="text-xs"><T>Argument 2 (Optional):</T></Label>
          <Input id={`cp-arg2-${plotSlot}`} type="number" value={selectedCpData.arg_2 === undefined ? '' : selectedCpData.arg_2} onChange={(e) => onControlPointPropertyChange('arg_2', e.target.value)} placeholder={translate("N/A")} className="h-7 mt-0.5" disabled={isFirstPlotAndPlaying} />
        </div>
      </div>
    </div>
  );
};

export default ControlPointProperties; 