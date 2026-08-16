import React, { useState, useEffect, useMemo } from 'react';
import { useModbus, RegisterData, CoilData } from '@/contexts/ModbusContext';
import { Button } from '@/components/ui/button';
import { Input } from '@/components/ui/input';
import { Label } from '@/components/ui/label';
import { Switch } from '@/components/ui/switch';

import { toast } from 'sonner';
import { T, translate } from '../i18n';
import { getModbusErrorDescription } from '../lib/modbusErrorMap';
import LEDBars from './LEDBars';
import { ArrowUpFromDot, StopCircle, Link2Off as WifiOff, Plus, Minus } from 'lucide-react';
import PressureProfileRow from './profiles/PressureProfileRow';

import { cn } from '@/lib/utils';
import { PRESS_CYLINDER_GROUP, PRESS_CYLINDER_REGISTER_NAMES, LOADCELL_GROUP, LOADCELL_REGISTER_NAMES, SOLENOID_GROUP, SOLENOID_REGISTER_NAMES } from '@/constants';


export const PRESS_CYLINDER_STATES = {
    STATE_IDLE: 0,
    STATE_MAXLOAD: 1,
    STATE_ERROR: 2
};

export const PRESS_CYLINDER_MODES = {
    MODE_MANUAL: 1,
    MODE_AUTO: 2,
    MODE_MANUAL_MULTI: 3,
    MODE_AUTO_MULTI: 4,
    MODE_AUTO_MULTI_BALANCED: 5,
    MODE_AUTO_DETECT: 7
};

export const pressCylinderStateToString = (value: number): string => {
    for (const key in PRESS_CYLINDER_STATES) {
        if (PRESS_CYLINDER_STATES[key as keyof typeof PRESS_CYLINDER_STATES] === value) {
            const stateKey = key.replace('STATE_', '').replace(/_/g, ' ');
            return translate(stateKey);
        }
    }
    return translate('UNKNOWN ({value})').replace('{value}', value.toString());
};

export const pressCylinderModeToString = (value: number): string => {
    for (const key in PRESS_CYLINDER_MODES) {
        if (PRESS_CYLINDER_MODES[key as keyof typeof PRESS_CYLINDER_MODES] === value) {
            const modeKey = key.replace('MODE_', '').replace(/_/g, ' ');
            return translate(modeKey);
        }
    }
    return translate('UNKNOWN ({value})').replace('{value}', value.toString());
}

