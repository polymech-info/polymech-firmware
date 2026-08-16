import React, { useState, useEffect, ChangeEvent } from 'react';
import { Input } from '@/components/ui/input';
import { Label } from '@/components/ui/label'; // Optional, if we add a general label

const MS_PER_SECOND = 1000;
const MS_PER_MINUTE = 60 * MS_PER_SECOND;
const MS_PER_HOUR = 60 * MS_PER_MINUTE;

interface TimeCodeEditorProps {
  totalMilliseconds: number;
  onDurationChange: (newTotalMilliseconds: number) => void;
  disabled?: boolean;
  idPrefix?: string; // For unique input ids
}

interface HMSType {
  h: number;
  m: number;
  s: number;
}

const millisecondsToHMS = (totalMs: number): HMSType => {
  if (isNaN(totalMs) || totalMs < 0) totalMs = 0;
  const h = Math.floor(totalMs / MS_PER_HOUR);
  const m = Math.floor((totalMs % MS_PER_HOUR) / MS_PER_MINUTE);
  const s = Math.floor((totalMs % MS_PER_MINUTE) / MS_PER_SECOND);
  return { h, m, s };
};

const hmsToMilliseconds = (h: number, m: number, s: number): number => {
  return (h * MS_PER_HOUR) + (m * MS_PER_MINUTE) + (s * MS_PER_SECOND);
};

export const TimeCodeEditor: React.FC<TimeCodeEditorProps> = ({
  totalMilliseconds,
  onDurationChange,
  disabled = false,
  idPrefix = 'timecode'
}) => {
  const [hms, setHms] = useState<HMSType>(millisecondsToHMS(totalMilliseconds));

  useEffect(() => {
    setHms(millisecondsToHMS(totalMilliseconds));
  }, [totalMilliseconds]);

  const handlePartChange = (part: keyof HMSType, valueStr: string) => {
    let value = parseInt(valueStr, 10);
    if (isNaN(value)) value = 0; // Default to 0 if parsing fails

    let newH = hms.h;
    let newM = hms.m;
    let newS = hms.s;

    if (part === 'h') newH = Math.max(0, value);
    else if (part === 'm') newM = Math.max(0, Math.min(59, value));
    else if (part === 's') newS = Math.max(0, Math.min(59, value));
    
    // Update internal state for immediate input feedback if desired, or directly call onDurationChange
    // For simplicity, let's update internal state and then call onDurationChange
    setHms({ h: newH, m: newM, s: newS });
    onDurationChange(hmsToMilliseconds(newH, newM, newS));
  };
  
  // Handle blur to ensure valid numbers (e.g., if user leaves it empty or with non-numeric)
  const handleBlur = (part: keyof HMSType, valueStr: string) => {
    let value = parseInt(valueStr, 10);
    if (isNaN(value) || value < 0) value = 0;

    let currentH = hms.h, currentM = hms.m, currentS = hms.s;
    if (part === 'h') currentH = Math.max(0, value);
    else if (part === 'm') currentM = Math.max(0, Math.min(59,value));
    else if (part === 's') currentS = Math.max(0, Math.min(59,value));

    // If the blurred value is different from current state, update and propagate
    if (currentH !== hms.h || currentM !== hms.m || currentS !== hms.s) {
        setHms({h: currentH, m: currentM, s: currentS});
        onDurationChange(hmsToMilliseconds(currentH, currentM, currentS));
    }
  };

  return (
    <div className="flex items-center space-x-1">
      <Input
        type="number"
        id={`${idPrefix}-h`}
        value={String(hms.h).padStart(2, '0')} // Pad with leading zero if needed, but type=number might not show it
        onChange={(e) => handlePartChange('h', e.target.value)}
        onBlur={(e) => handleBlur('h', e.target.value)}
        disabled={disabled}
        className="w-16 text-sm h-8 text-center tabular-nums"
        min="0"
        aria-label="Hours"
      />
      <span className="text-sm text-muted-foreground pb-0.5">:</span>
      <Input
        type="number"
        id={`${idPrefix}-m`}
        value={String(hms.m).padStart(2, '0')}
        onChange={(e) => handlePartChange('m', e.target.value)}
        onBlur={(e) => handleBlur('m', e.target.value)}
        disabled={disabled}
        className="w-16 text-sm h-8 text-center tabular-nums"
        min="0"
        max="59"
        aria-label="Minutes"
      />
      <span className="text-sm text-muted-foreground pb-0.5">:</span>
      <Input
        type="number"
        id={`${idPrefix}-s`}
        value={String(hms.s).padStart(2, '0')}
        onChange={(e) => handlePartChange('s', e.target.value)}
        onBlur={(e) => handleBlur('s', e.target.value)}
        disabled={disabled}
        className="w-16 text-sm h-8 text-center tabular-nums"
        min="0"
        max="59"
        aria-label="Seconds"
      />
    </div>
  );
};

export default TimeCodeEditor; 