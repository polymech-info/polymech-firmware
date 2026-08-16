import React, { useState, useEffect } from 'react';
import { useForm } from 'react-hook-form';
import { Button } from '@/components/ui/button';
import { Input } from '@/components/ui/input';
import {
  Form,
  FormControl,
  FormField,
  FormItem,
  FormLabel,
  FormMessage,
} from '@/components/ui/form';
import { Select, SelectContent, SelectItem, SelectTrigger, SelectValue } from '@/components/ui/select';
import BezierEditor from '@/components/profiles/bezier/BezierEditor';
import {
  ControlPoint,
  Profile as TemperatureProfile,
  PlotStatus,
  Controller,
  SignalPlotData,
  ProfileType
} from '@/types.js';
import { Download, Upload } from 'lucide-react';
import { toast } from '@/components/ui/use-toast';
import { useNavigate } from 'react-router-dom';

// Import necessary items
import { useModbus } from '@/contexts/ModbusContext';
import { RegisterData } from '@polymech/client-ts'; // Assuming RegisterData is needed
import modbusApiService from '@polymech/client-ts/modbusApiService'; // Added modbusApiService
import { getSlaveIdFromGroup } from '../../lib/controllerUtils';
import { SP_CMD_COMMAND_REGISTER_PREFIX } from '@/constants';
import { TimeCodeEditor } from '@/components/TimeCodeEditor';
import SignalPlotEditor from '@/components/SignalPlotEditor';
import MarkdownEditor from '@/components/MarkdownEditor';
import BezierControlPointList from '@/components/profiles/bezier/BezierControlPointList';

import { T, translate, getCurrentLang } from '../../i18n';

type FormData = {
  name: string;
  description: string;
  duration: number;
  targetRegisters?: number[];
  signalPlot?: number;
};

// Helper to transform UI (normalized 0-1) control points to service format (0-1000)
const transformUIControlPointsToService = (uiPoints: ControlPoint[]): ControlPoint[] => {
  const percentageMultiplier = 1000;
  return uiPoints.map(p => ({
    x: Math.round(p.x * percentageMultiplier), // Scale normalized x (0-1) to 0-1000
    y: Math.round(p.y * percentageMultiplier)  // Scale normalized y (0-1) to 0-1000
  }));
};

// Helper to transform service format (0-1000) control points to UI (normalized 0-1)
const transformServiceControlPointsToUI = (servicePoints: ControlPoint[]): ControlPoint[] => {
  const percentageDivisor = 1000;
  return servicePoints.map(p => ({
    x: Math.max(0, Math.min(1, (p.x || 0) / percentageDivisor)), // Ensure x is between 0 and 1
    y: Math.max(0, Math.min(1, (p.y || 0) / percentageDivisor))  // Ensure y is between 0 and 1
  }));
};

