import React, { useState, useEffect, useRef } from 'react';
import { useModbus } from '@/contexts/ModbusContext';
import modbusApiService, { PlungerSettingsResponse, PlungerSettingsUpdatePayload } from '@polymech/client-ts/modbusApiService';
import { Card, CardContent, CardHeader, CardTitle, CardDescription } from '@/components/ui/card';
import { Button } from '@/components/ui/button';
import { Label } from '@/components/ui/label';
import { Input } from '@/components/ui/input';
import { Separator } from '@/components/ui/separator';
import { toast } from 'sonner';
import { Home, ArrowDownToDot, Info, ShieldQuestion, Construction, StopCircle, ExternalLink, Timer, Settings2, Zap, TrendingUp, Activity, Play } from 'lucide-react';
import type { RegisterData } from "@polymech/client-ts/modbusService";
import LEDBars from './LEDBars';
import { Slider } from '@/components/ui/slider';
import { T } from '../i18n';

// Enums and helpers specific to Plunger or shared
const PLUNGER_COMMANDS = { 
    NONE: 0, HOME: 1, PLUNGE: 2, STOP: 3, INFO: 4, FILL: 5, REPLAY: 6
} as const;

const PLUNGER_STATES = {
    IDLE: 0, HOMING_MANUAL: 1, HOMING_AUTO: 2, PLUNGING_MANUAL: 3, PLUNGING_AUTO: 4,
    STOPPING: 5, JAMMED: 6, RESETTING_JAM: 7, RECORD: 8, REPLAY: 9, FILLING: 10, POST_FLOW: 11,
} as const;

const plungerStateToString = (value: number): string => {
    for (const key of Object.keys(PLUNGER_STATES)) {
        if (PLUNGER_STATES[key as keyof typeof PLUNGER_STATES] === value) {
            return key.replace(/_/g, ' '); 
        }
    }
    return `UNKNOWN_STATE (${value})`; 
};

// Neumorphic styles 
const neumorphicBase = "rounded-lg font-semibold transition-all duration-150 ease-in-out focus:outline-none focus:ring-2 focus:ring-offset-2 flex items-center justify-center";
const neumorphicLight = `bg-slate-100 text-slate-700 hover:bg-slate-200 shadow-[3px_3px_7px_#bec8e4,-3px_-3px_7px_#ffffff] active:shadow-[inset_3px_3px_7px_#bec8e4,inset_-3px_-3px_7px_#ffffff]`;
const neumorphicDark = `dark:bg-slate-800 dark:text-slate-300 dark:hover:bg-slate-700 dark:shadow-[3px_3px_7px_#2c3e50,-3px_-3px_7px_#4a6572] dark:active:shadow-[inset_3px_3px_7px_#2c3e50,inset_-3px_-3px_7px_#4a6572]`;
const neumorphicButtonClass = `${neumorphicBase} ${neumorphicLight} ${neumorphicDark} py-2 px-3 text-sm`; // Base smaller size

const neumorphicActiveLight = `bg-orange-400/30 text-orange-700 shadow-[inset_3px_3px_7px_#c87600,inset_-3px_-3px_7px_#ffe8cc]`; 
const neumorphicActiveDark = `dark:bg-orange-600/30 dark:text-orange-300 dark:shadow-[inset_3px_3px_7px_#8a5300,inset_-3px_-3px_7px_#ffc966]`; 
// Active class will compose with base size from its non-active counterpart
const neumorphicButtonActiveClassBase = `${neumorphicBase} ${neumorphicActiveLight} ${neumorphicActiveDark}`;

const neumorphicDestructiveBase = "rounded-lg font-semibold transition-all duration-150 ease-in-out focus:outline-none focus:ring-2 focus:ring-offset-2 flex items-center justify-center";
const neumorphicDestructiveLight = `bg-red-500 text-white hover:bg-red-600 shadow-[3px_3px_7px_#c82333,-3px_-3px_7px_#ff7878] active:shadow-[inset_3px_3px_7px_#c82333,inset_-3px_-3px_7px_#ff7878]`;
const neumorphicDestructiveDark = `dark:bg-red-600 dark:text-white dark:hover:bg-red-700 dark:shadow-[3px_3px_7px_#a71d2a,-3px_-3px_7px_#e85a5a] dark:active:shadow-[inset_3px_3px_7px_#a71d2a,inset_-3px_-3px_7px_#e85a5a]`;
const neumorphicDestructiveButtonClass = `${neumorphicDestructiveBase} ${neumorphicDestructiveLight} ${neumorphicDestructiveDark} py-2 px-3 text-sm`; // Base smaller size

