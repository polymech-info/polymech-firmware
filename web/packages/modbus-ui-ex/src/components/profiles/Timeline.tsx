import React from 'react';
import { SSignalControlPoint, ESignalType } from '../../types'; // Adjust path as needed

interface TimelineProps {
  plotSlot: number;
  plotIndex: number;
  plotDuration: number;
  controlPoints: SSignalControlPoint[];
  selectedCpInfo: { plotSlot: number; cpId: number } | null;
  isFirstPlotAndPlaying: boolean;
  isPlaying: boolean; // Specifically for the first plot
  playbackTimeMs: number; // Specifically for the first plot
  playbackHeadPositionPercent: number; // Specifically for the first plot
  timelineRef: (el: HTMLDivElement | null) => void;
  onTimelineDoubleClick: (event: React.MouseEvent<HTMLDivElement>, plotSlot: number, plotDuration: number) => void;
  onCpMouseDown: (event: React.MouseEvent<HTMLDivElement>, plotSlot: number, cpId: number, currentCpTime: number, plotIndex: number) => void;
  onRequestDeleteControlPoint: (plotSlot: number, cpId: number) => void;
  getActualTimeMs: (cpTime: number, plotDuration: number) => number;
  numMarkers: number;
  niceIntervalMs: number;
  formatMarkerLabel: (ms: number) => string;
  timelineMarkerKey: string;
  CONTROL_POINT_TIME_SCALE: number;
  isPlaybackMarkerDraggable?: boolean;
  onPlaybackMarkerMouseDown?: (event: React.MouseEvent<HTMLDivElement>) => void;
  isDraggingPlaybackMarker?: boolean;
}

const Timeline: React.FC<TimelineProps> = ({
  plotSlot,
  plotIndex,
  plotDuration,
  controlPoints,
  selectedCpInfo,
  isFirstPlotAndPlaying,
  isPlaying, 
  playbackTimeMs,
  playbackHeadPositionPercent,
  timelineRef,
  onTimelineDoubleClick,
  onCpMouseDown,
  onRequestDeleteControlPoint,
  getActualTimeMs,
  numMarkers,
  niceIntervalMs,
  formatMarkerLabel,
  timelineMarkerKey,
  CONTROL_POINT_TIME_SCALE,
  isPlaybackMarkerDraggable = false,
  onPlaybackMarkerMouseDown,
  isDraggingPlaybackMarker = false,
}) => {
  return (
    <div 
      ref={timelineRef}
      style={{ backgroundColor: '#f9f9f9', height: '60px', position: 'relative', border: '1px solid hsl(var(--border))', borderRadius: 'var(--radius)', padding: '0 10px', cursor: isFirstPlotAndPlaying ? 'default' : 'cell' }}
      onDoubleClick={(e) => !isFirstPlotAndPlaying && onTimelineDoubleClick(e, plotSlot, plotDuration)}
      key={timelineMarkerKey} // Ensures re-render when timeline dimensions might have changed
    >
      {/* Timeline Markers */}
      <div style={{ position: 'absolute', top: 0, left: '10px', right: '10px', height: '100%', display: 'flex', alignItems: 'center' }}>
        {[...Array(numMarkers)].map((_, i) => {
          const markerTimeMs = i * niceIntervalMs;
          if (markerTimeMs > plotDuration && plotDuration > 0 && markerTimeMs !== plotDuration) return null;
          const leftPercent = plotDuration > 0 ? (markerTimeMs / plotDuration) * 100 : 0;
          if (leftPercent > 100) return null;
          return (
            <div key={i} style={{ position: 'absolute', left: `${leftPercent}%`, top: '0px', height: '100%', width: '1px', backgroundColor: 'hsl(var(--border))', pointerEvents: 'none' }}>
              <span style={{ position: 'absolute', top: '-18px', left: '-8px', fontSize: '11px', color: 'hsl(var(--muted-foreground))', whiteSpace: 'nowrap' }}>
                {formatMarkerLabel(markerTimeMs)}
              </span>
            </div>
          );
        })}
      </div>

      {/* Control Points on Timeline */}
      {controlPoints.map(cp => {
        const actualTimeMs = getActualTimeMs(cp.time, plotDuration);
        const leftPositionPercent = cp.time / (CONTROL_POINT_TIME_SCALE / 100);
        const isSelected = selectedCpInfo?.plotSlot === plotSlot && selectedCpInfo?.cpId === cp.id;
        // isTriggered is specific to the first plot during playback
        const isTriggered = plotIndex === 0 && isPlaying && playbackTimeMs >= actualTimeMs && (playbackTimeMs - actualTimeMs < 100);

        return (
          <div
            key={`timeline-cp-${plotSlot}-${cp.id}`}
            title={`ID: ${cp.id}, Time: ${actualTimeMs.toFixed(0)}ms (${cp.time / 10}%), Type: ${ESignalType[cp.type]}`}
            onMouseDown={(e) => onCpMouseDown(e, plotSlot, cp.id, cp.time, plotIndex)}
            onDoubleClick={(e) => { e.stopPropagation(); !isFirstPlotAndPlaying && onRequestDeleteControlPoint(plotSlot, cp.id); }}
            style={{
              position: 'absolute',
              left: `${leftPositionPercent}%`,
              top: '50%',
              transform: 'translate(-50%, -50%)',
              width: isSelected || isTriggered ? '16px' : '12px',
              height: isSelected || isTriggered ? '16px' : '12px',
              backgroundColor: cp.type === ESignalType.MB_WRITE_COIL ? (cp.arg_1 === 1 ? 'hsl(var(--primary))' : 'hsl(var(--destructive))') : 'hsl(var(--secondary-foreground))',
              borderRadius: '50%',
              cursor: isFirstPlotAndPlaying ? 'default' : 'grab',
              border: `2px solid ${isSelected ? 'hsl(var(--ring))' : (isTriggered ? 'hsl(var(--primary))' : 'hsl(var(--background))' )}`,
              boxShadow: '0 1px 3px rgba(0,0,0,0.2)',
              zIndex: isSelected ? 10 : (isTriggered ? 9 : 1),
              transition: 'width 0.1s, height 0.1s, border-color 0.1s'
            }}
          />
        );
      })}

      {/* Playback Head (only for the first plot) */}
      {plotIndex === 0 && plotDuration > 0 && (
          <div 
            onMouseDown={isPlaybackMarkerDraggable && onPlaybackMarkerMouseDown ? onPlaybackMarkerMouseDown : undefined}
            style={{
              position: 'absolute', 
              left: `${playbackHeadPositionPercent}%`, 
              top: '0', 
              bottom: '0', 
              width: '8px', // Make it a bit wider to be grabbable
              backgroundColor: isDraggingPlaybackMarker ? 'hsl(var(--ring))' : 'hsl(var(--primary) / 0.7)', 
              cursor: isPlaybackMarkerDraggable ? (isDraggingPlaybackMarker ? 'grabbing' : 'grab') : 'default',
              pointerEvents: isPlaybackMarkerDraggable ? 'auto' : 'none', // Allow pointer events only if draggable
              zIndex: 20,
              transform: 'translateX(-50%)' // Center the wider marker
            }} 
          />
      )}
    </div>
  );
};

export default Timeline; 