import React, { useState, useEffect, useRef, useCallback, useMemo } from 'react';
import { SSignalControlPoint, ESignalState, ESignalType, SignalPlotData, SignalPlotCommand, PlotStatus } from '../types';
import { Card, CardContent, CardHeader, CardTitle, CardFooter } from '@/components/ui/card';
import { Input } from '@/components/ui/input';
import { Label } from '@/components/ui/label';

import { Button } from '@/components/ui/button';
import { PlayIcon, PauseIcon, SquareIcon as StopIcon, PlusCircleIcon, Trash2Icon, RotateCcw } from 'lucide-react';
import {
  AlertDialog,
  AlertDialogAction,
  AlertDialogCancel,
  AlertDialogContent,
  AlertDialogDescription,
  AlertDialogFooter,
  AlertDialogHeader,
  AlertDialogTitle,
} from '@/components/ui/alert-dialog';
import { useModbus } from '@/contexts/ModbusContext';
import { CoilSwitch } from '@/components/CoilSwitch';

import { SIGNAL_PLOT_REGISTER_NAMES } from '@/constants';
import { TimeCodeEditor } from './TimeCodeEditor';
import { toast } from 'sonner';
import { baseSampleCpTemplates } from './profiles/SamplePlot';
import modbusApiService from '@polymech/client-ts/modbusApiService';

// Import new components
import ControlPointList from './profiles/ControlPointList';
import ControlPointProperties from './profiles/ControlPointProperties';
import Timeline from './profiles/Timeline';
import TimelineControls from './profiles/TimelineControls';
import { calculateNiceTimeInterval, formatMarkerLabel } from './profiles/plotUtils';

import { T, translate } from '../i18n';

// A temporary polyfill for the missing API method
const getSignalPlotPolyfill = async (plotId: number): Promise<SignalPlotData | null> => {
  try {
    const plots = await modbusApiService.getSignalPlots();
    return plots.find(p => p.slot === plotId) || null;
  } catch (error) {
    console.error(`Polyfill failed to get signal plot ${plotId}:`, error);
    return null;
  }
};

const CONTROL_POINT_TIME_SCALE = 1000;

interface SignalPlotEditorProps {
  signalPlotId?: number;
  isEmbedded?: boolean;
}

interface UISignalPlot extends SignalPlotData {
  status: PlotStatus;
  elapsed: number;
}

interface SelectedControlPointInfo {
  plotSlot: number;
  cpId: number;
  timelineScreenWidth?: number;
}

interface DraggingCpDetails {
    plotSlot: number;
    cpId: number;
    initialMouseX: number;
    initialCpTime: number;
    timelineScreenWidth: number;
}