const PressCylinderControl: React.FC = () => {
    const {
        updateRegister,
        updateCoil,
        isConnected,
        registers: allModbusRegisters,
        coils: allModbusCoils,
        pressureProfiles
    } = useModbus();

    const pcregs = useMemo(() => allModbusRegisters.filter(r => r.group && r.group.includes(PRESS_CYLINDER_GROUP)), [allModbusRegisters]);
    const pcCoils = useMemo(() => allModbusCoils ? allModbusCoils.filter(c => c.group && c.group.includes(PRESS_CYLINDER_GROUP)) : [], [allModbusCoils]);
    const loadcellRegs = useMemo(() => allModbusRegisters.filter(r => r.group && r.group.includes(LOADCELL_GROUP)), [allModbusRegisters]);
    const loadcellPvRegs = useMemo(() => loadcellRegs.filter(r => r.name.includes(LOADCELL_REGISTER_NAMES.PV)), [loadcellRegs]);
    const solenoidCoils = useMemo(() => allModbusCoils ? allModbusCoils.filter(c => c.group && c.group.includes(SOLENOID_GROUP)) : [], [allModbusCoils]);
    const solenoidStateCoils = useMemo(() => solenoidCoils.filter(c => c.name.includes(SOLENOID_REGISTER_NAMES.STATE)), [solenoidCoils]);
    const stateReg = useMemo(() => pcregs.find(r => r.name.includes(PRESS_CYLINDER_REGISTER_NAMES.STATE)), [pcregs]);
    const pvRegs = useMemo(() => pcregs.filter(r => r.name.startsWith(PRESS_CYLINDER_REGISTER_NAMES.PRESSURE)), [pcregs]);
    const spReg = useMemo(() => pcregs.find(r => r.name.includes(PRESS_CYLINDER_REGISTER_NAMES.SP)), [pcregs]);
    const modeReg = useMemo(() => pcregs.find(r => r.name.includes(PRESS_CYLINDER_REGISTER_NAMES.MODE)), [pcregs]);
    const errorReg = useMemo(() => pcregs.find(r => r.name.includes(PRESS_CYLINDER_REGISTER_NAMES.ERROR)), [pcregs]);
    const enabledCoil = useMemo(() => pcCoils.find(c => c.name.includes(PRESS_CYLINDER_REGISTER_NAMES.ENABLED)), [pcCoils]);
    const interlockedCoil = useMemo(() => pcCoils.find(c => c.name.includes(PRESS_CYLINDER_REGISTER_NAMES.INTERLOCKED)), [pcCoils]);
    const commandRegAddr = useMemo(() => pcregs.find(r => r.name.includes(PRESS_CYLINDER_REGISTER_NAMES.COMMAND))?.address, [pcregs]);
    const spCmdRegAddr = useMemo(() => pcregs.find(r => r.name.includes(PRESS_CYLINDER_REGISTER_NAMES.SP))?.address, [pcregs]);

    const [localSp, setLocalSp] = useState<number>(0);
    const [localPvs, setLocalPvs] = useState<number[]>([0, 0]);
    const [localLoadcellPvs, setLocalLoadcellPvs] = useState<number[]>([0, 0]);
    const [isSavingSp, setIsSavingSp] = useState(false);
    const [optimisticEnabled, setOptimisticEnabled] = useState(false);
    const [optimisticMode, setOptimisticMode] = useState<number>(0);
    const [optimisticInterlocked, setOptimisticInterlocked] = useState(false);
    const [optimisticSolenoidStates, setOptimisticSolenoidStates] = useState<boolean[]>([false, false]);

    const errorCodeMap = useMemo(() => {
        const map = new Map<number, string>();
        if (!errorReg?.name) return map;

        const content = errorReg.name.substring(errorReg.name.indexOf('(') + 1, errorReg.name.lastIndexOf(')'));
        if (!content) return map;

        const pairs = content.split(',');
        for (const pair of pairs) {
            const [codeStr, text] = pair.split(':');
            const code = parseInt(codeStr, 10);
            if (!isNaN(code) && text) {
                map.set(code, text);
            }
        }
        return map;
    }, [errorReg?.name]);

    useEffect(() => {
        if (enabledCoil !== undefined) {
            setOptimisticEnabled(enabledCoil.value);
        }
    }, [enabledCoil]);

    useEffect(() => {
        if (modeReg !== undefined) {
            setOptimisticMode(modeReg.value);
        }
    }, [modeReg]);

    useEffect(() => {
        if (interlockedCoil !== undefined) {
            setOptimisticInterlocked(interlockedCoil.value);
        }
    }, [interlockedCoil]);

    useEffect(() => {
        if (pvRegs?.length) {
            const newPvs = pvRegs.map(r => r.value !== undefined ? r.value : 0);
            if (newPvs.length > 0) {
                setLocalPvs(newPvs);
            }
        }
    }, [pvRegs]);

    useEffect(() => {
        if (loadcellPvRegs?.length) {
            const newLoadcellPvs = loadcellPvRegs.map(r => r.value !== undefined ? r.value : 0);
            if (newLoadcellPvs.length > 0) {
                setLocalLoadcellPvs(newLoadcellPvs);
            }
        }
    }, [loadcellPvRegs]);

    useEffect(() => {
        if (solenoidStateCoils?.length) {
            const newSolenoidStates = solenoidStateCoils.map(c => c.value !== undefined ? c.value : false);
            if (newSolenoidStates.length > 0) {
                setOptimisticSolenoidStates(newSolenoidStates);
            }
        }
    }, [solenoidStateCoils]);

    useEffect(() => {
        if (spReg?.value !== undefined) {
            setLocalSp(spReg.value);
        }
    }, [spReg?.value]);

    const handleCommand = async (command: number) => {
        if (commandRegAddr === undefined) {
            toast.error(translate('Command register not found.'));
            return;
        }
        try {
            await updateRegister(commandRegAddr, command);
            toast.success(translate('Command {command} sent.').replace('{command}', command.toString()));
        } catch (error) {
            toast.error(translate('Failed to send command: {error}').replace('{error}', String(error)));
        }
    };

    const handleStop = async () => {
        if (!isConnected) {
            toast.error(translate('Not connected'));
            return;
        }
        if (!modeReg || !spCmdRegAddr) {
            toast.error(translate('Mode or SP register not found.'));
            return;
        }

        try {
            await updateRegister(modeReg.address, PRESS_CYLINDER_MODES.MODE_AUTO_DETECT);
            await updateRegister(spCmdRegAddr, 0); // Set SP to 0
            setOptimisticMode(PRESS_CYLINDER_MODES.MODE_AUTO_DETECT);
            setLocalSp(0);
            toast.success(translate('Stopped. Mode set to auto-detect, SP set to 0.'));
        } catch (error) {
            toast.error(translate('Failed to stop: {error}').replace('{error}', error instanceof Error ? error.message : translate('Unknown error')));
        }
    };

    const handlePresetSp = async (value: number) => {
        if (spCmdRegAddr === undefined) {
            toast.error(translate('Set Point command register not found.'));
            return;
        }
        if (!isConnected) {
            toast.error(translate('Not connected'));
            return;
        }

        const originalSp = localSp;
        setLocalSp(value);
        setIsSavingSp(true);
        try {
            await updateRegister(spCmdRegAddr, value);
            toast.success(translate('Set Point updated to {value}.').replace('{value}', value.toString()));
        } catch (error) {
            toast.error(translate('Failed to update Set Point: {error}').replace('{error}', String(error)));
            setLocalSp(originalSp);
        } finally {
            setIsSavingSp(false);
        }
    };

    const handleAdjustSp = (e: React.MouseEvent, baseDelta: number) => {
        const multiplier = e.shiftKey ? 10 : 1;
        const delta = baseDelta * multiplier;
        const newSp = Math.max(0, Math.min(100, localSp + delta));
        handlePresetSp(newSp);
    };

    const handleToggleChange = async (
        reg: RegisterData | CoilData | undefined,
        value: boolean | number,
        name: string,
        isCoil: boolean,
        setter: React.Dispatch<React.SetStateAction<any>>
    ) => {
        if (!reg) {
            toast.error(translate('{name} register/coil not found.').replace('{name}', name));
            return;
        }

        const previousValue = reg.value;
        setter(value);

        try {
            if (isCoil) {
                await updateCoil(reg.address, value as boolean);
            } else {
                await updateRegister(reg.address, typeof value === 'boolean' ? (value ? 1 : 0) : value);
            }
            toast.success(translate('{name} updated.').replace('{name}', name));
        } catch (error) {
            toast.error(translate('Failed to update {name}: {error}').replace('{name}', name).replace('{error}', error instanceof Error ? error.message : String(error)));
            setter(previousValue);
        }
    };

    const handleSolenoidToggle = async (index: number, value: boolean) => {
        const solenoidCoil = solenoidStateCoils[index];
        if (!solenoidCoil) {
            toast.error(translate('Solenoid {index} not found.').replace('{index}', (index + 1).toString()));
            return;
        }

        const previousStates = [...optimisticSolenoidStates];
        const newStates = [...optimisticSolenoidStates];
        newStates[index] = value;
        setOptimisticSolenoidStates(newStates);

        try {
            await updateCoil(solenoidCoil.address, value);
            toast.success(translate('Solenoid {index} updated.').replace('{index}', (index + 1).toString()));
        } catch (error) {
            toast.error(translate('Failed to update Solenoid {index}: {error}').replace('{index}', (index + 1).toString()).replace('{error}', error instanceof Error ? error.message : String(error)));
            setOptimisticSolenoidStates(previousStates);
        }
    };

    const handleSpKeyDown = (event: React.KeyboardEvent<HTMLInputElement>) => {
        if (event.key === 'Enter') {
            handleApplySp();
            event.currentTarget.blur();
        }
    };

    const handleSpInputChange = (e: React.ChangeEvent<HTMLInputElement>) => {
        const value = e.target.value === '' ? 0 : parseInt(e.target.value, 10);
        setLocalSp(Math.max(0, Math.min(100, isNaN(value) ? 0 : value)));
    };

    const handleApplySp = async () => {
        if (spCmdRegAddr === undefined) {
            toast.error(translate('Set Point command register not found.'));
            return;
        }
        setIsSavingSp(true);
        try {
            await updateRegister(spCmdRegAddr, localSp);
            toast.success(translate('Set Point updated to {localSp}.').replace('{localSp}', localSp.toString()));
        } catch (error) {
            toast.error(translate('Failed to update Set Point: {error}').replace('{error}', String(error)));
        } finally {
            setIsSavingSp(false);
        }
    };

    const errorValue = errorReg?.value;
    const firmwareErrorMessage = errorValue && errorValue > 0 ? errorCodeMap.get(errorValue) : undefined;

    // Check for Modbus errors (e.g. timeout) on any of the relevant registers
    const modbusErrorReg = useMemo(() => {
        return [...pcregs, ...loadcellRegs].find(r => r.error && r.error !== 0);
    }, [pcregs, loadcellRegs]);

    const modbusErrorValue = modbusErrorReg?.error;
    const modbusErrorMessage = modbusErrorValue ? getModbusErrorDescription(modbusErrorValue) : undefined;

    const displayErrorValue = modbusErrorValue || errorValue;
    const displayErrorMessage = modbusErrorMessage || firmwareErrorMessage;

    const currentState = stateReg?.value !== undefined ? pressCylinderStateToString(stateReg.value) : translate('N/A');
    const isPressing = !displayErrorMessage && localPvs.some(pv => pv > 5);
    const isAutoMode = [
        PRESS_CYLINDER_MODES.MODE_AUTO,
        PRESS_CYLINDER_MODES.MODE_AUTO_MULTI,
        PRESS_CYLINDER_MODES.MODE_AUTO_MULTI_BALANCED,
        PRESS_CYLINDER_MODES.MODE_AUTO_DETECT,
    ].includes(optimisticMode);

    return (
        <div className="glass-card p-2 flex flex-col items-center gap-2" id="press-cylinder-controls-container">
            <div className="flex flex-col md:flex-row items-stretch justify-center gap-2 bg-slate-200/50 dark:bg-slate-800/50 p-2 md:p-3 rounded-2xl border border-slate-300 dark:border-slate-700 shadow-none md:shadow-inner w-full">
                <div className="flex flex-col items-center justify-around w-full md:w-1/3 p-1">
                    <Label className="text-xs font-semibold"><T>PV</T></Label>
                    <div className="flex items-end h-48 md:h-64 gap-1">
                        <LEDBars orientation="vertical" value={localPvs[0]} barHeightClass="h-full" barWidthClass="w-6" />
                        <LEDBars orientation="vertical" value={localPvs[1]} barHeightClass="h-full" barWidthClass="w-6" />
                    </div>
                    <div className="flex text-xs text-muted-foreground mt-1 gap-1">
                        <span>({localLoadcellPvs[0] !== undefined && localLoadcellPvs[0] !== 65529 ? localLoadcellPvs[0] : 'N/A'})</span>
                        <span>({localLoadcellPvs[1] !== undefined && localLoadcellPvs[1] !== 65529 ? localLoadcellPvs[1] : 'N/A'})</span>
                    </div>
                </div>
                <div className="flex flex-col items-center justify-between gap-2 w-full md:w-2/3">
                    <div className="flex items-center justify-center gap-2">
                        <Button
                            onClick={() => handleCommand(3)}
                            disabled={commandRegAddr === undefined || !optimisticEnabled || !isConnected}
                            className={cn(
                                "w-16 h-16 rounded-full text-white shadow-xl transition-all duration-300 ease-in-out transform hover:scale-110 border-4",
                                !optimisticEnabled ? "bg-gray-500 border-gray-600/50 cursor-not-allowed" :
                                    isPressing ? "bg-yellow-500 border-yellow-400/50 hover:bg-yellow-600" : "bg-green-500 border-green-400/50 hover:bg-green-600"
                            )}
                            title={translate("Press")}
                        >
                            <ArrowUpFromDot className="w-8 h-8" />
                        </Button>
                        <Button
                            onClick={handleStop}
                            disabled={!optimisticEnabled || !isConnected}
                            className={cn(
                                "w-16 h-16 rounded-full text-white shadow-xl transition-all duration-300 ease-in-out transform hover:scale-110 border-4",
                                !optimisticEnabled ? "bg-gray-500 border-gray-600/50 cursor-not-allowed" : "bg-red-500 border-red-400/50 hover:bg-red-600"
                            )}
                            title={translate("Stop")}
                        >
                            <StopCircle className="w-8 h-8" />
                        </Button>
                    </div>
                    <div className="w-full px-2 space-y-2">
                        <div className="flex items-center justify-center w-full">
                            <div className="flex flex-col items-center gap-1">
                                <Label className="text-xs font-semibold"><T>Mode</T></Label>
                                <div className="flex flex-col gap-1">
                                    {Object.entries(PRESS_CYLINDER_MODES).map(([key, value]) => (
                                        <Button
                                            key={key}
                                            size="sm"
                                            variant={optimisticMode === value ? "default" : "outline"}
                                            className="h-6 px-2 text-xs leading-none whitespace-nowrap"
                                            onClick={() => handleToggleChange(modeReg, value, translate('Mode'), false, setOptimisticMode)}
                                            disabled={!optimisticEnabled || !isConnected}
                                        >
                                            {pressCylinderModeToString(value)}
                                        </Button>
                                    ))}
                                </div>
                            </div>
                        </div>
                        <div className="flex items-center gap-2">
                            <Label htmlFor="sp-input" className="text-xs whitespace-nowrap"><T>SP:</T></Label>
                            <Button
                                size="icon"
                                variant="outline"
                                className="h-8 w-8 flex-shrink-0"
                                onClick={(e) => handleAdjustSp(e, -1)}
                                disabled={isSavingSp || !optimisticEnabled || !isConnected}
                                title={translate("Decrease SP (hold Shift for -10)")}
                            >
                                <Minus className="h-4 w-4" />
                            </Button>
                            <Input
                                id="sp-input"
                                type="number"
                                value={localSp}
                                onChange={handleSpInputChange}
                                onKeyDown={handleSpKeyDown}
                                onBlur={handleApplySp}
                                min={0}
                                max={100}
                                step={1}
                                className="h-8 text-center"
                                disabled={isSavingSp || !optimisticEnabled || !isConnected}
                            />
                            <Button
                                size="icon"
                                variant="outline"
                                className="h-8 w-8 flex-shrink-0"
                                onClick={(e) => handleAdjustSp(e, 1)}
                                disabled={isSavingSp || !optimisticEnabled || !isConnected}
                                title={translate("Increase SP (hold Shift for +10)")}
                            >
                                <Plus className="h-4 w-4" />
                            </Button>
                        </div>

                        <div className="flex gap-2 pt-2 w-full">
                            <Button size="sm" variant="outline" className="flex-1 min-w-0 text-xs" onClick={() => handlePresetSp(30)} disabled={isSavingSp || !isConnected}><T>Low</T></Button>
                            <Button size="sm" variant="outline" className="flex-1 min-w-0 text-xs" onClick={() => handlePresetSp(70)} disabled={isSavingSp || !isConnected}><T>Mid</T></Button>
                            <Button size="sm" variant="outline" className="flex-1 min-w-0 text-xs" onClick={() => handlePresetSp(100)} disabled={isSavingSp || !isConnected}><T>Max</T></Button>
                        </div>
                    </div>
                </div>
            </div>
            <div className="text-center text-xs text-muted-foreground space-y-1">
                <div className="font-semibold truncate" title={translate("Press Cylinder Controls")}><T>Press Cylinder</T></div>
                <div className="font-bold flex justify-center items-center gap-2">
                    {displayErrorValue === 224 ? (
                        <WifiOff className="w-4 h-4 text-red-500" />
                    ) : displayErrorMessage ? (
                        <span className="text-red-500">{displayErrorMessage}</span>
                    ) : (
                        currentState
                    )}
                </div>
            </div>
            <div className="flex justify-center gap-4 mt-1">
                {solenoidStateCoils.map((solenoidCoil, index) => (
                    <div key={solenoidCoil.address} className="flex flex-col items-center gap-1">
                        <Label className="text-xs font-semibold">
                            <T>Solenoid</T> {index + 1}
                        </Label>
                        <Switch
                            checked={optimisticSolenoidStates[index] || false}
                            onCheckedChange={(checked) => handleSolenoidToggle(index, checked)}
                            disabled={!isConnected}
                        />
                    </div>
                ))}
                <div className="flex flex-col items-center gap-1">
                    <Label htmlFor="interlocked-switch" className="text-xs font-semibold"><T>Interlocked</T></Label>
                    <Switch
                        id="interlocked-switch"
                        checked={optimisticInterlocked}
                        onCheckedChange={(checked) => handleToggleChange(interlockedCoil, checked, translate('Interlocked'), true, setOptimisticInterlocked)}
                        disabled={!isConnected}
                    />
                </div>
            </div>

            {pressureProfiles && pressureProfiles.length > 0 && (
                <div className="w-full flex flex-col gap-2 mt-2">
                    <Label className="text-xs font-semibold pl-1"><T>Pressure Profiles</T></Label>
                    <div className="flex flex-col gap-2 p-2 rounded-xl bg-slate-100/50 dark:bg-slate-900/50 border border-slate-200 dark:border-slate-800">
                        {pressureProfiles.map(profile => (
                            <PressureProfileRow key={profile.id} profile={profile} />
                        ))}
                    </div>
                </div>
            )}
        </div>
    );
};

export default PressCylinderControl; 