interface ProfileFormProps {
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

const ProfileForm: React.FC<ProfileFormProps> = ({
  onSubmit,
  initialData,
  max,
  availableControllers
}) => {
  const navigate = useNavigate();
  // Log props on render
  const [controlPoints, setControlPoints] = useState<ControlPoint[]>(
    initialData?.controlPoints || defaultControlPoints
  );
  const [tempRange, setTempRange] = useState({
    max: initialData?.max || max
  });
  const [selectedTargetRegisters, setSelectedTargetRegisters] = useState<number[]>([]);
  const [availableSignalPlots, setAvailableSignalPlots] = useState<SignalPlotData[]>([]);

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
        setAvailableSignalPlots(plots);
      } catch (error) {
        console.error("Failed to fetch signal plots:", error);
        toast({ title: translate("Error"), description: translate("Could not load signal plots for selection."), variant: "destructive" });
      }
    };
    fetchSignalPlots();
  }, []);

  const form = useForm<FormData>({
    defaultValues: {
      name: initialData?.name || '',
      description: initialData?.description || '',
      duration: initialData?.duration || 30 * 60 * 1000, // Default to 30 minutes in ms
      targetRegisters: [], // Default to empty, will be synced by useEffect below
      signalPlot: initialData?.signalPlot
    }
  });

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

  // Effect to update the form's "targetRegisters" field when selectedTargetRegisters state changes
  React.useEffect(() => {
    form.setValue('targetRegisters', selectedTargetRegisters);
  }, [selectedTargetRegisters, form]);

  const handleTempRangeChange = (newMaxTemp: number) => {
    setTempRange({
      max: newMaxTemp
    });
  };

  const handleClearControlPoints = () => {
    setControlPoints(clearedControlPointsState);
  };

  const handleExportJson = () => {
    const formData = form.getValues();
    const serviceControlPoints = transformUIControlPointsToService(controlPoints);

    const profileToExport = {
      name: formData.name,
      description: formData.description,
      duration: formData.duration, // duration is now directly from form in ms
      max: tempRange.max,
      controlPoints: serviceControlPoints, // 0-1000 scaled
      targetRegisters: selectedTargetRegisters,
      signalPlot: formData.signalPlot,
      // Optionally include other fields if they should be part of the export
      // targetRegisters: initialData?.targetRegisters || [], // Example
    };

    const jsonString = JSON.stringify(profileToExport, null, 2);
    const blob = new Blob([jsonString], { type: "application/json" });
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = `${formData.name || 'profile'}.json`;
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
          form.setValue('name', importedProfile.name);
        }
        if (typeof importedProfile.description === 'string') {
          form.setValue('description', importedProfile.description);
        }
        if (typeof importedProfile.duration === 'number') { // Expects duration in ms from import
          form.setValue('duration', importedProfile.duration); // Set duration in ms
        }
        if (typeof importedProfile.max === 'number') {
          setTempRange({ max: importedProfile.max });
          // Note: BezierEditor max prop will update via tempRange.max
        }
        if (Array.isArray(importedProfile.controlPoints)) {
          // Expects controlPoints in 0-1000 scaled format, convert to UI normalized 0-1
          const uiControlPoints = transformServiceControlPointsToUI(importedProfile.controlPoints);
          setControlPoints(uiControlPoints);
        }
        if (Array.isArray(importedProfile.targetRegisters)) { // Import targetRegisters
          setSelectedTargetRegisters(importedProfile.targetRegisters.filter((tr: any) => typeof tr === 'number'));
        }
        if (typeof importedProfile.signalPlot === 'number') {
          form.setValue('signalPlot', importedProfile.signalPlot);
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

  const handleSubmit = (data: FormData) => {
    const submitData: Omit<TemperatureProfile, 'id' | 'createdAt' | 'updatedAt'> = {
      name: data.name,
      description: data.description,
      duration: data.duration, // Use duration in ms directly from form
      controlPoints: controlPoints, // Pass UI-formatted control points
      max: tempRange.max,
      slot: initialData?.slot || 0,
      status: initialData?.status || PlotStatus.IDLE,
      targetRegisters: selectedTargetRegisters,
      enabled: initialData?.enabled || false,
      signalPlot: data.signalPlot,
      // Add missing properties with default values
      children: initialData?.children || [],
      type: initialData?.type || ProfileType.Temperature,
      elapsed: initialData?.elapsed || 0,
      remaining: initialData?.remaining || 0,
      currentTemp: initialData?.currentTemp || 0,
    };
    onSubmit(submitData)    
  }

  return (
    <Form {...form}>
      <form
        onSubmit={form.handleSubmit(handleSubmit)}
        onKeyDown={(e) => {
          if (e.key === 'Enter' && (e.target as HTMLElement).tagName.toUpperCase() === 'INPUT') {
            e.preventDefault();
          }
        }}
        className="space-y-6"
      >
        <FormField
          control={form.control}
          name="name"
          render={({ field }) => (
            <FormItem>
              <FormLabel><T>Profile Name</T></FormLabel>
              <FormControl>
                <Input placeholder={translate("E.g., Quick Ramp Up")} {...field} />
              </FormControl>
              <FormMessage />
            </FormItem>
          )}
        />


        <FormField
          control={form.control}
          name="description"
          render={({ field }) => (
            <FormItem>
              <FormLabel><T>Description</T></FormLabel>
              <FormControl>
                <MarkdownEditor
                  value={field.value}
                  onValueChange={(newValue) => {
                    form.setValue('description', newValue);
                  }}
                />
              </FormControl>
              <FormMessage />
            </FormItem>
          )}
        />

        <FormField
          control={form.control}
          name="duration"
          render={({ field }) => (
            <FormItem>
              <FormLabel><T>Duration (hh:mm:ss)</T></FormLabel>
              <FormControl>
                <TimeCodeEditor
                  totalMilliseconds={field.value}
                  onDurationChange={(newDuration) => {
                    field.onChange(newDuration);
                    // Optionally, trigger validation if needed or let Zod handle it on submit
                    form.trigger('duration'); // Trigger validation for duration
                  }}
                />
              </FormControl>
              <FormMessage />
            </FormItem>
          )}
        />

        <div className="space-y-2">
          <label className="text-sm font-medium">
            <T>Temperature Curve</T>
          </label>
          <BezierEditor
            controlPoints={controlPoints}
            onChange={setControlPoints}
            max={tempRange.max}
            duration={form.watch('duration')}
            onTempRangeChange={handleTempRangeChange}
            className="border rounded-md p-2"
          />
          <BezierControlPointList
            controlPoints={controlPoints}
            onChange={setControlPoints}
            duration={form.watch('duration')}
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


        {form.watch('signalPlot') !== undefined && (
          <div className="pt-6 mt-6">
            <FormField
              control={form.control}
              name="signalPlot"
              render={({ field }) => (
                <FormItem>
                  <FormLabel><h3 className="text-lg font-medium mb-4"><T>Associated Signal Plot (Optional)</T></h3></FormLabel>
                  <Select
                    onValueChange={(value) => field.onChange(value === "@none" ? undefined : parseInt(value, 10))}
                    defaultValue={field.value === undefined ? "@none" : field.value?.toString()}
                  >
                    <FormControl>
                      <SelectTrigger>
                        <SelectValue placeholder={translate("Select a signal plot")} />
                      </SelectTrigger>
                    </FormControl>
                    <SelectContent>
                      <SelectItem value="@none"><T>None</T></SelectItem>
                      {availableSignalPlots.map((plot) => (
                        <SelectItem key={plot.slot} value={plot.slot.toString()}>
                          {plot.name} ({translate("Slot:")} {plot.slot})
                        </SelectItem>
                      ))}
                    </SelectContent>
                  </Select>
                  <FormMessage />
                </FormItem>
              )}
            />
            <div className="border-t mt-6">
              <SignalPlotEditor signalPlotId={form.watch('signalPlot')} isEmbedded={true} />
            </div>
          </div>
        )}

        <div className="space-y-2">
          <div className="flex justify-between items-center mb-2">
            <FormLabel><T>Target Controllers (Registers)</T></FormLabel>
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

        <div className="flex gap-2 mt-6">
          <Button type="button" variant="outline" onClick={handleExportJson} className="w-1/2">
            <Download className="mr-2 h-4 w-4" />
            <T>Export JSON</T>
          </Button>
          <input
            type="file"
            accept=".json"
            ref={fileInputRef}
            onChange={handleImportJson}
            style={{ display: 'none' }}
          />
          <Button type="button" variant="outline" onClick={triggerFileSelect} className="w-1/2">
            <Upload className="mr-2 h-4 w-4" />
            <T>Import JSON</T>
          </Button>
        </div>

        <Button type="submit" className="w-full mt-2">
          {initialData ? <T>Update Profile</T> : <T>Create Profile</T>}
        </Button>


      </form>
    </Form>
  );
};

export default ProfileForm;
