import React, { useState, useEffect } from 'react';
import { useModbus } from '@/contexts/ModbusContext';
import { Card, CardContent, CardHeader, CardTitle, CardDescription } from '@/components/ui/card';
import { Button } from '@/components/ui/button';
import { Label } from '@/components/ui/label';
import { Input } from '@/components/ui/input';
import { Slider } from '@/components/ui/slider';
import { Separator } from '@/components/ui/separator';
import { toast } from 'sonner';
import { Play, StopCircle, RotateCcw, RotateCw, AlertTriangle } from 'lucide-react';
import type { RegisterData } from "@polymech/client-ts/modbusService";
import { T } from '../i18n';

// Enums and Consts specific to VFD or shared (consider moving to utils if widely shared)
const SAKO_MAIN_COMMANDS = { 
    STOP_DECEL: 6,        
    STOP_FREE_RUN: 5,   
    FAULT_RESET: 7,       
} as const;

const SAKO_DIRECTION_ACTIONS = {
    FWD: 1,               
    REV: 2,               
    STOP: 6,              
} as const;

// Neumorphic styles (assuming these are defined globally or passed as props)
const neumorphicBase = "px-4 py-2 rounded-lg font-semibold transition-all duration-150 ease-in-out focus:outline-none focus:ring-2 focus:ring-offset-2 flex items-center justify-center";
const neumorphicLight = `bg-slate-100 text-slate-700 hover:bg-slate-200 shadow-[3px_3px_7px_#bec8e4,-3px_-3px_7px_#ffffff] active:shadow-[inset_3px_3px_7px_#bec8e4,inset_-3px_-3px_7px_#ffffff]`;
const neumorphicDark = `dark:bg-slate-800 dark:text-slate-300 dark:hover:bg-slate-700 dark:shadow-[3px_3px_7px_#2c3e50,-3px_-3px_7px_#4a6572] dark:active:shadow-[inset_3px_3px_7px_#2c3e50,inset_-3px_-3px_7px_#4a6572]`;
const neumorphicButtonClass = `${neumorphicBase} ${neumorphicLight} ${neumorphicDark} py-2 px-3 text-sm`;
const neumorphicDestructiveBase = "px-4 py-2 rounded-lg font-semibold transition-all duration-150 ease-in-out focus:outline-none focus:ring-2 focus:ring-offset-2 flex items-center justify-center";
const neumorphicDestructiveLight = `bg-red-500 text-white hover:bg-red-600 shadow-[3px_3px_7px_#c82333,-3px_-3px_7px_#ff7878] active:shadow-[inset_3px_3px_7px_#c82333,inset_-3px_-3px_7px_#ff7878]`;
const neumorphicDestructiveDark = `dark:bg-red-600 dark:text-white dark:hover:bg-red-700 dark:shadow-[3px_3px_7px_#a71d2a,-3px_-3px_7px_#e85a5a] dark:active:shadow-[inset_3px_3px_7px_#a71d2a,inset_-3px_-3px_7px_#e85a5a]`;
const neumorphicDestructiveButtonClass = `${neumorphicDestructiveBase} ${neumorphicDestructiveLight} ${neumorphicDestructiveDark} py-2 px-3 text-sm`;

interface VFDControlsProps {
  vfdRunFreqReg: RegisterData | undefined;
  vfdSetFreqDisplayReg: RegisterData | undefined;
  vfdCurrentReg: RegisterData | undefined;
  vfdPowerReg: RegisterData | undefined;
  vfdTorqueReg: RegisterData | undefined;
  vfdFaultReg: RegisterData | undefined;
  vfdRunningReg: RegisterData | undefined;
  vfdSetFreqCmdRegAddr: number | undefined;
  vfdDirectionCmdRegAddr: number | undefined;
  vfdMainCommandRegAddr: number | undefined;
  // Scaling factors could also be props if they vary or are configured elsewhere
  VFD_RUN_FREQ_DISPLAY_SCALE: number;
  VFD_SET_FREQ_DISPLAY_RAW: boolean;
  VFD_CURRENT_DISPLAY_RAW: boolean;
}

