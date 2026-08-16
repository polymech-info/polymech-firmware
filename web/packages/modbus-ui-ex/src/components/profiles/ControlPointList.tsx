import React from 'react';
import { SSignalControlPoint, ESignalType } from '../../types'; // Adjust path as needed

interface ControlPointListProps {
  plotSlot: number;
  controlPoints: SSignalControlPoint[];
  selectedCpInfo: { plotSlot: number; cpId: number } | null;
  isFirstPlotAndPlaying: boolean;
  onSelectControlPoint: (plotSlot: number, cpId: number, focusList?: boolean) => void;
  onRequestDeleteControlPoint: (plotSlot: number, cpId: number) => void;
  cpListRef: (el: HTMLUListElement | null) => void;
  onKeyDown: (event: React.KeyboardEvent<HTMLUListElement>) => void;
}

const ControlPointList: React.FC<ControlPointListProps> = ({
  plotSlot,
  controlPoints,
  selectedCpInfo,
  isFirstPlotAndPlaying,
  onSelectControlPoint,
  onRequestDeleteControlPoint,
  cpListRef,
  onKeyDown,
}) => {
  if (controlPoints.length === 0) {
    return <p className="text-xs text-muted-foreground italic">No control points.</p>;
  }

  return (
    <ul
      ref={cpListRef}
      tabIndex={0}
      onKeyDown={onKeyDown}
      className="list-none p-0 m-0 space-y-1 max-h-64 overflow-y-auto pr-1 outline-none focus:ring-1 focus:ring-ring rounded-md"
    >
      {controlPoints.map((cp) => {
        const isSelected = selectedCpInfo?.plotSlot === plotSlot && selectedCpInfo?.cpId === cp.id;
        return (
          <li
            key={cp.id}
            data-cp-id={cp.id}
            onClick={() => !isFirstPlotAndPlaying && onSelectControlPoint(plotSlot, cp.id, true)}
            onDoubleClick={(e) => { e.stopPropagation(); !isFirstPlotAndPlaying && onRequestDeleteControlPoint(plotSlot, cp.id); }}
            className={`text-xs p-1.5 rounded-md ${!isFirstPlotAndPlaying ? 'cursor-pointer hover:bg-muted' : 'cursor-default'} ${isSelected ? 'bg-muted ring-1 ring-primary' : ''} focus:outline-none focus:ring-1 focus:ring-primary-focus`}
            tabIndex={-1}
          >
            ID: {cp.id} - {cp.name || ESignalType[cp.type]}
          </li>
        );
      })}
    </ul>
  );
};

export default ControlPointList; 