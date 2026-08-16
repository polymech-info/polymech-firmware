import React from 'react';
import { SSignalControlPoint, ESignalType } from '../../types'; // Adjust path as needed
import { Button } from '@/components/ui/button';
import { Trash2Icon, ChevronUp, ChevronDown } from 'lucide-react';
import { T, translate } from '../../i18n';

interface ControlPointListProps {
  plotSlot: number;
  controlPoints: SSignalControlPoint[];
  selectedCpInfo: { plotSlot: number; cpId: number } | null;
  isFirstPlotAndPlaying: boolean;
  onSelectControlPoint: (plotSlot: number, cpId: number, focusList?: boolean) => void;
  onRequestDeleteControlPoint: (plotSlot: number, cpId: number) => void;
  onReorderControlPoint: (plotSlot: number, cpId: number, direction: 'up' | 'down') => void;
  onKeyDown: (event: React.KeyboardEvent<HTMLUListElement>) => void;
}

const ControlPointList = React.forwardRef<HTMLUListElement, ControlPointListProps>(({
  plotSlot,
  controlPoints,
  selectedCpInfo,
  isFirstPlotAndPlaying,
  onSelectControlPoint,
  onRequestDeleteControlPoint,
  onReorderControlPoint,
  onKeyDown,
}, ref) => {
  if (controlPoints.length === 0) {
    return <p className="text-xs text-muted-foreground italic">No control points.</p>;
  }

  return (
    <ul
      ref={ref}
      tabIndex={0}
      onKeyDown={onKeyDown}
      className="list-none p-0 m-0 space-y-1 max-h-64 overflow-y-auto pr-1 outline-none focus:ring-1 focus:ring-ring rounded-md"
      aria-label={translate("Control Points List")}
    >
      {controlPoints.map((cp, index) => {
        const isSelected = selectedCpInfo?.plotSlot === plotSlot && selectedCpInfo?.cpId === cp.id;
        const isFirst = index === 0;
        const isLast = index === controlPoints.length - 1;
        return (
          <li
            key={cp.id}
            id={`cp-list-item-${plotSlot}-${cp.id}`}
            data-cp-id={cp.id}
            onClick={() => !isFirstPlotAndPlaying && onSelectControlPoint(plotSlot, cp.id, true)}
            onDoubleClick={(e) => { e.stopPropagation(); !isFirstPlotAndPlaying && onRequestDeleteControlPoint(plotSlot, cp.id); }}
            onFocus={() => onSelectControlPoint(plotSlot, cp.id)}
            tabIndex={-1}
            className={`flex items-center justify-between p-2 pl-1 text-xs border-b last:border-b-0 cursor-pointer transition-colors ${
              isSelected ? 'bg-primary/20' : 'hover:bg-muted/50'
            }`}
            aria-selected={isSelected}
          >
            <div className="flex items-center">
              <div className="flex flex-col">
                <Button
                    variant="ghost"
                    size="icon"
                    className="h-5 w-5 text-muted-foreground hover:text-primary"
                    onClick={(e) => {
                        e.stopPropagation();
                        onReorderControlPoint(plotSlot, cp.id, 'up');
                    }}
                    disabled={isFirst}
                    aria-label={translate("Move control point up")}
                    title={translate("Move control point up")}
                >
                    <ChevronUp className="h-4 w-4" />
                </Button>
                <Button
                    variant="ghost"
                    size="icon"
                    className="h-5 w-5 text-muted-foreground hover:text-primary"
                    onClick={(e) => {
                        e.stopPropagation();
                        onReorderControlPoint(plotSlot, cp.id, 'down');
                    }}
                    disabled={isLast}
                    aria-label={translate("Move control point down")}
                    title={translate("Move control point down")}
                >
                    <ChevronDown className="h-4 w-4" />
                </Button>
              </div>
              <span className="font-mono text-muted-foreground w-12 ml-1">
                T: {cp.time}
              </span>
            </div>
            <span className="flex-grow truncate px-2" title={cp.name || `CP ${cp.id}`}>
              {cp.name || `CP ${cp.id}`}
            </span>
            <Button
              type="button"
              variant="ghost"
              size="icon"
              className="h-5 w-5 text-muted-foreground hover:text-primary"
              onClick={(e) => {
                e.stopPropagation();
                onRequestDeleteControlPoint(plotSlot, cp.id);
              }}
              aria-label={translate("Delete control point")}
              title={translate("Delete control point")}
            >
              <Trash2Icon className="h-4 w-4" />
            </Button>
          </li>
        );
      })}
    </ul>
  );
});

export default ControlPointList; 