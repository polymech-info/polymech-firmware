import React, { useState } from 'react';
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card';
import { Input } from '@/components/ui/input';
import { Button } from '@/components/ui/button';
import { Plus, X, Check, Trash2, ArrowRight, Star } from 'lucide-react';
import { T } from '../i18n';
import { useModbus } from '@/contexts/ModbusContext';
import { useFavorites } from '@/hooks/useFavorites';
import { useWatchedItems, WatchedItem } from '@/hooks/useWatchedItems';
import { AddressPicker, AddressGroup, AddressOption } from './modbus/AddressPicker';
import { CoilSwitch } from './CoilSwitch';
import { RegisterData, CoilData } from '@/contexts/ModbusContext';
import { E_FN_CODE } from '@polymech/client-ts';
import { toast } from 'sonner';
import { cn } from '@/lib/utils';
import EnumDisplay, { parseRegisterName } from './EnumDisplay';
import FlagDisplay from './FlagDisplay';

const humanizeRegisterName = (name: string) => {
  if (!name) return '';
  // Remove everything after :: if it exists, and strip enum/flag definitions in parentheses
  const baseName = name.split('::')[0].split('(')[0];
  return baseName
    .split(/[_\s]+/) // Split on both underscores and spaces
    .filter(word => word.length > 0) // Remove empty strings
    .map((word) => {
      if (word.length < 3) {
        return word.toUpperCase();
      } else {
        return word.charAt(0).toUpperCase() + word.slice(1).toLowerCase();
      }
    })
    .join(' ');
};



