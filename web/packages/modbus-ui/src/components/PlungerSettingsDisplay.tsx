import React, { useState, useEffect, FormEvent, useCallback } from 'react';
import modbusApiService, { PlungerSettingsResponse, PlungerSettingsUpdatePayload } from '@polymech/client-ts/modbusApiService';
import { Input } from '@/components/ui/input';
import { Button } from '@/components/ui/button';
import { Label } from '@/components/ui/label';
import { Checkbox } from '@/components/ui/checkbox';
import { Textarea } from '@/components/ui/textarea';
import { Slider } from '@/components/ui/slider';
import { toast } from 'sonner';
import { RotateCcw } from 'lucide-react';
import { T } from '../i18n';

const initialPlungerSettingsState: PlungerSettingsUpdatePayload = {
    speedSlowHz: 0,
    speedMediumHz: 0,
    speedFastHz: 0,
    speedFillPlungeHz: 0,
    speedFillHomeHz: 0,
    currentJamThresholdMa: 0,
    jammedDurationHomingMs: 0,
    jammedDurationMs: 0,
    autoModeHoldDurationMs: 0,
    maxUniversalJamTimeMs: 0,
    fillJoystickHoldDurationMs: 0,
    fillPlungedWaitDurationMs: 0,
    fillHomedWaitDurationMs: 0,
    recordHoldDurationMs: 0,
    maxRecordDurationMs: 0,
    replayDurationMs: 0,
    enablePostFlow: false,
    postFlowDurationMs: 0,
    postFlowSpeedHz: 0,
    currentPostFlowMa: 0,
    postFlowStoppingWaitMs: 0,
    postFlowCompleteWaitMs: 0,
    defaultMaxOperationDurationMs: 0,
};

interface SettingField {
    name: keyof PlungerSettingsUpdatePayload;
    label: string;
    type?: string;
    min?: number;
    max?: number;
    step?: number;
    unit?: string;
}