// Larger styles for md+ screens, applied on top of base small versions
const largeNeumorphicButtonClass = `${neumorphicButtonClass} md:py-3 md:px-6 md:text-lg`;
const largeNeumorphicDestructiveButtonClass = `${neumorphicDestructiveButtonClass} md:py-3 md:px-6 md:text-lg`;
const activeLargeNeumorphicButtonClass = `${neumorphicButtonActiveClassBase} md:py-3 md:px-6 md:text-lg`; // For active large buttons

interface PlungerControlCardProps {
  plungerStateReg: RegisterData | undefined;
  plungerCommandRegAddr: number | undefined;
  isDashboardView?: boolean;
  onPopout?: () => void;
  
  // VFD Current for LED Bar
  vfdCurrentDisplayValue: number | null;
  VFD_CURRENT_DISPLAY_RAW: boolean;
  peakVfdCurrent: number | null;
  // MAX_VFD_CURRENT_FOR_BAR: number; // This will be a const inside the component

  // Plunger settings for Jam Threshold and Max Op Time
  plungerSettings: PlungerSettingsResponse | null;
  plungerSettingsLoading: boolean; 
  vfdRunFreqReg: RegisterData | undefined;
  VFD_RUN_FREQ_DISPLAY_SCALE: number;
}

const MAX_VFD_CURRENT_FOR_BAR = 1500;
const MAX_JAM_THRESHOLD_CONFIG = 1500; 
const MAX_REPLAY_DURATION_MS = 30000; // Example: 30 seconds max for replay slider
const REPLAY_DURATION_STEP_MS = 100;  // Example: 100ms step

