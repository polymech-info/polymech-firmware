import { useState, useMemo } from 'react';
import { useModbus } from '@/contexts/ModbusContext';
import { Input } from '@/components/ui/input';
import { cn } from '@/lib/utils';
import { CoilSwitch } from '@/components/CoilSwitch';
import { T } from '../i18n';
import { CoilData } from '@/contexts/ModbusContext';

/*
enum E_FN_CODE : uint8_t
{
    FN_ANY_FUNCTION_CODE = 0x00, // Only valid for server to register function codes
    FN_READ_COIL = 0x01,
    FN_READ_DISCR_INPUT = 0x02,
    FN_READ_HOLD_REGISTER = 0x03,
    FN_READ_INPUT_REGISTER = 0x04,
    FN_WRITE_COIL = 0x05,
    FN_WRITE_HOLD_REGISTER = 0x06,
    FN_READ_EXCEPTION_SERIAL = 0x07,
    FN_DIAGNOSTICS_SERIAL = 0x08,
    FN_READ_COMM_CNT_SERIAL = 0x0B,
    FN_READ_COMM_LOG_SERIAL = 0x0C,
    FN_WRITE_MULT_COILS = 0x0F,
    FN_WRITE_MULT_REGISTERS = 0x10,
    FN_REPORT_SERVER_ID_SERIAL = 0x11,
    FN_READ_FILE_RECORD = 0x14,
    FN_WRITE_FILE_RECORD = 0x15,
    FN_MASK_WRITE_REGISTER = 0x16,
    FN_R_W_MULT_REGISTERS = 0x17,
    FN_READ_FIFO_QUEUE = 0x18,
    FN_ENCAPSULATED_INTERFACE = 0x2B,
    FN_USER_DEFINED_41 = 0x41,
    FN_USER_DEFINED_42 = 0x42,
    FN_USER_DEFINED_43 = 0x43,
    FN_USER_DEFINED_44 = 0x44,
    FN_USER_DEFINED_45 = 0x45,
    FN_USER_DEFINED_46 = 0x46,
    FN_USER_DEFINED_47 = 0x47,
    FN_USER_DEFINED_48 = 0x48,
    FN_USER_DEFINED_64 = 0x64,
    FN_USER_DEFINED_65 = 0x65,
    FN_USER_DEFINED_66 = 0x66,
    FN_USER_DEFINED_67 = 0x67,
    FN_USER_DEFINED_68 = 0x68,
    FN_USER_DEFINED_69 = 0x69,
    FN_USER_DEFINED_6A = 0x6A,
    FN_USER_DEFINED_6B = 0x6B,
    FN_USER_DEFINED_6C = 0x6C,
    FN_USER_DEFINED_6D = 0x6D,
    FN_USER_DEFINED_6E = 0x6E,
    FN_NONE = 0xFF,
};
*/
const CoilsDisplay = () => {
  const { coils, isConnected } = useModbus();
  const [searchTerm, setSearchTerm] = useState('');
  
  // Filter coils before rendering
  const filteredCoils = useMemo(() => {
    let baseCoils = coils.filter(coil => coil.type === 1 || coil.type === 5);

    if (!searchTerm) {
      return baseCoils;
    }

    const lowercasedSearchTerm = searchTerm.toLowerCase();
    return baseCoils.filter(coil =>
      coil.name.toLowerCase().includes(lowercasedSearchTerm) ||
      coil.address.toString().includes(lowercasedSearchTerm) ||
      (coil.group && coil.group.toLowerCase().includes(lowercasedSearchTerm))
    );
  }, [coils, searchTerm]);

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

  return (
    <div className="glass-morphism p-5 rounded-lg">
      <div className="flex justify-between items-center mb-4">
        <h2 className="text-xl font-bold text-gradient"><T>Coils</T></h2>
        <div className="w-1/3">
          <Input
            placeholder="Search by name, address, or group..."
            value={searchTerm}
            onChange={e => setSearchTerm(e.target.value)}
            className="bg-black/20 border-white/20 dark:bg-black/20 dark:border-white/20 light:bg-white/40 light:border-black/20"
          />
        </div>
      </div>
      
      {Object.keys(groupedCoils).length === 0 && (
        <div className="text-center py-10">
          <p className="text-muted-foreground">
            {searchTerm
              ? <T>No coils found for "{searchTerm}".</T>
              : isConnected 
              ? <T>No coils data available. Try refreshing.</T> 
              : <T>Connect to view coils data.</T>}
          </p>
        </div>
      )}
      
      <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
        {Object.entries(groupedCoils).map(([groupName, coilsInGroup]) => (
          <div key={groupName}>
            <h3 className="text-lg font-semibold mb-3 text-primary/80">
              <T>{groupName}</T>
            </h3>
            <div className="grid grid-cols-1 gap-3">
              {coilsInGroup.map((coil, index) => (
                <div 
                  key={`${groupName}-${index}`}
                  className={cn(
                    "border border-white/10 bg-black/30 rounded-md p-3 flex items-center justify-between dark:border-white/10 dark:bg-black/30 light:border-black/10 light:bg-white/30",
                    coil.value && "bg-primary/10 border-primary/30"
                  )}
                >
                  <div className="flex flex-col">
                    <span className="text-xs text-muted-foreground font-mono"><T>COIL</T> : {coil.name}::{coil.id}::{coil.group || <T>N/A</T>} </span>
                    <span className="font-mono font-bold">#{coil.address}</span>
                  </div>
                  
                  <CoilSwitch
                    address={coil.address}
                    value={coil.value}
                    disabled={!isConnected}
                    aria-label={`Toggle ${coil.name}`}
                  />
                </div>
              ))}
            </div>
          </div>
        ))}
      </div>
    </div>
  );
};

export default CoilsDisplay;