const CustomWidgets: React.FC = () => {
  const { coils, registers, updateRegister, isConnected } = useModbus();
  const { favoriteRegisters, favoriteCoils, isFavorite, toggleFavorite } = useFavorites();
  const { watchedItems, addWatchedItem, removeWatchedItem, clearAllWatchedItems } = useWatchedItems();
  const [showAddPicker, setShowAddPicker] = useState(false);
  const [editValues, setEditValues] = useState<Record<number, string>>({});
  const [pendingUpdates, setPendingUpdates] = useState<Set<number>>(new Set());

  const groupedItems = React.useMemo((): AddressGroup[] => {
    const favoriteRegisterSet = new Set(favoriteRegisters);
    const favoriteCoilSet = new Set(favoriteCoils);
    
    const createOption = (item: RegisterData | CoilData, source: 'register' | 'coil'): AddressOption => {
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

  const isRegisterWritable = (address: number): boolean => {
    const register = registers.find(r => r.address === address);
    if (!register) return false;
    return register.type === E_FN_CODE.FN_WRITE_HOLD_REGISTER ||
           register.type === E_FN_CODE.FN_WRITE_MULT_REGISTERS;
  };

  const isCoilWritable = (address: number): boolean => {
    const coil = coils.find(c => c.address === address);
    if (!coil) return false;
    if (!coil.type) return true;
    return coil.type === E_FN_CODE.FN_WRITE_COIL || coil.type === E_FN_CODE.FN_WRITE_MULT_COILS;
  };

  const handleAddItem = (value: string, source: 'register' | 'coil', title: string) => {
    const address = parseInt(value, 10);
    if (isNaN(address)) return;
    
    const success = addWatchedItem(address, source, title);
    if (!success) {
      toast.info("Item already added.");
      return;
    }

    setShowAddPicker(false);
    toast.success(`Added ${source} ${address} to watch list.`);
  };

  const handleRemoveItem = (id: string) => {
    removeWatchedItem(id);
    toast.success("Removed item from watch list.");
  };

  const handleClearAll = () => {
    if (watchedItems.length === 0) return;
    clearAllWatchedItems();
    toast.success("Cleared all watched items.");
  };

  const getCurrentValue = (item: WatchedItem) => {
    if (item.source === 'register') {
      const register = registers.find(r => r.address === item.address);
      return register?.value ?? 'N/A';
    } else {
      const coil = coils.find(c => c.address === item.address);
      return coil?.value ? 'ON' : 'OFF';
    }
  };

  // Register editing handlers (similar to RegistersDisplay)
  const handleEdit = (address: number, value: string) => {
    setEditValues(prev => ({
      ...prev,
      [address]: value.replace(/[^0-9]/g, '')
    }));
  };

  const handleCancelEdit = (address: number) => {
    setEditValues(prev => {
      const newValues = { ...prev };
      delete newValues[address];
      return newValues;
    });
  };

  const handleUpdateRegister = async (address: number) => {
    const newValue = parseInt(editValues[address] || '0');
    if (isNaN(newValue)) return;
    
    setPendingUpdates(prev => new Set([...prev, address]));
    try {
      await updateRegister(address, newValue);
      setEditValues(prev => {
        const newValues = { ...prev };
        delete newValues[address];
        return newValues;
      });
      toast.success(`Updated register ${address} to ${newValue}`);
    } catch (error) {
      toast.error("Failed to update register", {
        description: error instanceof Error ? error.message : String(error),
      });
    } finally {
      setPendingUpdates(prev => {
        const newSet = new Set([...prev]);
        newSet.delete(address);
        return newSet;
      });
    }
  };

  return (
    <Card id="custom-widgets" className="glass-card">
      <CardHeader>      
      </CardHeader>
      <CardContent>
        <div className="flex items-center justify-between mb-3">
          <h4 className="font-semibold text-slate-700 dark:text-white"><T>Watched Items</T></h4>
          <div className="flex items-center gap-2">
            {watchedItems.length > 0 && (
              <Button
                onClick={handleClearAll}
                size="sm"
                className="h-8 px-2 glass-button status-gradient-error text-white border-0"
                title="Clear all watched items"
              >
                <Trash2 className="h-3 w-3" />
              </Button>
            )}
            {!showAddPicker && (
              <Button
                onClick={() => setShowAddPicker(true)}
                size="sm"
                className="h-8 w-8 p-0 glass-button status-gradient-connected text-white border-0"
                title="Add new watched item"
              >
                <Plus className="h-4 w-4" />
              </Button>
            )}
          </div>
        </div>

        {showAddPicker && (
          <div className="mb-2 p-2 border border-slate-300/30 dark:border-white/10 rounded-lg bg-white/5 dark:bg-black/5">
            <div className="flex items-center gap-2 mb-1">
              <AddressPicker
                value=""
                onSelect={handleAddItem}
                groupedItems={groupedItems}
                itemLabelMap={itemLabelMap}
                placeholder="Select register or coil to watch..."
                className="flex-1"
              />
              <Button
                onClick={() => setShowAddPicker(false)}
                size="sm"
                className="h-8 w-8 p-0 glass-button"
              >
                <X className="h-4 w-4" />
              </Button>
            </div>
            <p className="text-xs text-slate-500 dark:text-slate-400">
              <T>Select a register or coil to add to your watch list</T>
            </p>
          </div>
        )}

        {watchedItems.length === 0 ? (
          <p className="text-sm text-slate-500 dark:text-slate-400 text-center py-2">
            <T>No items in watch list</T>
          </p>
        ) : (
          <div className="space-y-2">
            {watchedItems.map((item) => {
              const isEditing = item.address in editValues;
              const isPending = pendingUpdates.has(item.address);
              const isWritable = item.source === 'register' 
                ? isRegisterWritable(item.address)
                : isCoilWritable(item.address);

              return (
                <div
                  key={item.id}
                  className={cn(
                    "glass-card p-3 flex flex-col hover:shadow-xl transition-all duration-500",
                    isPending && "animate-pulse"
                  )}
                >
                  {/* Top row: Info and Edit controls */}
                  <div className="flex flex-wrap items-center justify-between w-full gap-y-2 mb-2">
                    <div className="flex items-center space-x-3 flex-grow min-w-[200px]">
                      <Button
                        variant="ghost"
                        size="icon"
                        className="h-8 w-8 shrink-0 group"
                        onClick={() => toggleFavorite(item.source, item.address)}
                      >
                        <Star className={cn("h-4 w-4 text-slate-500 dark:text-slate-400 group-hover:text-amber-400", isFavorite(item.source, item.address) && "fill-amber-400 text-amber-400")} />
                      </Button>
                      <div className="flex flex-col min-w-0 flex-1">
                        <span className="text-sm text-slate-700 dark:text-white font-medium break-words">
                          {item.source === 'register' ? (() => {
                            const register = registers.find(r => r.address === item.address);
                            if (!register) return humanizeRegisterName(item.title);
                            
                            const parsed = parseRegisterName(register.name);
                            if (parsed) {
                              return humanizeRegisterName(parsed.mainName);
                            }
                            return humanizeRegisterName(register.name);
                          })() : (() => {
                            return humanizeRegisterName(item.title);
                          })()}
                        </span>
                        <span className="text-xs text-slate-500 dark:text-slate-400 font-mono truncate">
                          {item.source === 'register' ? (() => {
                            const register = registers.find(r => r.address === item.address);
                            if (!register) return `#${item.address} • ${item.title}::${item.id}`;
                            return `#${register.address} • ${register.name.split('(')[0]}::${register.id}`;
                          })() : `#${item.address} • ${item.title}::${item.id}`}
                        </span>
                      </div>
                    </div>
                    
                    <div className="flex flex-col items-end">
                      {item.source === 'register' ? (
                        // Register display (matching RegistersDisplay functionality)
                        <>
                          <div className="flex items-center gap-2">
                            {(() => {
                              if (isEditing) {
                                return (
                                  <>
                                    <div className="font-mono font-bold text-lg text-right">
                                      <span className="text-slate-500 dark:text-slate-400 line-through">
                                        {(getCurrentValue(item) as number || 0).toString().padStart(5, '0')}
                                      </span>
                                    </div>
                                    <div className="flex items-center space-x-2">
                                      <ArrowRight className="h-4 w-4 text-slate-600 dark:text-slate-300" />
                                      <Input
                                        className="w-24 glass-input font-mono"
                                        value={editValues[item.address] || ''}
                                        onChange={e => handleEdit(item.address, e.target.value)}
                                        disabled={isPending}
                                        onKeyDown={(e) => {
                                          if (e.key === 'Enter') {
                                            handleUpdateRegister(item.address);
                                          } else if (e.key === 'Escape') {
                                            handleCancelEdit(item.address);
                                          } else if (e.key === 'ArrowUp') {
                                            e.preventDefault();
                                            const currentValue = parseInt(editValues[item.address] || '0');
                                            const newValue = Math.min(65535, currentValue + 1); // Max 16-bit value
                                            handleEdit(item.address, newValue.toString());
                                          } else if (e.key === 'ArrowDown') {
                                            e.preventDefault();
                                            const currentValue = parseInt(editValues[item.address] || '0');
                                            const newValue = Math.max(0, currentValue - 1); // Min 0
                                            handleEdit(item.address, newValue.toString());
                                          }
                                        }}
                                        autoFocus
                                      />
                                      <Button
                                        size="icon"
                                        onClick={() => handleUpdateRegister(item.address)}
                                        disabled={isPending || !isConnected}
                                        className="h-9 w-9 p-0 status-gradient-connected text-white border-0 shrink-0 hover:shadow-lg transition-all duration-300"
                                      >
                                        <Check className="h-4 w-4" />
                                      </Button>
                                    </div>
                                  </>
                                );
                              }
                              
                              return (
                                <div 
                                  className={cn(
                                    "font-mono font-bold text-lg text-right",
                                    isWritable && isConnected 
                                      ? "cursor-pointer hover:bg-blue-500/10 hover:text-blue-600 dark:hover:text-blue-400 px-2 py-1 rounded transition-all duration-200 hover:shadow-sm underline decoration-blue-500 decoration-2 underline-offset-2" 
                                      : "text-slate-700 dark:text-white"
                                  )}
                                  onClick={isWritable && isConnected ? () => handleEdit(item.address, (getCurrentValue(item) as number || 0).toString()) : undefined}
                                  title={isWritable && isConnected ? "Click to edit" : undefined}
                                >
                                  {(getCurrentValue(item) as number || 0).toString().padStart(5, '0')}
                                </div>
                              );
                            })()}
                          </div>
                          <div className="text-xs text-slate-500 dark:text-slate-400 font-mono">
                            <T>DEC</T>: {getCurrentValue(item)} | <T>HEX</T>: {((getCurrentValue(item) as number) || 0).toString(16).toUpperCase().padStart(4, '0')}
                          </div>
                        </>
                      ) : (
                        // Coil switch (same as before)
                        <div className="flex items-center gap-2">
                          {isWritable ? (
                            <CoilSwitch
                              address={item.address}
                              value={coils.find(c => c.address === item.address)?.value || false}
                              disabled={!isConnected}
                            />
                          ) : (
                            <span className="text-sm font-mono glass-card px-2 py-1 rounded text-slate-700 dark:text-white">
                              {getCurrentValue(item)}
                            </span>
                          )}
                        </div>
                      )}
                    </div>
                  </div>

                  {/* Enum and Flag Display Area - only for registers */}
                  {item.source === 'register' && (() => {
                    const register = registers.find(r => r.address === item.address);
                    if (!register) return null;
                    
                    const parsed = parseRegisterName(register.name);
                    if (!parsed) return null;

                    const isFlags = register.name.toLowerCase().includes('flags');

                    if (isFlags) {
                      return (
                        <div className="mt-2 text-xs">
                          <FlagDisplay
                            parsedFlags={parsed}
                            currentValue={register.value}
                            onValueChange={isWritable ? (value) => updateRegister(register.address, value) : undefined}
                          />
                        </div>
                      )
                    }

                    return (
                      <div className="mt-2 text-xs">
                        <EnumDisplay 
                          parsedEnum={parsed} 
                          currentValue={register.value} 
                          onValueClick={isWritable ? (value) => updateRegister(register.address, value) : undefined}
                        />
                      </div>
                    );
                  })()}
                  
                  {/* Remove button */}
                  <div className="flex justify-end mt-2">
                    <Button
                      onClick={() => handleRemoveItem(item.id)}
                      size="icon"
                      variant="ghost"
                      className="h-8 w-8 text-slate-500 hover:text-red-500 hover:bg-red-500/10 transition-colors"
                      title="Remove from watch list"
                    >
                      <X className="h-4 w-4" />
                    </Button>
                  </div>
                </div>
              );
            })}
          </div>
        )}
      </CardContent>
    </Card>
  );
};

export default CustomWidgets;