const PlungerSettingsDisplay = () => {
  const [settings, setSettings] = useState<PlungerSettingsUpdatePayload>(initialPlungerSettingsState);
  const [loading, setLoading] = useState<boolean>(true);
  const [error, setError] = useState<string | null>(null);
  const [isSaving, setIsSaving] = useState<boolean>(false);
  const [isRestoringDefaults, setIsRestoringDefaults] = useState<boolean>(false);
  const [jsonInput, setJsonInput] = useState<string>('');

  const fetchSettings = useCallback(async () => {
    try {
      setLoading(true);
      setError(null);
      const currentSettings = await modbusApiService.getPlungerSettings();
      setSettings({ ...initialPlungerSettingsState, ...currentSettings });
    } catch (err) {
      console.error("Failed to fetch plunger settings:", err);
      const errorMessage = err instanceof Error ? err.message : 'An unknown error occurred';
      setError(errorMessage);
      toast.error(<T>Failed to load plunger settings: {errorMessage}</T>);
    } finally {
      setLoading(false);
    }
  }, []);

  useEffect(() => {
    fetchSettings();
  }, [fetchSettings]);

  const handleChange = (e: React.ChangeEvent<HTMLInputElement>) => {
    const { name, value, type } = e.target;
    if (type !== 'checkbox') {
        const newValue = type === 'number' ? parseFloat(value) : value;
        setSettings(prevSettings => ({
            ...prevSettings,
            [name]: newValue,
        }));
    }
  };
  
  const handleCheckboxChange = (name: keyof PlungerSettingsUpdatePayload, checked: boolean) => {
    setSettings(prevSettings => ({
      ...prevSettings,
      [name]: checked,
    }));
  };

  const handleSliderChange = (name: keyof PlungerSettingsUpdatePayload, value: number[]) => {
    setSettings(prevSettings => ({
        ...prevSettings,
        [name]: value[0],
    }));
  };

  const handleSubmit = async (e: FormEvent<HTMLFormElement>) => {
    e.preventDefault();
    setIsSaving(true);
    setError(null);
    try {
      await modbusApiService.setPlungerSettings(settings);
      toast.success(<T>Plunger settings updated successfully!</T>);
    } catch (err) {
      console.error("Failed to save plunger settings:", err);
      const errorMessage = err instanceof Error ? err.message : 'An unknown error occurred while saving';
      setError(errorMessage);
      toast.error(<T>Failed to save plunger settings: {errorMessage}</T>);
    } finally {
      setIsSaving(false);
    }
  };

  const handleDownloadJson = () => {
    const jsonString = JSON.stringify(settings, null, 2);
    const blob = new Blob([jsonString], { type: 'application/json' });
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = 'plungerSettings.json';
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);
    URL.revokeObjectURL(url);
    toast.info(<T>Plunger settings JSON download initiated.</T>);
  };

  const handleLoadJson = () => {
    if (!jsonInput.trim()) {
      toast.error(<T>JSON input is empty.</T>);
      return;
    }
    try {
      const parsedJson = JSON.parse(jsonInput);
      const knownKeys: (keyof PlungerSettingsUpdatePayload)[] = ['speedSlowHz', 'enablePostFlow', 'currentJamThresholdMa'];
      const hasKnownKey = knownKeys.some(key => key in parsedJson);

      if (typeof parsedJson === 'object' && parsedJson !== null && hasKnownKey) {
        const newSettings: PlungerSettingsUpdatePayload = { 
            ...initialPlungerSettingsState, 
            ...parsedJson 
        };
        setSettings(newSettings);
        toast.success(<T>Plunger settings loaded from JSON!</T>);
        setJsonInput('');
      } else {
        toast.error(<T>Invalid or unrecognized JSON structure for Plunger Settings.</T>);
      }
    } catch (parseError) {
      console.error("Failed to parse JSON:", parseError);
      toast.error(<T>Failed to parse JSON. Please check the format.</T>);
    }
  };

  const handleLoadDefaults = async () => {
    setIsRestoringDefaults(true);
    setError(null);
    try {
      await modbusApiService.loadPlungerDefaults();
      toast.success(<T>Factory defaults restored to device.</T>);
      await fetchSettings();
      toast.info(<T>Settings reloaded from device.</T>);
    } catch (err) {
      console.error("Failed to load plunger defaults:", err);
      const errorMessage = err instanceof Error ? err.message : 'An unknown error occurred';
      setError(errorMessage);
      toast.error(<T>Failed to load defaults: {errorMessage}</T>);
    } finally {
      setIsRestoringDefaults(false);
    }
  };

  if (loading) {
    return <div className="p-4"><T>Loading plunger settings...</T></div>;
  }

  const renderInputField = (field: SettingField) => {
    const { name, label, type = 'number', min = 0, max, step = 1, unit = '' } = field;
    const currentValue = settings[name];
    const translatedLabel = <T>{label}</T>;

    return (
      <div key={name} className="flex flex-col space-y-1 mb-3">
        {type === 'checkbox' ? (
          <div className="flex items-center space-x-2 mt-2">
            <Checkbox 
                id={name} 
                name={name} 
                checked={Boolean(currentValue)} 
                onCheckedChange={(checkedState) => handleCheckboxChange(name, Boolean(checkedState))}
            />
            <Label htmlFor={name} className="text-sm font-medium cursor-pointer">{translatedLabel}</Label>
          </div>
        ) : type === 'slider' ? (
          <div className="space-y-2">
            <div className="flex justify-between items-center">
                <Label htmlFor={name} className="text-sm font-medium">{translatedLabel}</Label>
                <span className="text-sm text-muted-foreground">
                    {typeof currentValue === 'number' ? currentValue.toLocaleString() : 'N/A'} {unit}
                </span>
            </div>
            <Slider 
                id={name}
                min={min}
                max={max ?? 30000}
                step={step ?? 100}
                value={typeof currentValue === 'number' ? [currentValue] : [min || 0]}
                onValueChange={(value) => handleSliderChange(name, value)}
                className="w-full"
            />
          </div>
        ) : (
          <>
            <Label htmlFor={name} className="text-sm font-medium">{translatedLabel} {unit && `(${unit})`}</Label>
            <Input 
                id={name} 
                name={name} 
                type={type} 
                value={currentValue === undefined || currentValue === null ? '' : String(currentValue)} 
                onChange={handleChange} 
                className="mt-1 w-full"
                placeholder={`Enter ${label}`}
                min={min}
                max={max}
                step={step}
            />
          </>
        )}
      </div>
    );
  };

  const speedSettingsFields: SettingField[] = [
    { name: 'speedSlowHz', label: 'Slow', unit: 'Hz' },
    { name: 'speedMediumHz', label: 'Medium', unit: 'Hz' },
    { name: 'speedFastHz', label: 'Fast', unit: 'Hz' },
    { name: 'speedFillPlungeHz', label: 'Fill Plunge', unit: 'Hz' },
    { name: 'speedFillHomeHz', label: 'Fill Home', unit: 'Hz' },
  ];

  const timingSettingsFields: SettingField[] = [
    { name: 'jammedDurationHomingMs', label: 'Jammed - Homing', unit: 'ms' },
    { name: 'jammedDurationMs', label: 'Jammed - Operation', unit: 'ms' },
    { name: 'autoModeHoldDurationMs', label: 'Auto Mode Hold', unit: 'ms' },
    { name: 'maxUniversalJamTimeMs', label: 'Max Universal Jam', unit: 'ms' },
    { name: 'fillJoystickHoldDurationMs', label: 'Fill Joystick Hold', unit: 'ms' },
    { name: 'fillPlungedWaitDurationMs', label: 'Fill Plunged Wait', unit: 'ms' },
    { name: 'fillHomedWaitDurationMs', label: 'Fill Homed Wait', unit: 'ms' },
    { name: 'recordHoldDurationMs', label: 'Record Hold', unit: 'ms' },
    { name: 'maxRecordDurationMs', label: 'Max Record', unit: 'ms', type: 'slider', min: 1000, max: 60000, step: 1000 },
    { name: 'replayDurationMs', label: 'Replay', unit: 'ms', type: 'slider', min: 500, max: 30000, step: 100 },
    { name: 'defaultMaxOperationDurationMs', label: 'Max Operation', unit: 'ms' },
  ];

  const postFlowSettingsFields: SettingField[] = [
    { name: 'enablePostFlow', label: 'Enable Post Flow', type: 'checkbox' },
    { name: 'postFlowDurationMs', label: 'Duration', unit: 'ms', type: 'slider', min: 0, max: 10000, step: 100 },
    { name: 'postFlowSpeedHz', label: 'Speed', unit: 'Hz' },
    { name: 'postFlowStoppingWaitMs', label: 'Stopping Wait', unit: 'ms' },
    { name: 'postFlowCompleteWaitMs', label: 'Complete Wait', unit: 'ms' },
  ];
  
  const currentSettingsFields: SettingField[] = [
    { name: 'currentJamThresholdMa', label: 'Jam Threshold', unit: 'mA', type: 'slider', min:100, max: 2000, step: 50 },
    { name: 'currentPostFlowMa', label: 'Post Flow', unit: 'mA', type:'slider', min:100, max:2000, step:50 },
  ];

  return (
    <form onSubmit={handleSubmit} className="p-4 border rounded-md shadow-sm bg-card text-card-foreground space-y-6">
      <div className="flex justify-between items-center">
        <h3 className="text-lg font-semibold"><T>Plunger Settings</T></h3>
        <div className="flex space-x-2">
          <Button type="button" variant="outline" onClick={handleDownloadJson} size="sm" disabled={loading || isSaving || isRestoringDefaults}>
            <T>Download JSON</T>
          </Button>
          <Button type="button" variant="outline" onClick={handleLoadDefaults} size="sm" disabled={loading || isSaving || isRestoringDefaults}>
            <RotateCcw className="mr-2 h-4 w-4" /> <T>Load Defaults</T>
          </Button>
          <Button type="submit" disabled={isSaving || loading || isRestoringDefaults} size="sm">
            {isSaving ? <T>Saving...</T> : <T>Save Plunger Settings</T>}
          </Button>
        </div>
      </div>
      
      {error && <p className="text-red-500 bg-red-100 p-3 rounded-md mb-4"><T>Error</T>: {error}</p>}
      
      <div className="space-y-2 p-3 border rounded-md bg-muted/20">
        <Label htmlFor="jsonPasteArea" className="text-sm font-medium"><T>Load Settings from JSON</T></Label>
        <Textarea 
          id="jsonPasteArea"
          placeholder='Paste Plunger Settings JSON here...'
          value={jsonInput}
          onChange={(e) => setJsonInput(e.target.value)}
          rows={5}
          className="text-xs"
        />
        <Button type="button" variant="secondary" onClick={handleLoadJson} size="sm">
          <T>Load from JSON</T>
        </Button>
      </div>
      
      <div className="grid grid-cols-1 md:grid-cols-2 gap-x-6 gap-y-3">
        <div className="space-y-1 p-3 border rounded-md">
          <h4 className="text-md font-medium text-primary mb-3"><T>Speeds</T></h4>
          {speedSettingsFields.map(field => renderInputField(field))}
        </div>

        <div className="space-y-1 p-3 border rounded-md">
          <h4 className="text-md font-medium text-primary mb-3"><T>Current Thresholds</T></h4>
          {currentSettingsFields.map(field => renderInputField(field))}
        </div>
        
        <div className="space-y-1 p-3 border rounded-md md:col-span-2">
          <h4 className="text-md font-medium text-primary mb-3"><T>Post-Flow Configuration</T></h4>
          <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-x-6 gap-y-0">
            {postFlowSettingsFields.map(field => renderInputField(field))}
          </div>
        </div>

        <div className="space-y-1 p-3 border rounded-md md:col-span-2">
          <h4 className="text-md font-medium text-primary mb-3"><T>Timings & Durations</T></h4>
          <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-x-6 gap-y-0">
            {timingSettingsFields.map(field => renderInputField(field))}
          </div>
        </div>
      </div>
    </form>
  );
};

export default PlungerSettingsDisplay; 