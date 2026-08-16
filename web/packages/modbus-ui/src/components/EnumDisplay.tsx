import React from 'react';
import { T } from '../i18n';
import { cn } from '@/lib/utils';
import { ParsedRegister, parseRegisterName } from '@/lib/modbusUtils';
export { parseRegisterName };

interface EnumDisplayProps {
  parsedEnum: ParsedRegister;
  currentValue?: number;
  onValueClick?: (value: number) => void;
}

const EnumDisplay: React.FC<EnumDisplayProps> = ({ parsedEnum, currentValue, onValueClick }) => {
  const { enumValues } = parsedEnum;

  const handleItemClick = (value: number) => {
    if (onValueClick) {
      onValueClick(value);
    }
  };

  return (
    <div className="flex flex-col items-start text-xs space-y-1">
      {enumValues.map(({ val, label }) => (
        <div 
          key={val} 
          className={cn(
            "flex items-center rounded p-1 w-full text-slate-600 dark:text-slate-300", 
            { "font-bold text-indigo-600 dark:text-cyan-400": currentValue === val },
            onValueClick && "cursor-pointer hover:bg-slate-100 dark:hover:bg-white/10"
          )}
          onClick={() => handleItemClick(val)}
        >
          <span className="font-mono w-8 text-right pr-2">{val}:</span>
          <span className="font-mono"><T>{label}</T></span>
        </div>
      ))}
    </div>
  );
};

export default EnumDisplay; 