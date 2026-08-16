import { useMemo, useState, useCallback, useRef, useEffect } from 'react';
import { Button } from '@/components/ui/button';
import { useToast } from "@/components/ui/use-toast";
import { Play, Pause, StopCircle, Edit, Plus, Minus, HelpCircle } from 'lucide-react';
import { cn } from '@/lib/utils';
import { translate } from '../i18n';
import { useModbus } from '@/contexts/ModbusContext';
import { Profile, PlotStatus, TemperatureProfileCommand } from '@/types';
import { PROFILE_REGISTER_NAMES, PV_REGISTER_NAME_SUFFIX, SP_REGISTER_NAME_SUFFIX } from '@/constants';
import { getSlaveIdFromGroup, getControllerStatus } from '../lib/controllerUtils';
import { Progress } from './ui/progress';
import { transformServiceProfileToUI } from '../lib/profile-transformers';
import { useNavigate } from 'react-router-dom';
import modbusApiService from '@polymech/client-ts/modbusApiService';
import BezierEditor from '@/components/profiles/bezier/BezierEditor';
import DisplayMessagesPanel from './DisplayMessagesPanel';

const STATUS_HIGH_REGISTER_NAME = "Status High";
const STATUS_LOW_REGISTER_NAME = "Status Low";

const formatTime = (secs: number): string => {
    if (isNaN(secs) || secs < 0) return '00:00:00';
    const totalSeconds = Math.floor(secs);
    const hours = Math.floor(totalSeconds / 3600);
    const minutes = Math.floor((totalSeconds % 3600) / 60);
    const seconds = totalSeconds % 60;

    return [hours, minutes, seconds]
        .map(v => v.toString().padStart(2, '0'))
        .join(':');
};

