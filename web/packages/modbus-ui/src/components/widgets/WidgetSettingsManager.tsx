import React, { useState, useEffect } from 'react';
import { T } from '@/i18n';
import { Button } from '@/components/ui/button';
import { Input } from '@/components/ui/input';
import { Label } from '@/components/ui/label';
import { Select, SelectContent, SelectItem, SelectTrigger, SelectValue } from '@/components/ui/select';
import { Dialog, DialogContent, DialogHeader, DialogTitle, DialogFooter } from '@/components/ui/dialog';
import { WidgetDefinition } from '@/lib/widgetRegistry';
import { AddressPicker, AddressGroup, AddressOption } from '@/components/modbus/AddressPicker';
import { useModbus } from '@/contexts/ModbusContext';
import { useFavorites } from '@/hooks/useFavorites';

interface WidgetSettingsManagerProps {
  isOpen: boolean;
  onClose: () => void;
  onSave: (settings: Record<string, any>) => void;
  widgetDefinition: WidgetDefinition;
  currentProps: Record<string, any>;
}

const WidgetSettingsManagerComponent: React.FC<WidgetSettingsManagerProps> = ({
  isOpen,
  onClose,
  onSave,
  widgetDefinition,
  currentProps
}) => {
  const { coils, registers } = useModbus();
  const { favoriteRegisters, favoriteCoils } = useFavorites();
  const [settings, setSettings] = useState<Record<string, any>>(currentProps);

  // Reset settings when modal opens
  useEffect(() => {
    if (isOpen) {
      // Merge current props with default props
      const defaultProps = widgetDefinition.metadata.defaultProps || {};
      setSettings({ ...defaultProps, ...currentProps });
    }
  }, [isOpen, currentProps, widgetDefinition.metadata.defaultProps]);

  const handleSave = () => {
    onSave(settings);
    onClose();
  };

  const handleCancel = () => {
    setSettings(currentProps);
    onClose();
  };

  const updateSetting = (key: string, value: any) => {
    setSettings(prev => ({ ...prev, [key]: value }));
  };

  // Generate grouped items for address picker (if needed)
  const groupedItems = React.useMemo((): AddressGroup[] => {
    const favoriteRegisterSet = new Set(favoriteRegisters);
    const favoriteCoilSet = new Set(favoriteCoils);
    
    const createOption = (item: any, source: 'register' | 'coil'): AddressOption => {
      const prefix = source === 'register' ? '[R]' : '[C]';
      const name = item.name || `${source === 'register' ? 'Register' : 'Coil'} ${item.address}`;
      const group = item.group || 'Default';
      return {
        value: String(item.address),
        label: `${prefix} ${group}::${name} (${item.address})`,
        titleForSeries: `${group}::${name}`,
        source: source,
        group,
      };
    };

    const favoriteOptions = [
      ...registers.filter(r => favoriteRegisterSet.has(r.address)).map(r => createOption(r, 'register')),
      ...coils.filter(c => favoriteCoilSet.has(c.address)).map(c => createOption(c, 'coil'))
    ].sort((a,b) => a.label.localeCompare(b.label));

    const allNonFavoriteItems = [
      ...registers.filter(r => !favoriteRegisterSet.has(r.address)).map(r => createOption(r, 'register')),
      ...coils.filter(c => !favoriteCoilSet.has(c.address)).map(c => createOption(c, 'coil'))
    ];

    const groupedNonFavorites = allNonFavoriteItems.reduce((acc, item) => {
      const itemName = item.label.split('::')[1]?.split(' (')[0] || '';
      const group = (itemName.includes('Register') || itemName.includes('Coil')) ? 'Uncategorized' : item.label.split('::')[0].replace(/\[[RC]\] /,'');
      const groupName = `${group} (${item.source === 'register' ? 'Registers' : 'Coils'})`;

      if (!acc[groupName]) acc[groupName] = [];
      acc[groupName].push(item);
      return acc;
    }, {} as Record<string, AddressOption[]>);

    const result: AddressGroup[] = [];
    if(favoriteOptions.length > 0) {
      result.push({ label: `⭐ Favorites`, options: favoriteOptions });
    }

    Object.entries(groupedNonFavorites)
      .map(([label, options]) => ({
        label,
        options: options.sort((a, b) => a.label.localeCompare(b.label)),
      }))
      .sort((a, b) => a.label.localeCompare(b.label))
      .forEach(group => result.push(group));

    return result;
  }, [registers, coils, favoriteRegisters, favoriteCoils]);

  const itemLabelMap = React.useMemo(() => {
    const newMap = new Map<string, string>();
    registers.forEach(r => {
        newMap.set(`register-${r.address}`, `[R] ${r.group || 'Default'}::${r.name || `Register ${r.address}`} (${r.address})`);
    });
    coils.forEach(c => {
        newMap.set(`coil-${c.address}`, `[C] ${c.group || 'Default'}::${c.name || `Coil ${c.address}`} (${c.address})`);
    });
    return newMap;
  }, [registers, coils]);

  const handleAddressSelection = (value: string, source: 'register' | 'coil') => {
    const address = parseInt(value, 10);
    if (!isNaN(address)) {
      updateSetting('selectedAddress', address);
      updateSetting('selectedSource', source);
    }
  };

  const renderField = (key: string, config: any) => {
    const value = settings[key] ?? config.default;

    switch (config.type) {
      case 'number':
        return (
          <div key={key} className="space-y-2">
            <Label htmlFor={key}>
              <T>{config.label}</T>
            </Label>
            <Input
              id={key}
              type="number"
              min={config.min}
              max={config.max}
              value={value}
              onChange={(e) => updateSetting(key, parseInt(e.target.value) || config.default)}
              className="w-full"
            />
            {config.description && (
              <p className="text-xs text-slate-500 dark:text-slate-400">
                <T>{config.description}</T>
              </p>
            )}
          </div>
        );

      case 'text':
        return (
          <div key={key} className="space-y-2">
            <Label htmlFor={key}>
              <T>{config.label}</T>
            </Label>
            <Input
              id={key}
              type="text"
              value={value || ''}
              onChange={(e) => updateSetting(key, e.target.value)}
              className="w-full"
              placeholder={config.default}
            />
            {config.description && (
              <p className="text-xs text-slate-500 dark:text-slate-400">
                <T>{config.description}</T>
              </p>
            )}
          </div>
        );

      case 'select':
        return (
          <div key={key} className="space-y-2">
            <Label>
              <T>{config.label}</T>
            </Label>
            <Select value={value} onValueChange={(newValue) => updateSetting(key, newValue)}>
              <SelectTrigger className="w-full">
                <SelectValue placeholder={`Select ${config.label.toLowerCase()}`} />
              </SelectTrigger>
              <SelectContent>
                {config.options.map((option: any) => (
                  <SelectItem key={option.value} value={option.value}>
                    {option.label}
                  </SelectItem>
                ))}
              </SelectContent>
            </Select>
            {config.description && (
              <p className="text-xs text-slate-500 dark:text-slate-400">
                <T>{config.description}</T>
              </p>
            )}
          </div>
        );

      case 'address-picker':
        const currentValue = settings.selectedAddress && settings.selectedSource 
          ? `${settings.selectedSource}-${settings.selectedAddress}` 
          : '';
        
        return (
          <div key={key} className="space-y-2">
            <Label>
              <T>{config.label}</T>
            </Label>
            <AddressPicker
              value={currentValue}
              onSelect={handleAddressSelection}
              groupedItems={groupedItems}
              itemLabelMap={itemLabelMap}
              placeholder={config.placeholder || "Select register or coil..."}
              className="w-full"
            />
            {config.description && (
              <p className="text-xs text-slate-500 dark:text-slate-400">
                <T>{config.description}</T>
              </p>
            )}
          </div>
        );

      default:
        return null;
    }
  };

  const configSchema = widgetDefinition.metadata.configSchema || {};

  return (
    <Dialog open={isOpen} onOpenChange={onClose}>
      <DialogContent className="sm:max-w-lg max-w-[90vw]">
        <DialogHeader>
          <DialogTitle>
            <T>{widgetDefinition.metadata.name} Settings</T>
          </DialogTitle>
        </DialogHeader>
        
        <div className="space-y-4">
          {Object.entries(configSchema).map(([key, config]) => 
            renderField(key, config)
          )}

          {/* Preview for address picker */}
          {settings.selectedAddress && settings.selectedSource && (
            <div className="p-3 bg-slate-100 dark:bg-slate-800 rounded-lg">
              <p className="text-sm font-medium">
                <T>Selected</T>: {settings.selectedSource === 'register' ? 'Register' : 'Coil'} {settings.selectedAddress}
              </p>
              <p className="text-xs text-slate-500 dark:text-slate-400">
                <T>Slave</T> {settings.slaveId || 1}
              </p>
            </div>
          )}

          {/* Preview for address picker */}
          {widgetDefinition.metadata.id === 'address-picker' && (
            <div className="p-3 bg-slate-100 dark:bg-slate-800 rounded-lg">
              <p className="text-sm font-medium">
                <T>Preview</T>: 
                {settings.selectedAddress ? (
                  <span> {(settings.selectedSource && settings.selectedSource !== 'auto') ? (settings.selectedSource === 'register' ? 'Register' : 'Coil') : 'Auto-detect'} {settings.selectedAddress}</span>
                ) : (
                  <span> <T>No address selected</T></span>
                )}
              </p>
              <p className="text-xs text-slate-500 dark:text-slate-400">
                <T>Slave</T> {settings.slaveId === 0 ? '(All slaves)' : (settings.slaveId || 0)}
              </p>
            </div>
          )}

          {/* Preview for controller */}
          {widgetDefinition.metadata.id === 'cassandra-controller' && (
            <div className="p-3 bg-slate-100 dark:bg-slate-800 rounded-lg">
              <p className="text-sm font-medium">
                <T>Preview</T>: {settings.name || 'Controller 1'} (ID: {settings.slaveId || 1})
              </p>
            </div>
          )}
        </div>

        <DialogFooter>
          <Button variant="outline" onClick={handleCancel}>
            <T>Cancel</T>
          </Button>
          <Button onClick={handleSave}>
            <T>Save Settings</T>
          </Button>
        </DialogFooter>
      </DialogContent>
    </Dialog>
  );
};

// Custom comparison function for React.memo
const areWidgetSettingsPropsEqual = (
  prevProps: WidgetSettingsManagerProps,
  nextProps: WidgetSettingsManagerProps
): boolean => {
  // Always re-render if modal open state changes
  if (prevProps.isOpen !== nextProps.isOpen) return false;
  
  // Always re-render if widget definition changes
  if (prevProps.widgetDefinition.metadata.id !== nextProps.widgetDefinition.metadata.id) return false;
  
  // If modal is closed, don't re-render for prop changes (optimization)
  if (!prevProps.isOpen && !nextProps.isOpen) return true;
  
  // If modal is open, compare current props deeply
  if (prevProps.isOpen && nextProps.isOpen) {
    const prevPropsKeys = Object.keys(prevProps.currentProps);
    const nextPropsKeys = Object.keys(nextProps.currentProps);
    
    if (prevPropsKeys.length !== nextPropsKeys.length) return false;
    
    for (const key of prevPropsKeys) {
      if (prevProps.currentProps[key] !== nextProps.currentProps[key]) return false;
    }
  }
  
  return true;
};

// Export memoized component
export const WidgetSettingsManager = React.memo(WidgetSettingsManagerComponent, areWidgetSettingsPropsEqual);
