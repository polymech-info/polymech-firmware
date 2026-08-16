
import React, { useState, useEffect } from 'react';
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card';
import { Slider } from '@/components/ui/slider';
import { Input } from '@/components/ui/input';
import { Button } from '@/components/ui/button';
import { Progress } from '@/components/ui/progress';
import { HeatZone, Controller, TemperatureProfile } from '@/lib/api';
import { cn } from '@/lib/utils';
import {
  DropdownMenu,
  DropdownMenuContent,
  DropdownMenuItem,
  DropdownMenuTrigger,
} from "@/components/ui/dropdown-menu";
import { ChevronDown, Play, Thermometer, Clock, Square, Timer } from 'lucide-react';

interface ZoneMasterControlProps {
  zone: HeatZone;
  controllers: Controller[];
  profiles?: TemperatureProfile[];
  onUpdateAll: (zoneId: string, targetTemp: number) => void;
  onApplyProfileToZone?: (zoneId: string, profileId: string) => void;
  onStopZone?: (zoneId: string) => void;
}

const ZoneMasterControl: React.FC<ZoneMasterControlProps> = ({ 
  zone, 
  controllers,
  profiles,
  onUpdateAll,
  onApplyProfileToZone,
  onStopZone
}) => {
  const [targetTemp, setTargetTemp] = useState(50);
  const [inputValue, setInputValue] = useState('50');
  const [averageTemp, setAverageTemp] = useState(0);
  const [elapsedTime, setElapsedTime] = useState(0);
  const [remainingTime, setRemainingTime] = useState(0);
  const [activeProfileId, setActiveProfileId] = useState<string | null>(null);
  const [isZoneActive, setIsZoneActive] = useState(false);
  const [progressPercentage, setProgressPercentage] = useState(0);
  
  const minTemp = Math.min(...controllers.map(c => c.minTemp));
  const maxTemp = Math.max(...controllers.map(c => c.maxTemp));
  
  useEffect(() => {
    if (controllers.length === 0) return;
    
    const total = controllers.reduce((sum, controller) => sum + controller.currentTemp, 0);
    const avg = total / controllers.length;
    setAverageTemp(Math.round(avg * 10) / 10);
    
    const anyControllerRunning = controllers.some(c => c.isRunning);
    setIsZoneActive(anyControllerRunning);
    
    const runningController = controllers.find(c => c.isRunning && c.currentProfile);
    if (runningController && runningController.currentProfile) {
      setActiveProfileId(runningController.currentProfile);
    } else {
      setActiveProfileId(null);
    }
  }, [controllers]);
  
  useEffect(() => {
    let interval: NodeJS.Timeout;
    
    if (isZoneActive && activeProfileId) {
      const activeProfile = profiles?.find(p => p.id === activeProfileId);
      
      if (activeProfile) {
        const totalDuration = activeProfile.duration * 60;
        
        interval = setInterval(() => {
          setElapsedTime(prev => {
            const newElapsed = prev + 1;
            const newRemaining = Math.max(0, totalDuration - newElapsed);
            setRemainingTime(newRemaining);
            
            const progress = (newElapsed / totalDuration) * 100;
            setProgressPercentage(Math.min(100, progress));
            
            return newElapsed;
          });
        }, 1000);
      }
    } else {
      setElapsedTime(0);
      setRemainingTime(0);
      setProgressPercentage(0);
    }
    
    return () => {
      if (interval) clearInterval(interval);
    };
  }, [isZoneActive, activeProfileId, profiles]);
  
  const handleSliderChange = (values: number[]) => {
    const newTemp = values[0];
    setTargetTemp(newTemp);
    setInputValue(newTemp.toString());
  };
  
  const handleInputChange = (e: React.ChangeEvent<HTMLInputElement>) => {
    setInputValue(e.target.value);
  };
  
  const handleInputBlur = () => {
    const parsed = parseInt(inputValue, 10);
    if (!isNaN(parsed) && parsed >= minTemp && parsed <= maxTemp) {
      setTargetTemp(parsed);
      onUpdateAll(zone.id, parsed);
    } else {
      setInputValue(targetTemp.toString());
    }
  };
  
  const handleKeyDown = (e: React.KeyboardEvent<HTMLInputElement>) => {
    if (e.key === 'Enter') {
      handleInputBlur();
    }
  };
  
  const handleApplyToAll = () => {
    onUpdateAll(zone.id, targetTemp);
  };
  
  const handleStopZone = () => {
    onUpdateAll(zone.id, 0);
    
    if (onStopZone) {
      onStopZone(zone.id);
    }
  };
  
  const getTempColor = (temp: number) => {
    const range = maxTemp - minTemp;
    const ratio = (temp - minTemp) / range;
    
    if (ratio <= 0.33) return 'text-blue-400';
    if (ratio <= 0.66) return 'text-amber-400';
    return 'text-red-400';
  };

  const getHeatIntensity = (controllerIndex: number) => {
    if (controllers.length === 0 || !controllers[controllerIndex]) return 'bg-gray-200';
    
    const controller = controllers[controllerIndex];
    const range = controller.maxTemp - controller.minTemp;
    const ratio = (controller.currentTemp - controller.minTemp) / range;
    
    if (ratio <= 0.2) return 'bg-blue-200';
    if (ratio <= 0.4) return 'bg-blue-400';
    if (ratio <= 0.6) return 'bg-amber-400';
    if (ratio <= 0.8) return 'bg-orange-500';
    return 'bg-red-600';
  };
  
  const formatTime = (seconds: number) => {
    const mins = Math.floor(seconds / 60);
    const secs = seconds % 60;
    return `${mins.toString().padStart(2, '0')}:${secs.toString().padStart(2, '0')}`;
  };
  
  const controllerPositionMap = [
    { position: 'Top-Front', label: 'Nr.1' },
    { position: 'Bottom-Front', label: 'Nr.2' },
    { position: 'Top-Back', label: 'Nr.3' },
    { position: 'Bottom-Back', label: 'Nr.4' }
  ];
  
  const activeProfileName = activeProfileId
    ? profiles?.find(p => p.id === activeProfileId)?.name || 'Unknown Profile'
    : null;
  
  return (
    <Card className="w-full">
      <CardHeader className="p-4">
        <CardTitle className="flex justify-between items-center text-lg">
          <span>Master Control: {zone.name}</span>
          <div className={cn("font-mono", getTempColor(averageTemp))}>
            Avg: {averageTemp.toFixed(1)}°C
          </div>
        </CardTitle>
      </CardHeader>
      <CardContent className="space-y-4 p-4">
        {isZoneActive && (
          <div className="space-y-2 bg-secondary/30 rounded-md px-3 py-2 mb-2">
            {activeProfileName && (
              <div className="flex items-center justify-center gap-1 mb-1">
                <Play className="h-3.5 w-3.5 text-primary" />
                <span className="text-sm font-medium">{activeProfileName}</span>
              </div>
            )}
            
            <div className="flex flex-col gap-1 text-xs">
              <div className="flex items-center justify-between">
                <div className="flex items-center gap-1">
                  <Clock className="h-3.5 w-3.5 text-muted-foreground" />
                  <span className="text-muted-foreground">Time:</span>
                </div>
                <span className="font-mono">{formatTime(elapsedTime)}</span>
              </div>
              
              <div className="flex items-center justify-between">
                <div className="flex items-center gap-1">
                  <Timer className="h-3.5 w-3.5 text-muted-foreground" />
                  <span className="text-muted-foreground">Remaining:</span>
                </div>
                <span className="font-mono">{formatTime(remainingTime)}</span>
              </div>
            </div>
            
            <Progress value={progressPercentage} className="h-1.5 mt-1" />
          </div>
        )}
      
        <div className="mb-4 relative">
          <div className="border border-gray-300 rounded-md bg-gray-100 p-2">
            <div className="text-center text-xs text-gray-500 mb-1">Heat Press Visualization</div>
            <div className="flex justify-center mb-2">
              <Thermometer className="h-4 w-4 text-gray-500 mr-1" />
              <span className="text-xs">{zone.name} Elements</span>
            </div>
            <div className="grid grid-cols-2 gap-2 mb-2">
              {controllerPositionMap.slice(0, 2).map((pos, index) => {
                const controller = controllers[index];
                return (
                  <div 
                    key={index} 
                    className={cn(
                      "h-8 rounded-md transition-colors duration-300 flex items-center justify-center text-xs font-medium text-white shadow-inner",
                      controller ? getHeatIntensity(index) : "bg-gray-200"
                    )}
                  >
                    {pos.position} - {pos.label}
                  </div>
                );
              })}
            </div>
            <div className="grid grid-cols-2 gap-2">
              {controllerPositionMap.slice(2, 4).map((pos, index) => {
                const controllerIndex = index + 2;
                const controller = controllers[controllerIndex];
                return (
                  <div 
                    key={controllerIndex} 
                    className={cn(
                      "h-8 rounded-md transition-colors duration-300 flex items-center justify-center text-xs font-medium text-white shadow-inner",
                      controller ? getHeatIntensity(controllerIndex) : "bg-gray-200"
                    )}
                  >
                    {pos.position} - {pos.label}
                  </div>
                );
              })}
            </div>
          </div>
        </div>
        
        <div className="flex items-center gap-4">
          <div className="flex-1">
            <Slider 
              value={[targetTemp]} 
              min={minTemp} 
              max={maxTemp} 
              step={1}
              onValueChange={handleSliderChange}
              onValueCommit={handleApplyToAll}
              className="mb-2"
            />
            <div className="flex justify-between text-xs text-muted-foreground">
              <span>{minTemp}°C</span>
              <span>{maxTemp}°C</span>
            </div>
          </div>
          <div className="w-20">
            <Input
              type="number"
              min={minTemp}
              max={maxTemp}
              value={inputValue}
              onChange={handleInputChange}
              onBlur={handleInputBlur}
              onKeyDown={handleKeyDown}
              className="font-mono"
            />
          </div>
        </div>
        
        <div className="flex items-center justify-between">
          <Button 
            variant="destructive" 
            size="sm" 
            onClick={handleStopZone} 
            className="gap-1"
          >
            <Square className="h-3 w-3" />
            Stop
          </Button>
          
          {profiles && profiles.length > 0 && onApplyProfileToZone && (
            <DropdownMenu>
              <DropdownMenuTrigger asChild>
                <Button variant="outline" size="sm" className="gap-1">
                  <Play className="h-3 w-3" />
                  Profile <ChevronDown className="h-3 w-3 ml-1" />
                </Button>
              </DropdownMenuTrigger>
              <DropdownMenuContent align="end">
                {profiles.map(profile => (
                  <DropdownMenuItem 
                    key={profile.id}
                    onClick={() => onApplyProfileToZone(zone.id, profile.id)}
                  >
                    {profile.name} ({profile.duration} min)
                  </DropdownMenuItem>
                ))}
              </DropdownMenuContent>
            </DropdownMenu>
          )}
        </div>
      </CardContent>
    </Card>
  );
};

export default ZoneMasterControl;
