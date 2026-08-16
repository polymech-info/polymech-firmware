import React, { useState, useEffect } from 'react';
import { Card, CardContent, CardHeader, CardTitle, CardFooter } from '@/components/ui/card';
import { Button } from '@/components/ui/button';
import { Slider } from '@/components/ui/slider';
import { Progress } from '@/components/ui/progress';
import { Controller, TemperatureProfile } from '@/lib/api';
import { getTemperatureAtTime } from '@/lib/bezier';
import { Play, Pause, RefreshCw, Info, Pencil } from 'lucide-react';
import { cn } from '@/lib/utils';
import { Tooltip, TooltipContent, TooltipProvider, TooltipTrigger } from '@/components/ui/tooltip';
import { Badge } from '@/components/ui/badge';

interface TemperatureControllerProps {
  controller: Controller;
  profiles: TemperatureProfile[];
  onUpdate: (id: string, data: Partial<Controller>) => void;
  onStart: (id: string, profileId?: string) => void;
  onStop: (id: string) => void;
  onEdit?: (controller: Controller) => void;
}

const TemperatureController: React.FC<TemperatureControllerProps> = ({ 
  controller, 
  profiles, 
  onUpdate, 
  onStart, 
  onStop,
  onEdit
}) => {
  const [currentTemp, setCurrentTemp] = useState(controller.currentTemp);
  const [progress, setProgress] = useState(0);
  const [elapsedTime, setElapsedTime] = useState(0);
  const [totalTime, setTotalTime] = useState(0);
  
  const activeProfile = controller.currentProfile 
    ? profiles.find(p => p.id === controller.currentProfile) 
    : null;

  useEffect(() => {
    let interval: NodeJS.Timeout;
    
    if (controller.isRunning) {
      if (activeProfile) {
        setTotalTime(activeProfile.duration * 60);
        
        interval = setInterval(() => {
          setElapsedTime(prev => {
            const newElapsed = prev + 1;
            
            const normalizedTime = newElapsed / (activeProfile.duration * 60);
            setProgress(normalizedTime * 100);
            
            if (normalizedTime >= 1) {
              onStop(controller.id);
              return 0;
            }
            
            const newTemp = getTemperatureAtTime(
              activeProfile.controlPoints,
              normalizedTime,
              controller.minTemp,
              controller.maxTemp
            );
            
            const randomFactor = (Math.random() * 0.4) - 0.2;
            setCurrentTemp(Math.round((newTemp + randomFactor) * 10) / 10);
            
            return newElapsed;
          });
        }, Math.max(200, controller.updateInterval));
      } else {
        interval = setInterval(() => {
          setCurrentTemp(prev => {
            const diff = controller.targetTemp - prev;
            const step = Math.sign(diff) * (Math.min(Math.abs(diff), 0.3) + Math.random() * 0.2);
            return Math.round((prev + step) * 10) / 10;
          });
        }, Math.max(200, controller.updateInterval));
      }
    }
    
    return () => {
      if (interval) clearInterval(interval);
    };
  }, [controller.isRunning, controller.id, controller.targetTemp, 
      controller.minTemp, controller.maxTemp, controller.updateInterval, activeProfile, onStop]);
  
  const handleTargetTempChange = (value: number[]) => {
    onUpdate(controller.id, { targetTemp: value[0] });
  };
  
  const getTempColor = (temp: number) => {
    const range = controller.maxTemp - controller.minTemp;
    const ratio = (temp - controller.minTemp) / range;
    
    if (ratio <= 0.33) return 'text-temp-cold';
    if (ratio <= 0.66) return 'text-temp-warm';
    return 'text-temp-hot';
  };
  
  const formatTime = (seconds: number) => {
    const mins = Math.floor(seconds / 60);
    const secs = seconds % 60;
    return `${mins.toString().padStart(2, '0')}:${secs.toString().padStart(2, '0')}`;
  };
  
  const handleEdit = () => {
    if (onEdit) {
      onEdit(controller);
    }
  };
  
  return (
    <Card className="w-full">
      <CardHeader className="p-3">
        <CardTitle className="flex justify-between items-center text-lg">
          <div 
            className="flex items-center cursor-pointer hover:text-primary transition-colors group"
            onClick={handleEdit}
          >
            <span>{controller.name}</span>
            <Pencil className="h-3.5 w-3.5 ml-1 opacity-0 group-hover:opacity-100 transition-opacity" />
          </div>
          <div className="flex items-center gap-2">
            <TooltipProvider>
              <Tooltip>
                <TooltipTrigger asChild>
                  <span className="flex items-center text-xs text-muted-foreground">
                    <Info className="h-3.5 w-3.5 mr-1" />
                    ID: {controller.slaveId} / {controller.updateInterval}ms
                  </span>
                </TooltipTrigger>
                <TooltipContent>
                  <p>Modbus Slave ID: {controller.slaveId}</p>
                  <p>Update Interval: {controller.updateInterval}ms</p>
                </TooltipContent>
              </Tooltip>
            </TooltipProvider>
            {controller.isRunning && (
              <RefreshCw className="h-4 w-4 animate-spin" />
            )}
          </div>
        </CardTitle>
      </CardHeader>
      <CardContent className="space-y-3 p-3">
        <div className="flex justify-center">
          <div className={cn(
            "temperature-display text-3xl", 
            getTempColor(currentTemp)
          )}>
            {currentTemp.toFixed(1)}°C
          </div>
        </div>
        
        <div className="space-y-1">
          <div className="flex justify-between text-sm">
            <span>Target: {controller.targetTemp}°C</span>
            <span className="text-xs text-muted-foreground">
              {controller.minTemp}°C - {controller.maxTemp}°C
            </span>
          </div>
          <Slider 
            defaultValue={[controller.targetTemp]} 
            min={controller.minTemp} 
            max={controller.maxTemp} 
            step={1}
            onValueCommit={handleTargetTempChange}
            disabled={controller.isRunning && !!activeProfile}
          />
        </div>
        
        {controller.isRunning && activeProfile && (
          <div className="space-y-1">
            <div className="flex justify-between text-sm">
              <span>Profile: {activeProfile.name}</span>
              <span>{formatTime(elapsedTime)} / {formatTime(totalTime)}</span>
            </div>
            <Progress value={progress} className="h-2" />
          </div>
        )}
      </CardContent>
      <CardFooter className="flex justify-between p-3">
        {!controller.isRunning ? (
          <Button 
            className="flex-1 gap-2 text-sm h-8" 
            size="sm"
            onClick={() => onStart(controller.id)}
          >
            <Play className="h-3 w-3" />
            Start
          </Button>
        ) : (
          <Button 
            className="flex-1 gap-2 text-sm h-8" 
            variant="destructive" 
            size="sm"
            onClick={() => onStop(controller.id)}
          >
            <Pause className="h-3 w-3" />
            Stop
          </Button>
        )}
      </CardFooter>
    </Card>
  );
};

export default TemperatureController;
