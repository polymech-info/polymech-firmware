import React, { useMemo } from 'react';
import { Button } from '@/components/ui/button';
import { useToast } from "@/components/ui/use-toast";
import { RotateCw, RotateCcw, StopCircle } from 'lucide-react';
import { translate } from '../i18n';
import { useModbus } from '@/contexts/ModbusContext';
import { DELTA_VFD_REGISTER_NAMES, DELTA_VFD_DIRECTION_COMMANDS, DELTA_VFD_GROUP } from '@/constants';

// Using Delta VFD direction commands from constants

const VFD_PRESET_SPEEDS = {
  SLOW: 1500,
  MEDIUM: 2500,
  FAST: 4500,
} as const;

const VFD_RUN_FREQ_DISPLAY_SCALE = 100;  // Delta VFD RUNNING_FREQUENCY register returns centiHz (5000 = 50 Hz)
const VFD_SET_FREQ_DISPLAY_RAW = true;

const VFDControls = () => {
    const { 
        isConnected, 
        updateRegister,
        registers: allModbusRegisters,
    } = useModbus();
    const { toast } = useToast();

    const vfdRunFreqReg = useMemo(() => allModbusRegisters.find(r => r.group?.startsWith(DELTA_VFD_GROUP) && r.name === DELTA_VFD_REGISTER_NAMES.RUNNING_FREQUENCY), [allModbusRegisters]);
    const vfdSetFreqDisplayReg = useMemo(() => allModbusRegisters.find(r => r.group?.startsWith(DELTA_VFD_GROUP) && r.name === DELTA_VFD_REGISTER_NAMES.SET_FREQUENCY), [allModbusRegisters]);
    const vfdRunningReg = useMemo(() => allModbusRegisters.find(r => r.group?.startsWith(DELTA_VFD_GROUP) && r.name === DELTA_VFD_REGISTER_NAMES.IS_RUNNING), [allModbusRegisters]);
    const vfdSetFreqCmdReg = useMemo(() => allModbusRegisters.find(r => r.group?.startsWith(DELTA_VFD_GROUP) && r.name === DELTA_VFD_REGISTER_NAMES.CMD_FREQ), [allModbusRegisters]);
    const vfdDirectionCmdReg = useMemo(() => allModbusRegisters.find(r => r.group?.startsWith(DELTA_VFD_GROUP) && r.name === DELTA_VFD_REGISTER_NAMES.CMD_DIRECTION), [allModbusRegisters]);

    const handleVfdPresetCommand = async (speed: number) => {
        if (!isConnected) {
            toast({ title: "Not connected.", variant: "destructive" });
            return;
        }
        if (!vfdSetFreqCmdReg?.address || !vfdDirectionCmdReg?.address) {
            toast({ title: "VFD command registers not found.", variant: "destructive" });
            return;
        }

        const freqToSend = Math.max(0, Math.min(75, speed)); // CMD_FREQ register expects Hz (setFrequency handles centiHz conversion)

        try {
            await updateRegister(vfdSetFreqCmdReg.address, freqToSend);
            await updateRegister(vfdDirectionCmdReg.address, DELTA_VFD_DIRECTION_COMMANDS.FORWARD);
            toast({ title: `VFD command sent: FWD at ${speed} Hz` });
        } catch (error) {
            toast({ title: "VFD Preset Command Failed", description: `${error}`, variant: "destructive" });
        }
    };

    const handleVfdCommand = async (direction: number) => {
        if (!isConnected) {
            toast({ title: "Not connected.", variant: "destructive" });
            return;
        }
        if (!vfdSetFreqCmdReg?.address || !vfdDirectionCmdReg?.address) {
            toast({ title: "VFD command registers not found.", variant: "destructive" });
            return;
        }

        let freqToSend = 0;
        if (vfdSetFreqDisplayReg?.value) {
            let value = vfdSetFreqDisplayReg.value;
            if (VFD_SET_FREQ_DISPLAY_RAW) {
                freqToSend = Math.round(value); // SET_FREQUENCY register returns Hz
            } else {
                freqToSend = Math.round(value / VFD_RUN_FREQ_DISPLAY_SCALE); // Already in Hz
            }
            freqToSend = Math.max(0, Math.min(75, freqToSend)); // Limit to 75 Hz
        }

        try {
            if (direction !== DELTA_VFD_DIRECTION_COMMANDS.STOP) {
                await updateRegister(vfdSetFreqCmdReg.address, freqToSend);
            }
            await updateRegister(vfdDirectionCmdReg.address, direction);
            const directionName = Object.keys(DELTA_VFD_DIRECTION_COMMANDS).find(key => DELTA_VFD_DIRECTION_COMMANDS[key as keyof typeof DELTA_VFD_DIRECTION_COMMANDS] === direction) || `Value ${direction}`;
            toast({ title: `VFD Command Sent: ${directionName}` });
        } catch (error) {
            toast({ title: "VFD Command Failed", description: `${error}`, variant: "destructive" });
        }
    };

    return (
        <div className="flex flex-col items-center gap-2 p-2" id="vfd-controls-container">
            <div className="flex items-center justify-center gap-2 bg-slate-200/50 dark:bg-slate-800/50 p-2 rounded-2xl border-2 border-slate-300 dark:border-slate-700 shadow-inner">
                <Button onClick={() => handleVfdCommand(DELTA_VFD_DIRECTION_COMMANDS.FORWARD)} disabled={!isConnected} className="w-12 h-12 rounded-full text-white bg-green-500 hover:bg-green-600"><RotateCw className="w-8 h-8" /></Button>
                <Button onClick={() => handleVfdCommand(DELTA_VFD_DIRECTION_COMMANDS.REVERSE)} disabled={!isConnected} className="w-12 h-12 rounded-full text-white bg-blue-500 hover:bg-blue-600"><RotateCcw className="w-8 h-8" /></Button>
                <Button onClick={() => handleVfdCommand(DELTA_VFD_DIRECTION_COMMANDS.STOP)} disabled={!isConnected} variant="destructive" className="w-12 h-12 rounded-full"><StopCircle className="w-8 h-8" /></Button>
            </div>
            <div className="flex items-center justify-center gap-2 mt-2">
                <Button onClick={() => handleVfdPresetCommand(VFD_PRESET_SPEEDS.SLOW)} disabled={!isConnected} variant="outline" size="sm">Slow</Button>
                <Button onClick={() => handleVfdPresetCommand(VFD_PRESET_SPEEDS.MEDIUM)} disabled={!isConnected} variant="outline" size="sm">Medium</Button>
                <Button onClick={() => handleVfdPresetCommand(VFD_PRESET_SPEEDS.FAST)} disabled={!isConnected} variant="outline" size="sm">Fast</Button>
            </div>
            <div className="text-center text-xs text-muted-foreground mt-2 space-y-1">
                <div className="font-semibold truncate" title="VFD Controls">VFD Controls</div>
                <div className={`font-bold ${vfdRunningReg?.value === 1 ? 'text-green-500' : ''}`}>
                    {vfdRunningReg?.value === 1 ? translate('Running') : translate('Stopped')}
                </div>
                {vfdRunFreqReg?.value !== undefined && (
                <div className="font-mono font-bold text-sm text-primary">
                    {(vfdRunFreqReg.value / VFD_RUN_FREQ_DISPLAY_SCALE).toFixed(2)} Hz
                </div>
                )}
            </div>
        </div>
    );
};

export default VFDControls; 