const VFDControls: React.FC<VFDControlsProps> = ({
  vfdRunFreqReg,
  vfdSetFreqDisplayReg,
  vfdCurrentReg,
  vfdPowerReg,
  vfdTorqueReg,
  vfdFaultReg,
  vfdRunningReg,
  vfdSetFreqCmdRegAddr,
  vfdDirectionCmdRegAddr,
  vfdMainCommandRegAddr,
  VFD_RUN_FREQ_DISPLAY_SCALE,
  VFD_SET_FREQ_DISPLAY_RAW,
  VFD_CURRENT_DISPLAY_RAW
}) => {
  const { updateRegister } = useModbus();
  const [vfdSetFrequency, setVfdSetFrequency] = useState<number>(0); // User input in Hz

  useEffect(() => {
    if (vfdSetFreqDisplayReg) {
        let initialSliderValue = 0;
        if (VFD_SET_FREQ_DISPLAY_RAW) {
            initialSliderValue = Math.round(vfdSetFreqDisplayReg.value);
        } else {
            initialSliderValue = Math.round(vfdSetFreqDisplayReg.value / VFD_RUN_FREQ_DISPLAY_SCALE);
        }
      setVfdSetFrequency(Math.max(0, Math.min(75, initialSliderValue))); 
    }
  }, [vfdSetFreqDisplayReg, VFD_SET_FREQ_DISPLAY_RAW, VFD_RUN_FREQ_DISPLAY_SCALE]);

  const handleVfdMainCommand = async (sakoCommandValue: number) => {
    if (vfdMainCommandRegAddr === undefined) { toast.error(<T>VFD Main Command Address not found.</T>); return; }
    try {
      await updateRegister(vfdMainCommandRegAddr, sakoCommandValue);
      toast.success(<T>VFD Main Command {sakoCommandValue} sent.</T>);
    } catch (error) {
      toast.error(<T>Failed to send VFD Main Command.</T>);
      console.error('Error sending VFD Main Command:', error);
    }
  };
  
  const handleVfdSetFrequencyAndRun = async (direction: number) => {
    if (vfdSetFreqCmdRegAddr === undefined) { toast.error(<T>VFD Set Frequency Command Address not found.</T>); return; }
    if (vfdDirectionCmdRegAddr === undefined) { toast.error(<T>VFD Direction Command Address not found.</T>); return; }
    try {
      const freqToSend = Math.max(0, Math.min(75, Math.round(vfdSetFrequency)));
      await updateRegister(vfdSetFreqCmdRegAddr, freqToSend);
      toast.info(<T>Frequency {freqToSend} (0-75 value) sent.</T>);
      
      await updateRegister(vfdDirectionCmdRegAddr, direction);
      const directionName = Object.keys(SAKO_DIRECTION_ACTIONS).find(key => SAKO_DIRECTION_ACTIONS[key as keyof typeof SAKO_DIRECTION_ACTIONS] === direction) || `Value ${direction}`;
      toast.success(<T>VFD Run {directionName} command sent.</T>);
    } catch (error) {
      toast.error(<T>Failed to set VFD frequency and run.</T>);
      console.error('Error setting VFD frequency and run:', error);
    }
  };

  const handleVfdStop = async () => {
    if (vfdDirectionCmdRegAddr === undefined) { toast.error(<T>VFD Direction Command Address not found.</T>); return; }
    try {
      await updateRegister(vfdDirectionCmdRegAddr, SAKO_DIRECTION_ACTIONS.STOP);
      toast.success(<T>VFD Stop command sent.</T>);
    } catch (error) {
      toast.error(<T>Failed to send VFD Stop command.</T>);
      console.error('Error sending VFD Stop command:', error);
    }
  };

  return (
    <Card>
        <CardHeader>
            <CardTitle><T>VFD Control</T></CardTitle>
            <CardDescription><T>Monitor and control Variable Frequency Drive.</T></CardDescription>
        </CardHeader>
        <CardContent className="space-y-6">
          <div className="grid grid-cols-2 sm:grid-cols-3 lg:grid-cols-4 gap-x-4 gap-y-3 text-sm mb-4">
            <div><Label><T>Running Freq:</T></Label> <span className="font-bold">{vfdRunFreqReg ? (vfdRunFreqReg.value / VFD_RUN_FREQ_DISPLAY_SCALE).toFixed(2) : 'N/A'} Hz</span></div>
            <div><Label><T>Set Freq (Mon):</T></Label> <span className="font-bold">{vfdSetFreqDisplayReg ? (VFD_SET_FREQ_DISPLAY_RAW ? vfdSetFreqDisplayReg.value : (vfdSetFreqDisplayReg.value / VFD_RUN_FREQ_DISPLAY_SCALE).toFixed(2) ) : 'N/A'} Hz</span></div>
            <div><Label><T>Current:</T></Label> <span className="font-bold">{vfdCurrentReg ? (VFD_CURRENT_DISPLAY_RAW ? vfdCurrentReg.value : (vfdCurrentReg.value / 10).toFixed(1) ) : 'N/A'} {VFD_CURRENT_DISPLAY_RAW ? '(raw)':'A'}</span></div>
            <div><Label><T>Power:</T></Label> <span className="font-bold">{vfdPowerReg?.value ?? 'N/A'} kW</span></div>
            <div><Label><T>Torque:</T></Label> <span className="font-bold">{vfdTorqueReg?.value ?? 'N/A'} %</span></div>
            <div><Label><T>Fault Code:</T></Label> <span className="font-bold text-red-600">{vfdFaultReg?.value === 0 ? <T>None</T> : vfdFaultReg?.value ?? 'N/A'}</span></div>
            <div><Label><T>Status:</T></Label> <span className={`font-bold ${vfdRunningReg?.value === 1 ? 'text-green-600' : 'text-gray-600'}`}>{vfdRunningReg?.value === 1 ? <T>Running</T> : (vfdRunningReg?.value === 0 ? <T>Stopped</T> : <T>N/A</T>)}</span></div>
          </div>
          <Separator className="my-6" />
          <div className="mt-4 space-y-4"> 
            <div className="pb-2">
                <Label htmlFor="vfdSetFrequencySlider" className="block mb-1"><T>Set Target Frequency (0-75 Int):</T> <span className="font-bold">{vfdSetFrequency} Hz</span></Label>
                <div className="flex items-center space-x-4 mt-1">
                    <Slider
                      id="vfdSetFrequencySlider"
                      min={0}
                      max={75}
                      step={1} 
                      value={[vfdSetFrequency]}
                      onValueChange={(value) =>setVfdSetFrequency(value[0])}
                      className="flex-grow"
                    />
                    <Input 
                        type="number"
                        value={vfdSetFrequency} 
                        onChange={(e) => {
                            let val = parseInt(e.target.value, 10);
                            if (isNaN(val)) val = 0;
                            setVfdSetFrequency(Math.max(0, Math.min(75, val)));
                        }}
                        step={1}
                        min={0}
                        max={75}
                        className="w-24"
                    />
                </div>
            </div>
            <div className="space-y-2">
                <Label className="text-md font-medium"><T>Directional Commands</T></Label>
                <p className="text-xs text-muted-foreground"><T>Sets target frequency then sends command to direction register.</T></p>
                <div className="flex flex-wrap gap-3 items-center">
                    <Button className={neumorphicButtonClass} onClick={() => handleVfdSetFrequencyAndRun(SAKO_DIRECTION_ACTIONS.FWD)} disabled={!vfdDirectionCmdRegAddr || !vfdSetFreqCmdRegAddr}><RotateCw className="mr-2 h-4 w-4" /> <T>Run Forward</T></Button>
                    <Button className={neumorphicButtonClass} onClick={() => handleVfdSetFrequencyAndRun(SAKO_DIRECTION_ACTIONS.REV)} disabled={!vfdDirectionCmdRegAddr || !vfdSetFreqCmdRegAddr}><RotateCcw className="mr-2 h-4 w-4" /> <T>Run Reverse</T></Button>
                    <Button className={neumorphicDestructiveButtonClass} onClick={() => handleVfdStop()} disabled={!vfdDirectionCmdRegAddr}><StopCircle className="mr-2 h-4 w-4"/><T>Stop VFD</T></Button>
                </div>
            </div>
            <div className="space-y-2 pt-3">
                <Label className="text-md font-medium"><T>Other Commands</T></Label>
                <div className="flex flex-wrap gap-3 items-center">
                    <Button className={neumorphicButtonClass} variant="outline" onClick={() => handleVfdMainCommand(SAKO_MAIN_COMMANDS.FAULT_RESET)} disabled={!vfdMainCommandRegAddr}><AlertTriangle className="mr-2 h-4 w-4" /> <T>Reset Fault</T></Button>
                </div>
            </div>
          </div>
        </CardContent>
      </Card>
  );
}

export default VFDControls; 