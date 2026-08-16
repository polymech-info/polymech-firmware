import React from 'react';
import { useModbus } from '@/contexts/ModbusContext';
import { Card, CardContent, CardHeader, CardTitle, CardDescription } from '@/components/ui/card';
import { Button } from '@/components/ui/button';
import { Label } from '@/components/ui/label';
import { Switch } from '@/components/ui/switch';
import { Separator } from '@/components/ui/separator';
import { toast } from 'sonner';
import { ArrowUp, ArrowDown, ArrowLeft, ArrowRight, CircleDot } from 'lucide-react';
import type { RegisterData } from "@polymech/client-ts/modbusService";

// Enums and helpers needed by this component (can be moved to shared utils)
const E_POSITION = {
    CENTER: 0, UP: 1, DOWN: 2, LEFT: 3, RIGHT: 4, UNKNOWN: 5
} as const;
type PositionKeys = keyof typeof E_POSITION;
const positionToString = (value: number): PositionKeys => {
    for (const key in E_POSITION) {
        if (E_POSITION[key as PositionKeys] === value) return key as PositionKeys;
    }
    return 'UNKNOWN';
};

const E_MODE = {
    LOCAL: 0, REMOTE: 1
} as const;
type ModeKeys = keyof typeof E_MODE;
const modeToString = (value: number): ModeKeys => {
    return value === E_MODE.REMOTE ? 'REMOTE' : 'LOCAL';
};

// Neumorphic styles (assuming these are defined globally or passed as props if preferred)
const neumorphicBase = "px-4 py-2 rounded-lg font-semibold transition-all duration-150 ease-in-out focus:outline-none focus:ring-2 focus:ring-offset-2 flex items-center justify-center";
const neumorphicLight = `bg-slate-100 text-slate-700 hover:bg-slate-200 shadow-[3px_3px_7px_#bec8e4,-3px_-3px_7px_#ffffff] active:shadow-[inset_3px_3px_7px_#bec8e4,inset_-3px_-3px_7px_#ffffff]`;
const neumorphicDark = `dark:bg-slate-800 dark:text-slate-300 dark:hover:bg-slate-700 dark:shadow-[3px_3px_7px_#2c3e50,-3px_-3px_7px_#4a6572] dark:active:shadow-[inset_3px_3px_7px_#2c3e50,inset_-3px_-3px_7px_#4a6572]`;
const neumorphicButtonClass = `${neumorphicBase} ${neumorphicLight} ${neumorphicDark}`;
const neumorphicActiveLight = `bg-orange-400/30 text-orange-700 shadow-[inset_3px_3px_7px_#c87600,inset_-3px_-3px_7px_#ffe8cc]`; 
const neumorphicActiveDark = `dark:bg-orange-600/30 dark:text-orange-300 dark:shadow-[inset_3px_3px_7px_#8a5300,inset_-3px_-3px_7px_#ffc966]`; 
const neumorphicButtonActiveClass = `${neumorphicBase} ${neumorphicActiveLight} ${neumorphicActiveDark}`;

interface JoystickControlsProps {
  joystickPositionReg: RegisterData | undefined;
  joystickModeReg: RegisterData | undefined; // Pass the whole register for reading value for Switch
  joystickOverrideReg: RegisterData | undefined; // Pass the whole register for reading value for Switch
  joystickModeRegAddr: number | undefined;
  joystickOverrideRegAddr: number | undefined;
  joystickPositionCmdRegAddr: number | undefined;
}