const PlungerControlCard: React.FC<PlungerControlCardProps> = ({
  plungerStateReg,
  plungerCommandRegAddr,
  isDashboardView,
  onPopout,
  vfdCurrentDisplayValue,
  VFD_CURRENT_DISPLAY_RAW,
  peakVfdCurrent,
  plungerSettings,
  plungerSettingsLoading,
  vfdRunFreqReg,
  VFD_RUN_FREQ_DISPLAY_SCALE
}) => {
  const { updateRegister } = useModbus();
  const [localJamThreshold, setLocalJamThreshold] = useState<number>(0);
  const [isSavingJamThreshold, setIsSavingJamThreshold] = useState<boolean>(false);
  const [localReplayDurationMs, setLocalReplayDurationMs] = useState<number>(0);
  const [isSavingReplayDuration, setIsSavingReplayDuration] = useState<boolean>(false);

  const maxJamFromSettings = plungerSettings?.currentJamThresholdMa ? Math.max(MAX_JAM_THRESHOLD_CONFIG, plungerSettings.currentJamThresholdMa * 1.2) : MAX_JAM_THRESHOLD_CONFIG;

  useEffect(() => {
    if (plungerSettings) {
      setLocalJamThreshold(plungerSettings.currentJamThresholdMa);
      setLocalReplayDurationMs(plungerSettings.replayDurationMs);
    }
  }, [plungerSettings]);

  const handlePlungerCommand = async (command: number) => {
    if (plungerCommandRegAddr === undefined) { toast.error(<T>Plunger Command Address not found.</T>); return; }
    try {
      await updateRegister(plungerCommandRegAddr, command);
      const commandName = Object.keys(PLUNGER_COMMANDS).find(key => PLUNGER_COMMANDS[key as keyof typeof PLUNGER_COMMANDS] === command) || `Cmd ${command}`;
      toast.success(<T>Plunger command: {commandName} sent.</T>);
    } catch (error) {
      toast.error(<T>Failed to send Plunger command.</T>);
      console.error('Error sending Plunger command:', error);
    }
  };

  const handleJamThresholdInputChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    let val = parseInt(event.target.value, 10);
    if (isNaN(val)) val = 0;
    val = Math.max(0, Math.min(maxJamFromSettings, val)); 
    setLocalJamThreshold(val);
  };

  const handleApplyJamThreshold = async () => {
    if (localJamThreshold === undefined || isNaN(localJamThreshold)) {
        toast.error(<T>Invalid Jam Threshold value.</T>);
        return;
    }
    setIsSavingJamThreshold(true);
    try {
        const payload: PlungerSettingsUpdatePayload = { currentJamThresholdMa: localJamThreshold };
        await modbusApiService.setPlungerSettings(payload);
        toast.success(<T>Jam Threshold updated successfully!</T>);
    } catch (err) {
        toast.error(<T>Failed to update Jam Threshold: {err instanceof Error ? err.message : String(err)}</T>);
    } finally {
        setIsSavingJamThreshold(false);
    }
  };

  const handleReplayDurationSliderChange = (value: number[]) => {
    setLocalReplayDurationMs(value[0]);
  };

  const handleReplayDurationInputChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    let val = parseInt(event.target.value, 10);
    if (isNaN(val)) val = 0;
    val = Math.max(500, Math.min(MAX_REPLAY_DURATION_MS, val)); // Clamp to slider's min/max
    setLocalReplayDurationMs(val);
  };

  const handleApplyReplayDuration = async () => {
    setIsSavingReplayDuration(true);
    try {
      const payload: PlungerSettingsUpdatePayload = { replayDurationMs: localReplayDurationMs };
      await modbusApiService.setPlungerSettings(payload);
      toast.success(<T>Replay Duration updated successfully!</T>);
    } catch (err) {
      toast.error(<T>Failed to update Replay Duration: {err instanceof Error ? err.message : String(err)}</T>);
    } finally {
      setIsSavingReplayDuration(false);
    }
  };
  
  const currentPlungerStateValue = plungerStateReg?.value;
  const isHoming = currentPlungerStateValue === PLUNGER_STATES.HOMING_MANUAL || currentPlungerStateValue === PLUNGER_STATES.HOMING_AUTO;
  const isPlunging = currentPlungerStateValue === PLUNGER_STATES.PLUNGING_MANUAL || 
                     currentPlungerStateValue === PLUNGER_STATES.PLUNGING_AUTO || 
                     currentPlungerStateValue === PLUNGER_STATES.RECORD || 
                     currentPlungerStateValue === PLUNGER_STATES.REPLAY;
  const isFilling = currentPlungerStateValue === PLUNGER_STATES.FILLING || currentPlungerStateValue === PLUNGER_STATES.POST_FLOW;
  const isStoppingOrJammed = currentPlungerStateValue === PLUNGER_STATES.STOPPING || currentPlungerStateValue === PLUNGER_STATES.JAMMED;

  const vfdCurrentPercent = vfdCurrentDisplayValue !== null ? Math.min(100, (vfdCurrentDisplayValue / MAX_VFD_CURRENT_FOR_BAR) * 100) : 0;
  const jamThresholdPercent = plungerSettings ? (localJamThreshold / maxJamFromSettings) * 100 : 0;
  const maxOpTimeSeconds = plungerSettings ? (plungerSettings.defaultMaxOperationDurationMs / 1000).toFixed(1) : 'N/A';

  return (
    <Card>
        <CardHeader className="flex flex-row items-center justify-between p-3 md:p-4">
            {isDashboardView && onPopout && (
                <Button variant="outline" size="icon" onClick={onPopout} className="ml-auto h-7 w-7 md:h-8 md:w-8">
                    <ExternalLink className="h-3 w-3 md:h-4 md:w-4" />
                    <span className="sr-only"><T>Popout</T></span>
                </Button>
            )}
        </CardHeader>
        <CardContent className="space-y-4 md:space-y-6 p-2 md:p-4">
            <div className="flex justify-center w-full">
                <div className="text-center p-2 md:p-3 rounded-lg bg-slate-200 dark:bg-slate-700 shadow-md w-auto inline-block min-w-[320px] md:min-w-[200px]">
                    <Label className="text-xs md:text-sm text-slate-600 dark:text-slate-400"><T>Current Plunger State</T></Label>
                    <p className="font-semibold text-xl md:text-2xl text-blue-600 dark:text-blue-400 capitalize">
                        {plungerStateReg ? plungerStateToString(plungerStateReg.value) : <T>N/A</T>}
                    </p>
                </div>
            </div>
            
            <div className="flex flex-col md:flex-row items-center md:items-stretch justify-around gap-3 md:gap-4 pt- md:pt-4">
                {/* Column 1: VFD Current LED Bar */}
                <div className="flex flex-col items-center space-y-1 md:space-y-2 p-2 md:p-3 border rounded-md bg-slate-100 dark:bg-slate-800/50 w-full max-w-xs mx-auto md:w-[80px] lg:w-[100px] flex-shrink-0 h-[200px] md:h-[280px]">
                    <LEDBars 
                        orientation="vertical" 
                        value={vfdCurrentPercent} 
                        label="VFD Current"
                        unit={VFD_CURRENT_DISPLAY_RAW ? " (raw)" : " A"}
                        showValueText={true} 
                        barHeightClass="h-full" 
                        barWidthClass="w-6 md:w-8"
                        segments={8} 
                    />
                     {peakVfdCurrent !== null && (
                        <div className="text-xs text-center text-slate-500 dark:text-slate-400 mt-auto pt-1">
                            <TrendingUp className="inline h-3 w-3 mr-1" /><T>Peak</T>: {peakVfdCurrent.toFixed(VFD_CURRENT_DISPLAY_RAW ? 0 : 1)}
                        </div>
                    )}
                </div>

                {/* Column 2: Plunger Command Buttons & Replay Duration Controls */}
                <div className="flex flex-col items-center gap-4 w-full md:w-auto md:flex-grow">
                    <Button className={`${isHoming ? activeLargeNeumorphicButtonClass : largeNeumorphicButtonClass} w-3/4 md:w-full max-w-xs`} onClick={() => handlePlungerCommand(PLUNGER_COMMANDS.HOME)}>
                        <Home className="mr-1 md:mr-2 h-4 w-4 md:h-5 md:w-5"/><T>Home</T>
                    </Button>
                    <div className="grid grid-cols-3 gap-2 md:gap-3 w-full max-w-xs md:max-w-sm">
                        <Button className={`${isPlunging ? activeLargeNeumorphicButtonClass : largeNeumorphicButtonClass} w-full`} onClick={() => handlePlungerCommand(PLUNGER_COMMANDS.PLUNGE)}>
                            <ArrowDownToDot className="mr-1 md:mr-2 h-4 w-4 md:h-5 md:w-5"/><T>Plunge</T>
                        </Button>
                        <Button className={`${isStoppingOrJammed ? (neumorphicButtonActiveClassBase + ' md:py-3 md:px-6 md:text-lg bg-red-500/30 dark:bg-red-700/30') : largeNeumorphicDestructiveButtonClass} w-full`} onClick={() => handlePlungerCommand(PLUNGER_COMMANDS.STOP)}>
                            <StopCircle className="mr-1 md:mr-2 h-4 w-4 md:h-5 md:w-5"/><T>Stop</T>
                        </Button>
                        <Button className={`${isFilling ? activeLargeNeumorphicButtonClass : largeNeumorphicButtonClass} w-full`} onClick={() => handlePlungerCommand(PLUNGER_COMMANDS.FILL)}>
                            <Construction className="mr-1 md:mr-2 h-4 w-4 md:h-5 md:w-5"/><T>Fill</T>
                        </Button>
                    </div>
                    <Button className={`${neumorphicButtonClass} w-3/4 md:w-full max-w-xs md:mt-2`} variant="outline" onClick={() => handlePlungerCommand(PLUNGER_COMMANDS.INFO)}>
                        <Info className="mr-1 md:mr-2 h-3 w-3 md:h-4 md:w-4"/><T>Info</T>
                    </Button>
                    <div className="text-center mt-1 p-1 md:p-2 bg-slate-100 dark:bg-slate-800 rounded-md w-3/4 md:w-full max-w-xs">
                        <Label className="text-xs text-muted-foreground"><T>VFD Speed</T></Label>
                        <p className="font-semibold text-base md:text-lg text-blue-500 dark:text-blue-300">
                            {vfdRunFreqReg ? (vfdRunFreqReg.value / VFD_RUN_FREQ_DISPLAY_SCALE).toFixed(2) : 'N/A'} Hz
                        </p>
                    </div>
                    {plungerSettings && (
                        <div className="w-3/4 md:w-full max-w-xs pt-2 md:pt-3 mt-1 md:mt-2 border-t border-border">
                            <div className="flex justify-between items-center mb-1">
                                <Label htmlFor="replayDurationSlider" className="text-xs md:text-sm font-medium"><T>Replay Duration</T></Label>
                                <span className="text-xs md:text-sm font-bold text-purple-600 dark:text-purple-400">{(localReplayDurationMs / 1000).toFixed(1)} s</span>
                            </div>
                            <div className="flex items-center space-x-1 md:space-x-2">
                                <Slider
                                    id="replayDurationSlider"
                                    min={500} 
                                    max={MAX_REPLAY_DURATION_MS} 
                                    step={REPLAY_DURATION_STEP_MS} 
                                    value={[localReplayDurationMs]}
                                    onValueChange={handleReplayDurationSliderChange}
                                    disabled={plungerSettingsLoading || isSavingReplayDuration}
                                    className="flex-grow"
                                />
                                <Input 
                                    type="number"
                                    value={localReplayDurationMs}
                                    onChange={handleReplayDurationInputChange}
                                    min={500}
                                    max={MAX_REPLAY_DURATION_MS}
                                    step={REPLAY_DURATION_STEP_MS}
                                    className="h-7 text-xs w-20 p-1 text-center ml-2"
                                    disabled={plungerSettingsLoading || isSavingReplayDuration}
                                />
                                <Button 
                                    size="sm"
                                    variant="outline"
                                    onClick={handleApplyReplayDuration}
                                    disabled={isSavingReplayDuration || plungerSettingsLoading || localReplayDurationMs === plungerSettings.replayDurationMs}
                                    className={`${neumorphicButtonClass} p-1 h-7 w-7`}
                                    title="Apply Replay Duration Setting"
                                >
                                    {isSavingReplayDuration ? <Spinner /> : <Settings2 className="h-3 w-3"/>}
                                </Button>
                            </div>
                            <Button 
                                className={`${neumorphicButtonClass} w-full mt-3`} 
                                onClick={() => handlePlungerCommand(PLUNGER_COMMANDS.REPLAY)} 
                                disabled={plungerCommandRegAddr === undefined || isPlunging || isHoming || isFilling }
                                title="Execute Replay"
                            >
                                <Play className="mr-2 h-4 w-4" /> <T>Replay Now</T>
                            </Button>
                        </div>
                    )}
                </div>

                {/* Column 3: Jam Threshold LED Bar and Controls */}
                <div className="flex flex-col items-center space-y-1 md:space-y-2 p-2 md:p-3 border rounded-md bg-slate-100 dark:bg-slate-800/50 w-full max-w-xs mx-auto md:w-[80px] lg:w-[100px] flex-shrink-0 h-[200px] md:h-[280px]">
                    <LEDBars 
                        orientation="vertical" 
                        value={jamThresholdPercent} 
                        label="Jam Threshold Set"
                        unit="mA" 
                        showValueText={true}
                        barHeightClass="h-full" 
                        barWidthClass="w-6 md:w-8"
                        segments={8}
                    />
                    <div className="space-y-1 md:space-y-2 pt-1 w-full text-center mt-auto flex flex-col items-center">
                        <Label htmlFor="jamThresholdInputCtrl" className="text-xs block"><T>Adjust (mA):</T></Label>
                        <div className="flex items-center justify-center space-x-1">
                             <Input 
                                type="number"
                                id="jamThresholdInputCtrl"
                                value={localJamThreshold}
                                onChange={handleJamThresholdInputChange}
                                className="h-6 md:h-7 text-xs w-14 md:w-16 p-1 text-center"
                                disabled={plungerSettingsLoading || isSavingJamThreshold}
                                min={0} max={maxJamFromSettings}
                                step={10}
                            />
                            <Button 
                                size="sm" 
                                variant="outline"
                                onClick={handleApplyJamThreshold} 
                                disabled={isSavingJamThreshold || plungerSettingsLoading || localJamThreshold === plungerSettings?.currentJamThresholdMa}
                                className={`${neumorphicButtonClass} p-0.5 md:p-1 h-6 w-6 md:h-7 md:w-7`}
                                title="Apply Jam Threshold"
                            >
                                <Settings2 className="h-2.5 w-2.5 md:h-3 md:w-3"/>
                            </Button>
                        </div>
                    </div>
                </div>
            </div>
            
            <div className="flex items-center justify-center space-x-2 pt-2 md:pt-4 text-xs md:text-sm w-full">
                <Timer className="h-3 w-3 md:h-4 md:w-4 text-slate-500" />
                <Label className="font-medium"><T>Max Plunger Op. Time:</T></Label>
                <span className="font-bold text-slate-700 dark:text-slate-300">
                    {!plungerSettingsLoading ? `${maxOpTimeSeconds} s` : <T>Loading...</T>}
                </span>
            </div>
        </CardContent>
      </Card>
  );
}

// Spinner component for loading state on small button
const Spinner = () => (
  <svg className="animate-spin h-3 w-3 md:h-4 md:w-4 text-current" xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24">
    <circle className="opacity-25" cx="12" cy="12" r="10" stroke="currentColor" strokeWidth="4"></circle>
    <path className="opacity-75" fill="currentColor" d="M4 12a8 8 0 018-8V0C5.373 0 0 5.373 0 12h4zm2 5.291A7.962 7.962 0 014 12H0c0 3.042 1.135 5.824 3 7.938l3-2.647z"></path>
  </svg>
);

export default PlungerControlCard; 