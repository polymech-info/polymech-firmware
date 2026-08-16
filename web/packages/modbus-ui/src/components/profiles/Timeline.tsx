import React, { useRef } from 'react';
import { SSignalControlPoint, ESignalType } from '../../types'; // Adjust path as needed
import { cn } from '@/lib/utils'; // Import cn utility

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
  onTimelineTap: (event: React.TouchEvent<HTMLDivElement>, plotSlot: number, plotDuration: number) => void;
  onCpMouseDown: (event: React.MouseEvent<HTMLDivElement>, plotSlot: number, cpId: number, currentCpTime: number, plotIndex: number) => void;
  onCpTouchStart: (event: React.TouchEvent<HTMLDivElement>, plotSlot: number, cpId: number, currentCpTime: number, plotIndex: number) => void;
  onRequestDeleteControlPoint: (plotSlot: number, cpId: number) => void;
  getActualTimeMs: (cpTime: number, plotDuration: number) => number;
  numMarkers: number;
  niceIntervalMs: number;
  formatMarkerLabel: (ms: number) => string;
  timelineMarkerKey: string;
  CONTROL_POINT_TIME_SCALE: number;
  isPlaybackMarkerDraggable?: boolean;
  onPlaybackMarkerMouseDown?: (event: React.MouseEvent<HTMLDivElement>) => void;
  onPlaybackMarkerTouchStart?: (event: React.TouchEvent<HTMLDivElement>) => void;
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
  onTimelineTap,
  onCpMouseDown,
  onCpTouchStart,
  onRequestDeleteControlPoint,
  getActualTimeMs,
  numMarkers,
  niceIntervalMs,
  formatMarkerLabel,
  timelineMarkerKey,
  CONTROL_POINT_TIME_SCALE,
  isPlaybackMarkerDraggable = false,
  onPlaybackMarkerMouseDown,
  onPlaybackMarkerTouchStart,
  isDraggingPlaybackMarker = false,
}) => {
  const touchStartRef = useRef<{ x: number, y: number, time: number } | null>(null);

  const handleTouchStart = (event: React.TouchEvent<HTMLDivElement>) => {
    const touch = event.touches[0];
    touchStartRef.current = { x: touch.clientX, y: touch.clientY, time: Date.now() };
  };

  const handleTouchEnd = (event: React.TouchEvent<HTMLDivElement>) => {
    if (!touchStartRef.current) return;

    const touch = event.changedTouches[0];
    const { x, y, time } = touchStartRef.current;
    const deltaX = Math.abs(touch.clientX - x);
    const deltaY = Math.abs(touch.clientY - y);
    const deltaTime = Date.now() - time;

    // It's a "tap" if movement and time are minimal
    if (deltaX < 10 && deltaY < 10 && deltaTime < 200) {
      onTimelineTap(event, plotSlot, plotDuration);
    }
    touchStartRef.current = null;
  };

  return (
    <div 
      ref={timelineRef}
      className="signal-timeline"
      onDoubleClick={(e) => onTimelineDoubleClick(e, plotSlot, plotDuration)}
      onTouchStart={handleTouchStart}
      onTouchEnd={handleTouchEnd}
      key={timelineMarkerKey} // Ensures re-render when timeline dimensions might have changed
    >
      {/* Timeline Markers */}
      <div className="absolute top-0 left-[10px] right-[10px] h-full flex items-center">
        {[...Array(numMarkers)].map((_, i) => {
          const markerTimeMs = i * niceIntervalMs;
          if (markerTimeMs > plotDuration && plotDuration > 0 && markerTimeMs !== plotDuration) return null;
          const leftPercent = plotDuration > 0 ? (markerTimeMs / plotDuration) * 100 : 0;
          if (leftPercent > 100) return null;
          return (
            <div key={i} className="timeline-marker" style={{ left: `${leftPercent}%` }}>
              <span className="timeline-marker-label">
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

        const cpBackgroundColor = cp.type === ESignalType.MB_WRITE_COIL 
          ? (cp.arg_1 === 1 ? 'bg-primary' : 'bg-destructive') 
          : 'bg-secondary-foreground';
        
        const cpBorderColor = isSelected ? 'border-ring' : (isTriggered ? 'border-primary' : 'border-background');
        
        return (
          <div
            key={`timeline-cp-${plotSlot}-${cp.id}`}
            id={`timeline-cp-${plotSlot}-${cp.id}`}
            title={`ID: ${cp.id}, Time: ${actualTimeMs.toFixed(0)}ms (${cp.time / 10}%), Type: ${ESignalType[cp.type]}`}
            onMouseDown={(e) => onCpMouseDown(e, plotSlot, cp.id, cp.time, plotIndex)}
            onTouchStart={(e) => onCpTouchStart(e, plotSlot, cp.id, cp.time, plotIndex)}
            onDoubleClick={(e) => { e.stopPropagation(); onRequestDeleteControlPoint(plotSlot, cp.id); }}
            className={cn(
              "timeline-cp border-2",
              cpBackgroundColor,
              cpBorderColor,
              { 
                'w-4 h-4': isSelected || isTriggered, 
                'w-3 h-3': !isSelected && !isTriggered,
                'z-10': isSelected,
                'z-[9]': isTriggered && !isSelected,
                'z-[1]': !isSelected && !isTriggered
              }
            )}
            style={{ left: `${leftPositionPercent}%` }}
          />
        );
      })}

      {/* Playback Head (only for the first plot) */}
      {plotIndex === 0 && plotDuration > 0 && (
          <div 
            onMouseDown={isPlaybackMarkerDraggable && onPlaybackMarkerMouseDown ? onPlaybackMarkerMouseDown : undefined}
            onTouchStart={isPlaybackMarkerDraggable && onPlaybackMarkerTouchStart ? onPlaybackMarkerTouchStart : undefined}
            className={cn(
              "timeline-playback-head",
              isPlaybackMarkerDraggable ? (isDraggingPlaybackMarker ? 'cursor-grabbing' : 'cursor-grab') : 'cursor-default',
              isDraggingPlaybackMarker ? 'bg-ring' : 'bg-primary/70'
            )}
            style={{
              left: `${playbackHeadPositionPercent}%`, 
              pointerEvents: isPlaybackMarkerDraggable ? 'auto' : 'none',
            }} 
          />
      )}
    </div>
  );
};

export default Timeline; 