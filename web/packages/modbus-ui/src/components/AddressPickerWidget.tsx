import React, { useState, useMemo } from 'react';
import { useModbus } from '@/contexts/ModbusContext';
import { useFavorites } from '@/hooks/useFavorites';
import { AddressPicker, AddressGroup, AddressOption } from '@/components/modbus/AddressPicker';
import { T } from '@/i18n';
import { toast } from 'sonner';
import { E_FN_CODE } from '@polymech/client-ts';
import ModbusWidget from './ModbusWidget';
import { getSlaveIdFromGroup } from '@/lib/controllerUtils';



interface AddressPickerWidgetProps {
  isEditMode?: boolean;
  slaveId?: number;
  selectedAddress?: number | null;
  selectedSource?: 'register' | 'coil' | null | 'auto';
  // Widget instance management
  widgetInstanceId?: string;
  onPropsChange?: (props: Record<string, any>) => void;
}

const AddressPickerWidget: React.FC<AddressPickerWidgetProps> = ({ 
  isEditMode = false,
  slaveId = 0,
  selectedAddress: propSelectedAddress = null,
  selectedSource: propSelectedSource = 'auto',
  onPropsChange
}) => {
  // Convert 'auto' to null for selectedSource
  const normalizedSelectedSource = propSelectedSource === 'auto' ? null : propSelectedSource;
  const { coils, registers } = useModbus();
  const { favoriteRegisters, favoriteCoils } = useFavorites();
  const [selectedAddress, setSelectedAddress] = useState<number | null>(propSelectedAddress);
  const [selectedSource, setSelectedSource] = useState<'register' | 'coil' | null>(normalizedSelectedSource);

  // Sync local state with props when they change (e.g., from widget settings)
  React.useEffect(() => {
    setSelectedAddress(propSelectedAddress);
    setSelectedSource(normalizedSelectedSource);
  }, [propSelectedAddress, normalizedSelectedSource]);

  const groupedItems = useMemo((): AddressGroup[] => {
    const favoriteRegisterSet = new Set(favoriteRegisters);
    const favoriteCoilSet = new Set(favoriteCoils);
    
    // Filter function for slave ID (0 means show all)
    const matchesSlaveId = (item: any) => {
      if (slaveId === 0) return true; // 0 means show all slaves
      const itemSlaveId = getSlaveIdFromGroup(item.group);
      return itemSlaveId === slaveId;
    };
    
    const createOption = (item: any, source: 'register' | 'coil'): AddressOption => {
      const prefix = source === 'register' ? '[R]' : '[C]';
      const name = item.name || `${source === 'register' ? 'Register' : 'Coil'} ${item.address}`;
      const group = item.group || 'Default';
      return {
        value: `${source}-${item.address}`,
        label: `${prefix} ${group}::${name} (${item.address})`,
        titleForSeries: `${group}::${name}`,
        source: source,
        group,
      };
    };

    const favoriteOptions = [
      ...registers.filter(r => favoriteRegisterSet.has(r.address) && matchesSlaveId(r)).map(r => createOption(r, 'register')),
      ...coils.filter(c => favoriteCoilSet.has(c.address) && matchesSlaveId(c)).map(c => createOption(c, 'coil'))
    ].sort((a,b) => a.label.localeCompare(b.label));

    const allNonFavoriteItems = [
      ...registers.filter(r => !favoriteRegisterSet.has(r.address) && matchesSlaveId(r)).map(r => createOption(r, 'register')),
      ...coils.filter(c => !favoriteCoilSet.has(c.address) && matchesSlaveId(c)).map(c => createOption(c, 'coil'))
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
  }, [registers, coils, favoriteRegisters, favoriteCoils, slaveId]);
  
  const itemLabelMap = useMemo(() => {
    const newMap = new Map<string, string>();
    registers.forEach(r => {
        newMap.set(`register-${r.address}`, `[R] ${r.group || 'Default'}::${r.name || `Register ${r.address}`} (${r.address})`);
    });
    coils.forEach(c => {
        newMap.set(`coil-${c.address}`, `[C] ${c.group || 'Default'}::${c.name || `Coil ${c.address}`} (${c.address})`);
    });
    return newMap;
  }, [registers, coils]);

  const handleSelection = (value: string, source: 'register' | 'coil') => {
    // Extract address from value format like "register-123" or "coil-456"
    const addressStr = value.split('-')[1];
    const address = parseInt(addressStr, 10);
    if (isNaN(address)) return;
    
    setSelectedAddress(address);
    setSelectedSource(source);
    
    // Update props if callback provided (convert null to 'auto' for storage)
    onPropsChange?.({
      slaveId,
      selectedAddress: address,
      selectedSource: source || 'auto'
    });
    
    toast.success(`Selected ${source} ${address}`);
  };



  // Get current item data to determine function type
  const currentRegister = selectedSource === 'register' ? registers.find(r => r.address === selectedAddress) : null;
  const currentCoil = selectedSource === 'coil' ? coils.find(c => c.address === selectedAddress) : null;
  const currentItem = currentRegister || currentCoil;

  return (
    <div className="glass-card p-2">
      {!selectedAddress ? (
        // Address Selection Mode
        <div className="space-y-2">
          <p className="text-xs text-slate-500 dark:text-slate-400">
            <T>Select a register or coil address</T>
          </p>
          <AddressPicker
            value=""
            onSelect={(value, source) => handleSelection(value, source)}
            groupedItems={groupedItems}
            itemLabelMap={itemLabelMap}
            placeholder="Select register or coil..."
            className="w-full"
          />
        </div>
      ) : (
        // Selected Item Display Mode
        <div className="space-y-2">
          {/* Edit mode: Allow changing selection */}
          {isEditMode && (
            <div className="pb-2 border-b border-slate-300/30 dark:border-white/10">
              <AddressPicker
                value={`${selectedSource}-${selectedAddress}`}
                onSelect={(value, source) => handleSelection(value, source)}
                groupedItems={groupedItems}
                itemLabelMap={itemLabelMap}
                placeholder="Change address..."
                className="w-full text-xs"
              />
            </div>
          )}
          
          {/* Use ModbusWidget for display */}
          {currentItem && selectedAddress && selectedSource && (
            <div className="space-y-2">
              <ModbusWidget
                address={selectedAddress}
                functionType={currentItem.type || (selectedSource === 'register' ? E_FN_CODE.FN_WRITE_HOLD_REGISTER : E_FN_CODE.FN_WRITE_COIL)}
                className="border-0 shadow-none bg-transparent p-0"
              />
              
              {/* HEX display for registers */}
              {selectedSource === 'register' && (
                <div className="flex justify-end text-xs text-slate-500 dark:text-slate-400">
                  <span className="font-mono">
                    <T>HEX</T>: {((currentRegister?.value || 0)).toString(16).toUpperCase().padStart(4, '0')}
                  </span>
                </div>
              )}
            </div>
          )}
        </div>
      )}
    </div>
  );
};

export default AddressPickerWidget;