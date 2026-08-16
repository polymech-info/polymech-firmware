import React, { useState, useEffect } from 'react';
import { useModbus } from '@/contexts/ModbusContext'; // To get updateRegister
import { Card, CardContent, CardHeader, CardTitle, CardDescription } from '@/components/ui/card';
import { Button } from '@/components/ui/button';
import { Label } from '@/components/ui/label';
import { Slider } from '@/components/ui/slider'; 
import { Switch } from '@/components/ui/switch'; 
import { toast } from 'sonner'; 
import { Gauge, Zap } from 'lucide-react';

import type { RegisterData } from "@polymech/client-ts";

// Enums and helpers needed by this component
const E_MODE = {
    LOCAL: 0,
    REMOTE: 1
} as const;
type ModeKeys = keyof typeof E_MODE;

const modeToString = (value: number): ModeKeys => {
    return value === E_MODE.REMOTE ? 'REMOTE' : 'LOCAL';
};

// Neumorphic styles (can be moved to a shared file later if used by many components)
const neumorphicBase = "px-4 py-2 rounded-lg font-semibold transition-all duration-150 ease-in-out focus:outline-none focus:ring-2 focus:ring-offset-2 flex items-center justify-center";
const neumorphicLight = `bg-slate-100 text-slate-700 hover:bg-slate-200 shadow-[3px_3px_7px_#bec8e4,-3px_-3px_7px_#ffffff] active:shadow-[inset_3px_3px_7px_#bec8e4,inset_-3px_-3px_7px_#ffffff]`;
const neumorphicDark = `dark:bg-slate-800 dark:text-slate-300 dark:hover:bg-slate-700 dark:shadow-[3px_3px_7px_#2c3e50,-3px_-3px_7px_#4a6572] dark:active:shadow-[inset_3px_3px_7px_#2c3e50,inset_-3px_-3px_7px_#4a6572]`;
const neumorphicButtonClass = `${neumorphicBase} ${neumorphicLight} ${neumorphicDark}`;

interface PotentiometerControlsProps {
  pot1ValueReg: RegisterData | undefined;
  pot1ModeReg: RegisterData | undefined;
  pot1RemoteSettingReg: RegisterData | undefined;
  pot2ValueReg: RegisterData | undefined;
  pot2ModeReg: RegisterData | undefined;
  pot2RemoteSettingReg: RegisterData | undefined;
  // updateRegister: (address: number, value: number) => Promise<void>; // Will get from useModbus context instead
}

