import React, { useState, useEffect } from 'react';
import { T } from '@/i18n';
import { Button } from '@/components/ui/button';
import { Input } from '@/components/ui/input';
import { Label } from '@/components/ui/label';
import { Switch } from '@/components/ui/switch';
import { Dialog, DialogContent, DialogHeader, DialogTitle, DialogDescription, DialogFooter } from '@/components/ui/dialog';
import { LayoutContainer } from '@/lib/unifiedLayoutManager';

interface ContainerSettingsManagerProps {
  isOpen: boolean;
  onClose: () => void;
  onSave: (settings: Partial<LayoutContainer['settings']>) => void;
  currentSettings: LayoutContainer['settings'];
  containerInfo: {
    id: string;
    columns: number;
  };
}

export const ContainerSettingsManager: React.FC<ContainerSettingsManagerProps> = ({
  isOpen,
  onClose,
  onSave,
  currentSettings,
  containerInfo,
}) => {
  const [settings, setSettings] = useState<LayoutContainer['settings']>(() => ({
    collapsible: currentSettings?.collapsible || false,
    collapsed: currentSettings?.collapsed || false,
    title: currentSettings?.title || '',
    showTitle: currentSettings?.showTitle || false,
  }));

  // Reset settings when modal opens
  useEffect(() => {
    if (isOpen) {
      const newSettings = {
        collapsible: currentSettings?.collapsible || false,
        collapsed: currentSettings?.collapsed || false,
        title: currentSettings?.title || '',
        showTitle: currentSettings?.showTitle || false,
      };
      setSettings(newSettings);
    }
  }, [isOpen, currentSettings]);

  const handleSave = () => {
    onSave(settings);
    onClose();
  };

  const handleCancel = () => {
    setSettings({
      collapsible: currentSettings?.collapsible || false,
      collapsed: currentSettings?.collapsed || false,
      title: currentSettings?.title || '',
      showTitle: currentSettings?.showTitle || false,
    });
    onClose();
  };

  const updateSetting = <K extends keyof LayoutContainer['settings']>(
    key: K,
    value: LayoutContainer['settings'][K]
  ) => {
    setSettings(prev => ({ ...prev, [key]: value }));
  };

  return (
    <Dialog open={isOpen} onOpenChange={onClose}>
      <DialogContent className="sm:max-w-md max-w-[90vw]">
        <DialogHeader>
          <DialogTitle>
            Container Settings
          </DialogTitle>
          <DialogDescription>
            Configure display and behavior settings for container {containerInfo.id.split('-').pop()} ({containerInfo.columns} columns)
          </DialogDescription>
        </DialogHeader>
        
        <div className="space-y-6">
          {/* Title Settings */}
          <div className="space-y-4">
            <div className="flex items-center justify-between">
              <Label htmlFor="show-title" className="text-sm font-medium">
                Show Title
              </Label>
              <Switch
                id="show-title"
                checked={settings.showTitle}
                onCheckedChange={(checked) => updateSetting('showTitle', checked)}
              />
            </div>
            
            {settings.showTitle && (
              <div className="space-y-2">
                <Label htmlFor="container-title">
                  Title
                </Label>
                <Input
                  id="container-title"
                  type="text"
                  value={settings.title}
                  onChange={(e) => updateSetting('title', e.target.value)}
                  placeholder={`Container (${containerInfo.columns} col${containerInfo.columns !== 1 ? 's' : ''})`}
                  className="w-full"
                />
                <p className="text-xs text-slate-500 dark:text-slate-400">
                  Leave empty to use default title
                </p>
              </div>
            )}
          </div>

          {/* Collapsible Settings */}
          <div className="space-y-4">
            <div className="flex items-center justify-between">
              <div>
                <Label htmlFor="collapsible" className="text-sm font-medium">
                  Collapsible
                </Label>
                <p className="text-xs text-slate-500 dark:text-slate-400">
                  Allow users to collapse/expand this container
                </p>
              </div>
              <Switch
                id="collapsible"
                checked={settings.collapsible}
                onCheckedChange={(checked) => {
                  updateSetting('collapsible', checked);
                  // If disabling collapsible, also set collapsed to false
                  if (!checked) {
                    updateSetting('collapsed', false);
                  }
                }}
              />
            </div>
            
            {settings.collapsible && (
              <div className="flex items-center justify-between pl-4 border-l-2 border-slate-200 dark:border-slate-700">
                <div>
                  <Label htmlFor="initially-collapsed" className="text-sm font-medium">
                    Initially Collapsed
                  </Label>
                  <p className="text-xs text-slate-500 dark:text-slate-400">
                    Start with container collapsed
                  </p>
                </div>
                <Switch
                  id="initially-collapsed"
                  checked={settings.collapsed}
                  onCheckedChange={(checked) => updateSetting('collapsed', checked)}
                />
              </div>
            )}
          </div>
        </div>

        <DialogFooter>
          <Button variant="outline" onClick={handleCancel}>
            Cancel
          </Button>
          <Button onClick={handleSave}>
            Save Settings
          </Button>
        </DialogFooter>
      </DialogContent>
    </Dialog>
  );
};
