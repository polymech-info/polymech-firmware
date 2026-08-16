import { useState, useMemo, useEffect } from 'react';
import { useModbus } from '@/contexts/ModbusContext';
import { Input } from '@/components/ui/input';
import { cn } from '@/lib/utils';
import { T } from '../i18n';
import { CoilData } from '@/contexts/ModbusContext';
import { Button } from './ui/button';
import { toast } from 'sonner';
import { E_FN_CODE } from '@polymech/client-ts';
import { useFavorites } from '@/hooks/useFavorites';
import ModbusWidget from './ModbusWidget';

const humanizeCoilName = (name: string) => {
  if (!name) return '';
  // Remove everything after :: if it exists
  const baseName = name.split('::')[0];
  return baseName
    .split('_')
    .map((word) => word.charAt(0).toUpperCase() + word.slice(1).toLowerCase())
    .join(' ');
};

const isCoilWritable = (coil: CoilData) => {
  // Undefined or 0 (FN_ANY_FUNCTION_CODE) is treated as writable for legacy/default cases.
  if (!coil.type) {
    return true;
  }
  return coil.type === E_FN_CODE.FN_WRITE_COIL || coil.type === E_FN_CODE.FN_WRITE_MULT_COILS;
};

const CoilsDisplay = ({ showOnlyFavorites = false }: { showOnlyFavorites?: boolean }) => {
  const { coils, isConnected, updateCoil } = useModbus();
  
  const { favoriteCoils } = useFavorites();
  const [searchTerm, setSearchTerm] = useState(() => sessionStorage.getItem('coilSearchTerm') || '');
  
  useEffect(() => {
    sessionStorage.setItem('coilSearchTerm', searchTerm);
  }, [searchTerm]);

  // Filter coils before rendering
  const filteredCoils = useMemo(() => {
    let baseCoils = coils.filter(coil => 
      !coil.type || // Keep if type is missing/0
      coil.type === E_FN_CODE.FN_READ_COIL || 
      coil.type === E_FN_CODE.FN_WRITE_COIL ||
      coil.type === E_FN_CODE.FN_WRITE_MULT_COILS
    );

    if (showOnlyFavorites) {
      const favoriteSet = new Set(favoriteCoils);
      baseCoils = baseCoils.filter(c => favoriteSet.has(c.address));
    }

    if (!searchTerm) {
      return baseCoils;
    }

    const searchTerms = searchTerm.toLowerCase().split(' ').filter(term => term.trim() !== '');

    if (searchTerms.length === 0) {
      return baseCoils;
    }

    return baseCoils.filter(coil =>
      searchTerms.every(term =>
        coil.name.toLowerCase().includes(term) ||
        humanizeCoilName(coil.name).toLowerCase().includes(term) ||
        coil.address.toString().includes(term) ||
        (coil.group && coil.group.toLowerCase().includes(term)) ||
        (coil.group && humanizeCoilName(coil.group).toLowerCase().includes(term))
      )
    );
  }, [coils, searchTerm, showOnlyFavorites, favoriteCoils]);

  const groupedCoils = useMemo(() => {
    return filteredCoils.reduce((acc, coil) => {
      const group = coil.group || 'Uncategorized'; // Default group if none exists
      if (!acc[group]) {
        acc[group] = [];
      }
      acc[group].push(coil);
      return acc;
    }, {} as Record<string, CoilData[]>);
  }, [filteredCoils]);

  const handleBulkUpdate = (enable: boolean) => {
    if (!isConnected) {
      toast.error("Not connected.");
      return;
    }
    const writableCoils = filteredCoils
      .filter(isCoilWritable);
      
    if (writableCoils.length > 0) {
      const promises = writableCoils.map(coil => updateCoil(coil.address, enable));
      Promise.all(promises)
        .then(() => {
        //  toast.success(`Sent update for ${writableCoils.length} coil(s).`);
        })
        .catch((err) => {
          toast.error("Failed to update some coils.", {
            description: err.message,
          });
        });
    } else {
      toast.info("No writable coils to update in the current view.");
    }
  };

  return (
    <div className="glass-panel p-4 md:p-5 rounded-lg" id="coils-display">
      {/* Header Section */}
      <div className="flex flex-col md:flex-row md:items-center md:justify-between gap-4 mb-4">
        <h2 className="text-xl font-bold accent-text text-center md:text-left"><T>Coils</T></h2>
        <div className="flex flex-col sm:flex-row items-center gap-2 w-full md:w-auto">
          <Input
            placeholder="Search by name, readable name, address, or group..."
            value={searchTerm}
            onChange={e => setSearchTerm(e.target.value)}
            className="glass-input w-full"
          />
          {/* Action buttons are now in their own wrapping container */}
          {searchTerm && (
            <div className="flex items-center justify-center gap-2 flex-wrap">
              <Button onClick={() => handleBulkUpdate(true)} disabled={!isConnected || filteredCoils.length === 0} className="status-gradient-connected text-white border-0 hover:shadow-lg transition-all duration-300"><T>Enable All</T></Button>
              <Button onClick={() => handleBulkUpdate(false)} disabled={!isConnected || filteredCoils.length === 0} className="status-gradient-error text-white border-0 hover:shadow-lg transition-all duration-300"><T>Disable All</T></Button>
            </div>
          )}
        </div>
      </div>
      
      {/* Fallback Message */}
      {Object.keys(groupedCoils).length === 0 && (
        <div className="text-center py-10">
          <p className="text-slate-500 dark:text-slate-400">
            {searchTerm
              ? <T>No coils found for "{searchTerm}".</T>
              : isConnected 
              ? <T>No coils data available. Try refreshing.</T> 
              : <T>Connect to view coils data.</T>}
          </p>
        </div>
      )}
      
      {/* Coils Grid */}
      <div className="grid grid-cols-1 lg:grid-cols-2 gap-x-6 gap-y-8">
        {Object.entries(groupedCoils).map(([groupName, coilsInGroup]) => (
          <div key={groupName}>
            <h3 className="text-lg font-semibold mb-3 glass-text border-b border-slate-300/30 dark:border-white/10 pb-2">
              <T>{humanizeCoilName(groupName)}</T>
            </h3>
            <div className="grid grid-cols-1 gap-3">
              {coilsInGroup.map((coil) => (
                <ModbusWidget
                  key={coil.address}
                  address={coil.address}
                  functionType={coil.type || E_FN_CODE.FN_WRITE_COIL}
                  className={cn(coil.value && "border-emerald-400/50 ring-1 ring-emerald-400/30")}
                />
              ))}
            </div>
          </div>
        ))}
      </div>
    </div>
  );
};

export default CoilsDisplay;
