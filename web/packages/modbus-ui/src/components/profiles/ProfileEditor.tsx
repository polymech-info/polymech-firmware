import React, { useState, useEffect, useRef, useCallback } from 'react';
import { Button } from '@/components/ui/button';
import { Input } from '@/components/ui/input';
import { Label } from '@/components/ui/label';
import { Select, SelectContent, SelectItem, SelectTrigger, SelectValue } from '@/components/ui/select';
import BezierEditor from '@/components/profiles/bezier/BezierEditor';
import {
    ControlPoint,
    Profile as TemperatureProfile,
    PlotStatus,
    Controller,
    SignalPlotData,
    SSignalControlPoint,
    ESignalType,
    ESignalState,
} from '@/types.js';
import { Download, Upload, Save } from 'lucide-react';
import { toast } from '@/components/ui/use-toast';
import { useNavigate } from 'react-router-dom';

// Import necessary items
import { useModbus } from '@/contexts/ModbusContext';
import { RegisterData, ProfileSavePayload } from '@polymech/client-ts';
import modbusApiService from '@polymech/client-ts/modbusApiService';
import { getSlaveIdFromGroup } from '../../lib/controllerUtils';
import { SP_CMD_COMMAND_REGISTER_PREFIX } from '@/constants';
import { TimeCodeEditor } from '@/components/TimeCodeEditor';
import MarkdownEditor from '@/components/MarkdownEditor';
import BezierControlPointList from '@/components/profiles/bezier/BezierControlPointList';

// Signal Plot Components
import Timeline from '@/components/profiles/Timeline';
import SignalControlPointList from '@/components/profiles/ControlPointList';
import SignalControlPointProperties from '@/components/profiles/ControlPointProperties';
import CollapsibleSection from '../CollapsibleSection';
import { AlertDialog, AlertDialogAction, AlertDialogCancel, AlertDialogContent, AlertDialogDescription, AlertDialogFooter, AlertDialogHeader, AlertDialogTitle } from '@/components/ui/alert-dialog';
import { MultipleSelector, Option } from '../ui/multiple-selector';
import { transformUIControlPointsToService } from '../../lib/profile-transformers';

import { T, translate } from '../../i18n';

const CONTROL_POINT_TIME_SCALE = 1000;

interface ProfileEditorProps {
    onSubmit: (data: Omit<TemperatureProfile, 'id' | 'createdAt' | 'updatedAt'>) => void;
    initialData?: TemperatureProfile;
    max: number;
    availableControllers: Controller[];
}

const defaultControlPoints: ControlPoint[] = [
    { x: 0, y: 0 },
    { x: 1, y: 1 }
];

// Define the clear/reset state for control points
const clearedControlPointsState: ControlPoint[] = [
    { x: 0, y: 1 }, // Start at time 0, temp 100% (max temp)
    { x: 1, y: 0 }  // End at time 1 (full duration), temp 0%
];