const SignalPlotEditor: React.FC<SignalPlotEditorProps> = ({ signalPlotId, isEmbedded = false }) => {
  const { 
    coils: allModbusCoils, 
    registers: allModbusRegisters,
    updateCoil,
    updateRegister,
    isConnected,
  } = useModbus();

  const [rawPlots, setRawPlots] = useState<SignalPlotData[]>([]);
  const [selectedCpInfo, setSelectedCpInfo] = useState<SelectedControlPointInfo | null>(null);

  // Create a memoized lookup map for registers. This is a major performance optimization.
  // It prevents looping over the entire `allModbusRegisters` array multiple times on every render.
  const registersByName = useMemo(() => {
    const map = new Map<string, number | undefined>();
    for (const reg of allModbusRegisters) {
      map.set(`${reg.group}:${reg.name}`, reg.value);
    }
    return map;
  }, [allModbusRegisters]);

  // DERIVE PLOT STATE ON-THE-FLY: Instead of using a useEffect/useState pair that causes
  // a re-render loop, we compute the UI plots directly on each render. This is the correct
  // and most performant pattern for handling frequently changing external data.
  const plots: UISignalPlot[] = rawPlots.map((plot): UISignalPlot => {
    const findRegisterValue = (name: string) => registersByName.get(`${plot.name}:${name}`);

    const status = findRegisterValue(SIGNAL_PLOT_REGISTER_NAMES.STATUS) ?? PlotStatus.IDLE;
    const elapsedLw = findRegisterValue(SIGNAL_PLOT_REGISTER_NAMES.ELAPSED_LW) ?? 0;
    const elapsedHw = findRegisterValue(SIGNAL_PLOT_REGISTER_NAMES.ELAPSED_HW) ?? 0;
    const elapsedMs = (elapsedHw << 16) | elapsedLw;
    
    const derivedPlot: UISignalPlot = {
      ...plot,
      status: status as PlotStatus,
      elapsed: elapsedMs,
    };

    // If the plot finishes, clamp the elapsed time for a clean visual.
    if (derivedPlot.status === PlotStatus.FINISHED && derivedPlot.elapsed > derivedPlot.duration) {
      derivedPlot.elapsed = derivedPlot.duration;
    }
    return derivedPlot;
  });

  const firstPlot = plots[0];
  const firstPlotStatus = firstPlot?.status ?? PlotStatus.IDLE;
  const firstPlotIsPlaying = firstPlotStatus === PlotStatus.RUNNING;
  const playbackTimeMs = firstPlot?.elapsed || 0;
  const firstPlotDuration = firstPlot?.duration || 0;

  // Delete confirmation states
  const [showDeleteConfirmDialog, setShowDeleteConfirmDialog] = useState(false);
  const [cpToDelete, setCpToDelete] = useState<SelectedControlPointInfo | null>(null);
  const [showClearAllConfirmDialog, setShowClearAllConfirmDialog] = useState(false);
  const [plotToClearCps, setPlotToClearCps] = useState<number | null>(null);

  // Dragging, editing, and refs
  const [isDraggingCp, setIsDraggingCp] = useState(false);
  const [draggingCpDetails, setDraggingCpDetails] = useState<DraggingCpDetails | null>(null);
  const timelineRefs = useRef<Map<number, HTMLDivElement | null>>(new Map());
  const [editingPlotNameSlot, setEditingPlotNameSlot] = useState<number | null>(null);
  const [currentEditName, setCurrentEditName] = useState<string>("");
  const nameInputRef = useRef<HTMLInputElement | null>(null);
  const cpListRefs = useRef<Map<number, HTMLUListElement | null>>(new Map());
  const [timelinesMeasured, setTimelinesMeasured] = useState(0); 

  // File input refs
  const globalPlotsFileInputRef = useRef<HTMLInputElement>(null);
  const singlePlotFileInputRef = useRef<HTMLInputElement>(null);

  // State for single plot upload targeting
  const [uploadTargetPlotSlot, setUploadTargetPlotSlot] = useState<number | null>(null);

  // Helper function (can be outside component if it doesn't use component scope other than constants)
  const getActualTimeMs = (cpTime: number, plotDuration: number): number => {
    if (plotDuration === 0) return 0;
    return (cpTime / CONTROL_POINT_TIME_SCALE) * plotDuration;
  };

  const handlePlotCommand = async (plot: UISignalPlot, command: SignalPlotCommand) => {
    if (!isConnected) {
        toast.error(translate("Not connected to Modbus server."));
        return;
    }
    if (!plot) {
        toast.error(translate("Cannot send command: plot data is missing."));
        return;
    }
    
    const commandRegister = allModbusRegisters.find(
        reg => reg.group === plot.name && reg.name === SIGNAL_PLOT_REGISTER_NAMES.COMMAND
    );

    if (!commandRegister) {
        toast.error(translate('Command register for plot "{name}" not found.').replace('{name}', plot.name));
        return;
    }

    try {
        await updateRegister(commandRegister.address, command);
        toast.success(translate('Command {command} sent to plot "{name}".').replace('{command}', SignalPlotCommand[command]).replace('{name}', plot.name));
    } catch (error) {
        toast.error(translate('Failed to send command to plot "{name}": {error}').replace('{name}', plot.name).replace('{error}', (error instanceof Error ? error.message : String(error))));
    }
  };

  // --- Control Point Action Handler ---
  const executeControlPointAction = useCallback(async (controlPoint: SSignalControlPoint, plotName: string) => {
    const cpName = controlPoint.name || `CP ${controlPoint.id}`;
    console.log(`[CP EXECUTE] Action for ${cpName} on "${plotName}" | Type: ${ESignalType[controlPoint.type]} | Args: ${controlPoint.arg_0}, ${controlPoint.arg_1}, ${controlPoint.arg_2}`);

    switch (controlPoint.type) {
      case ESignalType.MB_WRITE_COIL:
        try {
          await updateCoil(controlPoint.arg_0, controlPoint.arg_1 === 1);
          console.log(`[CP EXECUTE SUCCESS] Coil ${controlPoint.arg_0} set to ${controlPoint.arg_1 === 1 ? 'ON' : 'OFF'}`);
          toast.success(translate('Action: Coil {coil} set to {value} by "{plot}"').replace('{coil}', controlPoint.arg_0.toString()).replace('{value}', controlPoint.arg_1 === 1 ? 'ON' : 'OFF').replace('{plot}', plotName));
        } catch (error) {
          console.error(`[CP EXECUTE FAILED] Coil ${controlPoint.arg_0}:`, error);
          toast.error(translate('Failed to write Coil {coil}: {error}').replace('{coil}', controlPoint.arg_0.toString()).replace('{error}', (error instanceof Error ? error.message : String(error))));
        }
        break;
      case ESignalType.MB_WRITE_HOLDING_REGISTER:
        try {
          await updateRegister(controlPoint.arg_0, controlPoint.arg_1);
          console.log(`[CP EXECUTE SUCCESS] Register ${controlPoint.arg_0} set to ${controlPoint.arg_1}`);
          toast.success(translate('Action: Register {register} set to {value} by "{plot}"').replace('{register}', controlPoint.arg_0.toString()).replace('{value}', controlPoint.arg_1.toString()).replace('{plot}', plotName));
        } catch (error) {
          console.error(`[CP EXECUTE FAILED] Register ${controlPoint.arg_0}:`, error);
          toast.error(translate('Failed to write Register {register}: {error}').replace('{register}', controlPoint.arg_0.toString()).replace('{error}', (error instanceof Error ? error.message : String(error))));
        }
        break;
      default:
        console.warn(`[CP EXECUTE SKIPPED] Type ${ESignalType[controlPoint.type]} on "${plotName}" not implemented.`);
        toast.warning(translate('Action: CP Type {type} ({type}) on "{plot}" not implemented.').replace('{type}', ESignalType[controlPoint.type]).replace('{plot}', plotName));
        break;
    }
  }, [updateCoil, updateRegister]);

  // --- Handler for manually executing a single CP from properties view ---
  const handleExecuteSingleControlPoint = useCallback(async (controlPoint: SSignalControlPoint) => {
    if (!selectedCpInfo) {
      toast.error(translate('Cannot execute action: No control point selected or context lost.'));
      return;
    }
    const plotForCp = plots.find(p => p.slot === selectedCpInfo.plotSlot);
    if (!plotForCp) {
      toast.error(translate('Cannot execute action: Plot with slot {slot} not found.').replace('{slot}', String(selectedCpInfo.plotSlot)));
      return;
    }
    // Ensure the passed controlPoint is the one actually selected to prevent stale data issues if any
    if (controlPoint.id !== selectedCpInfo.cpId) {
        console.warn("Executing action for a CP that might not be the primary selected one. This is generally okay if data is from selectedCpData.");
    }
    await executeControlPointAction(controlPoint, plotForCp.name);
  }, [plots, selectedCpInfo, executeControlPointAction]);

  // --- API Interaction ---
  const handleSavePlotToApi = async (plotData: SignalPlotData) => {
    if (!plotData) {
      toast.error(translate('Attempted to save an invalid plot.'));
      return;
    }
    try {
      await modbusApiService.saveSignalPlot(plotData);
      toast.success(translate('Plot "{name}" (Slot {slot}) saved to API.').replace('{name}', plotData.name).replace('{slot}', String(plotData.slot)));
    } catch (error) {
      toast.error(translate('Failed to save plot "{name}": {error}').replace('{name}', plotData.name).replace('{error}', (error instanceof Error ? error.message : String(error))));
    }
  };

  // 1. Fetch static plot data from API
  useEffect(() => {
    const fetchPlots = async () => {
      try {
        let fetchedPlots: SignalPlotData[];
        if (signalPlotId !== undefined) {
          const plot = await getSignalPlotPolyfill(signalPlotId);
          fetchedPlots = plot ? [plot] : [];
        } else {
          fetchedPlots = await modbusApiService.getSignalPlots();
        }
        
        const sanitizedPlots = fetchedPlots.map(plot => ({
          ...plot,
          controlPoints: plot.controlPoints || [],
        }));
        setRawPlots(sanitizedPlots);
        if (!isEmbedded) {
          toast.success(translate('Signal plots configuration loaded from API.'));
        }
      } catch (error) {
        console.error("Error fetching signal plots:", error);
        if (!isEmbedded) {
          toast.error(translate('Failed to load signal plots: {error}').replace('{error}', (error instanceof Error ? error.message : String(error))));
        }
      }
    };

    fetchPlots();
  }, [signalPlotId, isEmbedded]);
  
  // Effect to prevent text selection during CP drag
  useEffect(() => {
    if (isDraggingCp) {
      document.body.style.userSelect = 'none';
      document.body.style.webkitUserSelect = 'none'; // For Safari
    } else {
      document.body.style.userSelect = '';
      document.body.style.webkitUserSelect = '';
    }
    return () => {
      document.body.style.userSelect = '';
      document.body.style.webkitUserSelect = '';
    };
  }, [isDraggingCp]);

  // Focus input when editingPlotNameSlot changes
  useEffect(() => {
    if (editingPlotNameSlot !== null && nameInputRef.current) {
      nameInputRef.current.focus();
      nameInputRef.current.select(); // Select all text
    }
  }, [editingPlotNameSlot]);

  // Effect to force a re-render when timeline dimensions become available, ensuring markers are scaled correctly.
  useEffect(() => {
    // If there are no plots, there's nothing to do.
    if (plots.length === 0) {
      return;
    }

    // Check if all timeline elements have been rendered and have a measurable width.
    const allTimelinesReady = plots.every(
      (plot) => (timelineRefs.current.get(plot.slot)?.clientWidth ?? 0) > 0
    );

    // If not all timelines are ready, schedule a state update to trigger a re-render.
    // This is retried a few times with a short delay to allow the browser to calculate layout.
    // This is crucial for the initial render where widths might not be available immediately.
    if (!allTimelinesReady && timelinesMeasured < 5) {
      const timerId = setTimeout(() => setTimelinesMeasured((prev) => prev + 1), 50);
      return () => clearTimeout(timerId);
    }
  }, [plots, timelinesMeasured]);

  const handlePlay = () => {
    if (firstPlot) {
      handlePlotCommand(firstPlot, SignalPlotCommand.START);
    }
  };

  const handlePause = () => {
    if (firstPlot) {
      handlePlotCommand(firstPlot, SignalPlotCommand.PAUSE);
    }
  };

  const handleStop = () => {
    if (firstPlot) {
      handlePlotCommand(firstPlot, SignalPlotCommand.STOP);
    }
  };
  
  const handleResume = () => {
    if (firstPlot) {
      handlePlotCommand(firstPlot, SignalPlotCommand.RESUME);
    }
  };

  const handleDurationChange = (plotIndex: number, newTotalMilliseconds: number) => {
    let changedPlot: SignalPlotData | undefined;
    setRawPlots(prevPlots =>
      prevPlots.map((plot, index) => {
        if (index === plotIndex) {
          changedPlot = { ...plot, duration: Math.max(0, newTotalMilliseconds) };
          return changedPlot;
        }
        return plot;
      })
    );
    if (changedPlot) {
      handleSavePlotToApi(changedPlot);
    }
  };

  const handleSelectControlPoint = (plotSlot: number, cpId: number, focusList: boolean = false) => {
    setSelectedCpInfo({ plotSlot, cpId });

    if (focusList) {
      setTimeout(() => {
        const listEl = cpListRefs.current.get(plotSlot);
        if (listEl) {
          listEl.focus();
          const itemEl = listEl.querySelector(`[data-cp-id="${cpId}"]`) as HTMLLIElement;
          if(itemEl) itemEl.focus();
        }
      },0);
    }
  };

  const getSelectedControlPointData = (): SSignalControlPoint | null => {
    if (!selectedCpInfo) return null;
    const plot = plots.find(p => p.slot === selectedCpInfo.plotSlot);
    if (!plot) return null;
    return plot.controlPoints.find(cp => cp.id === selectedCpInfo.cpId) || null;
  };

  const handleControlPointPropertyChange = (
    propertyName: keyof Omit<SSignalControlPoint, 'id' | 'user' | 'state'>, 
    rawValue: string | number | boolean
  ) => {
    if (!selectedCpInfo) return;
    let changedPlot: SignalPlotData | undefined;
    setRawPlots(prevPlots =>
      prevPlots.map(plot => {
        if (plot.slot === selectedCpInfo.plotSlot) {
          const updatedControlPoints = plot.controlPoints.map(cp => {
            if (cp.id === selectedCpInfo.cpId) {
              let value: any = rawValue;
              if (propertyName === 'time') {
                value = Math.max(0, Math.min(Number(rawValue), CONTROL_POINT_TIME_SCALE));
              } else if (propertyName === 'arg_1' && cp.type === ESignalType.MB_WRITE_COIL) {
                value = rawValue ? 1 : 0;
              } else if (propertyName === 'name' || propertyName === 'description') {
                value = rawValue === '' ? undefined : String(rawValue);
              } else if (['arg_0', 'arg_1', 'arg_2', 'type'].includes(propertyName)) {
                if (propertyName === 'arg_2' && rawValue === '') {
                  const { arg_2, ...restOfCp } = cp;
                  return { ...restOfCp, [propertyName]: undefined };
                }
                value = Number(rawValue);
                if (isNaN(value)) { value = 0; }
              }
              return { ...cp, [propertyName]: value };
            }
            return cp;
          });
          changedPlot = { ...plot, controlPoints: updatedControlPoints };
          return changedPlot;
        }
        return plot;
      })
    );
    if (changedPlot) {
      handleSavePlotToApi(changedPlot);
    }
  };
  const selectedCpData = getSelectedControlPointData();
  const playbackHeadPositionPercent = firstPlotDuration > 0 ? (playbackTimeMs / firstPlotDuration) * 100 : 0;

  const requestDeleteControlPoint = (plotSlot: number, cpId: number) => {
    setCpToDelete({ plotSlot, cpId });
    setShowDeleteConfirmDialog(true);
  };

  const handleDeleteControlPointConfirm = () => {
    if (!cpToDelete) return;
    let changedPlot: SignalPlotData | undefined;
    setRawPlots(prevPlots =>
      prevPlots.map(plot => {
        if (plot.slot === cpToDelete.plotSlot) {
          const updatedControlPoints = plot.controlPoints.filter(cp => cp.id !== cpToDelete.cpId);
          if (selectedCpInfo && selectedCpInfo.plotSlot === cpToDelete.plotSlot && selectedCpInfo.cpId === cpToDelete.cpId) {
            setSelectedCpInfo(null);
          }
          changedPlot = { ...plot, controlPoints: updatedControlPoints };
          return changedPlot;
        }
        return plot;
      })
    );
    if (changedPlot) {
      handleSavePlotToApi(changedPlot);
    }
    setShowDeleteConfirmDialog(false);
    setCpToDelete(null);
  };

  const handleTimelineDoubleClick = (event: React.MouseEvent<HTMLDivElement>, plotSlot: number, plotDuration: number) => {
    if (plotDuration <= 0) return;
    const timelineRect = event.currentTarget.getBoundingClientRect();
    const clickX = event.clientX - timelineRect.left;
    const timelineWidth = timelineRect.width - 20; 
    
    if (timelineWidth <= 0) return;

    const clickPercent = Math.max(0, Math.min(1, clickX / timelineWidth));
    const newTimeScaled = Math.round(clickPercent * CONTROL_POINT_TIME_SCALE);
    
    let changedPlot: SignalPlotData | undefined;
    setRawPlots(prevPlots => 
      prevPlots.map(plot => {
        if (plot.slot === plotSlot) {
          const existingIds = new Set(plot.controlPoints.map(cp => cp.id));
          let newId = 1;
          while(existingIds.has(newId)) {
            newId++;
          }
          const newCp: SSignalControlPoint = {
            id: newId,
            time: newTimeScaled,
            state: ESignalState.STATE_ON,
            type: ESignalType.MB_WRITE_COIL,
            arg_0: 0,
            arg_1: 0,
            arg_2: undefined, 
            name: `CP ${newId}`,
            description: "New control point",
          };
          const updatedControlPoints = [...plot.controlPoints, newCp].sort((a,b) => a.time - b.time);
          
          setSelectedCpInfo({ plotSlot, cpId: newId });

          changedPlot = { ...plot, controlPoints: updatedControlPoints };
          return changedPlot;
        }
        return plot;
      })
    );
    if (changedPlot) {
      handleSavePlotToApi(changedPlot);
    }
  };

  // Drag handlers
  const handleGlobalCpDragMouseMove = useCallback((event: MouseEvent) => {
    if (!isDraggingCp || !draggingCpDetails) return;
    event.preventDefault();

    const { plotSlot, cpId, initialMouseX, initialCpTime, timelineScreenWidth } = draggingCpDetails;
    const currentMouseX = event.clientX;
    const deltaX = currentMouseX - initialMouseX;

    if (timelineScreenWidth <= 0) return;

    const deltaTimeScaled = (deltaX / timelineScreenWidth) * CONTROL_POINT_TIME_SCALE;
    let newTimeScaled = Math.round(initialCpTime + deltaTimeScaled);
    newTimeScaled = Math.max(0, Math.min(CONTROL_POINT_TIME_SCALE, newTimeScaled));

    setRawPlots(prevPlots =>
      prevPlots.map(p => {
        if (p.slot === plotSlot) {
          return {
            ...p,
            controlPoints: p.controlPoints.map(cp =>
              cp.id === cpId ? { ...cp, time: newTimeScaled } : cp
            )
          };
        }
        return p;
      })
    );
  }, [isDraggingCp, draggingCpDetails]);

  const handleGlobalCpDragMouseUp = useCallback((event: MouseEvent) => {
    if (!isDraggingCp || !draggingCpDetails) return;
    event.preventDefault();
    
    let changedPlotData: SignalPlotData | undefined;
    setRawPlots(prevPlots => 
        prevPlots.map(p => {
            if (p.slot === draggingCpDetails.plotSlot) {
                const sortedControlPoints = [...p.controlPoints].sort((a,b) => a.time - b.time);
                changedPlotData = { ...p, controlPoints: sortedControlPoints};
                return changedPlotData;
            }
            return p;
        })
    );

    if (changedPlotData) {
        handleSavePlotToApi(changedPlotData);
    }

    setIsDraggingCp(false);
    setDraggingCpDetails(null);

    window.removeEventListener('mousemove', handleGlobalCpDragMouseMove);
    window.removeEventListener('mouseup', handleGlobalCpDragMouseUp);
  }, [isDraggingCp, draggingCpDetails, handleGlobalCpDragMouseMove, handleSavePlotToApi]);

  const handleCpMouseDown = (
    event: React.MouseEvent<HTMLDivElement>,
    plotSlot: number,
    cpId: number,
    currentCpTime: number,
    plotIndex: number
  ) => {
    event.preventDefault();
    event.stopPropagation();

    if (plotIndex === 0 && firstPlotIsPlaying) return;

    const timelineDiv = timelineRefs.current.get(plotSlot);
    if (!timelineDiv) return;

    const timelineScreenWidth = timelineDiv.clientWidth - 20;
    if (timelineScreenWidth <= 0) return;

    handleSelectControlPoint(plotSlot, cpId, true);
    setIsDraggingCp(true);
    setDraggingCpDetails({
      plotSlot,
      cpId,
      initialMouseX: event.clientX,
      initialCpTime: currentCpTime,
      timelineScreenWidth
    });

    window.addEventListener('mousemove', handleGlobalCpDragMouseMove);
    window.addEventListener('mouseup', handleGlobalCpDragMouseUp);
  };

  // Plot Name Editing Handlers
  const handlePlotNameClick = (plotSlot: number, currentName: string) => {
    if (editingPlotNameSlot === plotSlot) return;
    if (firstPlotIsPlaying && plots.findIndex(p => p.slot === plotSlot) === 0) return;
    setCurrentEditName(currentName);
    setEditingPlotNameSlot(plotSlot);
  };

  const handlePlotNameInputChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    setCurrentEditName(event.target.value);
  };

  const savePlotName = () => {
    if (editingPlotNameSlot === null) return;
    let changedPlot: SignalPlotData | undefined;
    setRawPlots(prevPlots =>
      prevPlots.map(plot => {
        if (plot.slot === editingPlotNameSlot) {
          changedPlot = { ...plot, name: currentEditName.trim() || translate('Plot {slot}').replace("{slot}", String(plot.slot)) };
          return changedPlot;
        }
        return plot;
      })
    );
    if (changedPlot) {
      handleSavePlotToApi(changedPlot);
    }
    setEditingPlotNameSlot(null);
  };

  const cancelPlotNameEdit = () => {
    setEditingPlotNameSlot(null);
  };

  const handlePlotNameKeyDown = (event: React.KeyboardEvent<HTMLInputElement>) => {
    if (event.key === 'Enter') {
      event.preventDefault();
      savePlotName();
    } else if (event.key === 'Escape') {
      event.preventDefault();
      cancelPlotNameEdit();
    }
  };

  const handleCpListKeyDown = (event: React.KeyboardEvent<HTMLUListElement>, plotSlot: number) => {
    const plot = plots.find(p => p.slot === plotSlot);
    if (!plot || plot.controlPoints.length === 0) return;

    const sortedCps = [...plot.controlPoints].sort((a,b) => a.time - b.time);

    let currentIndex = -1;
    if (selectedCpInfo && selectedCpInfo.plotSlot === plotSlot) {
        currentIndex = sortedCps.findIndex(cp => cp.id === selectedCpInfo.cpId);
    }

    let nextIndex = currentIndex;

    switch (event.key) {
      case 'ArrowUp':
        event.preventDefault();
        nextIndex = currentIndex > 0 ? currentIndex - 1 : sortedCps.length - 1;
        break;
      case 'ArrowDown':
        event.preventDefault();
        nextIndex = currentIndex < sortedCps.length - 1 ? currentIndex + 1 : 0;
        break;
      case 'Home':
        event.preventDefault();
        nextIndex = 0;
        break;
      case 'End':
        event.preventDefault();
        nextIndex = sortedCps.length - 1;
        break;
      default:
        return;
    }

    if (nextIndex !== -1 && sortedCps[nextIndex]) {
      const nextCpId = sortedCps[nextIndex].id;
      handleSelectControlPoint(plotSlot, nextCpId);
      const listElement = cpListRefs.current.get(plotSlot);
      const itemElement = listElement?.querySelector(`[data-cp-id="${nextCpId}"]`) as HTMLLIElement;
      if (itemElement) {
        itemElement.scrollIntoView({ behavior: 'smooth', block: 'nearest' });
      }
    }
  };

  const requestClearAllControlPoints = (plotSlot: number) => {
    setPlotToClearCps(plotSlot);
    setShowClearAllConfirmDialog(true);
  };

  const handleClearAllControlPointsConfirm = () => {
    if (plotToClearCps === null) return;
    let changedPlot: SignalPlotData | undefined;
    setRawPlots(prevPlots => 
      prevPlots.map(plot => {
        if (plot.slot === plotToClearCps) {
          changedPlot = { ...plot, controlPoints: [] };
          return changedPlot;
        }
        return plot;
      })
    );
    if (changedPlot) {
      handleSavePlotToApi(changedPlot);
    }
    if (selectedCpInfo && selectedCpInfo.plotSlot === plotToClearCps) {
        setSelectedCpInfo(null);
    }
    setShowClearAllConfirmDialog(false);
    setPlotToClearCps(null);
    toast.success(translate('All control points cleared for the plot.'));
  };

  const handleAddSampleControlPoints = (plotSlot: number, plotDuration: number) => {
    if (plotDuration <= 0) {
        toast.error(translate('Cannot add sample CPs to a plot with zero duration.'));
        return;
    }
    let changedPlot: SignalPlotData | undefined;
    setRawPlots(prevPlots => 
        prevPlots.map(plot => {
            if (plot.slot === plotSlot) {
                const existingCps = plot.controlPoints;
                let maxId = existingCps.reduce((max, cp) => Math.max(max, cp.id), 0);
                
                const timePercentages = [0.1, 0.25, 0.4, 0.55, 0.7, 0.85];
                const newSampleCps = baseSampleCpTemplates.slice(0, Math.min(baseSampleCpTemplates.length, timePercentages.length)).map((template, index) => ({
                    ...template,
                    id: ++maxId,
                    time: Math.round(timePercentages[index] * CONTROL_POINT_TIME_SCALE),
                    name: template.name || translate('Sample CP {id}').replace("{id}", String(maxId)),
                    description: template.description || translate('Sample type: {type}').replace("{type}", ESignalType[template.type]),
                }));

                const combinedCps = [...existingCps, ...newSampleCps].sort((a,b) => a.time - b.time);
                toast.success(translate('Added {count} sample control points.').replace('{count}', String(newSampleCps.length)));
                changedPlot = { ...plot, controlPoints: combinedCps };
                return changedPlot;
            }
            return plot;
        })
    );
    if (changedPlot) {
      handleSavePlotToApi(changedPlot);
    }
  };

  // --- GLOBAL PLOT JSON HANDLERS ---
  const handleDownloadAllPlotsJson = () => {
    if (rawPlots.length === 0) {
      toast.info(translate('There are no plots to download.'));
      return;
    }
    const jsonString = JSON.stringify(rawPlots, null, 2);
    const blob = new Blob([jsonString], { type: 'application/json' });
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = 'signal-plots-config-all.json';
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);
    URL.revokeObjectURL(url);
    toast.success(translate('All signal plots configuration downloaded.'));
  };

  const handleGlobalUploadJsonSelected = async (event: React.ChangeEvent<HTMLInputElement>) => {
    const file = event.target.files?.[0];
    if (!file) return;
    const reader = new FileReader();
    reader.onload = async (e) => {
      try {
        const text = e.target?.result;
        if (typeof text !== 'string') throw new Error(translate('Failed to read file content.'));
        const uploadedPlots = JSON.parse(text) as SignalPlotData[];
        if (!Array.isArray(uploadedPlots)) throw new Error(translate('Invalid JSON format: Expected an array of plots.'));
        
        setRawPlots(uploadedPlots);
        
        for (const plot of uploadedPlots) {
          await handleSavePlotToApi(plot);
        }

        setSelectedCpInfo(null);
        toast.success(translate('All signal plots configuration loaded and saved to API.'));
      } catch (error) {
        console.error('Error processing global signal plots JSON file:', error);
        toast.error(translate('Failed to load and save signal plots: {error}').replace('{error}', (error instanceof Error ? error.message : String(error))));
      }
      if (globalPlotsFileInputRef.current) globalPlotsFileInputRef.current.value = "";
    };
    reader.readAsText(file);
  };

  const triggerGlobalUploadJson = () => {
    globalPlotsFileInputRef.current?.click();
  };

  // --- SINGLE PLOT JSON HANDLERS ---
  const handleDownloadSinglePlotJson = (plotToDownload: SignalPlotData) => {
    if (!plotToDownload) {
        toast.error(translate('Plot data is not available for download.'));
        return;
    }
    const plotNameSafe = (plotToDownload.name || translate('untitled')).replace(/[^a-z0-9]/gi, '_').toLowerCase();
    const jsonString = JSON.stringify(plotToDownload, null, 2);
    const blob = new Blob([jsonString], { type: 'application/json' });
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = `signal-plot-${plotToDownload.slot}-${plotNameSafe}.json`;
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);
    URL.revokeObjectURL(url);
    toast.success(translate('Configuration for plot "{name}" downloaded.').replace('{name}', plotToDownload.name));
  };

  const handleSinglePlotUploadSelected = async (event: React.ChangeEvent<HTMLInputElement>) => {
    const file = event.target.files?.[0];
    if (!file || uploadTargetPlotSlot === null) {
      if (uploadTargetPlotSlot === null) toast.error(translate('No plot targeted for upload.'));
      if (singlePlotFileInputRef.current) singlePlotFileInputRef.current.value = "";
      return;
    }

    const targetSlot = uploadTargetPlotSlot; 
    setUploadTargetPlotSlot(null); 

    const reader = new FileReader();
    reader.onload = async (e) => {
      try {
        const text = e.target?.result;
        if (typeof text !== 'string') throw new Error(translate('Failed to read file content.'));
        const uploadedPlotData = JSON.parse(text) as SignalPlotData;

        if (typeof uploadedPlotData !== 'object' || uploadedPlotData === null || !('controlPoints' in uploadedPlotData) || !('duration' in uploadedPlotData)) {
          throw new Error(translate('Invalid JSON format: Expected a single plot object with controlPoints and duration.'));
        }
        if (uploadedPlotData.slot !== undefined && uploadedPlotData.slot !== targetSlot) {
            toast.info(translate('Uploaded plot had slot {uploadedSlot}, but will be loaded into slot {targetSlot}.').replace('{uploadedSlot}', String(uploadedPlotData.slot)).replace('{targetSlot}', String(targetSlot)));
        }
        
        const finalPlotData = { ...uploadedPlotData, slot: targetSlot };

        setRawPlots(prevPlots => 
          prevPlots.map(p => 
            p.slot === targetSlot 
            ? finalPlotData 
            : p
          )
        );
        
        await handleSavePlotToApi(finalPlotData);

        if (selectedCpInfo?.plotSlot === targetSlot) setSelectedCpInfo(null);

      } catch (error) {
        console.error(`Error processing JSON for plot slot ${targetSlot}:`, error);
        toast.error(translate('Failed to load configuration for plot {slot}: {error}').replace('{slot}', String(targetSlot)).replace('{error}', (error instanceof Error ? error.message : String(error))));
      }
      if (singlePlotFileInputRef.current) singlePlotFileInputRef.current.value = "";
    };
    reader.readAsText(file);
  };

  const triggerSinglePlotUpload = (plotSlot: number) => {
    setUploadTargetPlotSlot(plotSlot);
    singlePlotFileInputRef.current?.click();
  };

  return (
    <div className={isEmbedded ? "" : "p-4"}>
      <div className={isEmbedded ? "space-y-4" : "max-w-6xl mx-auto space-y-6"}>
        {!isEmbedded && (
          <div className="flex justify-between items-center mb-6">
              <h1 className="text-2xl font-bold"><T>Signal Plot Editor</T></h1>
              <div className="flex items-center space-x-2">
                <Button variant="outline" size="sm" onClick={handleDownloadAllPlotsJson}><T>Download All JSON</T></Button>
                <Button variant="outline" size="sm" onClick={triggerGlobalUploadJson}><T>Upload All JSON</T></Button>
                <input type="file" ref={globalPlotsFileInputRef} onChange={handleGlobalUploadJsonSelected} accept=".json" style={{ display: 'none' }} />
                <input type="file" ref={singlePlotFileInputRef} onChange={handleSinglePlotUploadSelected} accept=".json" style={{ display: 'none' }} />
              </div>
          </div>
        )}
        
        {plots.length === 0 && (
          <Card><CardContent className="pt-6"><p className="text-muted-foreground text-center"><T>No signal plots found.</T></p></CardContent></Card>
        )}

        {plots.map((plot, plotIndex) => {
          const isFirstPlot = plotIndex === 0;
          const isPlotRunningOrPaused = isFirstPlot && (plot.status === PlotStatus.RUNNING || plot.status === PlotStatus.PAUSED);
          
          const enableCoil = allModbusCoils.find(
            c => c.group === plot.name && c.name === SIGNAL_PLOT_REGISTER_NAMES.ENABLE_CMD
          );
          const timelineDiv = timelineRefs.current.get(plot.slot);
          const timelineScreenWidth = timelineDiv?.clientWidth ? timelineDiv.clientWidth - 20 : 0;
          const niceIntervalMs = calculateNiceTimeInterval(plot.duration, timelineScreenWidth);
          const numMarkers = plot.duration > 0 && niceIntervalMs > 0 ? Math.floor(plot.duration / niceIntervalMs) + 1 : 1;
          const timelineMarkerKey = `timeline-markers-${plot.slot}-${timelinesMeasured}`;
          const currentPlotHasDuration = plot.duration > 0;
          const currentPlotHasCps = plot.controlPoints.length > 0;

          return (
          <Card key={plot.slot} className={`shadow-lg w-full overflow-hidden ${isFirstPlot ? 'border-primary border-2' : ''}`}>
            <CardHeader className="pb-3 pt-4 bg-muted/30 border-b">
              <CardTitle className="text-lg font-semibold flex items-center justify-between">
                <div className="flex items-center flex-grow min-w-0">
                  {editingPlotNameSlot === plot.slot ? (
                    <Input
                      ref={nameInputRef}
                      type="text"
                      value={currentEditName}
                      onChange={handlePlotNameInputChange}
                      onBlur={savePlotName}
                      onKeyDown={handlePlotNameKeyDown}
                      className="text-lg font-semibold h-8 mr-2 flex-grow"
                      disabled={isPlotRunningOrPaused}
                    />
                  ) : (
                    <span 
                      onClick={() => !isPlotRunningOrPaused && handlePlotNameClick(plot.slot, plot.name)}
                      className={`text-primary underline decoration-dotted decoration-primary/70 hover:decoration-solid ${!isPlotRunningOrPaused ? 'cursor-pointer' : 'cursor-default'} mr-2 flex-grow truncate`}
                      title={plot.name}
                    >
                      {plot.name}
                    </span>
                  )}
                  <span className="text-muted-foreground text-nowrap">({translate('Slot:')} {plot.slot})</span>
                </div>
                
                <div className="ml-auto pl-2 flex-shrink-0">
                  {enableCoil ? (
                    <CoilSwitch 
                      address={enableCoil.address} 
                      value={enableCoil.value} 
                      disabled={isPlotRunningOrPaused}
                      aria-label={translate('Enable plot {name}').replace('{name}', plot.name)}
                    />
                  ) : (
                    <CoilSwitch address={-1} value={false} disabled={true} aria-label={translate('Enable control unavailable for {name}').replace('{name}', plot.name)} />
                  )}
                </div>
              </CardTitle>
            </CardHeader>
            <CardContent className="pt-4 pb-4 space-y-4">
              <div className="space-y-4">
                <div className="flex items-center space-x-2">
                  <Label htmlFor={`duration-timecode-${plot.slot}`} className="text-sm whitespace-nowrap self-center">
                    <T>Duration:</T>
                  </Label>
                  <TimeCodeEditor 
                    totalMilliseconds={plot.duration}
                    onDurationChange={(newMs) => handleDurationChange(plotIndex, newMs)}
                    disabled={isPlotRunningOrPaused}
                    idPrefix={`plot-${plot.slot}-duration`}
                  />
                  <span className="text-sm text-muted-foreground self-center">
                    ({translate('Total:')} {(plot.duration / 1000).toFixed(1)}s)
                  </span>
                </div>
                <div>
                  <h3 className="text-sm font-medium text-muted-foreground mb-1.5"><T>Timeline:</T></h3>
                  <Timeline
                    plotSlot={plot.slot}
                    plotIndex={plotIndex}
                    plotDuration={plot.duration}
                    controlPoints={plot.controlPoints}
                    selectedCpInfo={selectedCpInfo}
                    isFirstPlotAndPlaying={isPlotRunningOrPaused && plot.status === PlotStatus.RUNNING}
                    isPlaying={isPlotRunningOrPaused && plot.status === PlotStatus.RUNNING} 
                    playbackTimeMs={isFirstPlot ? playbackTimeMs : 0} 
                    playbackHeadPositionPercent={isFirstPlot ? playbackHeadPositionPercent : 0}
                    timelineRef={el => timelineRefs.current.set(plot.slot, el)}
                    onTimelineDoubleClick={handleTimelineDoubleClick}
                    onCpMouseDown={handleCpMouseDown}
                    onRequestDeleteControlPoint={requestDeleteControlPoint}
                    getActualTimeMs={getActualTimeMs}
                    numMarkers={numMarkers}
                    niceIntervalMs={niceIntervalMs}
                    formatMarkerLabel={formatMarkerLabel}
                    timelineMarkerKey={timelineMarkerKey}
                    CONTROL_POINT_TIME_SCALE={CONTROL_POINT_TIME_SCALE}
                    isPlaybackMarkerDraggable={false}
                    isDraggingPlaybackMarker={false}
                  />
                  {isFirstPlot && (
                    <TimelineControls 
                      hasDuration={currentPlotHasDuration}
                      onPlay={handlePlay}
                      onPause={handlePause}
                      onStop={handleStop}
                      onResume={handleResume}
                      plotStatus={firstPlotStatus}
                      playbackTimeMs={playbackTimeMs}
                      plotDuration={firstPlotDuration}
                    />
                  )}
                </div>
              </div>
              <div className="grid grid-cols-1 md:grid-cols-2 gap-6 pt-4 border-t border-border">
                <div className="space-y-2">
                  <h3 className="text-sm font-medium text-muted-foreground"><T>Control Points</T> ({plot.controlPoints.length}):</h3>
                  <ControlPointList 
                    plotSlot={plot.slot}
                    controlPoints={[...plot.controlPoints].sort((a,b) => a.time - b.time)}
                    selectedCpInfo={selectedCpInfo}
                    isFirstPlotAndPlaying={isPlotRunningOrPaused}
                    onSelectControlPoint={handleSelectControlPoint}
                    onRequestDeleteControlPoint={requestDeleteControlPoint}
                    cpListRef={el => cpListRefs.current.set(plot.slot, el)}
                    onKeyDown={(e) => handleCpListKeyDown(e, plot.slot)}
                  />
                </div>
                <div className="space-y-2">
                  <h3 className="text-sm font-medium text-muted-foreground"><T>Properties:</T></h3>
                  {selectedCpInfo?.plotSlot === plot.slot ? (
                    <ControlPointProperties 
                        plotSlot={plot.slot}
                        selectedCpData={selectedCpData} 
                        isFirstPlotAndPlaying={isPlotRunningOrPaused}
                        plotDuration={plot.duration}
                        getActualTimeMs={getActualTimeMs}
                        onControlPointPropertyChange={handleControlPointPropertyChange}
                        CONTROL_POINT_TIME_SCALE={CONTROL_POINT_TIME_SCALE}
                        allModbusCoils={allModbusCoils}
                        allModbusRegisters={allModbusRegisters}
                        onExecuteControlPoint={handleExecuteSingleControlPoint}
                    />
                  ) : (<p className="text-xs text-muted-foreground italic">{plot.controlPoints.length > 0 ? <T>Select a control point to see its properties.</T> : <T>No control points to select.</T>}</p>)}
                </div>
              </div>
            </CardContent>
            <CardFooter className="flex justify-between items-center space-x-2 py-3 bg-muted/30 border-t">
                <div>
                    <Button 
                        variant="outline"
                        size="sm"
                        onClick={() => handleAddSampleControlPoints(plot.slot, plot.duration)}
                        disabled={isPlotRunningOrPaused || plot.duration <= 0}
                        title={translate('Add a set of sample control points to this plot')}
                        className="mr-2"
                    >
                        <PlusCircleIcon className="h-4 w-4 mr-2" />
                        <T>Add Samples</T>
                    </Button>
                    <Button 
                        variant="outline"
                        className="text-destructive border-destructive hover:bg-destructive/10 hover:text-destructive"
                        size="sm"
                        onClick={() => requestClearAllControlPoints(plot.slot)}
                        disabled={isPlotRunningOrPaused || plot.controlPoints.length === 0}
                        title={translate('Remove all control points from this plot')}
                    >
                        <Trash2Icon className="h-4 w-4 mr-2" />
                        <T>Clear All CPs</T>
                    </Button>
                </div>
                <div>
                    <Button
                        variant="outline"
                        size="sm"
                        onClick={() => handleDownloadSinglePlotJson(plot)}
                        disabled={isPlotRunningOrPaused}
                        title={translate('Download JSON for {name}').replace('{name}', plot.name)}
                        className="mr-2"
                    >
                        <T>Download Plot</T>
                    </Button>
                    <Button
                        variant="outline"
                        size="sm"
                        onClick={() => triggerSinglePlotUpload(plot.slot)}
                        disabled={isPlotRunningOrPaused}
                        title={translate('Upload JSON for {name}').replace('{name}', plot.name)}
                    >
                        <T>Upload Plot</T>
                    </Button>
                </div>
            </CardFooter>
          </Card>
        );
      })}
      </div>
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
      <AlertDialog open={showClearAllConfirmDialog} onOpenChange={setShowClearAllConfirmDialog}>
        <AlertDialogContent>
          <AlertDialogHeader>
            <AlertDialogTitle><T>Clear All Control Points?</T></AlertDialogTitle>
            <AlertDialogDescription>
              <T>This action cannot be undone. This will remove all control points from this plot. Are you sure you want to proceed?</T>
            </AlertDialogDescription>
          </AlertDialogHeader>
          <AlertDialogFooter>
            <AlertDialogCancel onClick={() => setPlotToClearCps(null)}><T>Cancel</T></AlertDialogCancel>
            <AlertDialogAction onClick={handleClearAllControlPointsConfirm} className="bg-destructive hover:bg-destructive/90">
              <T>Clear All</T>
            </AlertDialogAction>
          </AlertDialogFooter>
        </AlertDialogContent>
      </AlertDialog>
    </div>
  );
};

export default SignalPlotEditor; 