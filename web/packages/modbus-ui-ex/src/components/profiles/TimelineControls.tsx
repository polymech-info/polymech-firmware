import React from 'react';
import { Button } from '@/components/ui/button';
import {
  PlayIcon,
  PauseIcon,
  SquareIcon as StopIcon,
  RotateCcw, // Using this for Resume
} from 'lucide-react';
import { PlotStatus } from '@/types';
import { T, translate } from '@/i18n';

interface TimelineControlsProps {
  // Playback Props
  onPlay: () => void;
  onPause: () => void;
  onStop: () => void;
  onResume: () => void;
  plotStatus: PlotStatus;
  playbackTimeMs: number;
  plotDuration: number;
  hasDuration: boolean;
}

const TimelineControls: React.FC<TimelineControlsProps> = ({
  onPlay,
  onPause,
  onStop,
  onResume,
  plotStatus,
  playbackTimeMs,
  plotDuration,
  hasDuration,
}) => {

  const isIdle = plotStatus === PlotStatus.IDLE;
  const isRunning = plotStatus === PlotStatus.RUNNING;
  const isPaused = plotStatus === PlotStatus.PAUSED;
  const isFinished = plotStatus === PlotStatus.FINISHED || plotStatus === PlotStatus.STOPPED;
  
  const canPlay = (isIdle || isFinished) && hasDuration;
  const canPause = isRunning;
  const canStop = hasDuration;
  const canResume = isPaused;

  const getStatusText = () => {
    switch(plotStatus) {
      case PlotStatus.RUNNING:
        return <span className='text-primary text-xs ml-2'><T>Running</T></span>;
      case PlotStatus.PAUSED:
        return <span className='text-amber-500 text-xs ml-2'><T>Paused</T></span>;
      case PlotStatus.FINISHED:
         return <span className='text-green-500 text-xs ml-2'><T>Finished</T></span>;
      case PlotStatus.STOPPED:
          return <span className='text-destructive text-xs ml-2'><T>Stopped</T></span>;
      case PlotStatus.IDLE:
          if (playbackTimeMs > 0 && playbackTimeMs < plotDuration) {
              return <span className='text-muted-foreground text-xs ml-2'><T>Ready</T></span>;
          }
          return null;
      default:
        return null;
    }
  }

  return (
    <div className="flex items-center justify-center space-x-2 mt-2 mb-1 flex-wrap">
      {/* Playback Controls */}
      <Button 
        variant="outline" 
        size="icon" 
        onClick={onPlay} 
        disabled={!canPlay}
        title={translate('Play from start')}
      >
        <PlayIcon className="h-4 w-4" />
      </Button>

      <Button 
        variant="outline" 
        size="icon" 
        onClick={onPause} 
        disabled={!canPause} 
        title={translate('Pause')}
      >
        <PauseIcon className="h-4 w-4" />
      </Button>
      
      {isPaused && (
        <Button 
          variant="outline" 
          size="icon" 
          onClick={onResume} 
          disabled={!canResume}
          title={translate('Resume')}
        >
          <RotateCcw className="h-4 w-4" />
        </Button>
      )}

      <Button 
        variant="outline" 
        size="icon" 
        onClick={onStop} 
        disabled={!canStop}
        title={translate('Stop and reset')}
        className="text-destructive hover:text-destructive"
      >
        <StopIcon className="h-4 w-4" />
      </Button>

      {/* Time Display */}
      {hasDuration && (
        <div className="text-xs text-muted-foreground min-w-[120px] text-center ml-2 tabular-nums">
          {(playbackTimeMs / 1000).toFixed(2)}s / {(plotDuration / 1000).toFixed(2)}s
        </div>
      )}
      {getStatusText()}
    </div>
  );
};

export default TimelineControls; 