import React, { useState, useMemo } from 'react';
import { useModbus } from '@/contexts/ModbusContext';
import { Input } from '@/components/ui/input';
import { Button } from '@/components/ui/button';
import { Check, ArrowRight } from 'lucide-react';
import { cn } from '@/lib/utils';
import { RegisterData as Register } from '@/contexts/ModbusContext'; // Import correct type
import { getModbusErrorDescription } from '@/lib/modbusErrorMap'; // Import error map
import { LineChart, Line, ResponsiveContainer, XAxis, YAxis, Tooltip, CartesianGrid } from 'recharts';
import { T } from '../i18n'; 
import { E_FN_CODE } from '@polymech/client-ts';

// Define tracked registers here or import from constants
const REGISTERS_TO_CHART = [1017, 1033, 1037];

const RegistersDisplay = () => {
  const { registers, updateRegister, isConnected, registerHistory } = useModbus();
  const [editValues, setEditValues] = useState<Record<number, string>>({});
  const [pendingUpdates, setPendingUpdates] = useState<Set<number>>(new Set());
  const [searchTerm, setSearchTerm] = useState('');

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

  const filteredRegisters = useMemo(() => {
    let baseRegisters = registers.filter(register => 
      register.type === E_FN_CODE.FN_READ_HOLD_REGISTER || 
      register.type === E_FN_CODE.FN_WRITE_HOLD_REGISTER ||
      register.type === E_FN_CODE.FN_WRITE_MULT_REGISTERS
    );

    if (!searchTerm) {
      return baseRegisters;
    }

    const lowercasedSearchTerm = searchTerm.toLowerCase();
    return baseRegisters.filter(register => 
      register.name.toLowerCase().includes(lowercasedSearchTerm) ||
      register.address.toString().includes(lowercasedSearchTerm) ||
      (register.group && register.group.toLowerCase().includes(lowercasedSearchTerm))
    );
  }, [registers, searchTerm]);

  const groupedRegisters = useMemo(() => {
    return filteredRegisters.reduce((acc, register) => {
      const group = register.group || 'Uncategorized'; // Default group if none exists
      if (!acc[group]) {
        acc[group] = [];
      }
      acc[group].push(register);
      return acc;
    }, {} as Record<string, Register[]>);
  }, [filteredRegisters]);

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
    } finally {
      setPendingUpdates(prev => {
        const newSet = new Set([...prev]);
        newSet.delete(address);
        return newSet;
      });
    }
  };

  return (
    <div className="glass-morphism p-5 rounded-lg">
      <div className="flex justify-between items-center mb-4">
        <h2 className="text-xl font-bold text-gradient"><T>Registers</T></h2>
        <div className="w-1/3">
          <Input
            placeholder="Search by name, address, or group..."
            value={searchTerm}
            onChange={e => setSearchTerm(e.target.value)}
            className="bg-black/20 border-white/20 dark:bg-black/20 dark:border-white/20 light:bg-white/40 light:border-black/20"
          />
        </div>
      </div>

      {Object.keys(groupedRegisters).length === 0 && (
        <div className="text-center py-10">
          <p className="text-muted-foreground">
            {searchTerm
              ? <T>No registers found for "{searchTerm}".</T>
              : isConnected
                ? <T>No register data available. Try refreshing.</T>
                : <T>Connect to view register data.</T>}
          </p>
        </div>
      )}

      <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
        {Object.entries(groupedRegisters).map(([groupName, registersInGroup]) => {
          // Find the register containing the status/error code for this group
          const statusRegister = registersInGroup.find(reg => reg.name.includes(' Status'));
          // Use the error field from the status register if available
          const errorCode = statusRegister?.error ?? -1; // Default to -1 if not found
          const errorDescription = `${getModbusErrorDescription(errorCode)} : ${errorCode}`
          const displayError = errorCode !== 0 && errorDescription !== "Success";
          
          return (
            <div key={groupName}>
              <h3 className="text-lg font-semibold mb-3 text-primary/80">
                <T>{groupName}</T>
                {displayError && (
                  <span className="text-xs font-normal text-red-500 ml-2">
                    (<T>Error</T>: {errorDescription})
                  </span>
                )}
              </h3>
              <div className="grid grid-cols-1 gap-3">
                {registersInGroup.map((register, index) => {
                  const isEditing = register.address in editValues;
                  const isPending = pendingUpdates.has(register.address);
                  // Check if this register should have a chart
                  const shouldChart = REGISTERS_TO_CHART.includes(register.address);
                  // Get the relevant history, default to empty array
                  const historyData = registerHistory[register.address] || [];

                  return (
                    <div
                      key={`${groupName}-${index}`}
                      className={cn(
                        "border border-white/10 bg-black/30 rounded-md p-3 flex flex-col dark:border-white/10 dark:bg-black/30 light:border-black/10 light:bg-white/30",
                        isPending && "animate-pulse"
                      )}
                    >
                      {/* Top row: Info and Edit controls */}
                      <div className="flex items-center justify-between w-full mb-2">
                        <div className="flex items-center space-x-3 flex-1">
                          <div className="flex flex-col min-w-[70px]">
                            <span className="text-xs text-muted-foreground font-mono"><T>REGISTER</T> : <T>{register.name}</T>::{register.id}</span>
                            <span className="font-mono font-bold">{register.address}</span>
                          </div>
                          <div className="flex-1 flex items-center gap-2">
                            <div className="flex-1">
                              <div className="font-mono font-bold text-lg">
                                {register.value.toString().padStart(5, '0')}
                              </div>
                              <div className="text-xs text-muted-foreground font-mono">
                                <T>DEC</T>: {register.value} | <T>HEX</T>: {register.value.toString(16).toUpperCase().padStart(4, '0')}
                              </div>
                            </div>
                            {isEditing && (
                              <div className="flex items-center space-x-2">
                                <ArrowRight className="h-4 w-4 text-primary" />
                                <Input
                                  className="w-24 bg-black/40 border-white/20 dark:bg-black/40 dark:border-white/20 light:bg-white/40 light:border-black/20 font-mono"
                                  value={editValues[register.address] || ''}
                                  onChange={e => handleEdit(register.address, e.target.value)}
                                  disabled={isPending}
                                  onKeyDown={(e) => {
                                    if (e.key === 'Enter') {
                                      handleUpdateRegister(register.address);
                                    } else if (e.key === 'Escape') {
                                      handleCancelEdit(register.address);
                                    }
                                  }}
                                />
                                <Button
                                  size="sm"
                                  onClick={() => handleUpdateRegister(register.address)}
                                  disabled={isPending || !isConnected}
                                  className="h-8 w-8 p-0 bg-primary/80 hover:bg-primary"
                                >
                                  <Check className="h-4 w-4" />
                                </Button>
                              </div>
                            )}
                          </div>
                        </div>
                        {/* Edit Button (conditionally rendered based on access) */}
                        {!isEditing && (register.access === 2 || register.access === 3) && (
                          <Button
                            variant="outline"
                            size="sm"
                            onClick={() => handleEdit(register.address, register.value.toString())}
                            disabled={!isConnected}
                            className="bg-primary/20 border-primary/40 hover:bg-primary/30"
                          >
                            <T>Edit</T>
                          </Button>
                        )}
                      </div>
                      
                      {/* Bottom row: Chart - Render container if shouldChart */}
                      {shouldChart && (
                        <div className="w-full aspect-square mt-2">
                          <ResponsiveContainer width="100%" height="100%">
                            <LineChart 
                                data={historyData}
                                margin={{ top: 5, right: 5, left: 0, bottom: 15 }}
                            >
                              <CartesianGrid strokeDasharray="3 3" strokeOpacity={0.3} />
                              <XAxis 
                                dataKey="timestamp" 
                                tickFormatter={(timestamp) => new Date(timestamp).toLocaleTimeString()} 
                                tick={{ fontSize: 9 }}
                                height={15}
                                axisLine={false}
                                tickLine={false}
                              />
                              <YAxis 
                                domain={[0, 300]} 
                                tick={{ fontSize: 9 }}
                                width={30}
                              /> 
                              <Tooltip contentStyle={{ background: 'rgba(0,0,0,0.7)', border: 'none', fontSize: '10px' }} itemStyle={{ color: '#fff' }} labelFormatter={(label) => new Date(label).toLocaleTimeString()} />
                              {historyData.length > 0 && (
                                <Line 
                                  type="monotone" 
                                  dataKey="value" 
                                  stroke="#8884d8" 
                                  strokeWidth={1.5} 
                                  dot={true}
                                  isAnimationActive={true}
                                />
                              )}
                            </LineChart>
                          </ResponsiveContainer>
                        </div>
                      )}
                      {/* If not charting, maybe add a spacer or ensure consistent height? */}
                      {!shouldChart && (
                        <div className="flex-1"></div> // Placeholder to maintain flex layout for non-chart items
                      )}
                    </div>
                  );
                })}
              </div>
            </div>
          );
        })}
      </div>
    </div>
  );
};

export default RegistersDisplay;