const JoystickControls: React.FC<JoystickControlsProps> = ({
  joystickPositionReg,
  joystickModeReg,
  joystickOverrideReg,
  joystickModeRegAddr,
  joystickOverrideRegAddr,
  joystickPositionCmdRegAddr
}) => {
  const { updateRegister } = useModbus();

  const handleJoystickModeChange = async (newMode: number) => {
    if (joystickModeRegAddr === undefined) {
      toast.error('Joystick mode register not found.');
      return;
    }
    try {
      await updateRegister(joystickModeRegAddr, newMode);
      toast.success(`Joystick mode updated to ${modeToString(newMode)}`);
    } catch (error) {
      toast.error('Failed to update Joystick mode.');
      console.error('Error updating Joystick mode:', error);
    }
  };

  const handleJoystickOverrideChange = async (overrideActive: boolean) => {
    if (joystickOverrideRegAddr === undefined) {
      toast.error('Joystick override register not found.');
      return;
    }
    try {
      await updateRegister(joystickOverrideRegAddr, overrideActive ? 1 : 0);
      toast.success(`Joystick override ${overrideActive ? 'activated' : 'deactivated'}`);
    } catch (error) {
      toast.error('Failed to update Joystick override.');
      console.error('Error updating Joystick override:', error);
    }
  };

  const handleJoystickDirectionButton = async (newPosition: number) => {
    if (joystickModeRegAddr === undefined || joystickPositionCmdRegAddr === undefined) {
      toast.error('Joystick mode or position command register not found.');
      return;
    }
    try {
      await updateRegister(joystickModeRegAddr, E_MODE.REMOTE);
      toast.info('Joystick mode set to REMOTE.');
      await updateRegister(joystickPositionCmdRegAddr, newPosition);
      toast.success(`Joystick position command: ${positionToString(newPosition)} sent.`);
    } catch (error) {
      toast.error('Failed to send Joystick command.');
      console.error('Error sending Joystick command:', error);
    }
  };

  return (
    <Card>
        <CardHeader>
            <CardTitle>Joystick</CardTitle>
            <CardDescription>Control joystick direction. Mode will be set to Remote.</CardDescription>
        </CardHeader>
        <CardContent className="space-y-4">
            <div className="grid grid-cols-3 gap-2 items-center justify-items-center max-w-xs mx-auto p-4 bg-slate-200 dark:bg-slate-700 rounded-xl shadow-inner">
                <div></div> 
                <Button 
                    className={`${joystickPositionReg?.value === E_POSITION.UP ? neumorphicButtonActiveClass : neumorphicButtonClass} w-16 h-16`}
                    onClick={() => handleJoystickDirectionButton(E_POSITION.UP)} 
                    disabled={!joystickPositionCmdRegAddr || !joystickModeRegAddr} 
                    aria-label="Up"
                >
                    <ArrowUp size={28}/>
                </Button>
                <div></div>
                <Button 
                    className={`${joystickPositionReg?.value === E_POSITION.LEFT ? neumorphicButtonActiveClass : neumorphicButtonClass} w-16 h-16`}
                    onClick={() => handleJoystickDirectionButton(E_POSITION.LEFT)} 
                    disabled={!joystickPositionCmdRegAddr || !joystickModeRegAddr} 
                    aria-label="Left"
                >
                    <ArrowLeft size={28}/>
                </Button>
                <Button 
                    className={`${joystickPositionReg?.value === E_POSITION.CENTER ? neumorphicButtonActiveClass : neumorphicButtonClass} w-16 h-16`}
                    onClick={() => handleJoystickDirectionButton(E_POSITION.CENTER)} 
                    disabled={!joystickPositionCmdRegAddr || !joystickModeRegAddr} 
                    aria-label="Center"
                >
                    <CircleDot size={28}/>
                </Button>
                <Button 
                    className={`${joystickPositionReg?.value === E_POSITION.RIGHT ? neumorphicButtonActiveClass : neumorphicButtonClass} w-16 h-16`}
                    onClick={() => handleJoystickDirectionButton(E_POSITION.RIGHT)} 
                    disabled={!joystickPositionCmdRegAddr || !joystickModeRegAddr} 
                    aria-label="Right">
                    <ArrowRight size={28}/>
                </Button>
                <div></div>
                <Button 
                    className={`${joystickPositionReg?.value === E_POSITION.DOWN ? neumorphicButtonActiveClass : neumorphicButtonClass} w-16 h-16`}
                    onClick={() => handleJoystickDirectionButton(E_POSITION.DOWN)} 
                    disabled={!joystickPositionCmdRegAddr || !joystickModeRegAddr} 
                    aria-label="Down">
                    <ArrowDown size={28}/>
                </Button>
                <div></div>
            </div>
            <div className="text-center mt-4">
                 <Label>Current Position: </Label>
                 <span className="font-semibold text-lg">{joystickPositionReg ? positionToString(joystickPositionReg.value) : 'N/A'}</span>
            </div>
            <Separator className="my-4"/>
            <div className="flex items-center justify-between">
                <Label htmlFor="joystickModeControl">Mode:</Label>
                <div className="flex items-center space-x-2">
                    <Switch 
                      id="joystickModeControl" 
                      checked={joystickModeReg?.value === E_MODE.REMOTE} 
                      onCheckedChange={(checked) => handleJoystickModeChange(checked ? E_MODE.REMOTE : E_MODE.LOCAL)}
                      disabled={joystickModeRegAddr === undefined}
                    />
                    <span>{joystickModeReg ? modeToString(joystickModeReg.value) : 'N/A'}</span>
                </div>
            </div>
            <Separator className="my-4"/>
            <div className="flex items-center justify-between">
                <Label htmlFor="joystickOverrideControl">Override Active:</Label>
                 <div className="flex items-center space-x-2">
                    <Switch 
                      id="joystickOverrideControl" 
                      checked={joystickOverrideReg?.value === 1}
                      onCheckedChange={handleJoystickOverrideChange}
                      disabled={joystickOverrideRegAddr === undefined}
                    />
                    <span>{joystickOverrideReg ? (joystickOverrideReg.value === 1 ? 'Yes' : 'No') : 'N/A'}</span>
                </div>
            </div>
        </CardContent>
      </Card>
  );
}

export default JoystickControls; 