const PotentiometerControls: React.FC<PotentiometerControlsProps> = ({
  pot1ValueReg,
  pot1ModeReg,
  pot1RemoteSettingReg,
  pot2ValueReg,
  pot2ModeReg,
  pot2RemoteSettingReg
}) => {
  const { updateRegister } = useModbus(); // Get updateRegister from context

  const [pot1RemoteValue, setPot1RemoteValue] = useState<number>(0);
  const [pot2RemoteValue, setPot2RemoteValue] = useState<number>(0);

  useEffect(() => {
    if (pot1ModeReg?.value === E_MODE.REMOTE && pot1RemoteSettingReg) {
      setPot1RemoteValue(pot1RemoteSettingReg.value);
    }
  }, [pot1RemoteSettingReg, pot1ModeReg]);

  useEffect(() => {
    if (pot2ModeReg?.value === E_MODE.REMOTE && pot2RemoteSettingReg) {
      setPot2RemoteValue(pot2RemoteSettingReg.value);
    }
  }, [pot2RemoteSettingReg, pot2ModeReg]);

  const handlePotModeChange = async (potNumber: 1 | 2, newMode: number) => {
    const address = potNumber === 1 ? pot1ModeReg?.address : pot2ModeReg?.address;
    if (address === undefined) {
      toast.error(`POT ${potNumber} mode register not found.`);
      return;
    }
    try {
      await updateRegister(address, newMode);
      toast.success(`POT ${potNumber} mode updated to ${modeToString(newMode)}`);
    } catch (error) {
      toast.error(`Failed to update POT ${potNumber} mode.`);
      console.error(`Error updating POT ${potNumber} mode:`, error);
    }
  };

  const handlePotRemoteValueChange = (potNumber: 1 | 2, value: number) => {
    if (potNumber === 1) setPot1RemoteValue(value);
    else setPot2RemoteValue(value);
  };

  const applyPotRemoteValue = async (potNumber: 1 | 2) => {
    const address = potNumber === 1 ? pot1RemoteSettingReg?.address : pot2RemoteSettingReg?.address;
    const value = potNumber === 1 ? pot1RemoteValue : pot2RemoteValue;
    const modeReg = potNumber === 1 ? pot1ModeReg : pot2ModeReg;
    if (modeReg?.value !== E_MODE.REMOTE) {
        toast.warning(`POT ${potNumber} is not in Remote mode. Value not applied.`);
        return;
    }
    if (address === undefined) {
      toast.error(`POT ${potNumber} remote value register not found.`);
      return;
    }
    try {
      await updateRegister(address, value);
      toast.success(`POT ${potNumber} remote value set to ${value}`);
    } catch (error) {
      toast.error(`Failed to set POT ${potNumber} remote value.`);
      console.error(`Error setting POT ${potNumber} remote value:`, error);
    }
  };

  return (
    <div className="grid grid-cols-1 md:grid-cols-2 gap-8">
        <Card className="flex flex-col">
          <CardHeader>
            <CardTitle className="flex items-center"><Gauge className="mr-2 h-5 w-5 text-blue-500"/>Speed Control</CardTitle>
            <CardDescription>Adjust overall speed.</CardDescription>
          </CardHeader>
          <CardContent className="flex-grow flex flex-col justify-between space-y-4">
            <div className="space-y-3">
                <p className="text-sm">Current Value: <span className="font-bold text-blue-600 dark:text-blue-400 text-lg">{pot1ValueReg?.value ?? 'N/A'}</span></p>
                <div className="flex items-center space-x-2">
                    <Label htmlFor="pot1Mode">Mode:</Label>
                    <Switch 
                    id="pot1Mode"
                    checked={pot1ModeReg?.value === E_MODE.REMOTE}
                    onCheckedChange={(checked) => handlePotModeChange(1, checked ? E_MODE.REMOTE : E_MODE.LOCAL)}
                    disabled={!pot1ModeReg}
                    />
                    <span className="font-medium">{pot1ModeReg ? modeToString(pot1ModeReg.value) : 'N/A'}</span>
                </div>
            </div>
            {pot1ModeReg?.value === E_MODE.REMOTE && (
              <div className="space-y-3 pt-2">
                <Label htmlFor="pot1RemoteValSlider" className="text-sm">Set Remote Speed (0-255): <span className="font-bold text-blue-600 dark:text-blue-400">{pot1RemoteValue}</span></Label>
                <Slider
                  id="pot1RemoteValSlider"
                  min={0}
                  max={255} 
                  step={1}
                  value={[pot1RemoteValue]}
                  onValueChange={(value) => handlePotRemoteValueChange(1, value[0])}
                  disabled={!pot1RemoteSettingReg}
                  className="my-2"
                />
                <Button onClick={() => applyPotRemoteValue(1)} size="sm" className={`${neumorphicButtonClass} w-full`} disabled={!pot1RemoteSettingReg}>Apply Speed</Button>
              </div>
            )}
          </CardContent>
        </Card>

        <Card className="flex flex-col">
          <CardHeader>
            <CardTitle className="flex items-center"><Zap className="mr-2 h-5 w-5 text-orange-500" />Jam Sensitivity</CardTitle>
            <CardDescription>Adjust jam detection threshold.</CardDescription>
          </CardHeader>
          <CardContent className="flex-grow flex flex-col justify-between space-y-4">
            <div className="space-y-3">
                <p className="text-sm">Current Value: <span className="font-bold text-orange-600 dark:text-orange-400 text-lg">{pot2ValueReg?.value ?? 'N/A'}</span></p>
                <div className="flex items-center space-x-2">
                    <Label htmlFor="pot2Mode">Mode:</Label>
                    <Switch 
                    id="pot2Mode"
                    checked={pot2ModeReg?.value === E_MODE.REMOTE}
                    onCheckedChange={(checked) => handlePotModeChange(2, checked ? E_MODE.REMOTE : E_MODE.LOCAL)}
                    disabled={!pot2ModeReg}
                    />
                    <span className="font-medium">{pot2ModeReg ? modeToString(pot2ModeReg.value) : 'N/A'}</span>
                </div>
            </div>
            {pot2ModeReg?.value === E_MODE.REMOTE && (
              <div className="space-y-3 pt-2">
                <Label htmlFor="pot2RemoteValSlider" className="text-sm">Set Remote Sensitivity (0-255): <span className="font-bold text-orange-600 dark:text-orange-400">{pot2RemoteValue}</span></Label>
                <Slider
                  id="pot2RemoteValSlider"
                  min={0}
                  max={255} 
                  step={1}
                  value={[pot2RemoteValue]}
                  onValueChange={(value) => handlePotRemoteValueChange(2, value[0])}
                  disabled={!pot2RemoteSettingReg}
                  className="my-2"
                />
                <Button onClick={() => applyPotRemoteValue(2)} size="sm" className={`${neumorphicButtonClass} w-full`} disabled={!pot2RemoteSettingReg}>Apply Sensitivity</Button>
              </div>
            )}
          </CardContent>
        </Card>
      </div>
  );
}

export default PotentiometerControls; 