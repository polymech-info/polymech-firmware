import React from 'react';
import { cn } from '@/lib/utils';

interface LedBarProps extends React.HTMLAttributes<HTMLDivElement> {
  value: number;
  max?: number;
}

const LedBar = React.forwardRef<HTMLDivElement, LedBarProps>(
  ({ className, value, max = 100, ...props }, ref) => {
    const percentage = max > 0 ? (value / max) * 100 : 0;
    const isOverload = percentage > 100;
    const displayPercentage = Math.min(percentage, 200);

    const normalWidth = isOverload ? '100%' : `${displayPercentage}%`;
    const overloadWidth = isOverload ? `${Math.min(displayPercentage - 100, 100)}%` : '0%';

    return (
      <div
        ref={ref}
        className={cn('relative h-4 w-full overflow-hidden rounded-full bg-slate-200', className)}
        {...props}
      >
        <div
          className="h-full bg-green-500 transition-all duration-300"
          style={{ width: normalWidth }}
        />
        {isOverload && (
          <div
            className="absolute top-0 left-0 h-full bg-yellow-400 transition-all duration-300"
            style={{ width: normalWidth }}
          >
            <div
              className="h-full bg-red-500 transition-all duration-300"
              style={{ width: overloadWidth }}
            />
          </div>
        )}
      </div>
    );
  }
);

LedBar.displayName = 'LedBar';

export { LedBar }; 