const ProfileEditor: React.FC<ProfileEditorProps> = ({
    onSubmit,
    initialData,
    max,
    availableControllers,
}) => {
    const navigate = useNavigate();

    // --- Local State Management ---
    const [name, setName] = useState(initialData?.name || '');
    const [description, setDescription] = useState(initialData?.description || '');
    const [duration, setDuration] = useState(initialData?.duration || 30 * 60 * 1000);
    const [signalPlotId, setSignalPlotId] = useState<number | undefined>(initialData?.signalPlot);
    const [children, setChildren] = useState<number[]>(initialData?.children || []);
    const [availableProfiles, setAvailableProfiles] = useState<TemperatureProfile[]>([]);


    const [controlPoints, setControlPoints] = useState<ControlPoint[]>(
        initialData?.controlPoints || defaultControlPoints
    );
    const [tempRange, setTempRange] = useState({
        max: initialData?.max || max
    });
    const [selectedTargetRegisters, setSelectedTargetRegisters] = useState<number[]>([]);
    const [availableSignalPlots, setAvailableSignalPlots] = useState<SignalPlotData[]>([]);

    const { profiles: liveProfiles } = useModbus();
    const liveProfileData = initialData ? liveProfiles.find(p => p.slot === initialData.slot) : undefined;
    const isRunning = liveProfileData?.status === PlotStatus.RUNNING || liveProfileData?.status === PlotStatus.PAUSED || liveProfileData?.status === PlotStatus.INITIALIZING;
    const elapsedTime = liveProfileData?.elapsed;

    // --- START: Signal Plot State ---
    const { registers: allModbusRegisters, coils: allModbusCoils } = useModbus();
    const [signalPlotData, setSignalPlotData] = useState<SignalPlotData | null>(null);
    const [isSignalPlotDirty, setIsSignalPlotDirty] = useState(false);
    const [selectedSignalCpInfo, setSelectedSignalCpInfo] = useState<{ plotSlot: number; cpId: number } | null>(null);
    const timelineRefs = useRef<Map<number, HTMLDivElement | null>>(new Map());
    const signalCpListRef = useRef<HTMLUListElement>(null);
    const [timelinesMeasured, setTimelinesMeasured] = useState(0);

    const [showDeleteConfirmDialog, setShowDeleteConfirmDialog] = useState(false);
    const [cpToDelete, setCpToDelete] = useState<{ plotSlot: number; cpId: number } | null>(null);

    // --- END: Signal Plot State ---

    useEffect(() => {
        const fetchAllProfiles = async () => {
            try {
                // We fetch all profiles to make them available for selection as children.
                const profiles = await modbusApiService.getProfiles();
                // Filter out the current profile from the list of available-for-selection profiles
                const filteredProfiles = initialData
                    ? profiles.filter(p => p.slot !== initialData.slot)
                    : profiles;
                setAvailableProfiles(filteredProfiles as TemperatureProfile[]);
            } catch (error) {
                console.error("Failed to fetch profiles for child selection:", error);
                toast({ title: "Error", description: "Could not load profiles for child selection.", variant: "destructive" });
            }
        };
        fetchAllProfiles();
    }, [initialData]);

    // Effect to initialize selectedTargetRegisters ensuring it's always an array of 8 numbers
    React.useEffect(() => {
        const initialValues = initialData?.targetRegisters || [];
        const newSelected: number[] = [];
        for (let i = 0; i < 8; i++) {
            newSelected.push(initialValues[i] !== undefined ? initialValues[i] : 0);
        }
        setSelectedTargetRegisters(newSelected);
    }, [initialData?.targetRegisters]);

    // Effect to fetch available signal plots
    useEffect(() => {
        const fetchSignalPlots = async () => {
            try {
                const plots = await modbusApiService.getSignalPlots();
                setAvailableSignalPlots(plots as SignalPlotData[]);
            } catch (error) {
                console.error("Failed to fetch signal plots:", error);
                toast({ title: translate("Error"), description: translate("Could not load signal plots for selection."), variant: "destructive" });
            }
        };
        fetchSignalPlots();
    }, []);

    // --- START: Signal Plot Effects and Handlers ---
    useEffect(() => {
        const fetchPlot = async (plotId: number) => {
            try {
                const plots = await modbusApiService.getSignalPlots();
                const plot = plots.find(p => p.slot === plotId);
                const plotWithDefaults = plot ? { ...plot, controlPoints: plot.controlPoints || [] } : null;

                if (plotWithDefaults) {
                    // Sync duration on load
                    plotWithDefaults.duration = duration;
                }

                setSignalPlotData(plotWithDefaults as SignalPlotData);
                setIsSignalPlotDirty(false);
                setSelectedSignalCpInfo(null);
            } catch (error) {
                toast({ title: translate("Error"), description: translate("Could not load signal plot {plotId}.").replace('{plotId}', plotId.toString()), variant: "destructive" });
                setSignalPlotData(null);
            }
        };

        if (signalPlotId !== undefined) {
            fetchPlot(signalPlotId);
        } else {
            setSignalPlotData(null);
        }
    }, [signalPlotId, duration]); // Use local duration state

    useEffect(() => {
        if (signalPlotData && signalPlotData.duration !== duration) {
            setSignalPlotData(prevData => {
                if (!prevData) return null;
                return { ...prevData, duration: duration };
            });
            setIsSignalPlotDirty(true);
        }
    }, [duration, signalPlotData]);

    // Force re-render for timeline measurements
    useEffect(() => {
        if (signalPlotData && (timelineRefs.current.get(signalPlotData.slot)?.clientWidth ?? 0) === 0 && timelinesMeasured < 5) {
            const timerId = setTimeout(() => setTimelinesMeasured(p => p + 1), 50);
            return () => clearTimeout(timerId);
        }
    }, [signalPlotData, timelinesMeasured]);

    const handleSignalPlotSave = async () => {
        if (!signalPlotData) return;
        try {
            await modbusApiService.saveSignalPlot(signalPlotData);
            toast({ title: translate("Signal Plot Saved"), description: translate('Plot "{name}" was saved successfully.').replace('{name}', signalPlotData.name) });
            setIsSignalPlotDirty(false);
        } catch (error) {
            toast({ title: translate("Error"), description: translate('Failed to save plot "{name}".').replace('{name}', signalPlotData.name), variant: "destructive" });
        }
    };

    const handleSignalCpPropertyChange = (
        propertyName: keyof Omit<SSignalControlPoint, 'id' | 'user' | 'state'>,
        rawValue: string | number | boolean
    ) => {
        if (!selectedSignalCpInfo) return;

        setSignalPlotData(prevPlot => {
            if (!prevPlot) return null;
            const updatedControlPoints = prevPlot.controlPoints.map(cp => {
                if (cp.id === selectedSignalCpInfo.cpId) {
                    let value: any = rawValue;
                    // Basic value conversion, can be expanded as in SignalPlotEditor
                    if (propertyName === 'time') {
                        value = Math.max(0, Math.min(Number(rawValue), CONTROL_POINT_TIME_SCALE));
                    } else if (['arg_0', 'arg_1', 'arg_2', 'type'].includes(propertyName)) {
                        value = Number(rawValue);
                        if (isNaN(value)) { value = 0; }
                    }
                    return { ...cp, [propertyName]: value };
                }
                return cp;
            });
            setIsSignalPlotDirty(true);
            return { ...prevPlot, controlPoints: updatedControlPoints };
        });
    };

    const getActualTimeMs = useCallback((cpTime: number, plotDuration: number): number => {
        if (plotDuration === 0) return 0;
        return (cpTime / CONTROL_POINT_TIME_SCALE) * plotDuration;
    }, []);

    const getSelectedSignalCpData = useCallback((): SSignalControlPoint | null => {
        if (!selectedSignalCpInfo || !signalPlotData) return null;
        return signalPlotData.controlPoints.find(cp => cp.id === selectedSignalCpInfo.cpId) || null;
    }, [selectedSignalCpInfo, signalPlotData]);

    const [draggingCpDetails, setDraggingCpDetails] = useState<{
        plotSlot: number;
        cpId: number;
        initialMouseX: number;
        initialCpTime: number;
        timelineScreenWidth: number;
    } | null>(null);

    const handleTimelineDoubleClick = (event: React.MouseEvent<HTMLDivElement>) => {
        if (!signalPlotData) return;
        const timelineRect = event.currentTarget.getBoundingClientRect();
        const clickX = event.clientX - timelineRect.left;
        const timelineScreenWidth = timelineRect.width - 20;

        if (timelineScreenWidth <= 0) return;

        const clickPercent = Math.max(0, Math.min(1, clickX / timelineScreenWidth));
        const newTimeScaled = Math.round(clickPercent * CONTROL_POINT_TIME_SCALE);

        setSignalPlotData(prevPlot => {
            if (!prevPlot) return null;
            const existingIds = new Set(prevPlot.controlPoints.map(cp => cp.id));
            let newId = 1;
            while (existingIds.has(newId)) {
                newId++;
            }
            const newCp: SSignalControlPoint = {
                id: newId,
                time: newTimeScaled,
                state: ESignalState.STATE_ON,
                type: ESignalType.MB_WRITE_COIL,
                arg_0: 0,
                arg_1: 0,
                name: `CP ${newId}`,
            };
            const updatedControlPoints = [...prevPlot.controlPoints, newCp].sort((a, b) => a.time - b.time);

            setSelectedSignalCpInfo({ plotSlot: prevPlot.slot, cpId: newId });
            setIsSignalPlotDirty(true);
            return { ...prevPlot, controlPoints: updatedControlPoints };
        });
    };

    const handleGlobalCpDragMouseMove = useCallback((event: MouseEvent) => {
        if (!draggingCpDetails || !signalPlotData) return;
        event.preventDefault();

        const { cpId, initialMouseX, initialCpTime, timelineScreenWidth } = draggingCpDetails;
        const currentMouseX = event.clientX;
        const deltaX = currentMouseX - initialMouseX;

        if (timelineScreenWidth <= 0) return;

        const deltaTimeScaled = (deltaX / timelineScreenWidth) * CONTROL_POINT_TIME_SCALE;
        let newTimeScaled = Math.round(initialCpTime + deltaTimeScaled);
        newTimeScaled = Math.max(0, Math.min(CONTROL_POINT_TIME_SCALE, newTimeScaled));

        setSignalPlotData(prevPlot => {
            if (!prevPlot) return null;
            return {
                ...prevPlot,
                controlPoints: prevPlot.controlPoints.map(cp =>
                    cp.id === cpId ? { ...cp, time: newTimeScaled } : cp
                )
            };
        });
    }, [draggingCpDetails, signalPlotData]);

    const handleGlobalCpDragMouseUp = useCallback(() => {
        if (!draggingCpDetails) return;

        setSignalPlotData(prevPlot => {
            if (!prevPlot) return null;
            return { ...prevPlot, controlPoints: [...prevPlot.controlPoints].sort((a, b) => a.time - b.time) };
        });

        setIsSignalPlotDirty(true);
        setDraggingCpDetails(null);

        window.removeEventListener('mousemove', handleGlobalCpDragMouseMove);
        window.removeEventListener('mouseup', handleGlobalCpDragMouseUp);
    }, [draggingCpDetails, handleGlobalCpDragMouseMove]);


    const handleCpMouseDown = (
        event: React.MouseEvent<HTMLDivElement>,
        plotSlot: number,
        cpId: number,
        currentCpTime: number
    ) => {
        event.preventDefault();
        event.stopPropagation();
        const timelineDiv = timelineRefs.current.get(plotSlot);
        if (!timelineDiv) return;

        const timelineScreenWidth = timelineDiv.clientWidth - 20;

        setSelectedSignalCpInfo({ plotSlot, cpId });
        signalCpListRef.current?.focus();

        setDraggingCpDetails({
            plotSlot,
            cpId,
            initialMouseX: event.clientX,
            initialCpTime: currentCpTime,
            timelineScreenWidth,
        });

        window.addEventListener('mousemove', handleGlobalCpDragMouseMove);
        window.addEventListener('mouseup', handleGlobalCpDragMouseUp);
    };

    // Get registers from Modbus context
    const { registers } = useModbus();

    const controllersWithSpAddress = React.useMemo(() => {
        if (!registers || registers.length === 0) {
            return [];
        }
        return availableControllers
            .map(controller => {
                const spCmdRegister = registers.find(
                    (reg: RegisterData) =>
                        getSlaveIdFromGroup(reg.group) === controller.slaveId &&
                        reg.name.startsWith(SP_CMD_COMMAND_REGISTER_PREFIX)
                );
                return {
                    controller,
                    spCmdRegisterAddress: spCmdRegister ? spCmdRegister.address : null,
                };
            })
            .filter((item): item is { controller: Controller; spCmdRegisterAddress: number } => item.spCmdRegisterAddress !== null);
    }, [availableControllers, registers]);

    const handleAddAll = () => {
        const allAddresses = controllersWithSpAddress.map(item => item.spCmdRegisterAddress);
        const newSelection = allAddresses.slice(0, 8);
        while (newSelection.length < 8) {
            newSelection.push(0);
        }
        setSelectedTargetRegisters(newSelection);

        if (allAddresses.length > 8) {
            toast({
                title: translate("Limit Reached"),
                description: translate("Selected the first 8 available controllers."),
            });
        }
    };

    const handleRemoveAll = () => {
        setSelectedTargetRegisters(Array(8).fill(0));
    };

    const handleTempRangeChange = (newMaxTemp: number) => {
        setTempRange({
            max: newMaxTemp
        });
    };

    const handleClearControlPoints = () => {
        setControlPoints(clearedControlPointsState);
    };

    const formatMillisecondsToHHMMSS = (ms: number) => {
        const totalSeconds = Math.floor(ms / 1000);
        const hours = Math.floor(totalSeconds / 3600);
        const minutes = Math.floor((totalSeconds % 3600) / 60);
        const seconds = totalSeconds % 60;

        const pad = (num: number) => num.toString().padStart(2, '0');

        return `${pad(hours)}:${pad(minutes)}:${pad(seconds)}`;
    };

    const handleExportMarkdown = () => {
        const formData = {
            name,
            description,
            duration,
            controlPoints,
            max: tempRange.max,
            signalPlot: signalPlotId,
        };
        const profileName = formData.name || 'Untitled Profile';

        let markdownContent = `# Profile: ${profileName}\n\n`;
        markdownContent += `**Description**: ${formData.description || 'N/A'}\n`;
        markdownContent += `**Duration**: ${formData.duration / 1000} seconds\n\n`;


        // --- Temperature Chapter ---
        markdownContent += `## Temperature Profile\n\n`;
        markdownContent += `**Max Temperature**: ${tempRange.max}°C\n\n`;
        markdownContent += `| Point | Time (s) | Temperature (°C) |\n`;
        markdownContent += `|---|---|---|\n`;
        formData.controlPoints.forEach((p, i) => {
            const timeSec = (p.x * formData.duration / 1000).toFixed(2);
            const tempC = (p.y * tempRange.max).toFixed(2);
            markdownContent += `| ${i + 1} | ${timeSec} | ${tempC} |\n`;
        });
        markdownContent += `\n`;

        markdownContent += `### Temperature Sequence\n\n`;
        formData.controlPoints.forEach((p, i) => {
            const timeFormatted = formatMillisecondsToHHMMSS(p.x * formData.duration);
            const tempC = (p.y * tempRange.max).toFixed(1);
            markdownContent += `* **At ${timeFormatted}**: Set temperature to **${tempC}°C**. \n  * **Note**: [Operator to fill in details]\n`;
        });
        markdownContent += `\n`;


        // --- Signals Chapter ---
        if (signalPlotData) {
            markdownContent += `## Signal Plot: ${signalPlotData.name}\n\n`;
            if (signalPlotData.controlPoints && signalPlotData.controlPoints.length > 0) {
                markdownContent += `| Name | Time (s) | Type | Arguments |\n`;
                markdownContent += `|---|---|---|---|\n`;
                signalPlotData.controlPoints.forEach(cp => {
                    const timeSec = getActualTimeMs(cp.time, signalPlotData.duration) / 1000;
                    const args = [cp.arg_0, cp.arg_1, cp.arg_2].filter(arg => arg !== undefined).join(', ');
                    markdownContent += `| ${cp.name || `CP ${cp.id}`} | ${timeSec.toFixed(2)} | ${ESignalType[cp.type]} | ${args} |\n`;
                });

                markdownContent += `### Signal Sequence\n\n`;
                signalPlotData.controlPoints.forEach(cp => {
                    const timeFormatted = formatMillisecondsToHHMMSS(getActualTimeMs(cp.time, signalPlotData.duration));
                    let actionDescription = '';
                    if (cp.type === ESignalType.DISPLAY_MESSAGE) {
                        actionDescription = `Display message: "${cp.description || '[No message content]'}"`;
                    } else {
                        actionDescription = `Execute ${ESignalType[cp.type]} with args (${[cp.arg_0, cp.arg_1, cp.arg_2].filter(arg => arg !== undefined).join(', ')})`;
                    }
                    markdownContent += `* **At ${timeFormatted}**: ${actionDescription}. \n  * **Note**: [Operator to fill in details]\n`;
                });

            } else {
                markdownContent += `No signal control points defined.\n`;
            }
        } else {
            markdownContent += `## Signals\n\n`;
            markdownContent += `No signal plot associated with this profile.\n`;
        }

        const blob = new Blob([markdownContent], { type: 'text/markdown' });
        const url = URL.createObjectURL(blob);
        const a = document.createElement('a');
        a.href = url;
        a.download = `${profileName.replace(/ /g, '_')}.md`;
        document.body.appendChild(a);
        a.click();
        document.body.removeChild(a);
        URL.revokeObjectURL(url);
    };

    const handleExportJson = () => {
        console.log('Exporting JSON...');
        debugger
        const serviceControlPoints = controlPoints.map(p => ({
            x: Math.round(p.x * 1000),
            y: Math.round(p.y * 1000)
        }));

        const profileToExport = {
            name,
            description,
            duration,
            max: tempRange.max,
            controlPoints: serviceControlPoints,
            targetRegisters: selectedTargetRegisters,
            signalPlot: signalPlotId,
            children,
            enabled: initialData?.enabled ?? true,
            pressureProfile: initialData?.pressureProfile,
        };

        const jsonString = JSON.stringify(profileToExport, null, 2);
        const blob = new Blob([jsonString], { type: "application/json" });
        const url = URL.createObjectURL(blob);
        const a = document.createElement('a');
        a.href = url;
        a.download = `${name || 'profile'}.json`;
        document.body.appendChild(a);
        a.click();
        document.body.removeChild(a);
        URL.revokeObjectURL(url);
    };

    const fileInputRef = React.useRef<HTMLInputElement>(null);

    const handleImportJson = (event: React.ChangeEvent<HTMLInputElement>) => {
        const file = event.target.files?.[0];
        if (!file) return;

        const reader = new FileReader();
        reader.onload = (e) => {
            try {
                const text = e.target?.result as string;
                const importedProfile = JSON.parse(text);

                // Validate and set form values
                // Basic validation, can be expanded with Zod or similar
                if (typeof importedProfile.name === 'string') {
                    setName(importedProfile.name);
                }
                if (typeof importedProfile.description === 'string') {
                    setDescription(importedProfile.description);
                }
                if (typeof importedProfile.duration === 'number') { // Expects duration in ms from import
                    setDuration(importedProfile.duration); // Set duration in ms
                }
                if (typeof importedProfile.max === 'number') {
                    setTempRange({ max: importedProfile.max });
                }
                if (Array.isArray(importedProfile.controlPoints)) {
                    // Expects controlPoints in 0-1000 scaled format, convert to UI normalized 0-1
                    const uiControlPoints = importedProfile.controlPoints.map(p => ({
                        x: Math.max(0, Math.min(1, p.x / 1000)),
                        y: Math.max(0, Math.min(1, p.y / 1000))
                    }));
                    setControlPoints(uiControlPoints);
                }
                if (Array.isArray(importedProfile.targetRegisters)) { // Import targetRegisters
                    setSelectedTargetRegisters(importedProfile.targetRegisters.filter((tr: any) => typeof tr === 'number'));
                }
                if (typeof importedProfile.signalPlot === 'number') {
                    setSignalPlotId(importedProfile.signalPlot);
                }
                if (Array.isArray(importedProfile.children)) {
                    setChildren(importedProfile.children.filter((c: any) => typeof c === 'number'));
                }
                toast({ title: translate("Profile Imported"), description: translate("Loaded profile: {name}").replace("{name}", importedProfile.name || translate('Untitled')) });
            } catch (error) {
                console.error("Failed to import JSON profile:", error);
                toast({ title: translate("Import Error"), description: translate("Invalid JSON file or format."), variant: "destructive" });
            } finally {
                // Reset file input to allow importing the same file again if needed
                if (fileInputRef.current) {
                    fileInputRef.current.value = "";
                }
            }
        };
        reader.readAsText(file);
    };

    const triggerFileSelect = () => fileInputRef.current?.click();

    const handleSubmit = () => {
        const submitData: ProfileSavePayload = {
            name,
            description,
            duration,
            controlPoints: transformUIControlPointsToService(controlPoints),
            max: tempRange.max,
            slot: initialData?.slot || 0,
            targetRegisters: selectedTargetRegisters,
            signalPlot: signalPlotId,
            children: children,
        };
        onSubmit(submitData as any);
    }

    const requestDeleteControlPoint = (plotSlot: number, cpId: number) => {
        setCpToDelete({ plotSlot, cpId });
        setShowDeleteConfirmDialog(true);
    };

    const handleDeleteControlPointConfirm = () => {
        if (!cpToDelete || !signalPlotData) return;

        setSignalPlotData(prevPlot => {
            if (!prevPlot) return null;
            const updatedControlPoints = prevPlot.controlPoints.filter(cp => cp.id !== cpToDelete.cpId);
            if (selectedSignalCpInfo && selectedSignalCpInfo.plotSlot === cpToDelete.plotSlot && selectedSignalCpInfo.cpId === cpToDelete.cpId) {
                setSelectedSignalCpInfo(null);
            }
            setIsSignalPlotDirty(true);
            return { ...prevPlot, controlPoints: updatedControlPoints };
        });

        setShowDeleteConfirmDialog(false);
        setCpToDelete(null);
    };

    const handleSignalCpKeyDown = (e: React.KeyboardEvent) => {
        if (!selectedSignalCpInfo || !signalPlotData) return;

        const sortedCps = [...signalPlotData.controlPoints].sort((a, b) => a.time - b.time);
        const cpIndex = sortedCps.findIndex(cp => cp.id === selectedSignalCpInfo.cpId);

        if (cpIndex === -1) return;

        const point = sortedCps[cpIndex];
        if (!point) return;

        let newTime = point.time;
        const step = 50; // 5% of 1000

        if (e.key === 'ArrowLeft') {
            newTime -= step;
        } else if (e.key === 'ArrowRight') {
            newTime += step;
        } else {
            return;
        }

        e.preventDefault();

        const minTime = cpIndex > 0 ? sortedCps[cpIndex - 1].time : 0;
        const maxTime = cpIndex < sortedCps.length - 1 ? sortedCps[cpIndex + 1].time : CONTROL_POINT_TIME_SCALE;

        const boundedTime = Math.max(minTime, Math.min(maxTime, newTime));
        const finalTime = Math.round(Math.max(0, Math.min(CONTROL_POINT_TIME_SCALE, boundedTime)));

        setSignalPlotData(prevPlot => {
            if (!prevPlot) return null;
            const updatedControlPoints = prevPlot.controlPoints.map(cp => {
                if (cp.id === selectedSignalCpInfo.cpId) {
                    return { ...cp, time: finalTime };
                }
                return cp;
            });
            setIsSignalPlotDirty(true);
            return { ...prevPlot, controlPoints: updatedControlPoints };
        });
    };

    const profileOptions: Option[] = availableProfiles.map(p => ({
        label: `${p.name} (Slot: ${p.slot})`,
        value: p.slot.toString(),
    }));

    const handleChildrenChange = (options: Option[]) => {
        setChildren(options.map(o => parseInt(o.value, 10)));
    };

    return (
        <div className="space-y-6">
            <div>
                <Label htmlFor="profile-name"><T>Profile Name</T></Label>
                <Input
                    id="profile-name"
                    placeholder={translate("E.g., Quick Ramp Up")}
                    value={name}
                    onChange={(e) => setName(e.target.value)}
                />
            </div>

            <div>
                <Label><T>Description</T></Label>
                <MarkdownEditor
                    value={description}
                    onValueChange={setDescription}
                />
            </div>

            <div>
                <Label><T>Duration (hh:mm:ss)</T></Label>
                <TimeCodeEditor
                    totalMilliseconds={duration}
                    onDurationChange={setDuration}
                />
            </div>

            <div className="space-y-2 border p-4 rounded-md">
                <label className="text-base font-medium">
                    <T>Profile Curves</T>
                </label>
                <div className='pt-2'>
                    <div className="grid grid-cols-1 gap-4">
                        <div>

                        </div>
                        <div className="w-full">
                            <BezierEditor
                                controlPoints={controlPoints}
                                onChange={setControlPoints}
                                max={tempRange.max}
                                duration={duration}
                                onTempRangeChange={handleTempRangeChange}
                                className="border rounded-md"
                                isRunning={isRunning}
                                elapsedTime={elapsedTime}
                                signalControlPoints={signalPlotData?.controlPoints}
                            />
                        </div>
                    </div>
                </div>

                {signalPlotData && (
                    <div className="pt-4">
                        <div className="px-6">
                            <Timeline
                                plotSlot={signalPlotData.slot}
                                plotIndex={0}
                                plotDuration={signalPlotData.duration}
                                controlPoints={signalPlotData.controlPoints}
                                selectedCpInfo={selectedSignalCpInfo}
                                isFirstPlotAndPlaying={false} // Not playing in form
                                isPlaying={false}
                                playbackTimeMs={0}
                                playbackHeadPositionPercent={0}
                                timelineRef={el => timelineRefs.current.set(signalPlotData.slot, el)}
                                getActualTimeMs={getActualTimeMs}
                                CONTROL_POINT_TIME_SCALE={CONTROL_POINT_TIME_SCALE}
                                onTimelineDoubleClick={handleTimelineDoubleClick}
                                onTimelineTap={() => { }}
                                onCpMouseDown={handleCpMouseDown}
                                onCpTouchStart={() => { }}
                                onRequestDeleteControlPoint={requestDeleteControlPoint}
                                numMarkers={0}
                                niceIntervalMs={0}
                                formatMarkerLabel={() => ''}
                                timelineMarkerKey={''}
                            />
                        </div>
                    </div>
                )}

                <CollapsibleSection
                    id="temp-profile-cps"
                    title={<T>Temperature Control Points</T>}
                    storageKey="temp-profile-cps-collapsible"
                    className="m-4"
                >
                    <div >
                        <BezierControlPointList
                            controlPoints={controlPoints}
                            onChange={setControlPoints}
                            duration={duration}
                            max={tempRange.max}
                        />
                        <Button
                            type="button"
                            variant="outline"
                            size="sm"
                            onClick={handleClearControlPoints}
                            className="mt-2"
                        >
                            <T>Clear Curve to Default Ramp</T>
                        </Button>
                    </div>
                </CollapsibleSection>

                {signalPlotData && (
                    <CollapsibleSection
                        className="m-4"
                        id="signal-plot-details"
                        title={<T>Signal Control Point Details</T>}
                        storageKey={`profile-form-signal-details-${signalPlotData.slot}`}
                        headerContent={
                            <Button size="sm" variant="outline" type="button" onClick={(e) => { e.stopPropagation(); handleSignalPlotSave(); }} disabled={!isSignalPlotDirty}>
                                <Save className="h-4 w-4 mr-2" />
                                <T>Save Signal Plot</T>
                            </Button>
                        }
                    >
                        <div className="grid grid-cols-1 md:grid-cols-2 gap-6 pt-4 border-t border-border">
                            <div className="space-y-2">
                                <h3 className="text-sm font-medium text-muted-foreground"><T>Control Points</T> ({signalPlotData.controlPoints.length}):</h3>
                                <SignalControlPointList
                                    ref={signalCpListRef}
                                    plotSlot={signalPlotData.slot}
                                    controlPoints={[...signalPlotData.controlPoints].sort((a, b) => a.time - b.time)}
                                    selectedCpInfo={selectedSignalCpInfo}
                                    onSelectControlPoint={(plotSlot, cpId, focusList) => {
                                        setSelectedSignalCpInfo({ plotSlot, cpId });
                                        if (focusList) {
                                            signalCpListRef.current?.focus();
                                        }
                                    }}
                                    isFirstPlotAndPlaying={false}
                                    onRequestDeleteControlPoint={requestDeleteControlPoint}
                                    onReorderControlPoint={() => { }}
                                    onKeyDown={handleSignalCpKeyDown}
                                />
                            </div>
                            <div className="space-y-2">
                                <h3 className="text-sm font-medium text-muted-foreground"><T>Properties:</T></h3>
                                {selectedSignalCpInfo ? (
                                    <SignalControlPointProperties
                                        plotSlot={signalPlotData.slot}
                                        selectedCpData={getSelectedSignalCpData()}
                                        isFirstPlotAndPlaying={false}
                                        plotDuration={signalPlotData.duration}
                                        getActualTimeMs={getActualTimeMs}
                                        onControlPointPropertyChange={handleSignalCpPropertyChange}
                                        CONTROL_POINT_TIME_SCALE={CONTROL_POINT_TIME_SCALE}
                                        allModbusCoils={allModbusCoils}
                                        allModbusRegisters={allModbusRegisters}
                                        onExecuteControlPoint={async () => { }}
                                    />
                                ) : (<p className="text-xs text-muted-foreground italic">{signalPlotData.controlPoints.length > 0 ? <T>Select a control point to see its properties.</T> : <T>No control points to select.</T>}</p>)}
                            </div>
                        </div>
                    </CollapsibleSection>
                )}

                <div className="m-4">
                    <Label><T>Associated Signal Plot (Optional)</T></Label>
                    <Select
                        onValueChange={(value) => {
                            const newId = value === "@none" ? undefined : parseInt(value, 10);
                            setSignalPlotId(newId);
                        }}
                        value={signalPlotId === undefined ? "@none" : signalPlotId.toString()}
                    >
                        <SelectTrigger>
                            <SelectValue placeholder={translate("Select a signal plot to associate and edit")} />
                        </SelectTrigger>
                        <SelectContent>
                            <SelectItem value="@none"><T>None</T></SelectItem>
                            {availableSignalPlots.map((plot) => (
                                <SelectItem key={plot.slot} value={plot.slot.toString()}>
                                    {plot.name} ({translate("Slot:")} {plot.slot})
                                </SelectItem>
                            ))}
                        </SelectContent>
                    </Select>
                </div>

                <div className="m-4">
                    <Label><T>Child Profiles (Sub-plots)</T></Label>
                    <MultipleSelector
                        value={profileOptions.filter(p => children.includes(parseInt(p.value, 10)))}
                        onChange={handleChildrenChange}
                        defaultOptions={profileOptions}
                        placeholder="Select profiles to run as children..."
                        emptyIndicator={
                            <p className="text-center text-lg leading-10 text-gray-600 dark:text-gray-400">
                                no results found
                            </p>
                        }
                    />
                    <p className="text-xs text-muted-foreground mt-1">
                        <T>Selected child profiles will start, stop, pause, and resume with this parent profile.</T>
                    </p>
                </div>

                <div className="space-y-2 m-4">
                    <div className="flex justify-between items-center mb-2">
                        <Label><T>Target Controllers (Registers)</T></Label>
                        <div className="flex gap-2">
                            <Button type="button" size="sm" variant="outline" onClick={handleAddAll}>
                                <T>Add all</T>
                            </Button>
                            <Button type="button" size="sm" variant="outline" onClick={handleRemoveAll}>
                                <T>Remove all</T>
                            </Button>
                        </div>
                    </div>
                    <div className="grid grid-cols-2 gap-2 p-2 border rounded-md max-h-32 overflow-y-auto">
                        {controllersWithSpAddress
                            .map(({ controller, spCmdRegisterAddress }) => {
                                const address = spCmdRegisterAddress;

                                return (
                                    <div key={controller.id} className="flex items-center space-x-2">
                                        <input
                                            type="checkbox"
                                            id={`controller-sp-cmd-${address}`}
                                            checked={selectedTargetRegisters.includes(address)}
                                            onChange={(e) => {
                                                const spAddressToToggle = address;

                                                setSelectedTargetRegisters((currentSelection) => {
                                                    const newSelection = [...currentSelection];

                                                    if (e.target.checked) {
                                                        if (newSelection.includes(spAddressToToggle)) {
                                                            return newSelection;
                                                        }
                                                        const firstOpenSlotIndex = newSelection.indexOf(0);
                                                        if (firstOpenSlotIndex !== -1) {
                                                            newSelection[firstOpenSlotIndex] = spAddressToToggle;
                                                        } else {
                                                            toast({
                                                                title: translate("Limit Reached"),
                                                                description: translate("All 8 target controller slots are filled."),
                                                                variant: "destructive",
                                                            });
                                                            return currentSelection;
                                                        }
                                                    } else {
                                                        const indexToRemove = newSelection.indexOf(spAddressToToggle);
                                                        if (indexToRemove !== -1) {
                                                            newSelection[indexToRemove] = 0;
                                                        }
                                                    }
                                                    return newSelection;
                                                });
                                            }}
                                            className="form-checkbox h-4 w-4 text-blue-600 transition duration-150 ease-in-out"
                                        />
                                        <label
                                            htmlFor={`controller-sp-cmd-${address}`}
                                            className="text-sm font-medium text-gray-700"
                                        >
                                            {controller.name} ({translate("Slave:")} {controller.slaveId}, {translate("SP CMD Addr:")} {address})
                                        </label>
                                    </div>
                                );
                            })}
                        {controllersWithSpAddress.length === 0 && (
                            <p className="text-sm text-muted-foreground col-span-2"><T>No controllers available.</T></p>
                        )}
                    </div>
                </div>
            </div>

            <div className="flex gap-2 mt-6">
                <Button type="button" variant="outline" onClick={handleExportJson} className="flex-1">
                    <Download className="mr-2 h-4 w-4" />
                    <T>Export JSON</T>
                </Button>
                <Button type="button" variant="outline" onClick={handleExportMarkdown} className="flex-1">
                    <Download className="mr-2 h-4 w-4" />
                    <T>Download as Markdown</T>
                </Button>
                <input
                    type="file"
                    accept=".json"
                    ref={fileInputRef}
                    onChange={handleImportJson}
                    style={{ display: 'none' }}
                />
                <Button type="button" variant="outline" onClick={triggerFileSelect} className="flex-1">
                    <Upload className="mr-2 h-4 w-4" />
                    <T>Import JSON</T>
                </Button>
            </div>

            <Button onClick={handleSubmit} className="w-full mt-2">
                {initialData ? <T>Update Profile</T> : <T>Create Profile</T>}
            </Button>

            <AlertDialog open={showDeleteConfirmDialog} onOpenChange={setShowDeleteConfirmDialog}>
                <AlertDialogContent>
                    <AlertDialogHeader>
                        <AlertDialogTitle><T>Are you sure?</T></AlertDialogTitle>
                        <AlertDialogDescription>
                            <T>This action cannot be undone. This will permanently delete the control point.</T>
                        </AlertDialogDescription>
                    </AlertDialogHeader>
                    <AlertDialogFooter>
                        <AlertDialogCancel onClick={() => setCpToDelete(null)}><T>Cancel</T></AlertDialogCancel>
                        <AlertDialogAction onClick={handleDeleteControlPointConfirm} className="bg-destructive hover:bg-destructive/90">
                            <T>Delete</T>
                        </AlertDialogAction>
                    </AlertDialogFooter>
                </AlertDialogContent>
            </AlertDialog>
        </div>
    );
};

export default ProfileEditor; 