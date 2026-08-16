import React, { useState, useEffect, useRef, useCallback, useMemo } from 'react';
import { SSignalControlPoint, ESignalState, ESignalType, SignalPlotData, SignalPlotCommand, PlotStatus } from '../types';
import { Card, CardContent, CardFooter } from '@/components/ui/card';
import { Input } from '@/components/ui/input';
import { Label } from '@/components/ui/label';
import { Select, SelectContent, SelectItem, SelectTrigger, SelectValue } from '@/components/ui/select';

import { Button } from '@/components/ui/button';
import { PlusCircleIcon, Trash2Icon, RotateCcw, SaveIcon, CopyPlus, Loader2, HelpCircle } from 'lucide-react';
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
import CollapsibleSection from './CollapsibleSection';
import ControlPointList from './profiles/ControlPointList';
import ControlPointProperties from './profiles/ControlPointProperties';
import Timeline from './profiles/Timeline';
import TimelineControls from './profiles/TimelineControls';
import { calculateNiceTimeInterval, formatMarkerLabel } from './profiles/plotUtils';
import ControlPointDialog, { NewControlPointData } from './profiles/ControlPointDialog';

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
  const [dirtyPlotSlots, setDirtyPlotSlots] = useState<Set<number>>(new Set());
  const [isLoading, setIsLoading] = useState(true);

  // State for the new CP creation dialog
  const [isCreateCpDialogOpen, setCreateCpDialogOpen] = useState(false);
  const [newCpInitialData, setNewCpInitialData] = useState<{ plotSlot: number; time: number; duration: number } | null>(null);

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
    const elapsedLw = findRegisterValue(SIGNAL_PLOT_REGISTER_NAMES.ELAPSED) ?? 0;
    const elapsedHw = findRegisterValue(SIGNAL_PLOT_REGISTER_NAMES.ELAPSED) ?? 0;
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

  const [copyDialogOpen, setCopyDialogOpen] = useState(false);
  const [plotToCopy, setPlotToCopy] = useState<SignalPlotData | null>(null);
  const [targetPlotSlot, setTargetPlotSlot] = useState<string | null>(null);

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
  const [isDraggingPlaybackMarker, setIsDraggingPlaybackMarker] = useState(false);
  const timelineDragInfo = useRef<{ width: number, left: number } | null>(null);

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
          await updateCoil(controlPoint?.arg_0 ?? 0, controlPoint?.arg_1 === 1);
          console.log(`[CP EXECUTE SUCCESS] Coil ${controlPoint.arg_0} set to ${controlPoint.arg_1 === 1 ? 'ON' : 'OFF'}`);
          toast.success(translate('Action: Coil {coil} set to {value} by "{plot}"').replace('{coil}', controlPoint?.arg_0?.toString() ?? '').replace('{value}', controlPoint.arg_1 === 1 ? 'ON' : 'OFF').replace('{plot}', plotName));
        } catch (error) {
          console.error(`[CP EXECUTE FAILED] Coil ${controlPoint.arg_0}:`, error);
          toast.error(translate('Failed to write Coil {coil}: {error}').replace('{coil}', controlPoint?.arg_0?.toString() ?? '').replace('{error}', (error instanceof Error ? error.message : String(error))));
        }
        break;
      case ESignalType.MB_WRITE_HOLDING_REGISTER:
        try {
          await updateRegister(controlPoint?.arg_0 ?? 0, controlPoint?.arg_1 ?? 0);
          console.log(`[CP EXECUTE SUCCESS] Register ${controlPoint.arg_0} set to ${controlPoint.arg_1}`);
          toast.success(translate('Action: Register {register} set to {value} by "{plot}"').replace('{register}', controlPoint?.arg_0?.toString() ?? '').replace('{value}', controlPoint?.arg_1?.toString() ?? '').replace('{plot}', plotName));
        } catch (error) {
          console.error(`[CP EXECUTE FAILED] Register ${controlPoint.arg_0}:`, error);
          toast.error(translate('Failed to write Register {register}: {error}').replace('{register}', controlPoint?.arg_0?.toString() ?? '').replace('{error}', (error instanceof Error ? error.message : String(error))));
        }
        break;
      case ESignalType.START_PIDS:
        toast.info(`[CP EXECUTE] START_PIDS on "${plotName}"`);
        break;
      case ESignalType.STOP_PIDS:
        toast.info(`[CP EXECUTE] STOP_PIDS on "${plotName}"`);
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
      // After a successful save, remove the plot from the dirty set
      setDirtyPlotSlots(prev => {
        const newSet = new Set(prev);
        newSet.delete(plotData.slot);
        return newSet;
      });
    } catch (error) {
      toast.error(translate('Failed to save plot "{name}": {error}').replace('{name}', plotData.name).replace('{error}', (error instanceof Error ? error.message : String(error))));
    }
  };

  const handleRevertPlot = async (plotSlot: number) => {
    try {
      // Refetch the original plot data from the "source of truth"
      const originalPlot = await getSignalPlotPolyfill(plotSlot);
      if (originalPlot) {
        // Replace the plot in state with the original version
        setRawPlots(prevPlots =>
          prevPlots.map(p => (p.slot === plotSlot ? { ...originalPlot, controlPoints: originalPlot.controlPoints || [] } : p))
        );
        // Remove the plot from the dirty set
        setDirtyPlotSlots(prev => {
          const newSet = new Set(prev);
          newSet.delete(plotSlot);
          return newSet;
        });
        // Unselect CP if it belonged to the reverted plot
        if (selectedCpInfo?.plotSlot === plotSlot) {
          setSelectedCpInfo(null);
        }
        toast.info(translate('Changes for plot in slot {slot} have been reverted.').replace('{slot}', String(plotSlot)));
      } else {
        toast.error(translate('Could not find original data for plot in slot {slot} to revert.').replace('{slot}', String(plotSlot)));
      }
    } catch (error) {
      toast.error(translate('Failed to revert plot: {error}').replace('{error}', (error instanceof Error ? error.message : String(error))));
    }
  };

  const openCopyToDialog = (plot: SignalPlotData) => {
    setPlotToCopy(plot);
    setTargetPlotSlot(null);
    setCopyDialogOpen(true);
  };

  const handleCopyToConfirm = async () => {
    if (!plotToCopy || targetPlotSlot === null) {
      toast.error(translate("Source or destination plot not selected."));
      return;
    }

    const targetSlot = Number(targetPlotSlot);
    const targetPlot = rawPlots.find(p => p.slot === targetSlot);
    if (!targetPlot) {
      toast.error(translate("Target plot not found."));
      return;
    }

    const payload: SignalPlotData = {
      ...plotToCopy,
      slot: targetSlot,
    };

    try {
      await handleSavePlotToApi(payload);
      setRawPlots(prevPlots =>
        prevPlots.map(p => (p.slot === targetSlot ? payload : p))
      );
      toast.success(
        translate('Successfully copied plot "{sourceName}" to slot {targetSlot}.').replace('{sourceName}', plotToCopy.name).replace('{targetSlot}', String(targetSlot))
      );
    } catch (error) {
      console.error("Copy to failed", error);
    } finally {
      setCopyDialogOpen(false);
      setPlotToCopy(null);
      setTargetPlotSlot(null);
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
          fetchedPlots = await modbusApiService.getSignalPlots() as SignalPlotData[];
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
      } finally {
        setIsLoading(false);
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
      const timerId = setTimeout(() => {
        setTimelinesMeasured((prev) => prev + 1);
      }, 50);
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

  const handleUpdatePlaybackTime = async (newTimeMs: number) => {
    if (!firstPlot || !isConnected) return;

    const clampedTime = Math.max(0, Math.min(newTimeMs, firstPlot.duration));

    const elapsedLwReg = allModbusRegisters.find(r => r.group === firstPlot.name && r.name === SIGNAL_PLOT_REGISTER_NAMES.ELAPSED);
    const elapsedHwReg = allModbusRegisters.find(r => r.group === firstPlot.name && r.name === SIGNAL_PLOT_REGISTER_NAMES.ELAPSED);

    if (!elapsedLwReg || !elapsedHwReg) {
      toast.error("Elapsed time registers not found for the plot.");
      return;
    }

    const lw = clampedTime & 0xFFFF;
    const hw = (clampedTime >> 16) & 0xFFFF;

    try {
      // Update registers one by one.
      await updateRegister(elapsedLwReg.address, lw);
      await updateRegister(elapsedHwReg.address, hw);
    } catch (error) {
      toast.error(`Failed to update playback time: ${error instanceof Error ? error.message : String(error)}`);
    }
  };

  const handleDurationChange = (plotIndex: number, newTotalMilliseconds: number) => {
    setRawPlots(prevPlots =>
      prevPlots.map((plot, index) => {
        if (index === plotIndex) {
          setDirtyPlotSlots(prev => new Set(prev).add(plot.slot));
          return { ...plot, duration: Math.max(0, newTotalMilliseconds) };
        }
        return plot;
      })
    );
  };

  const handleSelectControlPoint = (plotSlot: number, cpId: number, focusList: boolean = false) => {
    setSelectedCpInfo({ plotSlot, cpId });

    if (focusList) {
      setTimeout(() => {
        const listEl = cpListRefs.current.get(plotSlot);
        if (listEl) {
          listEl.focus();
          const itemEl = listEl.querySelector(`[data-cp-id="${cpId}"]`) as HTMLLIElement;
          if (itemEl) itemEl.focus();
        }
      }, 0);
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
          setDirtyPlotSlots(prev => new Set(prev).add(plot.slot));
          return { ...plot, controlPoints: updatedControlPoints };
        }
        return plot;
      })
    );
  };
  const selectedCpData = getSelectedControlPointData();
  const playbackHeadPositionPercent = firstPlotDuration > 0 ? (playbackTimeMs / firstPlotDuration) * 100 : 0;

  const requestDeleteControlPoint = (plotSlot: number, cpId: number) => {
    setCpToDelete({ plotSlot, cpId });
    setShowDeleteConfirmDialog(true);
  };

  const handleDeleteControlPointConfirm = () => {
    if (!cpToDelete) return;

    setRawPlots(prevPlots =>
      prevPlots.map(plot => {
        if (plot.slot === cpToDelete.plotSlot) {
          const updatedControlPoints = plot.controlPoints.filter(cp => cp.id !== cpToDelete.cpId);
          if (selectedCpInfo && selectedCpInfo.plotSlot === cpToDelete.plotSlot && selectedCpInfo.cpId === cpToDelete.cpId) {
            setSelectedCpInfo(null);
          }
          setDirtyPlotSlots(prev => new Set(prev).add(plot.slot));
          return { ...plot, controlPoints: updatedControlPoints };
        }
        return plot;
      })
    );

    setShowDeleteConfirmDialog(false);
    setCpToDelete(null);
  };

  const openCreateControlPointDialog = (clickX: number, timelineScreenWidth: number, plotSlot: number, plotDuration: number) => {
    if (plotDuration <= 0) {
      toast.error(translate('Cannot add a control point to a plot with zero duration.'));
      return;
    }
    const timelineWidth = timelineScreenWidth - 20; // Account for padding
    if (timelineWidth <= 0) return;

    const clickPercent = Math.max(0, Math.min(1, clickX / timelineWidth));
    const newTimeScaled = Math.round(clickPercent * CONTROL_POINT_TIME_SCALE);

    setNewCpInitialData({ plotSlot, time: newTimeScaled, duration: plotDuration });
    setCreateCpDialogOpen(true);
  };

  const handleTimelineDoubleClick = (event: React.MouseEvent<HTMLDivElement>, plotSlot: number, plotDuration: number) => {
    const timelineRect = event.currentTarget.getBoundingClientRect();
    openCreateControlPointDialog(event.clientX - timelineRect.left, timelineRect.width, plotSlot, plotDuration);
  };

  const handleTimelineTap = (event: React.TouchEvent<HTMLDivElement>, plotSlot: number, plotDuration: number) => {
    const timelineRect = event.currentTarget.getBoundingClientRect();
    openCreateControlPointDialog(event.changedTouches[0].clientX - timelineRect.left, timelineRect.width, plotSlot, plotDuration);
  };

  const handleCreateControlPointConfirm = (newCpData: NewControlPointData) => {
    if (!newCpInitialData) return;
    const { plotSlot } = newCpInitialData;

    setRawPlots(prevPlots =>
      prevPlots.map(plot => {
        if (plot.slot === plotSlot) {
          const existingIds = new Set(plot.controlPoints.map(cp => cp.id));
          let newId = 1;
          while (existingIds.has(newId)) {
            newId++;
          }
          const newCp: SSignalControlPoint = {
            ...newCpData,
            id: newId,
            state: ESignalState.STATE_ON,
          };
          const updatedControlPoints = [...plot.controlPoints, newCp].sort((a, b) => a.time - b.time);

          // Automatically select the newly created point
          setTimeout(() => handleSelectControlPoint(plotSlot, newId, true), 0);

          setDirtyPlotSlots(prev => new Set(prev).add(plot.slot));
          return { ...plot, controlPoints: updatedControlPoints };
        }
        return plot;
      })
    );

    setCreateCpDialogOpen(false);
    setNewCpInitialData(null);
    toast.success("Control point created successfully.");
  };

  // Drag handlers
  const handleGlobalCpDragMouseMove = useCallback((event: MouseEvent | TouchEvent) => {
    if (!isDraggingCp || !draggingCpDetails) return;
    event.preventDefault();

    const { plotSlot, cpId, initialMouseX, initialCpTime, timelineScreenWidth } = draggingCpDetails;
    const currentMouseX = 'touches' in event ? event.touches[0].clientX : event.clientX;
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

  const handleGlobalCpDragMouseUp = useCallback((event: MouseEvent | TouchEvent) => {
    if (!isDraggingCp || !draggingCpDetails) return;
    event.preventDefault();

    let changedPlotData: SignalPlotData | undefined;
    setRawPlots(prevPlots =>
      prevPlots.map(p => {
        if (p.slot === draggingCpDetails.plotSlot) {
          const sortedControlPoints = [...p.controlPoints].sort((a, b) => a.time - b.time);
          changedPlotData = { ...p, controlPoints: sortedControlPoints };
          setDirtyPlotSlots(prev => new Set(prev).add(p.slot));
          return changedPlotData;
        }
        return p;
      })
    );

    setIsDraggingCp(false);
    setDraggingCpDetails(null);

    window.removeEventListener('mousemove', handleGlobalCpDragMouseMove);
    window.removeEventListener('mouseup', handleGlobalCpDragMouseUp);
    window.removeEventListener('touchmove', handleGlobalCpDragMouseMove as any);
    window.removeEventListener('touchend', handleGlobalCpDragMouseUp as any);
  }, [isDraggingCp, draggingCpDetails, handleGlobalCpDragMouseMove]);

  const handleCpMouseDown = (
    event: React.MouseEvent<HTMLDivElement>,
    plotSlot: number,
    cpId: number,
    currentCpTime: number,
    plotIndex: number
  ) => {
    event.preventDefault();
    event.stopPropagation();
    startDragging(event.clientX, plotSlot, cpId, currentCpTime, plotIndex);
  };

  const handleCpTouchStart = (
    event: React.TouchEvent<HTMLDivElement>,
    plotSlot: number,
    cpId: number,
    currentCpTime: number,
    plotIndex: number
  ) => {
    event.stopPropagation();
    startDragging(event.touches[0].clientX, plotSlot, cpId, currentCpTime, plotIndex);
  };

  const startDragging = (
    clientX: number,
    plotSlot: number,
    cpId: number,
    currentCpTime: number,
    plotIndex: number
  ) => {
    const timelineDiv = timelineRefs.current.get(plotSlot);
    if (!timelineDiv) return;

    const timelineScreenWidth = timelineDiv.clientWidth - 20;
    if (timelineScreenWidth <= 0) return;

    handleSelectControlPoint(plotSlot, cpId, true);
    setIsDraggingCp(true);
    setDraggingCpDetails({
      plotSlot,
      cpId,
      initialMouseX: clientX,
      initialCpTime: currentCpTime,
      timelineScreenWidth
    });

    window.addEventListener('mousemove', handleGlobalCpDragMouseMove);
    window.addEventListener('mouseup', handleGlobalCpDragMouseUp);
    window.addEventListener('touchmove', handleGlobalCpDragMouseMove as any);
    window.addEventListener('touchend', handleGlobalCpDragMouseUp as any);
  };

  // Plot Name Editing Handlers
  const handlePlotNameClick = (plotSlot: number, currentName: string) => {
    if (editingPlotNameSlot === plotSlot) return;
    setCurrentEditName(currentName);
    setEditingPlotNameSlot(plotSlot);
  };

  const handlePlotNameInputChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    setCurrentEditName(event.target.value);
  };

  const savePlotName = () => {
    if (editingPlotNameSlot === null) return;

    setRawPlots(prevPlots =>
      prevPlots.map(plot => {
        if (plot.slot === editingPlotNameSlot) {
          setDirtyPlotSlots(prev => new Set(prev).add(plot.slot));
          return { ...plot, name: currentEditName.trim() || translate('Plot {slot}').replace("{slot}", String(plot.slot)) };
        }
        return plot;
      })
    );

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

    const sortedCps = [...plot.controlPoints].sort((a, b) => a.time - b.time);

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

    setRawPlots(prevPlots =>
      prevPlots.map(plot => {
        if (plot.slot === plotToClearCps) {
          setDirtyPlotSlots(prev => new Set(prev).add(plot.slot));
          return { ...plot, controlPoints: [] };
        }
        return plot;
      })
    );

    if (selectedCpInfo && selectedCpInfo.plotSlot === plotToClearCps) {
      setSelectedCpInfo(null);
    }
    setShowClearAllConfirmDialog(false);
    setPlotToClearCps(null);
    toast.success(translate('All control points cleared for the plot.'));
  };

  const handleReorderControlPoint = (plotSlot: number, cpId: number, direction: 'up' | 'down') => {
    setRawPlots(prevPlots =>
      prevPlots.map(plot => {
        if (plot.slot === plotSlot) {
          const sortedCps = [...plot.controlPoints].sort((a, b) => a.time - b.time);
          const cpIndex = sortedCps.findIndex(cp => cp.id === cpId);

          if (cpIndex === -1) return plot;

          let newTime: number | undefined;

          if (direction === 'up' && cpIndex > 0) {
            const prevCp = sortedCps[cpIndex - 1];
            newTime = prevCp.time - 10;
          } else if (direction === 'down' && cpIndex < sortedCps.length - 1) {
            const nextCp = sortedCps[cpIndex + 1];
            newTime = nextCp.time + 10;
          } else {
            return plot;
          }

          // Clamp to bounds
          newTime = Math.max(0, Math.min(newTime, CONTROL_POINT_TIME_SCALE));

          const updatedControlPoints = plot.controlPoints.map(cp =>
            cp.id === cpId ? { ...cp, time: newTime } : cp
          );
          setDirtyPlotSlots(prev => new Set(prev).add(plot.slot));
          return { ...plot, controlPoints: updatedControlPoints };
        }
        return plot;
      })
    );
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

          const combinedCps = [...existingCps, ...newSampleCps].sort((a, b) => a.time - b.time);
          toast.success(translate('Added {count} sample control points.').replace('{count}', String(newSampleCps.length)));
          setDirtyPlotSlots(prev => new Set(prev).add(plot.slot));
          return { ...plot, controlPoints: combinedCps };
        }
        return plot;
      })
    );
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

  const handlePlaybackMarkerDragStart = (clientX: number) => {
    if (!firstPlot || firstPlotIsPlaying) return;
    const timelineDiv = timelineRefs.current.get(firstPlot.slot);
    if (!timelineDiv) return;

    setIsDraggingPlaybackMarker(true);
    timelineDragInfo.current = {
      width: timelineDiv.clientWidth - 20, // Usable width
      left: timelineDiv.getBoundingClientRect().left + 10,
    };

    window.addEventListener('mousemove', handlePlaybackMarkerDragMove);
    window.addEventListener('mouseup', handlePlaybackMarkerDragEnd);
    window.addEventListener('touchmove', handlePlaybackMarkerDragMove as any);
    window.addEventListener('touchend', handlePlaybackMarkerDragEnd as any);
  };

  const handlePlaybackMarkerDragMove = (event: MouseEvent | TouchEvent) => {
    if (!timelineDragInfo.current || !firstPlot) return;
    event.preventDefault();

    const { width, left } = timelineDragInfo.current;
    if (width <= 0) return;

    const clientX = 'touches' in event ? event.touches[0].clientX : event.clientX;
    const positionX = clientX - left;
    const progress = Math.max(0, Math.min(1, positionX / width));
    const newTimeMs = progress * firstPlot.duration;

    handleUpdatePlaybackTime(newTimeMs);
  };

  const handlePlaybackMarkerDragEnd = () => {
    setIsDraggingPlaybackMarker(false);
    timelineDragInfo.current = null;
    window.removeEventListener('mousemove', handlePlaybackMarkerDragMove);
    window.removeEventListener('mouseup', handlePlaybackMarkerDragEnd);
    window.removeEventListener('touchmove', handlePlaybackMarkerDragMove as any);
    window.removeEventListener('touchend', handlePlaybackMarkerDragEnd as any);
  };

  const handlePlaybackMarkerMouseDown = (event: React.MouseEvent<HTMLDivElement>) => {
    event.preventDefault();
    handlePlaybackMarkerDragStart(event.clientX);
  };

  const handlePlaybackMarkerTouchStart = (event: React.TouchEvent<HTMLDivElement>) => {
    event.preventDefault();
    handlePlaybackMarkerDragStart(event.touches[0].clientX);
  };

  return (
    <div className={isEmbedded ? "" : "p-4"} id={`signal-plot-editor-container${signalPlotId !== undefined ? `-${signalPlotId}` : ''}`}>
      <div className={isEmbedded ? "space-y-4" : "max-w-6xl mx-auto space-y-6"}>
        {!isEmbedded && (
          <div className="flex justify-between items-center mb-6">
            <h1 className="text-2xl font-bold accent-text"><T>Signal Plot Editor</T></h1>
            <div className="flex items-center space-x-2">
              <Button size="sm" onClick={handleDownloadAllPlotsJson} className="glass-button"><T>Download All JSON</T></Button>
              <Button size="sm" onClick={triggerGlobalUploadJson} className="glass-button"><T>Upload All JSON</T></Button>
              <Button asChild size="sm" className="glass-button">
                <a href="https://polymech.info/en/resources/cassandra/signalplots/" target="_blank" rel="noopener noreferrer">
                  <HelpCircle className="h-4 w-4 mr-2" />
                  <T>Help</T>
                </a>
              </Button>
              <input type="file" ref={globalPlotsFileInputRef} onChange={handleGlobalUploadJsonSelected} accept=".json" style={{ display: 'none' }} />
              <input type="file" ref={singlePlotFileInputRef} onChange={handleSinglePlotUploadSelected} accept=".json" style={{ display: 'none' }} />
            </div>
          </div>
        )}

        {isLoading ? (
          <Card className="glass-card"><CardContent className="pt-6 flex justify-center items-center h-24"><Loader2 className="h-8 w-8 animate-spin text-slate-500 dark:text-slate-400" /></CardContent></Card>
        ) : plots.length === 0 ? (
          <Card className="glass-card"><CardContent className="pt-6"><p className="text-slate-500 dark:text-slate-400 text-center"><T>No signal plots found.</T></p></CardContent></Card>
        ) : plots.map((plot, plotIndex) => {
          const isFirstPlot = plotIndex === 0;
          const isPlotRunningOrPaused = isFirstPlot && (plot.status === PlotStatus.RUNNING || plot.status === PlotStatus.PAUSED);

          const enableCoil = allModbusCoils.find(
            c => c.group === plot.name && c.name === SIGNAL_PLOT_REGISTER_NAMES.ENABLED
          );
          const timelineDiv = timelineRefs.current.get(plot.slot);
          const timelineScreenWidth = timelineDiv?.clientWidth ? timelineDiv.clientWidth - 20 : 0;
          const niceIntervalMs = calculateNiceTimeInterval(plot.duration, timelineScreenWidth);
          const numMarkers = plot.duration > 0 && niceIntervalMs > 0 ? Math.floor(plot.duration / niceIntervalMs) + 1 : 1;
          const timelineMarkerKey = `timeline-markers-${plot.slot}-${timelinesMeasured}`;
          const currentPlotHasDuration = plot.duration > 0;

          const firstEnabledPlotIndex = plots.findIndex(p => {
            const coil = allModbusCoils.find(c => c.group === p.name && c.name === SIGNAL_PLOT_REGISTER_NAMES.ENABLED);
            return !!coil?.value;
          });

          const initiallyOpen = (firstEnabledPlotIndex === -1 && plotIndex === 0) || (plotIndex === firstEnabledPlotIndex);

          const title = (
            <div className="flex items-center flex-grow min-w-0">
              {editingPlotNameSlot === plot.slot ? (
                <Input
                  ref={nameInputRef}
                  type="text"
                  value={currentEditName}
                  onChange={handlePlotNameInputChange}
                  onBlur={savePlotName}
                  onKeyDown={handlePlotNameKeyDown}
                  onClick={(e) => e.stopPropagation()}
                  className="text-lg font-semibold h-8 mr-2 flex-grow"
                />
              ) : (
                <span
                  onClick={(e) => { e.stopPropagation(); handlePlotNameClick(plot.slot, plot.name); }}
                  className={`text-indigo-600 dark:text-cyan-400 underline decoration-dotted decoration-indigo-600/70 dark:decoration-cyan-400/70 hover:decoration-solid cursor-pointer mr-2 flex-grow truncate`}
                  title={plot.name}
                >
                  {plot.name}
                </span>
              )}
              <span className="text-slate-500 dark:text-slate-400 text-nowrap">({translate('Slot:')} {plot.slot})</span>
            </div>
          );

          const headerContent = (
            <div className="ml-auto pl-2 flex-shrink-0">
              {enableCoil ? (
                <CoilSwitch
                  address={enableCoil.address}
                  value={enableCoil.value}
                  aria-label={translate('Enable plot {name}').replace('{name}', plot.name)}
                />
              ) : (
                <CoilSwitch address={-1} value={false} disabled={true} aria-label={translate('Enable control unavailable for {name}').replace('{name}', plot.name)} />
              )}
            </div>
          );

          return (
            <CollapsibleSection
              key={plot.slot}
              id={`signal-plot-collapsible-${plot.slot}`}
              storageKey={`signal-plot-collapsible-${plot.slot}`}
              title={title}
              headerContent={headerContent}
              initiallyOpen={initiallyOpen}
              asCard
              className={`w-full overflow-hidden glass-panel ${isFirstPlot ? 'border-emerald-400/50 ring-1 ring-emerald-400/30' : ''}`}
              headerClassName="pb-3 pt-4 glass-card"
              titleClassName="text-lg font-semibold"
              contentClassName="p-0"
              buttonClassName="text-slate-600 dark:text-white/80 hover:text-slate-800 dark:hover:text-white"
            >
              <CardContent className="pt-4 pb-4 space-y-4">
                <div className="space-y-4">
                  <div className="flex items-center space-x-2">
                    <Label htmlFor={`duration-timecode-${plot.slot}`} className="text-sm whitespace-nowrap self-center text-slate-600 dark:text-slate-300">
                      <T>Duration:</T>
                    </Label>
                    <TimeCodeEditor
                      totalMilliseconds={plot.duration}
                      onDurationChange={(newMs) => handleDurationChange(plotIndex, newMs)}
                      idPrefix={`plot-${plot.slot}-duration`}
                    />
                    <span className="text-sm text-slate-500 dark:text-slate-400 self-center">
                      ({translate('Total:')} {(plot.duration / 1000).toFixed(1)}s)
                    </span>
                  </div>
                  <div>
                    <h3 className="text-sm font-medium text-slate-600 dark:text-slate-300 mb-1.5"><T>Timeline:</T></h3>
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
                      onTimelineTap={handleTimelineTap}
                      onCpMouseDown={handleCpMouseDown}
                      onCpTouchStart={handleCpTouchStart}
                      onRequestDeleteControlPoint={requestDeleteControlPoint}
                      getActualTimeMs={getActualTimeMs}
                      numMarkers={numMarkers}
                      niceIntervalMs={niceIntervalMs}
                      formatMarkerLabel={formatMarkerLabel}
                      timelineMarkerKey={timelineMarkerKey}
                      CONTROL_POINT_TIME_SCALE={CONTROL_POINT_TIME_SCALE}
                      isPlaybackMarkerDraggable={isFirstPlot && !isPlotRunningOrPaused}
                      isDraggingPlaybackMarker={isDraggingPlaybackMarker}
                      onPlaybackMarkerMouseDown={handlePlaybackMarkerMouseDown}
                      onPlaybackMarkerTouchStart={handlePlaybackMarkerTouchStart}
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
                <div className="grid grid-cols-1 md:grid-cols-2 gap-6 pt-4 border-t border-slate-300/30 dark:border-white/10">
                  <div className="space-y-2">
                    <h3 className="text-sm font-medium text-slate-600 dark:text-slate-300"><T>Control Points</T> ({plot.controlPoints.length}):</h3>
                    <ControlPointList
                      ref={el => cpListRefs.current.set(plot.slot, el)}
                      plotSlot={plot.slot}
                      controlPoints={[...plot.controlPoints].sort((a, b) => a.time - b.time)}
                      selectedCpInfo={selectedCpInfo}
                      isFirstPlotAndPlaying={isPlotRunningOrPaused}
                      onSelectControlPoint={handleSelectControlPoint}
                      onRequestDeleteControlPoint={requestDeleteControlPoint}
                      onReorderControlPoint={handleReorderControlPoint}
                      onKeyDown={(e) => handleCpListKeyDown(e, plot.slot)}
                    />
                  </div>
                  <div className="space-y-2">
                    <h3 className="text-sm font-medium text-slate-600 dark:text-slate-300"><T>Properties:</T></h3>
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
                    ) : (<p className="text-xs text-slate-500 dark:text-slate-400 italic">{plot.controlPoints.length > 0 ? <T>Select a control point to see its properties.</T> : <T>No control points to select.</T>}</p>)}
                  </div>
                </div>
              </CardContent>
              <CardFooter className="flex justify-between items-center space-x-2 py-3 glass-card border-t border-slate-300/30 dark:border-white/10">
                {dirtyPlotSlots.has(plot.slot) ? (
                  <div className="flex items-center space-x-2">
                    <Button size="sm" onClick={() => handleSavePlotToApi(plot)} className="status-gradient-connected text-white border-0">
                      <SaveIcon className="h-4 w-4 mr-2" />
                      <T>Save Changes</T>
                    </Button>
                    <Button size="sm" onClick={() => handleRevertPlot(plot.slot)} className="glass-button">
                      <RotateCcw className="h-4 w-4 mr-2" />
                      <T>Revert</T>
                    </Button>
                  </div>
                ) : (
                  <div>
                    <Button
                      size="sm"
                      onClick={() => handleAddSampleControlPoints(plot.slot, plot.duration)}
                      disabled={plot.duration <= 0}
                      title={translate('Add a set of sample control points to this plot')}
                      className="mr-2 glass-button status-gradient-connected text-white border-0"
                    >
                      <PlusCircleIcon className="h-4 w-4 mr-2" />
                      <T>Add Samples</T>
                    </Button>
                    <Button
                      size="sm"
                      onClick={() => requestClearAllControlPoints(plot.slot)}
                      disabled={plot.controlPoints.length === 0}
                      title={translate('Remove all control points from this plot')}
                      className="glass-button status-gradient-error text-white border-0"
                    >
                      <Trash2Icon className="h-4 w-4 mr-2" />
                      <T>Clear All CPs</T>
                    </Button>
                  </div>
                )}
                <div>
                  <Button
                    size="sm"
                    onClick={() => handleDownloadSinglePlotJson(plot)}
                    disabled={dirtyPlotSlots.has(plot.slot)}
                    title={translate('Download JSON for {name}').replace('{name}', plot.name)}
                    className="mr-2 glass-button"
                  >
                    <T>Download Plot</T>
                  </Button>
                  <Button
                    size="sm"
                    onClick={() => triggerSinglePlotUpload(plot.slot)}
                    title={translate('Upload JSON for {name}').replace('{name}', plot.name)}
                    className="mr-2 glass-button"
                  >
                    <T>Upload Plot</T>
                  </Button>
                  <Button
                    size="sm"
                    onClick={() => openCopyToDialog(plot)}
                    title={translate('Copy this plot to another slot...')}
                    className="glass-button bg-gradient-to-r from-cyan-400 to-blue-500 text-white border-0"
                  >
                    <CopyPlus className="h-4 w-4 mr-2" />
                    <T>Copy to...</T>
                  </Button>
                </div>
              </CardFooter>
            </CollapsibleSection>
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
            <AlertDialogAction onClick={handleDeleteControlPointConfirm} className="status-gradient-error text-white border-0">
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
            <AlertDialogAction onClick={handleClearAllControlPointsConfirm} className="status-gradient-error text-white border-0">
              <T>Clear All</T>
            </AlertDialogAction>
          </AlertDialogFooter>
        </AlertDialogContent>
      </AlertDialog>

      {newCpInitialData && (
        <ControlPointDialog
          isOpen={isCreateCpDialogOpen}
          onClose={() => setCreateCpDialogOpen(false)}
          onConfirm={handleCreateControlPointConfirm}
          initialTimeScaled={newCpInitialData.time}
          plotDuration={newCpInitialData.duration}
          allModbusCoils={allModbusCoils}
          allModbusRegisters={allModbusRegisters}
          getActualTimeMs={getActualTimeMs}
        />
      )}

      <AlertDialog open={copyDialogOpen} onOpenChange={setCopyDialogOpen}>
        <AlertDialogContent>
          <AlertDialogHeader>
            <AlertDialogTitle>{translate('Copy "{plotName}" to...').replace('{plotName}', plotToCopy?.name || '')}</AlertDialogTitle>
            <AlertDialogDescription>
              {translate('Select a destination plot. The content of "{plotName}" will overwrite the selected plot. This action cannot be undone.').replace('{plotName}', plotToCopy?.name || '')}
            </AlertDialogDescription>
          </AlertDialogHeader>

          <div className="py-4">
            <Select onValueChange={setTargetPlotSlot} value={targetPlotSlot ?? undefined}>
              <SelectTrigger className="glass-input">
                <SelectValue placeholder={translate("Select a plot to overwrite")} />
              </SelectTrigger>
              <SelectContent className="glass-panel border-0">
                {rawPlots
                  .filter(p => p.slot !== plotToCopy?.slot)
                  .map(p => (
                    <SelectItem key={p.slot} value={String(p.slot)}>
                      {p.name || `${translate('Plot')} ${p.slot}`} ({translate('Slot')}: {p.slot})
                    </SelectItem>
                  ))
                }
              </SelectContent>
            </Select>
          </div>

          <AlertDialogFooter>
            <AlertDialogCancel><T>Cancel</T></AlertDialogCancel>
            <AlertDialogAction onClick={handleCopyToConfirm} disabled={targetPlotSlot === null} className="glass-button bg-gradient-to-r from-cyan-400 to-blue-500 text-white border-0">
              <T>Copy and Overwrite</T>
            </AlertDialogAction>
          </AlertDialogFooter>
        </AlertDialogContent>
      </AlertDialog>

    </div>
  );
};

export default SignalPlotEditor; 