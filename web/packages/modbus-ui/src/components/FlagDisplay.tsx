import React from 'react';
import { T } from '../i18n';
import { cn } from '@/lib/utils';
import { ParsedRegister } from '@/lib/modbusUtils';
import { Checkbox } from '@/components/ui/checkbox';

interface FlagDisplayProps {
  parsedFlags: ParsedRegister;
  currentValue?: number;
  onValueChange?: (value: number) => void;
}

const FlagDisplay: React.FC<FlagDisplayProps> = ({ parsedFlags, currentValue = 0, onValueChange }) => {
  const { enumValues } = parsedFlags;

  const handleFlagToggle = (flagValue: number, checked: boolean) => {
    if (onValueChange) {
      const newValue = checked
        ? currentValue | flagValue
        : currentValue & ~flagValue;
      onValueChange(newValue);
    }
  };

  return (
    <div className="flex flex-col items-start text-xs space-y-2">
      {enumValues.map(({ val, label }) => {
        const isChecked = (currentValue & val) === val;
        return (
          <div key={val} className="flex items-center space-x-2">
            <Checkbox
              id={`flag-${val}`}
              checked={isChecked}
              onCheckedChange={(checked) => handleFlagToggle(val, !!checked)}
              disabled={!onValueChange}
            />
            <label
              htmlFor={`flag-${val}`}
              className={cn(
                "font-mono leading-none text-slate-600 dark:text-slate-300",
                !onValueChange ? "cursor-not-allowed opacity-70" : "cursor-pointer"
              )}
            >
              <T>{label}</T> (<span className="text-slate-500 dark:text-slate-400">{val}</span>)
            </label>
          </div>
        );
      })}
    </div>
  );
};

export default FlagDisplay; 