import React from 'react';
import { Button } from '@/components/ui/button';
import { ChevronUp, ChevronDown, ChevronLeft, ChevronRight } from 'lucide-react';

interface WidgetMovementControlsProps {
  onMove: (direction: 'up' | 'down' | 'left' | 'right') => void;
  canMoveUp?: boolean;
  canMoveDown?: boolean;
  canMoveLeft?: boolean;
  canMoveRight?: boolean;
  className?: string;
}

export const WidgetMovementControls: React.FC<WidgetMovementControlsProps> = ({
  onMove,
  canMoveUp = true,
  canMoveDown = true,
  canMoveLeft = true,
  canMoveRight = true,
  className = ''
}) => {
  return (
    <div className={`relative w-16 h-16 ${className}`}>
      {/* Cross/Plus pattern with connecting lines */}
      
      {/* Vertical line */}
      <div className="absolute top-2 bottom-2 left-1/2 w-0.5 bg-slate-300 dark:bg-slate-600 transform -translate-x-1/2 opacity-30"></div>
      
      {/* Horizontal line */}
      <div className="absolute left-2 right-2 top-1/2 h-0.5 bg-slate-300 dark:bg-slate-600 transform -translate-y-1/2 opacity-30"></div>

      {/* Up button - top center */}
      <Button
        size="icon"
        variant="ghost"
        className="absolute top-0 left-1/2 transform -translate-x-1/2 h-5 w-5 bg-blue-500 hover:bg-blue-600 text-white rounded-full shadow-lg border-2 border-white dark:border-slate-800 transition-all duration-200 hover:scale-110"
        onClick={(e) => {
          e.stopPropagation();
          onMove('up');
        }}
        disabled={!canMoveUp}
        title="Move up"
      >
        <ChevronUp className="h-2.5 w-2.5" />
      </Button>

      {/* Left button - middle left */}
      <Button
        size="icon"
        variant="ghost"
        className="absolute top-1/2 left-0 transform -translate-y-1/2 h-5 w-5 bg-green-500 hover:bg-green-600 text-white rounded-full shadow-lg border-2 border-white dark:border-slate-800 transition-all duration-200 hover:scale-110"
        onClick={(e) => {
          e.stopPropagation();
          onMove('left');
        }}
        disabled={!canMoveLeft}
        title="Move left"
      >
        <ChevronLeft className="h-2.5 w-2.5" />
      </Button>

      {/* Center dot */}
      <div className="absolute top-1/2 left-1/2 transform -translate-x-1/2 -translate-y-1/2 w-1.5 h-1.5 bg-slate-400 dark:bg-slate-500 rounded-full opacity-60"></div>

      {/* Right button - middle right */}
      <Button
        size="icon"
        variant="ghost"
        className="absolute top-1/2 right-0 transform -translate-y-1/2 h-5 w-5 bg-green-500 hover:bg-green-600 text-white rounded-full shadow-lg border-2 border-white dark:border-slate-800 transition-all duration-200 hover:scale-110"
        onClick={(e) => {
          e.stopPropagation();
          onMove('right');
        }}
        disabled={!canMoveRight}
        title="Move right"
      >
        <ChevronRight className="h-2.5 w-2.5" />
      </Button>

      {/* Down button - bottom center */}
      <Button
        size="icon"
        variant="ghost"
        className="absolute bottom-0 left-1/2 transform -translate-x-1/2 h-5 w-5 bg-blue-500 hover:bg-blue-600 text-white rounded-full shadow-lg border-2 border-white dark:border-slate-800 transition-all duration-200 hover:scale-110"
        onClick={(e) => {
          e.stopPropagation();
          onMove('down');
        }}
        disabled={!canMoveDown}
        title="Move down"
      >
        <ChevronDown className="h-2.5 w-2.5" />
      </Button>
    </div>
  );
};
