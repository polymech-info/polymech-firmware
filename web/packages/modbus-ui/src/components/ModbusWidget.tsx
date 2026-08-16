import React, { useState } from 'react';
import { useModbus } from '@/contexts/ModbusContext';
import { Input } from '@/components/ui/input';
import { Button } from '@/components/ui/button';
import { Check, ArrowRight, Star } from 'lucide-react';
import { cn } from '@/lib/utils';
import { E_FN_CODE } from '@polymech/client-ts';
import { useFavorites } from '@/hooks/useFavorites';
import EnumDisplay, { parseRegisterName } from './EnumDisplay';
import FlagDisplay from './FlagDisplay';
import { CoilSwitch } from './CoilSwitch';

const humanizeRegisterName = (name: string) => {
  if (!name) return '';
  const baseName = name.split('::')[0].split('(')[0];
  return baseName
    .split(/[_\s]+/)
    .filter(word => word.length > 0)
    .map((word) => {
      if (word.length < 3) {
        return word.toUpperCase();
      } else {
        return word.charAt(0).toUpperCase() + word.slice(1).toLowerCase();
      }
    })
    .join(' ');
};

const humanizeCoilName = (name: string) => {
  if (!name) return '';
  const baseName = name.split('::')[0];
  return baseName
    .split('_')
    .map((word) => word.charAt(0).toUpperCase() + word.slice(1).toLowerCase())
    .join(' ');
};

interface ModbusWidgetProps {
  address: number;
  functionType: E_FN_CODE;
  className?: string;
}

