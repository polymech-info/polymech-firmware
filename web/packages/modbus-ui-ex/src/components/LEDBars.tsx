import React from 'react';

interface LEDBarsProps {
  value: number; // Input value 0-100
  label?: string;
  segments?: number;
  showValueText?: boolean;
  unit?: string;
  orientation?: 'horizontal' | 'vertical';
  barWidthClass?: string; // e.g., w-full for horizontal, w-8 for vertical
  barHeightClass?: string; // e.g., h-48 for vertical bar container, h-5 for horizontal
}

const LEDBars: React.FC<LEDBarsProps> = ({
  value = 0,
  label,
  segments = 10,
  showValueText = true,
  unit = '%',
  orientation = 'horizontal',
  barWidthClass,
  barHeightClass,
}) => {
  const normalizedValue = Math.max(0, Math.min(100, value));

  const getSegmentColor = (segmentIndex: number, isActive: boolean): string => {
    const segmentThreshold = (segmentIndex + 1) * (100 / segments);
    let baseColorClass = 'bg-slate-300 dark:bg-slate-700';

    if (segmentThreshold <= 40) baseColorClass = 'bg-green-500';
    else if (segmentThreshold <= 70) baseColorClass = 'bg-yellow-500';
    else baseColorClass = 'bg-red-500';

    return isActive ? baseColorClass : 'bg-slate-300 dark:bg-slate-600 opacity-30';
  };

  const isVertical = orientation === 'vertical';

  const effectiveBarWidth = barWidthClass ?? (isVertical ? 'w-8' : 'w-full');
  const effectiveBarHeight = barHeightClass ?? (isVertical ? 'h-48' : 'h-5');

  return (
    <div className={`flex flex-col items-center ${isVertical ? effectiveBarWidth : 'w-full'} ${isVertical ? effectiveBarHeight : ''} space-y-1`}>
      {label && <Label className={`text-xs text-muted-foreground ${isVertical ? 'mb-1' : ''}`}>{label}</Label>}
      <div 
        className={`flex ${isVertical ? 'flex-col-reverse justify-end flex-grow' : 'w-full items-center'} ${effectiveBarWidth} ${isVertical ? 'h-full' : effectiveBarHeight} rounded-md bg-slate-200 dark:bg-slate-700 p-0.5 shadow-inner`}
      >
        {Array.from({ length: segments }).map((_, i) => {
          const activeThreshold = Math.ceil(normalizedValue / (100 / segments));
          const isTrulyActive = i < activeThreshold;
          const segmentKey = `segment-${i}`;
          const marginClass = isVertical ? 
                                (i === 0 ? '' : 'mt-0.5') :
                                (i === segments - 1 ? '' : 'mr-0.5');
          
          return (
            <div
              key={segmentKey}
              className={`rounded-sm transition-colors duration-150 ${isVertical ? 'w-full flex-1' : 'h-full flex-1'} ${getSegmentColor(i, isTrulyActive)} ${marginClass}`}
              title={`${i+1}/${segments}`}
            />
          );
        })}
      </div>
      {showValueText && (
        <span className={`text-xs font-medium text-muted-foreground ${isVertical ? 'mt-auto pt-1' : ''}`}>
          {normalizedValue.toFixed(0)}{unit}
        </span>
      )}
    </div>
  );
};

// Simple Label component (can be replaced by ShadCN Label if preferred)
const Label: React.FC<{ children: React.ReactNode; className?: string; htmlFor?: string }> = ({ children, className, htmlFor }) => (
    <label htmlFor={htmlFor} className={`block text-xs font-medium text-gray-700 dark:text-gray-300 ${className}`}>
        {children}
    </label>
);

export default LEDBars; 