import React, { useState, useEffect, FormEvent, useMemo, useRef } from 'react';
import modbusApiService from '@polymech/client-ts/modbusApiService';
import { useModbus } from '../contexts/ModbusContext';
import { Settings, PartitionConfig, ControllerConfig, Setting } from '@polymech/client-ts';
import { Input } from '@/components/ui/input';
import { Button } from '@/components/ui/button';
import { Label } from '@/components/ui/label';
import { Switch } from '@/components/ui/switch';
import { toast } from 'sonner';
import { T } from '../i18n';
import { Card, CardContent, CardHeader } from '@/components/ui/card';
import { Trash2, PlusCircle, HelpCircle } from 'lucide-react';
import CollapsibleSection from './CollapsibleSection';
import PlungerSettingsDisplay from './PlungerSettingsDisplay';

const humanizeSettingName = (name: string) => {
  if (!name) return '';
  return name
    .split('_')
    .map((word) => word.charAt(0).toUpperCase() + word.slice(1).toLowerCase())
    .join(' ');
};

const SettingsDisplay = () => {
  const { featureFlags, serverSettings } = useModbus();
  const [settings, setSettings] = useState<Settings | null>(null);
  const [loading, setLoading] = useState<boolean>(true);
  const [error, setError] = useState<string | null>(null);
  const [isSaving, setIsSaving] = useState<boolean>(false);
  const fileInputRef = useRef<HTMLInputElement>(null);

  useEffect(() => {
    const fetchSettings = async () => {
      try {
        setLoading(true);
        setError(null);
        const currentSettings = await modbusApiService.getSettings();
        setSettings(currentSettings);
      } catch (err) {
        console.error("Failed to fetch cassandra settings:", err);
        const errorMessage = err instanceof Error ? err.message : 'An unknown error occurred';
        setError(errorMessage);
        toast.error(<T>Failed to load Cassandra settings: {errorMessage}</T>);
      } finally {
        setLoading(false);
      }
    };

    fetchSettings();
  }, []);

  const handleMasterChange = (e: React.ChangeEvent<HTMLInputElement>) => {
    if (!settings) return;
    setSettings({ ...settings, master: e.target.value });
  };

  const handleSlaveChange = (index: number, value: string) => {
    if (!settings) return;
    const newSlaves = [...settings.slaves];
    newSlaves[index] = value;
    setSettings({ ...settings, slaves: newSlaves });
  };

  const addSlave = () => {
    if (!settings || settings.slaves.length >= 4) return;
    setSettings({ ...settings, slaves: [...settings.slaves, ""] });
  };

  const removeSlave = (index: number) => {
    if (!settings) return;
    const newSlaves = settings.slaves.filter((_, i) => i !== index);
    setSettings({ ...settings, slaves: newSlaves });
  };

  const handleSettingChange = (name: string, value: string | boolean | number) => {
    setSettings(currentSettings => {
      if (!currentSettings?.settings) return currentSettings;

      const newSettings = currentSettings.settings.map(s =>
        s.name === name ? { ...s, value } : s
      );

      return { ...currentSettings, settings: newSettings };
    });
  };

  const handlePartitionChange = (partitionIndex: number, field: keyof PartitionConfig, value: any) => {
    if (!settings) return;
    const newPartitions = [...settings.partitions];
    (newPartitions[partitionIndex] as any)[field] = value;
    setSettings({ ...settings, partitions: newPartitions });
  };

  const handleControllerChange = (partitionIndex: number, controllerIndex: number, field: keyof ControllerConfig, value: any) => {
    if (!settings) return;
    const newPartitions = [...settings.partitions];
    const newControllers = [...newPartitions[partitionIndex].controllers];

    let processedValue = value;
    if (field === 'slaveid') {
      processedValue = parseInt(value, 10);
      if (isNaN(processedValue)) {
        processedValue = 0; // or handle error
      }
    }

    (newControllers[controllerIndex] as any)[field] = processedValue;
    newPartitions[partitionIndex].controllers = newControllers;
    setSettings({ ...settings, partitions: newPartitions });
  };

  const addPartition = () => {
    if (!settings) return;
    const newPartition: PartitionConfig = {
      name: "New Partition",
      controllers: [],
    };
    setSettings({ ...settings, partitions: [...settings.partitions, newPartition] });
  };

  const removePartition = (partitionIndex: number) => {
    if (!settings) return;
    const newPartitions = settings.partitions.filter((_, index) => index !== partitionIndex);
    setSettings({ ...settings, partitions: newPartitions });
  };

  const addController = (partitionIndex: number) => {
    if (!settings) return;
    const newController: ControllerConfig = {
      name: "New Controller",
      slaveid: 0,
      enabled: true
    };
    const newPartitions = [...settings.partitions];
    newPartitions[partitionIndex].controllers.push(newController);
    setSettings({ ...settings, partitions: newPartitions });
  };

  const removeController = (partitionIndex: number, controllerIndex: number) => {
    if (!settings) return;
    const newPartitions = [...settings.partitions];
    const newControllers = newPartitions[partitionIndex].controllers.filter((_, index) => index !== controllerIndex);
    newPartitions[partitionIndex].controllers = newControllers;
    setSettings({ ...settings, partitions: newPartitions });
  };

  const handleSubmit = async (e: FormEvent<HTMLFormElement>) => {
    e.preventDefault();
    if (!settings) return;

    setIsSaving(true);
    setError(null);
    try {
      await modbusApiService.setSettings(settings);
      toast.success(<T>Cassandra settings updated successfully!</T>);
    } catch (err) {
      console.error("Failed to save cassandra settings:", err);
      const errorMessage = err instanceof Error ? err.message : 'An unknown error occurred while saving';
      setError(errorMessage);
      toast.error(<T>Failed to save Cassandra settings: {errorMessage}</T>);
    } finally {
      setIsSaving(false);
    }
  };

  const handleExport = () => {
    if (!settings) {
      toast.error(<T>No settings to export.</T>);
      return;
    }
    const jsonString = `data:text/json;charset=utf-8,${encodeURIComponent(
      JSON.stringify(settings, null, 2)
    )}`;
    const link = document.createElement("a");
    link.href = jsonString;
    link.download = "cassandra-settings.json";
    link.click();
    toast.success(<T>Settings exported successfully!</T>);
  };

  const handleImportClick = () => {
    fileInputRef.current?.click();
  };

  const handleFileChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    const file = event.target.files?.[0];
    if (!file) return;

    const reader = new FileReader();
    reader.onload = (e) => {
      try {
        const text = e.target?.result;
        if (typeof text === 'string') {
          const importedSettings = JSON.parse(text);
          // TODO: Add validation with Zod schema here
          setSettings(importedSettings);
          toast.success(<T>Settings imported successfully!</T>);
        }
      } catch (err) {
        console.error("Failed to import settings:", err);
        const errorMessage = err instanceof Error ? err.message : 'An unknown error occurred';
        toast.error(<T>Failed to import settings: {errorMessage}</T>);
      }
    };
    reader.readAsText(file);
    // Reset file input value to allow re-uploading the same file
    event.target.value = '';
  };

  const groupedSettings = useMemo(() => {
    if (!settings?.settings) return {};
    return settings.settings.reduce((acc, setting) => {
      const group = setting.group || 'general';
      if (!acc[group]) {
        acc[group] = [];
      }
      acc[group].push(setting);
      return acc;
    }, {} as Record<string, Setting[]>);
  }, [settings?.settings]);

  if (loading) return <p><T>Loading Cassandra settings...</T></p>;
  if (error) return <p className="text-red-500 bg-red-100 p-3 rounded-md"><T>Error</T>: {error}</p>;
  if (!settings) return <p><T>No settings data found.</T></p>;

  return (
    <>
      <form id="cassandra-settings-display" onSubmit={handleSubmit} className="space-y-4 glass-panel p-4 rounded-xl">
        {settings.settings && (
          <CollapsibleSection
            title={<T>General Settings</T>}
            initiallyOpen={true}
            storageKey="settings-general-collapsible"
            className="glass-panel"
            titleClassName="glass-text"
            headerClassName="flex justify-between items-center p-2 rounded-t-lg"
            contentClassName="p-2 glass-card rounded-b-lg"
            buttonClassName="text-slate-600 dark:text-white/80 hover:text-slate-800 dark:hover:text-white"
          >
            <div className="space-y-2">
              {Object.entries(groupedSettings).map(([group, groupSettings]) => (
                <CollapsibleSection
                  key={group}
                  title={group.charAt(0).toUpperCase() + group.slice(1)}
                  initiallyOpen={true}
                  storageKey={`settings-group-${group}-collapsible`}
                  className="border border-slate-300/30 dark:border-white/10 rounded-lg bg-white/5 dark:bg-black/5"
                  titleClassName="text-slate-700 dark:text-white"
                  headerClassName="flex justify-between items-center p-2 rounded-t-lg"
                  contentClassName="p-2"
                  buttonClassName="text-slate-600 dark:text-white/80 hover:text-slate-800 dark:hover:text-white"
                >
                  <div className="space-y-4 pt-2">
                    {groupSettings.map((setting) => (
                      <div key={`${group}-${setting.name}`} className="flex items-center justify-between">
                        <Label htmlFor={`setting-${setting.name}`} className="text-slate-700 dark:text-white">{humanizeSettingName(setting.name)}</Label>
                        <div className="w-1/2">
                          {setting.type === 'bool' && (
                            <Switch
                              id={`setting-${setting.name}`}
                              checked={Boolean(setting.value)}
                              onCheckedChange={(checked) => handleSettingChange(setting.name, checked)}
                              className="data-[state=checked]:bg-emerald-500"
                            />
                          )}
                          {setting.type === 'long' && (
                            <Input
                              id={`setting-${setting.name}`}
                              type="number"
                              value={Number(setting.value)}
                              onChange={(e) => handleSettingChange(setting.name, parseInt(e.target.value, 10))}
                              className="glass-input"
                            />
                          )}
                        </div>
                      </div>
                    ))}
                  </div>
                </CollapsibleSection>
              ))}
            </div>
          </CollapsibleSection>
        )}
        <CollapsibleSection
          title={<T>Master Configuration</T>}
          initiallyOpen={true}
          storageKey="settings-master-config-collapsible"
          className="glass-panel"
          titleClassName="glass-text"
          headerClassName="flex justify-between items-center p-2 rounded-t-lg"
          contentClassName="p-2 glass-card rounded-b-lg"
          buttonClassName="text-slate-600 dark:text-white/80 hover:text-slate-800 dark:hover:text-white"
        >
          <div className="grid gap-2">
            <Label htmlFor="master-name" className="text-slate-700 dark:text-white"><T>Master Name</T></Label>
            <Input id="master-name" value={settings.master} onChange={handleMasterChange} className="glass-input" />
          </div>
        </CollapsibleSection>

        <CollapsibleSection
          title={<T>Slaves</T>}
          initiallyOpen={true}
          storageKey="settings-slaves-collapsible"
          className="glass-panel"
          titleClassName="glass-text"
          headerClassName="flex justify-between items-center p-2 rounded-t-lg"
          contentClassName="p-2 glass-card rounded-b-lg"
          buttonClassName="text-slate-600 dark:text-white/80 hover:text-slate-800 dark:hover:text-white"
        >
          <p className="text-sm text-slate-500 dark:text-slate-400 -mt-2 mb-4"><T>Manage slave devices (max 1).</T></p>
          <div className="space-y-2">
            {settings.slaves.map((slave, index) => (
              <div key={index} className="flex items-center gap-2">
                <Input
                  value={slave}
                  onChange={(e) => handleSlaveChange(index, e.target.value)}
                  placeholder="Enter slave name or hostname"
                  className="glass-input"
                />
                <Button size="icon" onClick={() => removeSlave(index)} className="glass-button status-gradient-error text-white border-0">
                  <Trash2 className="h-4 w-4" />
                </Button>
              </div>
            ))}
            <Button
              onClick={addSlave}
              disabled={settings.slaves.length >= 4}
              className="mt-2 glass-button status-gradient-connected text-white border-0"
            >
              <PlusCircle className="h-4 w-4 mr-2" />
              <T>Add Slave</T>
            </Button>
            {settings.slaves.length >= 4 && (
              <p className="text-xs text-slate-500 dark:text-slate-400 pt-2">
                <T>Maximum number of slaves reached.</T>
              </p>
            )}
          </div>
        </CollapsibleSection>

        {featureFlags.ENABLE_OMRON_E5 && (
          <CollapsibleSection
            title={<T>Partitions</T>}
            initiallyOpen={true}
            storageKey="settings-partitions-collapsible"
            className="glass-panel"
            titleClassName="glass-text"
            headerClassName="flex justify-between items-center p-2 rounded-t-lg"
            contentClassName="p-2 glass-card rounded-b-lg"
            buttonClassName="text-slate-600 dark:text-white/80 hover:text-slate-800 dark:hover:text-white"
          >
            <p className="text-sm text-slate-500 dark:text-slate-400 -mt-2 mb-4"><T>Manage controller partitions.</T></p>
            <div className="space-y-4">
              {settings.partitions.map((partition, pIndex) => (
                <Card key={pIndex} className="glass-card">
                  <CardHeader className="flex flex-row items-center justify-between pb-2">
                    <div className="flex-grow">
                      <Input
                        value={partition.name}
                        onChange={(e) => handlePartitionChange(pIndex, 'name', e.target.value)}
                        className="text-lg font-semibold glass-input"
                      />
                    </div>
                    <Button size="icon" onClick={() => removePartition(pIndex)} className="glass-button status-gradient-error text-white border-0">
                      <Trash2 className="h-4 w-4" />
                    </Button>
                  </CardHeader>
                  <CardContent className="space-y-4">
                    {partition.controllers.map((controller, cIndex) => (
                      <div key={cIndex} className="flex items-center gap-4 p-2 border border-slate-300/30 dark:border-white/10 rounded-lg bg-white/5 dark:bg-black/5">
                        <div className="grid grid-cols-3 gap-4 flex-grow">
                          <div>
                            <Label className="text-slate-700 dark:text-white"><T>Name</T></Label>
                            <Input
                              value={controller.name}
                              onChange={(e) => handleControllerChange(pIndex, cIndex, 'name', e.target.value)}
                              className="glass-input"
                            />
                          </div>
                          <div>
                            <Label className="text-slate-700 dark:text-white"><T>Slave ID</T></Label>
                            <Input
                              type="number"
                              value={controller.slaveid}
                              onChange={(e) => handleControllerChange(pIndex, cIndex, 'slaveid', e.target.value)}
                              className="glass-input"
                            />
                          </div>
                          <div className="flex flex-col justify-end">
                            <div className="flex items-center space-x-2">
                              <Switch
                                checked={controller.enabled}
                                onCheckedChange={(checked) => handleControllerChange(pIndex, cIndex, 'enabled', checked)}
                                id={`enabled-switch-${pIndex}-${cIndex}`}
                                className="data-[state=checked]:bg-emerald-500"
                              />
                              <Label htmlFor={`enabled-switch-${pIndex}-${cIndex}`} className="text-slate-700 dark:text-white"><T>Enabled</T></Label>
                            </div>
                          </div>
                        </div>
                        <Button size="icon" onClick={() => removeController(pIndex, cIndex)} className="glass-button status-gradient-error text-white border-0">
                          <Trash2 className="h-4 w-4" />
                        </Button>
                      </div>
                    ))}
                    <Button
                      onClick={() => addController(pIndex)}
                      className="mt-2 glass-button status-gradient-connected text-white border-0"
                    >
                      <PlusCircle className="h-4 w-4 mr-2" />
                      <T>Add Controller</T>
                    </Button>
                  </CardContent>
                </Card>
              ))}
            </div>
            <Button
              onClick={addPartition}
              className="mt-4 glass-button status-gradient-connected text-white border-0"
            >
              <PlusCircle className="h-4 w-4 mr-2" />
              <T>Add Partition</T>
            </Button>
          </CollapsibleSection>
        )}

        <div className="space-y-3 border-t border-slate-300/30 dark:border-white/10 pt-4 mt-6">
          <div className="grid grid-cols-3 gap-2">
            <Button type="button" onClick={handleImportClick} className="glass-button bg-gradient-to-r from-cyan-400 to-blue-500 text-white border-0">
              <T>Import JSON</T>
            </Button>
            <Button type="button" onClick={handleExport} className="glass-button">
              <T>Export JSON</T>
            </Button>
            <Button asChild className="glass-button">
              <a href="https://polymech.info/en/resources/cassandra/settings" target="_blank" rel="noopener noreferrer">
                <HelpCircle className="h-4 w-4 mr-2" />
                <T>Help</T>
              </a>
            </Button>
          </div>
          <input
            type="file"
            ref={fileInputRef}
            onChange={handleFileChange}
            accept=".json"
            style={{ display: 'none' }}
          />
          <Button type="submit" disabled={isSaving || loading} className="w-full status-gradient-connected text-white border-0 text-lg font-semibold py-3">
            {isSaving ? <T>Saving...</T> : <T>Save All Settings</T>}
          </Button>
        </div>
      </form>

      {/* Add PlungerSettingsDisplay for Elena HMI */}
      {serverSettings?.name === 'elena' && (
        <div className="mt-6">
          <CollapsibleSection
            title={<T>Plunger Settings</T>}
            storageKey="settings-plunger-collapsible"
            initiallyOpen={false}
          >
            <PlungerSettingsDisplay />
          </CollapsibleSection>
        </div>
      )}
    </>
  );
};

export default SettingsDisplay;
