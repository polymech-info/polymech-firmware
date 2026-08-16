import { useState, useMemo, useEffect } from 'react';
import { useModbus } from '@/contexts/ModbusContext';
import { Input } from '@/components/ui/input';
import { Button } from '@/components/ui/button';
import { RegisterData as Register } from '@/contexts/ModbusContext'; // Import correct type
import { getModbusErrorDescription } from '@/lib/modbusErrorMap'; // Import error map
import { T } from '../i18n'; 
import { E_FN_CODE } from '@polymech/client-ts';
import { toast } from 'sonner';
import { useFavorites } from '@/hooks/useFavorites';
import ModbusWidget from './ModbusWidget';

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

const isRegisterWritable = (register: Register): boolean => {
  return register.type === E_FN_CODE.FN_WRITE_HOLD_REGISTER ||
         register.type === E_FN_CODE.FN_WRITE_MULT_REGISTERS;
};


const RegistersDisplay = ({ showOnlyFavorites = false }: { showOnlyFavorites?: boolean }) => {
  const { registers, updateMultipleRegisters, isConnected } = useModbus();
  const { favoriteRegisters } = useFavorites();
  const [searchTerm, setSearchTerm] = useState(() => sessionStorage.getItem('registerSearchTerm') || '');
  const [bulkSetValue, setBulkSetValue] = useState('');

  useEffect(() => {
    sessionStorage.setItem('registerSearchTerm', searchTerm);
  }, [searchTerm]);



  const filteredRegisters = useMemo(() => {
    let baseRegisters = registers.filter(register => 
      register.type === E_FN_CODE.FN_READ_HOLD_REGISTER || 
      register.type === E_FN_CODE.FN_WRITE_HOLD_REGISTER ||
      register.type === E_FN_CODE.FN_WRITE_MULT_REGISTERS
    );

    if (showOnlyFavorites) {
      const favoriteSet = new Set(favoriteRegisters);
      baseRegisters = baseRegisters.filter(r => favoriteSet.has(r.address));
    }

    const trimmedSearchTerm = searchTerm.trim().toLowerCase();

    if (!trimmedSearchTerm) {
      return baseRegisters;
    }

    const orGroups = trimmedSearchTerm.split('|').map(s => s.trim()).filter(Boolean);

    return baseRegisters.filter(register => {
      return orGroups.some(group => {
        const andTerms = group.split(' ').filter(t => t);
        if (andTerms.length === 0) return false;

        return andTerms.every(term => {
          const humanizedName = humanizeRegisterName(register.name);
          
          return register.name.toLowerCase().includes(term) ||
                 humanizedName.toLowerCase().includes(term) ||
                 register.address.toString().includes(term) ||
                 (register.group && register.group.toLowerCase().includes(term)) ||
                 (register.group && humanizeRegisterName(register.group).toLowerCase().includes(term));
        });
      });
    });
  }, [registers, searchTerm, showOnlyFavorites, favoriteRegisters]);

  const groupedRegisters = useMemo(() => {
    const grouped = filteredRegisters.reduce((acc, register) => {
      const group = register.group || 'Uncategorized'; // Default group if none exists
      if (!acc[group]) {
        acc[group] = [];
      }
      acc[group].push(register);
      return acc;
    }, {} as Record<string, Register[]>);

    // Sort registers within each group by address
    for (const groupName in grouped) {
      grouped[groupName].sort((a, b) => a.address - b.address);
    }
    return grouped;
  }, [filteredRegisters]);

   // console.log(groupedRegisters);



  const handleBulkUpdate = () => {
    if (!isConnected) {
        toast.error("Not connected.");
        return;
    }
    const value = parseInt(bulkSetValue, 10);
    if (isNaN(value)) {
        toast.error("Please enter a valid number to set.");
        return;
    }

    const writableRegisters = filteredRegisters.filter(isRegisterWritable);

    if (writableRegisters.length === 0) {
        toast.info("No writable registers in the current view.");
        return;
    }

    const updates = writableRegisters.map(reg => ({ address: reg.address, value }));
    updateMultipleRegisters(updates);
  };

  return (
    <div className="glass-panel p-4 md:p-5 rounded-lg" id="registers-display">
      <div className="flex flex-col md:flex-row md:items-center md:justify-between gap-4 mb-4">
        <h2 className="text-xl font-bold accent-text text-center md:text-left"><T>Registers</T></h2>
        <div className="flex flex-col sm:flex-row items-center gap-2 w-full md:w-auto">
          <Input
            placeholder="Search by name, readable name, address, or group..."
            value={searchTerm}
            onChange={e => setSearchTerm(e.target.value)}
            className="glass-input w-full"
          />
          {searchTerm && (
            <div className="flex items-center justify-center gap-2 flex-wrap">
              <Input
                  placeholder="Set value"
                  value={bulkSetValue}
                  onChange={e => setBulkSetValue(e.target.value.replace(/[^0-9]/g, ''))}
                  className="w-28 glass-input font-mono"
              />
              <Button onClick={handleBulkUpdate} disabled={!isConnected || filteredRegisters.length === 0} className="status-gradient-connected text-white border-0 hover:shadow-lg transition-all duration-300">
                  <T>Set All</T>
              </Button>
            </div>
          )}
        </div>
      </div>

      {Object.keys(groupedRegisters).length === 0 && (
        <div className="text-center py-10">
          <p className="text-slate-500 dark:text-slate-400">
            {searchTerm
              ? <T>No registers found for "{searchTerm}".</T>
              : isConnected
                ? <T>No register data available. Try refreshing.</T>
                : <T>Connect to view register data.</T>}
          </p>
        </div>
      )}

      <div className="grid grid-cols-1 lg:grid-cols-2 gap-x-6 gap-y-8">
        {Object.entries(groupedRegisters).map(([groupName, registersInGroup]) => {
          // Find the register containing the status/error code for this group
          const statusRegister = registersInGroup.find(reg => reg.name.includes(' Status'));
          // Use the error field from the status register if available
          const errorCode = statusRegister?.error ?? -1;
          
          const errorDescription = `${getModbusErrorDescription(errorCode)} : ${errorCode}`
          const displayError = errorCode !== 0 && errorCode !== 1 && !errorDescription;
          
          return (
            <div key={groupName}>
              <h3 className="text-lg font-semibold mb-3 glass-text border-b border-slate-300/30 dark:border-white/10 pb-2">
                <T>{humanizeRegisterName(groupName)}</T>
                {displayError && (
                  <span className="text-xs font-normal text-red-500 ml-2">
                    (<T>Error</T>: {errorDescription})
                  </span>
                )}
              </h3>
              <div className="grid grid-cols-1 gap-3">
                {registersInGroup.map((register, index) => (
                  <ModbusWidget
                      key={`${groupName}-${index}`}
                    address={register.address}
                    functionType={register.type}
                  />
                ))}
              </div>
            </div>
          );
        })}
      </div>
    </div>
  );
};

export default RegistersDisplay;