const ModbusWidget: React.FC<ModbusWidgetProps> = ({ address, functionType, className }) => {
  const { registers, coils, updateRegister, isConnected } = useModbus();
  const { isFavorite, toggleFavorite } = useFavorites();
  const [editValue, setEditValue] = useState<string>('');
  const [isPending, setIsPending] = useState(false);
  const [isEditing, setIsEditing] = useState(false);

  // Determine if this is a register or coil based on function type
  const isRegister = [
    E_FN_CODE.FN_READ_HOLD_REGISTER,
    E_FN_CODE.FN_WRITE_HOLD_REGISTER,
    E_FN_CODE.FN_WRITE_MULT_REGISTERS,
    E_FN_CODE.FN_READ_INPUT_REGISTER
  ].includes(functionType);

  const isCoil = [
    E_FN_CODE.FN_READ_COIL,
    E_FN_CODE.FN_WRITE_COIL,
    E_FN_CODE.FN_WRITE_MULT_COILS,
    E_FN_CODE.FN_READ_DISCR_INPUT
  ].includes(functionType);

  // Find the data item
  const registerData = isRegister ? registers.find(r => r.address === address) : null;
  const coilData = isCoil ? coils.find(c => c.address === address) : null;
  const data = registerData || coilData;

  if (!data) {
    return (
      <div className={cn("glass-card p-3 opacity-50", className)}>
        <div className="text-sm text-slate-500 dark:text-slate-400">
          No data found for address {address} (FC: {functionType})
        </div>
      </div>
    );
  }

  // Check if writable
  const isWritable = isRegister 
    ? (registerData?.type === E_FN_CODE.FN_WRITE_HOLD_REGISTER || registerData?.type === E_FN_CODE.FN_WRITE_MULT_REGISTERS)
    : (coilData && (!coilData.type || coilData.type === E_FN_CODE.FN_WRITE_COIL || coilData.type === E_FN_CODE.FN_WRITE_MULT_COILS));

  const handleEdit = (value: string) => {
    if (isRegister) {
      setEditValue(value.replace(/[^0-9]/g, ''));
    }
  };

  const handleCancelEdit = () => {
    setEditValue('');
    setIsEditing(false);
  };

  const handleStartEdit = () => {
    if (isRegister && registerData) {
      setEditValue((registerData.value ?? 0).toString());
      setIsEditing(true);
    }
  };

  const handleUpdateRegister = async () => {
    if (!isRegister || !registerData) return;
    
    const newValue = parseInt(editValue || '0');
    if (isNaN(newValue)) return;
    
    setIsPending(true);
    try {
      await updateRegister(address, newValue);
      setIsEditing(false);
      setEditValue('');
    } finally {
      setIsPending(false);
    }
  };

  const favoriteType = isRegister ? 'register' : 'coil';
  const humanizedName = isRegister ? humanizeRegisterName(data.name) : humanizeCoilName(data.name);

  return (
    <div className={cn("glass-card p-3 flex flex-col hover:shadow-xl transition-all duration-500", isPending && "animate-pulse", className)}>
      {/* Top row: Info and controls */}
      <div className="flex flex-wrap items-center justify-between w-full gap-y-2 mb-2">
        <div className="flex items-center space-x-3 flex-grow min-w-[200px]">
          <Button
            variant="ghost"
            size="icon"
            className="h-8 w-8 shrink-0 group"
            onClick={() => toggleFavorite(favoriteType, address)}
          >
            <Star className={cn("h-4 w-4 text-slate-500 dark:text-slate-400 group-hover:text-amber-400", isFavorite(favoriteType, address) && "fill-amber-400 text-amber-400")} />
          </Button>
          <div className="flex flex-col min-w-0 flex-1">
            <span className="text-sm text-slate-700 dark:text-white font-medium break-words">
              {humanizedName}
            </span>
            <span className="text-xs text-slate-500 dark:text-slate-400 font-mono truncate">
              #{address} • {data.group}  {data.name.split('(')[0]}
            </span>
          </div>
        </div>

        {/* Value display/edit area */}
        <div className="flex flex-col items-end">
          <div className="flex items-center gap-2">
            {isRegister && registerData ? (
              // Register value display/edit
              isEditing ? (
                <>
                  <div className="font-mono font-bold text-lg text-right">
                    <span className="text-slate-500 dark:text-slate-400 line-through">
                      {(registerData.value ?? 0).toString().padStart(5, '0')}
                    </span>
                  </div>
                  <div className="flex items-center space-x-2">
                    <ArrowRight className="h-4 w-4 text-slate-600 dark:text-slate-300" />
                    <Input
                      className="w-24 glass-input font-mono"
                      value={editValue}
                      onChange={e => handleEdit(e.target.value)}
                      disabled={isPending}
                      onKeyDown={(e) => {
                        if (e.key === 'Enter') {
                          handleUpdateRegister();
                        } else if (e.key === 'Escape') {
                          handleCancelEdit();
                        } else if (e.key === 'ArrowUp') {
                          e.preventDefault();
                          const currentValue = parseInt(editValue || '0');
                          const newValue = Math.min(65535, currentValue + 1);
                          handleEdit(newValue.toString());
                        } else if (e.key === 'ArrowDown') {
                          e.preventDefault();
                          const currentValue = parseInt(editValue || '0');
                          const newValue = Math.max(0, currentValue - 1);
                          handleEdit(newValue.toString());
                        }
                      }}
                      autoFocus
                    />
                    <Button
                      size="icon"
                      onClick={handleUpdateRegister}
                      disabled={isPending || !isConnected}
                      className="h-9 w-9 p-0 status-gradient-connected text-white border-0 shrink-0 hover:shadow-lg transition-all duration-300"
                    >
                      <Check className="h-4 w-4" />
                    </Button>
                  </div>
                </>
              ) : (
                <div 
                  className={cn(
                    "font-mono font-bold text-lg text-right",
                    isWritable && isConnected 
                      ? "cursor-pointer hover:bg-blue-500/10 hover:text-blue-600 dark:hover:text-blue-400 px-2 py-1 rounded transition-all duration-200 hover:shadow-sm underline decoration-blue-500 decoration-2 underline-offset-2" 
                      : "text-slate-700 dark:text-white"
                  )}
                  onClick={isWritable && isConnected ? handleStartEdit : undefined}
                  title={isWritable && isConnected ? "Click to edit" : undefined}
                >
                  {(registerData.value ?? 0).toString().padStart(5, '0')}
                </div>
              )
            ) : isCoil && coilData ? (
              // Coil switch
              <CoilSwitch
                address={address}
                value={coilData.value}
                disabled={!isConnected || !isWritable}
                aria-label={`Toggle ${coilData.name}`}
              />
            ) : null}
          </div>
        </div>
      </div>
      
      {/* Enum and Flag Display Area for registers */}
      {isRegister && registerData && (() => {
        const parsed = parseRegisterName(registerData.name);
        if (!parsed) return null;

        const isFlags = registerData.name.toLowerCase().includes('flags');

        if (isFlags) {
          return (
            <div className="mt-2 text-xs">
              <FlagDisplay
                parsedFlags={parsed}
                currentValue={registerData.value}
                onValueChange={isWritable ? (value) => updateRegister(address, value) : undefined}
              />
            </div>
          );
        }

        return (
          <div className="mt-2 text-xs">
            <EnumDisplay 
              parsedEnum={parsed} 
              currentValue={registerData.value} 
              onValueClick={isWritable ? (value) => updateRegister(address, value) : undefined}
            />
          </div>
        );
      })()}
    </div>
  );
};

export default ModbusWidget;