const ProfilePlayback = () => {
    const navigate = useNavigate();
    const {
        isConnected,
        updateRegister,
        profiles: contextProfiles,
        registers: allModbusRegisters,
        coils: allModbusCoils,
        settings,
        pressureProfiles,
    } = useModbus();
    const { toast } = useToast();

    // Optimistic state management
    const [optimisticStatus, setOptimisticStatus] = useState<{ profileName: string; status: PlotStatus; timestamp: number } | null>(null);
    const optimisticTimeoutRef = useRef<NodeJS.Timeout | null>(null);
    const [showDescription, setShowDescription] = useState(false);

    // Scrubber state
    const [sliderValue, setSliderValue] = useState(0);
    const [isDragging, setIsDragging] = useState(false);

    // Cleanup timeout on unmount
    useEffect(() => {
        return () => {
            if (optimisticTimeoutRef.current) {
                clearTimeout(optimisticTimeoutRef.current);
            }
        };
    }, []);

    const liveUiProfiles = useMemo((): Profile[] => {
        if (!contextProfiles || !allModbusRegisters || !settings || !allModbusCoils) return [];
        return contextProfiles.map(pService =>
            transformServiceProfileToUI(pService, allModbusRegisters, allModbusCoils, settings.partitions)
        );
    }, [contextProfiles, allModbusRegisters, allModbusCoils, settings]);

    const controllerNameToSlaveIdMap = useMemo(() => {
        const map = new Map<string, number>();
        if (!settings) return map;
        settings.partitions.forEach(partition => {
            partition.controllers?.forEach(controller => {
                if (controller.name) {
                    map.set(controller.name, controller.slaveid);
                }
            });
        });
        return map;
    }, [settings]);


    const primaryProfile = useMemo(() => {
        if (!liveUiProfiles) return null;

        const activeProfile = liveUiProfiles.find(
            p => p.status === PlotStatus.RUNNING || p.status === PlotStatus.PAUSED || p.status === PlotStatus.INITIALIZING
        );
        if (activeProfile) {
            // Apply optimistic status if it matches this profile and is recent
            if (optimisticStatus &&
                optimisticStatus.profileName === activeProfile.name &&
                Date.now() - optimisticStatus.timestamp < 5000) {
                return { ...activeProfile, status: optimisticStatus.status };
            }
            return activeProfile;
        }
        const firstEnabledProfile = liveUiProfiles.find(p => p.enabled);
        if (firstEnabledProfile) {
            // Apply optimistic status if it matches this profile and is recent
            if (optimisticStatus &&
                optimisticStatus.profileName === firstEnabledProfile.name &&
                Date.now() - optimisticStatus.timestamp < 5000) {
                return { ...firstEnabledProfile, status: optimisticStatus.status };
            }
        }
        return firstEnabledProfile;
    }, [liveUiProfiles, optimisticStatus]);

    const associatedPressureProfile = useMemo(() => {
        if (!primaryProfile || primaryProfile.pressureProfile === undefined || !pressureProfiles) return undefined;
        return pressureProfiles.find(p => p.slot === primaryProfile.pressureProfile);
    }, [primaryProfile, pressureProfiles]);

    const warmupProgress = useMemo(() => {
        if (primaryProfile?.status !== PlotStatus.INITIALIZING) {
            return null;
        }

        const controllerNames = primaryProfile.associatedControllerNames;
        if (!controllerNames || controllerNames.length === 0) {
            return 100; // Instantly ready if no controllers associated
        }

        const progresses: number[] = [];
        for (const name of controllerNames) {
            const slaveId = controllerNameToSlaveIdMap.get(name);
            if (slaveId === undefined) continue;

            const pvRegister = allModbusRegisters.find(
                reg => getSlaveIdFromGroup(reg.group) === slaveId && reg.name.endsWith(PV_REGISTER_NAME_SUFFIX)
            );
            const spRegister = allModbusRegisters.find(
                reg => getSlaveIdFromGroup(reg.group) === slaveId && reg.name.endsWith(SP_REGISTER_NAME_SUFFIX)
            );

            if (pvRegister && spRegister && typeof pvRegister.value === 'number' && typeof spRegister.value === 'number') {
                const sp = spRegister.value;
                const pv = pvRegister.value;

                if (sp > 0) {
                    const progress = Math.min(100, (pv / sp) * 100);
                    progresses.push(progress);
                } else {
                    progresses.push(pv === 0 ? 100 : 0);
                }
            }
        }

        if (progresses.length === 0) {
            return 0; // No controllers with valid registers found
        }

        const totalProgress = progresses.reduce((acc, p) => acc + p, 0);
        return totalProgress / progresses.length;

    }, [primaryProfile, allModbusRegisters, controllerNameToSlaveIdMap]);

    const warmupAverageTemp = useMemo(() => {
        if (primaryProfile?.status !== PlotStatus.INITIALIZING) {
            return null;
        }

        const controllerNames = primaryProfile.associatedControllerNames;
        if (!controllerNames || controllerNames.length === 0) {
            return null;
        }

        const temperatures: number[] = [];
        for (const name of controllerNames) {
            const slaveId = controllerNameToSlaveIdMap.get(name);
            if (slaveId === undefined) continue;

            const pvRegister = allModbusRegisters.find(
                reg => getSlaveIdFromGroup(reg.group) === slaveId && reg.name.endsWith(PV_REGISTER_NAME_SUFFIX)
            );

            if (pvRegister && typeof pvRegister.value === 'number') {
                temperatures.push(pvRegister.value);
            }
        }

        if (temperatures.length === 0) {
            return null;
        }

        const avgTemp = temperatures.reduce((acc, temp) => acc + temp, 0) / temperatures.length;
        return avgTemp;

    }, [primaryProfile, allModbusRegisters, controllerNameToSlaveIdMap]);

    const warmupTargetTemp = useMemo(() => {
        if (primaryProfile?.status !== PlotStatus.INITIALIZING) {
            return null;
        }

        const controllerNames = primaryProfile.associatedControllerNames;
        if (!controllerNames || controllerNames.length === 0) {
            return null;
        }

        const targetTemperatures: number[] = [];
        for (const name of controllerNames) {
            const slaveId = controllerNameToSlaveIdMap.get(name);
            if (slaveId === undefined) continue;

            const spRegister = allModbusRegisters.find(
                reg => getSlaveIdFromGroup(reg.group) === slaveId && reg.name.endsWith(SP_REGISTER_NAME_SUFFIX)
            );

            if (spRegister && typeof spRegister.value === 'number') {
                targetTemperatures.push(spRegister.value);
            }
        }

        if (targetTemperatures.length === 0) {
            return null;
        }

        const avgTargetTemp = targetTemperatures.reduce((acc, temp) => acc + temp, 0) / targetTemperatures.length;
        return avgTargetTemp;

    }, [primaryProfile, allModbusRegisters, controllerNameToSlaveIdMap]);

    const controllerDetails = useMemo(() => {
        if (!primaryProfile || (
            primaryProfile.status !== PlotStatus.INITIALIZING &&
            primaryProfile.status !== PlotStatus.RUNNING &&
            primaryProfile.status !== PlotStatus.PAUSED
        )) {
            return [];
        }

        const controllerNames = primaryProfile.associatedControllerNames;
        if (!controllerNames || controllerNames.length === 0) {
            return [];
        }

        const details = [];
        for (const name of controllerNames) {
            const slaveId = controllerNameToSlaveIdMap.get(name);
            if (slaveId === undefined) continue;

            const pvRegister = allModbusRegisters.find(
                reg => getSlaveIdFromGroup(reg.group) === slaveId && reg.name.endsWith(PV_REGISTER_NAME_SUFFIX)
            );
            const spRegister = allModbusRegisters.find(
                reg => getSlaveIdFromGroup(reg.group) === slaveId && reg.name.endsWith(SP_REGISTER_NAME_SUFFIX)
            );

            const statusHighRegister = allModbusRegisters.find(
                reg => getSlaveIdFromGroup(reg.group) === slaveId && reg.name === STATUS_HIGH_REGISTER_NAME
            );
            const statusLowRegister = allModbusRegisters.find(
                reg => getSlaveIdFromGroup(reg.group) === slaveId && reg.name === STATUS_LOW_REGISTER_NAME
            );

            const controllerStatus = getControllerStatus(statusHighRegister, statusLowRegister);

            if (pvRegister || spRegister) {
                details.push({
                    name,
                    pv: typeof pvRegister?.value === 'number' ? pvRegister.value : null,
                    sp: typeof spRegister?.value === 'number' ? spRegister.value : null,
                    isHeating: controllerStatus.isHeating
                });
            }
        }

        return details;
    }, [primaryProfile, allModbusRegisters, controllerNameToSlaveIdMap]);

    // Helper function to set optimistic status
    const setOptimisticProfileStatus = useCallback((profileName: string, status: PlotStatus) => {
        setOptimisticStatus({ profileName, status, timestamp: Date.now() });

        // Clear any existing timeout
        if (optimisticTimeoutRef.current) {
            clearTimeout(optimisticTimeoutRef.current);
        }

        // Auto-clear optimistic status after 5 seconds
        optimisticTimeoutRef.current = setTimeout(() => {
            setOptimisticStatus(null);
        }, 5000);
    }, []);

    // Helper function to clear optimistic status
    const clearOptimisticStatus = useCallback(() => {
        setOptimisticStatus(null);
        if (optimisticTimeoutRef.current) {
            clearTimeout(optimisticTimeoutRef.current);
            optimisticTimeoutRef.current = null;
        }
    }, []);

    // Helper function to get expected status after command
    const getExpectedStatusAfterCommand = useCallback((currentStatus: PlotStatus, command: TemperatureProfileCommand): PlotStatus => {
        switch (command) {
            case TemperatureProfileCommand.START:
                return PlotStatus.INITIALIZING;
            case TemperatureProfileCommand.PAUSE:
                return PlotStatus.PAUSED;
            case TemperatureProfileCommand.RESUME:
                return PlotStatus.RUNNING;
            case TemperatureProfileCommand.STOP:
                return PlotStatus.STOPPED;
            default:
                return currentStatus;
        }
    }, []);

    const handlePrimaryProfileCommand = async (command: TemperatureProfileCommand) => {
        if (!primaryProfile || !primaryProfile.name) {
            toast({ title: "No active profile to control.", variant: "destructive" });
            return;
        }
        if (!isConnected) {
            toast({ title: "Not connected.", variant: "destructive" });
            return;
        }

        const commandRegisterEntry = allModbusRegisters.find(
            reg => reg.group === primaryProfile.name && reg.name.includes(PROFILE_REGISTER_NAMES.COMMAND)
        );

        if (!commandRegisterEntry) {
            toast({
                title: "Command Error",
                description: `Command register for profile '${primaryProfile.name}' not found.`,
                variant: "destructive"
            });
            return;
        }

        // Apply optimistic update immediately
        const expectedStatus = getExpectedStatusAfterCommand(primaryProfile.status, command);
        setOptimisticProfileStatus(primaryProfile.name, expectedStatus);

        try {
            await updateRegister(commandRegisterEntry.address, command);
            toast({
                title: "Profile Command Sent",
                description: `${TemperatureProfileCommand[command]} sent to profile '${primaryProfile.name}'.`
            });

            // Clear optimistic status after successful command - let backend state take over
            setTimeout(() => {
                clearOptimisticStatus();
            }, 1000);
        } catch (error) {
            // Revert optimistic update on failure
            clearOptimisticStatus();
            toast({
                title: "Command Failed",
                description: `Failed to send command: ${error instanceof Error ? error.message : String(error)}`,
                variant: "destructive"
            });
        }
    };

    // Optimistic overrides state
    const [optimisticOverrides, setOptimisticOverrides] = useState<{ [key: string]: { sp: { targetRegister: number; offset: number }[] } }>({});

    // Clear optimistic override when profile updates match
    useEffect(() => {
        if (!primaryProfile || !optimisticOverrides[primaryProfile.name]) return;

        const optimisticSp = optimisticOverrides[primaryProfile.name].sp;
        const serverSp = primaryProfile.overrides?.sp || [];

        // Check if server has caught up to all optimistic overrides
        const allCaughtUp = optimisticSp.every(opt =>
            serverSp.some(srv => srv.targetRegister === opt.targetRegister && srv.offset === opt.offset)
        );

        if (allCaughtUp) {
            setOptimisticOverrides(prev => {
                const next = { ...prev };
                delete next[primaryProfile.name];
                return next;
            });
        }
    }, [primaryProfile, optimisticOverrides]);

    // ... existing timeouts cleanup ...

    const handleOverrideChange = async (targetRegister: number, change: number) => {
        if (!primaryProfile) return;

        // We need the original service profile to save, not the UI one
        const sourceProfile = contextProfiles.find(p => p.slot === primaryProfile.slot);
        if (!sourceProfile) return;

        const currentOverrides = sourceProfile.overrides?.sp || [];
        // Check optimistic state first
        const optimisticProfileOverrides = optimisticOverrides[primaryProfile.name]?.sp;
        const baseOverrides = optimisticProfileOverrides || currentOverrides;

        const existingOverrideIndex = baseOverrides.findIndex(o => o.targetRegister === targetRegister);
        const newOverrides = [...baseOverrides];

        let newOffsetValue = 0;

        if (existingOverrideIndex >= 0) {
            newOffsetValue = newOverrides[existingOverrideIndex].offset + change;
            if (newOffsetValue === 0) {
                newOverrides.splice(existingOverrideIndex, 1);
            } else {
                newOverrides[existingOverrideIndex] = {
                    ...newOverrides[existingOverrideIndex],
                    offset: newOffsetValue
                };
            }
        } else {
            newOffsetValue = change;
            newOverrides.push({ targetRegister, offset: change });
        }

        // Apply optimistic update
        setOptimisticOverrides(prev => ({
            ...prev,
            [primaryProfile.name]: { sp: newOverrides }
        }));

        const updatedProfile = {
            ...sourceProfile,
            overrides: {
                ...sourceProfile.overrides,
                sp: newOverrides
            }
        };

        try {
            await modbusApiService.saveProfile(updatedProfile);
            // toast({ title: translate("Override Saved"), description: translate("Temperature offset updated.") });

            // Clear optimistic override after a delay to allow server sync
            setTimeout(() => {
                setOptimisticOverrides(prev => {
                    const next = { ...prev };
                    delete next[primaryProfile.name];
                    return next;
                });
            }, 10000);

        } catch (e) {
            console.error(e);
            toast({ title: translate("Error"), description: translate("Failed to save override."), variant: "destructive" });
            // Revert optimistic update
            setOptimisticOverrides(prev => {
                const next = { ...prev };
                delete next[primaryProfile.name];
                return next;
            });
        }
    };

    return (
        <div className="glass-card p-2 flex flex-col items-center gap-2" id="playback-controls-container">
            <div className="flex gap-2 bg-slate-200/50 dark:bg-slate-800/50 p-2 md:p-3 rounded-2xl border border-slate-300 dark:border-slate-700 shadow-none md:shadow-inner">
                <div className="flex items-center justify-center gap-2 md:gap-4">
                    {/* Play/Start Button */}
                    {(!primaryProfile || primaryProfile.status === PlotStatus.IDLE || primaryProfile.status === PlotStatus.STOPPED || primaryProfile.status === PlotStatus.FINISHED) && (
                        <Button
                            onClick={() => handlePrimaryProfileCommand(TemperatureProfileCommand.START)}
                            disabled={!primaryProfile || !isConnected || !primaryProfile.enabled}
                            className={cn(
                                "w-20 h-20 rounded-full text-white shadow-xl transition-all duration-300 ease-in-out transform hover:scale-110 border-4",
                                !primaryProfile || !isConnected || !primaryProfile.enabled
                                    ? "bg-gray-500 border-gray-600/50 cursor-not-allowed"
                                    : "bg-green-500 border-green-400/50 hover:bg-green-600"
                            )}
                            title={!primaryProfile || !primaryProfile.enabled ? translate("No enabled profile") : translate("Start Profile")}
                        >
                            <Play className="w-10 h-10 ml-1" />
                        </Button>
                    )}

                    {/* Pause Button */}
                    {(primaryProfile?.status === PlotStatus.RUNNING || primaryProfile?.status === PlotStatus.INITIALIZING) && (
                        <Button
                            onClick={() => handlePrimaryProfileCommand(TemperatureProfileCommand.PAUSE)}
                            disabled={!isConnected}
                            className={cn(
                                "w-20 h-20 rounded-full text-white shadow-xl transition-all duration-300 ease-in-out transform hover:scale-110 border-4",
                                primaryProfile?.status === PlotStatus.INITIALIZING
                                    ? "bg-orange-500 border-orange-400/50"
                                    : "bg-yellow-500 border-yellow-400/50 hover:bg-yellow-600"
                            )}
                            title={primaryProfile?.status === PlotStatus.INITIALIZING ? translate("Pause Warmup") : translate("Pause Profile")}
                        >
                            <Pause className="w-10 h-10" />
                        </Button>
                    )}

                    {/* Resume Button */}
                    {primaryProfile?.status === PlotStatus.PAUSED && (
                        <Button
                            onClick={() => handlePrimaryProfileCommand(TemperatureProfileCommand.RESUME)}
                            disabled={!isConnected}
                            className="w-20 h-20 rounded-full text-white shadow-xl transition-all duration-300 ease-in-out transform hover:scale-110 border-4 bg-primary border-sky-400/50 hover:bg-primary/90 animate-primary-glow"
                            title={translate("Resume Profile")}
                        >
                            <Play className="w-10 h-10 ml-1" />
                        </Button>
                    )}

                    <Button
                        onClick={() => handlePrimaryProfileCommand(TemperatureProfileCommand.STOP)}
                        disabled={!primaryProfile || !isConnected || (primaryProfile.status !== PlotStatus.RUNNING && primaryProfile.status !== PlotStatus.PAUSED && primaryProfile.status !== PlotStatus.INITIALIZING)}
                        variant="destructive"
                        className="w-12 h-12 rounded-full shadow-xl border-4 border-red-800/80"
                        title={translate("Stop Profile")}
                    >
                        <StopCircle className="w-8 h-8" />
                    </Button>
                </div>
                {primaryProfile && (
                    <div className="grid grid-cols-2 gap-x-2 text-[10px] text-slate-500 dark:text-slate-400 px-2 pt-1 border-l border-r border-slate-300/50 dark:border-slate-600/50 w-full">

                        <div className="text-center border-r border-slate-300/50 dark:border-slate-600/50">
                            <span className="font-semibold p-1 block text-slate-700 dark:text-slate-200">{formatTime(primaryProfile.duration / 1000)}</span>
                            <span className="uppercase tracking-wider text-[9px]">{translate('Total')}</span>
                        </div>
                        <div className="text-center">
                            <span className="font-semibold p-1 block text-slate-700 dark:text-slate-200">{primaryProfile.max}°C</span>
                            <span className="uppercase tracking-wider text-[9px]">{translate('Max')}</span>
                        </div>
                    </div>
                )}
            </div>
            {primaryProfile && (
                <div className="text-center w-full max-w-xs mx-auto space-y-1">
                    <div className="flex items-center justify-center gap-1">
                        <div className="font-semibold truncate text-muted-foreground text-xs" title={primaryProfile.name}>
                            {primaryProfile.name}
                        </div>
                        <Button
                            variant="ghost"
                            size="icon"
                            className="h-6 w-6 text-muted-foreground hover:text-foreground"
                            onClick={() => navigate(`/profiles/edit/${primaryProfile.slot}`)}
                            title={translate("Edit Profile")}
                        >
                            <Edit className="h-4 w-4" />
                        </Button>
                        <Button
                            variant="ghost"
                            size="icon"
                            className={cn(
                                "h-6 w-6 text-muted-foreground hover:text-foreground transition-colors",
                                showDescription && "text-blue-500 bg-blue-500/10 hover:bg-blue-500/20"
                            )}
                            onClick={() => setShowDescription(!showDescription)}
                            title={translate("Show Description")}
                        >
                            <HelpCircle className="h-4 w-4" />
                        </Button>
                    </div>

                    {showDescription && (
                        <div className="mt-2 text-xs text-muted-foreground bg-neutral-100 dark:bg-neutral-900/50 p-2 rounded-md border border-neutral-200 dark:border-neutral-800 animate-in fade-in slide-in-from-top-1 duration-200 text-left">
                            {primaryProfile.description || translate("No description available.")}
                        </div>
                    )}

                    {/* System Messages Panel */}
                    <div className="w-full mb-4">
                        <DisplayMessagesPanel collapsible={false} minimal={true} />
                    </div>

                    {/* Profile Chart */}
                    <div className="w-full">
                        <BezierEditor
                            controlPoints={primaryProfile.controlPoints}
                            onChange={() => { }}
                            max={primaryProfile.max}
                            duration={primaryProfile.duration}
                            readonly
                            showGridLabels={false}
                            className="h-40 w-full"
                            elapsedTime={primaryProfile.elapsed}
                            isRunning={primaryProfile.status === PlotStatus.RUNNING || primaryProfile.status === PlotStatus.PAUSED || primaryProfile.status === PlotStatus.INITIALIZING}
                            currentTemp={primaryProfile.currentTemp}
                            pressureProfile={associatedPressureProfile}
                            activeView="temperature"
                        />
                    </div>

                    {/* Scrubber / Slider - moved here */}
                    {(primaryProfile.status === PlotStatus.RUNNING || primaryProfile.status === PlotStatus.PAUSED) && primaryProfile.elapsed !== undefined && (
                        <div className="px-1 pt-2 pb-2 w-full">
                            <input
                                type="range"
                                min={0}
                                max={primaryProfile.duration / 1000}
                                value={isDragging ? sliderValue : primaryProfile.elapsed / 1000}
                                onChange={(e) => {
                                    setSliderValue(parseInt(e.target.value));
                                    setIsDragging(true);
                                }}
                                onMouseUp={async (e) => {
                                    const newValue = parseInt((e.target as HTMLInputElement).value);
                                    if (!isNaN(newValue)) {
                                        const elapsedReg = allModbusRegisters.find(r =>
                                            r.name.endsWith("Elapsed") &&
                                            (r.group === primaryProfile.name || r.group === `TempProfile_${primaryProfile.id}_Slot_${primaryProfile.slot}`)
                                        );

                                        if (elapsedReg) {
                                            try {
                                                await updateRegister(elapsedReg.address, newValue);
                                            } catch (error) {
                                                console.error("Failed to seek profile", error);
                                            } finally {
                                                setIsDragging(false);
                                            }
                                        } else {
                                            setIsDragging(false);
                                        }
                                    } else {
                                        setIsDragging(false);
                                    }
                                }}
                                onTouchEnd={async () => {
                                    const newValue = sliderValue;
                                    const elapsedReg = allModbusRegisters.find(r =>
                                        r.name.endsWith("Elapsed") &&
                                        (r.group === primaryProfile.name || r.group === `TempProfile_${primaryProfile.id}_Slot_${primaryProfile.slot}`)
                                    );

                                    if (elapsedReg) {
                                        try {
                                            await updateRegister(elapsedReg.address, newValue);
                                        } catch (error) {
                                            console.error("Failed to seek profile", error);
                                        } finally {
                                            setIsDragging(false);
                                        }
                                    } else {
                                        setIsDragging(false);
                                    }
                                }}
                                className="w-full h-2 bg-slate-200 rounded-lg appearance-none cursor-pointer dark:bg-slate-700 accent-blue-500"
                            />
                        </div>
                    )}

                    {warmupProgress !== null && (
                        <div className="mt-2">
                            <Progress value={warmupProgress} className="w-full h-3" />
                            <div className="text-xs text-muted-foreground font-semibold mt-1">{Math.round(warmupProgress)}%</div>
                        </div>
                    )}

                    {primaryProfile.status === PlotStatus.RUNNING && primaryProfile.currentTemp !== undefined && (
                        <div className="font-bold text-3xl text-blue-500">
                            {primaryProfile.currentTemp.toFixed(1)}°C
                        </div>
                    )}

                    {primaryProfile.status === PlotStatus.PAUSED && (
                        <div className="font-bold text-lg text-yellow-500 animate-pulse">
                            {translate('PAUSED')}
                        </div>
                    )}

                    {primaryProfile.status === PlotStatus.INITIALIZING && (
                        <div className="w-full px-4 text-center">
                            <div className="font-bold text-lg text-orange-500 animate-pulse">
                                {translate('WARMUP')}
                            </div>
                            {(warmupAverageTemp !== null || warmupTargetTemp !== null) && (
                                <div className="mt-1 space-y-1">
                                    {warmupAverageTemp !== null && (
                                        <div className="font-bold text-2xl text-orange-500">
                                            {warmupAverageTemp.toFixed(1)}°C
                                        </div>
                                    )}
                                    {warmupTargetTemp !== null && (
                                        <div className="text-sm text-muted-foreground">
                                            {translate('Target')}: <span className="font-semibold text-orange-400">{warmupTargetTemp.toFixed(1)}°C</span>
                                        </div>
                                    )}
                                </div>
                            )}


                        </div>
                    )}

                    {(primaryProfile.status === PlotStatus.RUNNING || primaryProfile.status === PlotStatus.PAUSED) && primaryProfile.elapsed !== undefined && primaryProfile.remaining !== undefined && (
                        <div className="space-y-1">
                            <div className="bg-neutral-100 dark:bg-neutral-900 border border-neutral-200 dark:border-neutral-700 rounded-lg p-1 md:p-2 shadow-none md:shadow-inner text-sm space-y-2">
                                <div className="flex justify-between items-baseline">
                                    <span className="text-muted-foreground">{translate('Elapsed:')}</span>
                                    <span className="font-mono font-semibold text-neutral-700 dark:text-neutral-300">
                                        {formatTime(primaryProfile.elapsed / 1000)}
                                    </span>
                                </div>
                                <div className="bg-white dark:bg-black/20 rounded p-1 md:p-2 text-center shadow-none md:shadow-inner">
                                    <div className="text-xs text-muted-foreground uppercase tracking-widest">{translate('Remaining')}</div>
                                    <div className="font-mono font-bold text-3xl text-blue-500 tracking-wider">
                                        {formatTime(Math.max(0, primaryProfile.remaining / 1000))}
                                    </div>
                                </div>
                            </div>

                            {/* Scrubber / Slider removed from here */}


                        </div>
                    )}

                    {/* Time Override Controls */}
                    {(primaryProfile.status === PlotStatus.RUNNING || primaryProfile.status === PlotStatus.PAUSED) && (
                        <div className="flex items-center justify-center gap-4 py-2">
                            {(() => {
                                const overrideReg = allModbusRegisters.find(
                                    reg => reg.name === "TimeOverride" &&
                                        (reg.group === primaryProfile.name || reg.group === `TempProfile_${primaryProfile.id}_Slot_${primaryProfile.slot}`)
                                );
                                const currentOverride = typeof overrideReg?.value === 'number' ? overrideReg.value : 0;

                                return (
                                    <>
                                        <Button
                                            variant="outline"
                                            size="sm"
                                            className="h-8 px-2 text-xs hover:bg-slate-100 dark:hover:bg-slate-800"
                                            onClick={() => overrideReg && updateRegister(overrideReg.address, Math.max(0, currentOverride - 10))}
                                            disabled={!isConnected || !overrideReg || currentOverride <= 0}
                                            title="-10 Minutes"
                                        >
                                            <Minus className="h-3 w-3 mr-1" /> 10m
                                        </Button>
                                        <div className="text-xs font-mono text-muted-foreground min-w-[70px] text-center bg-slate-50 dark:bg-slate-900/50 py-1 rounded border border-slate-100 dark:border-slate-800">
                                            {currentOverride > 0 ? '+' : ''}{currentOverride} min
                                        </div>
                                        <Button
                                            variant="outline"
                                            size="sm"
                                            className="h-8 px-2 text-xs hover:bg-slate-100 dark:hover:bg-slate-800"
                                            onClick={() => overrideReg && updateRegister(overrideReg.address, currentOverride + 10)}
                                            disabled={!isConnected || !overrideReg}
                                            title="+10 Minutes"
                                        >
                                            <Plus className="h-3 w-3 mr-1" /> 10m
                                        </Button>
                                    </>
                                );
                            })()}
                        </div>
                    )}

                    {/* PID List - Always Visible */}
                    {controllerDetails.length > 0 && (
                        <div className="bg-neutral-100 dark:bg-neutral-900 border border-neutral-200 dark:border-neutral-700 rounded-lg p-1 md:p-2 shadow-none md:shadow-inner">
                            <div className="text-xs text-muted-foreground uppercase tracking-widest mb-2 text-center">PIDs</div>
                            <div className="space-y-2">
                                {controllerDetails.map((controller, index) => {
                                    // Find the offset for this controller if available
                                    let regAddr = -1;

                                    // Priority 1: Check if any of the profile's target registers belong to this controller
                                    if (primaryProfile.targetRegisters) {
                                        const controllerSlaveId = controllerNameToSlaveIdMap.get(controller.name);
                                        if (controllerSlaveId !== undefined) {
                                            for (const targetAddr of primaryProfile.targetRegisters) {
                                                if (!targetAddr) continue;
                                                // We need to look up the register in allModbusRegisters to know which device it belongs to
                                                const reg = allModbusRegisters.find(r => r.address === targetAddr);
                                                if (reg && getSlaveIdFromGroup(reg.group) === controllerSlaveId) {
                                                    regAddr = targetAddr;
                                                    break;
                                                }
                                            }
                                        }
                                    }

                                    // Priority 2: Fallback to dynamic lookup if not found in targetRegisters
                                    if (regAddr === -1) {
                                        const spRegister = allModbusRegisters.find(
                                            reg => reg.name.endsWith(SP_REGISTER_NAME_SUFFIX) &&
                                                controllerNameToSlaveIdMap.get(controller.name) === getSlaveIdFromGroup(reg.group)
                                        );
                                        regAddr = spRegister ? spRegister.address : -1;
                                    }

                                    // Check optimistic overrides first, then fall back to profile data
                                    const optimisticProfileOverrides = optimisticOverrides[primaryProfile.name]?.sp;
                                    const activeOverrides = optimisticProfileOverrides || primaryProfile.overrides?.sp;
                                    const offsetData = activeOverrides?.find(o => o.targetRegister === regAddr);
                                    const currentOffset = offsetData ? offsetData.offset : 0;
                                    return (
                                        <div key={index} className={cn(
                                            "flex flex-col text-xs p-2 rounded-md border transition-all duration-200",
                                            "shadow-[inset_0_1px_3px_rgba(0,0,0,0.006)] dark:shadow-[inset_0_1px_3px_rgba(0,0,0,0.001)]",
                                            controller.isHeating
                                                ? "bg-red-50/50 dark:bg-red-900/20 border-red-200/50 dark:border-red-800/50 animate-pulse"
                                                : "bg-white/50 dark:bg-black/20 border-black/5 dark:border-white/5 text-muted-foreground"
                                        )}>
                                            <span
                                                className="font-medium truncate mb-1 text-center"
                                                title={`${controller.name}${controller.isHeating ? ' (Heating)' : ''}`}
                                            >
                                                {controller.name}
                                            </span>
                                            <div className="flex justify-center gap-4 text-xs items-center">
                                                <span className="font-mono">
                                                    PV: <span className="font-semibold text-blue-500">
                                                        {controller.pv !== null ? `${controller.pv.toFixed(1)}°C` : '--'}
                                                    </span>
                                                </span>
                                                <div className="flex items-center gap-1">
                                                    <span className="font-mono">
                                                        SP: <span className="font-semibold text-blue-400">
                                                            {controller.sp !== null ? `${controller.sp.toFixed(1)}°C` : '--'}
                                                        </span>
                                                    </span>
                                                    <div className="flex bg-white/10 rounded ml-1 items-center">
                                                        <button
                                                            className="p-1 hover:bg-white/20 rounded-l transition-colors"
                                                            onClick={() => handleOverrideChange(regAddr, -1)}
                                                            disabled={regAddr === -1}
                                                        >
                                                            <Minus className="h-3 w-3" />
                                                        </button>
                                                        <div className="text-[10px] px-1.5 min-w-[24px] text-center font-mono font-medium border-l border-r border-white/10">
                                                            {currentOffset > 0 ? '+' : ''}{currentOffset}
                                                        </div>
                                                        <button
                                                            className="p-1 hover:bg-white/20 rounded-r transition-colors"
                                                            onClick={() => handleOverrideChange(regAddr, 1)}
                                                            disabled={regAddr === -1}
                                                        >
                                                            <Plus className="h-3 w-3" />
                                                        </button>
                                                    </div>
                                                </div>
                                            </div>
                                        </div>
                                    );
                                })}
                            </div>
                        </div>
                    )}

                </div>
            )
            }
        </div >
    );
};

export default ProfilePlayback; 