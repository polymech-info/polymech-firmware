import React, { useState, useEffect, useMemo } from 'react';
import { Card, CardContent, CardFooter, CardHeader, CardTitle } from '@/components/ui/card';
import { Button } from '@/components/ui/button';
import { Profile, PlotStatus, TemperatureProfileCommand } from '@/types';
import BezierEditor from '@/components/profiles/bezier/BezierEditor';
import { Edit, Trash2, Play, Pause, StopCircle, Copy, CopyPlus } from 'lucide-react';
import { Switch } from "@/components/ui/switch";
import { Label } from "@/components/ui/label";
import { T, translate } from '../../i18n';
import { useModbus } from '../../contexts/ModbusContext';
import { getSlaveIdFromGroup, findCoilForProfile } from '../../lib/controllerUtils';
import { PV_REGISTER_NAME_SUFFIX, PROFILE_REGISTER_NAMES } from '../../constants';
import { useNavigate } from 'react-router-dom';

import {
  Select,
  SelectContent,
  SelectItem,
  SelectTrigger,
  SelectValue,
} from '@/components/ui/select';

interface ProfileCardProps {
  profile: Profile;
  onDelete: (id: string) => void;
  onCommand: (profileSlot: number, command: TemperatureProfileCommand) => void;
  zones?: { id: string, name: string }[];
  onApplyToZone?: (profileId: string, zoneId: string) => void;
  onDuplicate: (profileToDuplicate: Profile) => void;
  onCopyTo: (profileToCopy: Profile) => void;
  canDuplicate?: boolean;
}

const formatDuration = (ms: number): string => {
  if (isNaN(ms) || ms < 0) return '00h 00min 00s';
  const totalSeconds = Math.floor(ms / 1000);
  const hours = Math.floor(totalSeconds / 3600);
  const minutes = Math.floor((totalSeconds % 3600) / 60);
  const seconds = totalSeconds % 60;

  // For durations less than 1 hour, show minutes and seconds
  if (hours === 0) {
    const paddedMinutes = minutes.toString().padStart(2, '0');
    const paddedSeconds = seconds.toString().padStart(2, '0');
    return `${paddedMinutes}min ${paddedSeconds}s`;
  }

  // For durations 1+ hours, show hours and minutes
  const paddedHours = hours.toString().padStart(2, '0');
  const paddedMinutes = minutes.toString().padStart(2, '0');
  return `${paddedHours}h ${paddedMinutes}min`;
};

// Helper function to get status text
const getStatusText = (status: PlotStatus): string => {
  switch (status) {
    case PlotStatus.IDLE:
      return 'Idle';
    case PlotStatus.RUNNING:
      return 'Running';
    case PlotStatus.INITIALIZING:
      return 'Warmup';
    case PlotStatus.PAUSED:
      return 'Paused';
    case PlotStatus.FINISHED:
      return 'Finished';
    case PlotStatus.STOPPED:
      return 'Stopped';
    case PlotStatus.WAITING:
      return 'Waiting';
    default:
      return 'Unknown';
  }
};

const ProfileCard: React.FC<ProfileCardProps> = ({
  profile,
  onDelete,
  onCommand,
  zones,
  onApplyToZone,
  onDuplicate,
  onCopyTo,
  canDuplicate
}) => {
  const navigate = useNavigate();
  const profileId = String(profile.slot);
  const [plainTextDescription, setPlainTextDescription] = useState('');
  const { registers, settings, coils, updateCoil, pressureProfiles, updateRegister } = useModbus();
  const [isToggling, setIsToggling] = useState(false);
  const [sliderValue, setSliderValue] = useState(0);
  const [isDragging, setIsDragging] = useState(false);

  const associatedPressureProfile = useMemo(() => {
    if (profile.pressureProfile === undefined || !pressureProfiles) return undefined;
    return pressureProfiles.find(p => p.slot === profile.pressureProfile);
  }, [profile.pressureProfile, pressureProfiles]);

  const enableCoil = useMemo(() => {
    if (!coils || !profile.name) return null;
    return findCoilForProfile(
      coils,
      profile.name,
      profile.slot,
      PROFILE_REGISTER_NAMES.ENABLED
    );
  }, [coils, profile.name, profile.slot]);

  const isEnabled = enableCoil ? enableCoil.value : false;
  // console.log("isEnabled",profile.enabled,enableCoil);

  const handleToggle = async (newState: boolean) => {
    if (!enableCoil) return;
    setIsToggling(true);
    try {
      await updateCoil(enableCoil.address, newState);
    } catch (error) {
      console.error(`Failed to toggle profile ${profile.slot}`, error);
    } finally {
      setIsToggling(false);
    }
  };

  // Create a name-to-slaveid map from the partition config
  const controllerNameToSlaveIdMap = React.useMemo(() => {
    const map = new Map<string, number>();
    if (!settings) return map;
    settings.partitions.forEach(partition => {
      partition.controllers?.forEach(controller => {
        if (controller.name) {
          map.set(controller.name, controller.slaveid);
        }
      });
    });
    return map;
  }, [settings]);

  const getControllerPv = (controllerName: string): number | string => {
    const slaveid = controllerNameToSlaveIdMap.get(controllerName);

    if (slaveid === undefined) {
      // Fallback or error for controllers not in the static config
      // This might happen if profiles are associated with controllers not in PARTITION_CONFIG
      const fallbackSlaveId = getSlaveIdFromGroup(controllerName);
      if (fallbackSlaveId) {
        // You can decide if you want to support this fallback.
        // For now, let's just log a warning and return N/A if not in the map.
      }
      console.warn(`Could not determine slaveid for controller from settings: ${controllerName}`, controllerNameToSlaveIdMap);
      return 'N/A';
    }

    const pvRegister = registers.find(
      reg => getSlaveIdFromGroup(reg.group) === slaveid && reg.name.endsWith(PV_REGISTER_NAME_SUFFIX)
    );

    if (pvRegister && typeof pvRegister.value === 'number') {
      return pvRegister.value.toFixed(1);
    } else {
      // It's possible for the register to exist but the value not be a number yet.
      // Or for the register not to be found immediately.
      console.error(`PV register not found for controller ${controllerName} (slaveid: ${slaveid})`);
    }

    return 'N/A';
  };

  useEffect(() => {
    const getPlainTextFromMarkdown = async (markdown: string = '') => {
      return markdown;
    };

    if (profile.description) {
      getPlainTextFromMarkdown(profile.description);
    } else {
      setPlainTextDescription('');
    }
  }, [profile.description]);

  return (
    <Card className="w-full flex flex-col glass-card border-0 glass-shimmer hover:shadow-2xl transition-all duration-500">
      <CardHeader className="pb-2">
        <div className="flex justify-between items-start">
          <CardTitle className="text-lg font-semibold accent-text">
            {profile.name}
          </CardTitle>
          <div className="flex items-center space-x-3">
            <div className={`status-indicator ${isEnabled ? 'status-connected' : 'status-disconnected'}`}></div>
            <Switch
              id={`enable-profile-${profile.slot}`}
              checked={isEnabled}
              onCheckedChange={handleToggle}
              disabled={isToggling}
              className="data-[state=checked]:bg-emerald-500"
            />
            <Label htmlFor={`enable-profile-${profile.slot}`} className="text-xs text-slate-600 dark:text-white/70 font-medium">
              {isEnabled ? <T>Enabled</T> : <T>Disabled</T>}
            </Label>
          </div>
        </div>
        <div className="text-xs text-slate-600 dark:text-white/60 pt-1">
          <div className="grid grid-cols-2 gap-x-4 gap-y-1">
            <div>
              <span className="text-slate-600 dark:text-slate-300">{formatDuration(profile.duration)} <T>Total</T></span>
            </div>
            <div>
              <span className="font-semibold text-indigo-600 dark:text-indigo-300"><T>{getStatusText(profile.status)}</T></span>
            </div>
            <div>
              <span className="text-slate-600 dark:text-slate-300">{profile.max}°C <T>Max</T></span>
            </div>
            {(profile.status === PlotStatus.RUNNING || profile.status === PlotStatus.PAUSED || profile.status === PlotStatus.INITIALIZING || profile.status === PlotStatus.WAITING) && (
              <>
                {profile.currentTemp !== undefined && (
                  <div>
                    <span className="font-semibold text-cyan-600 dark:text-cyan-300">{profile.currentTemp}°C <T>Now</T></span>
                  </div>
                )}
                {profile.elapsed !== undefined && (
                  <div>
                    <span className="font-semibold text-emerald-600 dark:text-emerald-300">{formatDuration(profile.elapsed)} <T>Elapsed</T></span>
                  </div>
                )}
                {profile.remaining !== undefined && (
                  <div>
                    <span className="font-semibold text-amber-600 dark:text-amber-300">{formatDuration(profile.remaining)} <T>Remaining</T></span>
                  </div>
                )}

              </>
            )}
          </div>
        </div>
      </CardHeader>
      <CardContent className="space-y-3 pt-2 flex-grow">
        {plainTextDescription && (
          <p className="text-sm text-slate-600 dark:text-white/60 mb-3">
            {plainTextDescription}
          </p>
        )}

        {profile.associatedControllerNames && profile.associatedControllerNames.length > 0 && (
          <div className="pt-2 glass-card p-3">
            <p className="text-xs font-medium text-slate-600 dark:text-slate-300 mb-2">{translate("Associated Controllers:")}</p>
            <div className="grid grid-cols-2 gap-x-4 gap-y-2 text-xs">
              {profile.associatedControllerNames.map((name, index) => {
                const slaveid = controllerNameToSlaveIdMap.get(name);
                return (
                  <div key={index} className="flex justify-between items-center">
                    <span className="text-slate-600 dark:text-white/70">{name}{slaveid !== undefined ? ` (${slaveid})` : ''}:</span>
                    <span className="font-semibold text-indigo-600 dark:text-indigo-300">{getControllerPv(name)}°C</span>
                  </div>
                );
              })}
            </div>
          </div>
        )}

        <div className="flex-grow">
          <BezierEditor
            controlPoints={profile.controlPoints}
            onChange={() => { }}
            max={profile.max}
            duration={profile.duration}
            readonly
            showGridLabels={false}
            className="h-40 w-full"
            elapsedTime={profile.elapsed}
            isRunning={profile.status === PlotStatus.RUNNING || profile.status === PlotStatus.PAUSED || profile.status === PlotStatus.INITIALIZING || profile.status === PlotStatus.WAITING}
            currentTemp={profile.currentTemp}
            pressureProfile={associatedPressureProfile}
            activeView="temperature"
          />
        </div>

        {/* Scrubber / Slider */}
        <div className="px-1 pt-2">
          <div className="flex justify-between text-[10px] text-slate-500 dark:text-slate-400 mb-1">
            <span>{formatDuration(isDragging ? sliderValue * 1000 : profile.elapsed)}</span>
            <span>{formatDuration(profile.duration)}</span>
          </div>
          <input
            type="range"
            min={0}
            max={profile.duration / 1000} // Duration is in ms, slider in seconds
            value={isDragging ? sliderValue : profile.elapsed / 1000} // Elapsed is in ms
            disabled={profile.status === PlotStatus.IDLE || profile.status === PlotStatus.STOPPED}
            onChange={(e) => {
              setSliderValue(parseInt(e.target.value));
              setIsDragging(true);
            }}
            onMouseUp={async (e) => {
              const newValue = parseInt((e.target as HTMLInputElement).value);
              if (!isNaN(newValue)) {
                const elapsedReg = registers.find(r =>
                  r.name.startsWith("Elapsed") &&
                  (r.group === profile.name || r.group === `TempProfile_${profile.id}_Slot_${profile.slot}`)
                );

                if (elapsedReg) {
                  try {
                    console.log("Seeking profile", profile.elapsed, newValue);
                    await updateRegister(elapsedReg.address, newValue);
                  } catch (error) {
                    console.error("Failed to seek profile", error);
                  } finally {
                    setIsDragging(false);
                  }
                } else {
                  console.warn("Could not find ELAPSED register for profile", profile);
                  setIsDragging(false);
                }
              } else {
                setIsDragging(false);
              }
            }}
            onTouchEnd={async () => {
              const newValue = sliderValue;
              const elapsedReg = registers.find(r =>
                r.name.endsWith("Elapsed") &&
                (r.group === profile.name || r.group === `TempProfile_${profile.id}_Slot_${profile.slot}`)
              );

              if (elapsedReg) {
                try {
                  await updateRegister(elapsedReg.address, newValue);
                } catch (error) {
                  console.error("Failed to seek profile", error);
                } finally {
                  setIsDragging(false);
                }
              } else {
                setIsDragging(false);
              }
            }}
            className="w-full h-2 bg-slate-200 rounded-lg appearance-none cursor-pointer dark:bg-slate-700 accent-indigo-600"
          />
        </div>

        {zones && zones.length > 0 && onApplyToZone && (
          <div className="space-y-2 pt-3 glass-card p-3">
            <p className="text-sm font-medium text-slate-600 dark:text-slate-300">Apply to zone:</p>
            <Select onValueChange={(zoneId) => onApplyToZone(profileId, zoneId)}>
              <SelectTrigger className="w-full glass-input">
                <SelectValue placeholder={<T>Select zone</T>} />
              </SelectTrigger>
              <SelectContent className="glass-panel border-0">
                {zones.map((zone) => (
                  <SelectItem key={zone.id} value={zone.id} className="text-slate-700 dark:text-white/90 hover:bg-slate-100 dark:hover:bg-white/10">
                    {zone.name}
                  </SelectItem>
                ))}
              </SelectContent>
            </Select>
          </div>
        )}
      </CardContent>
      <CardFooter className="flex justify-between gap-2 pt-4">
        <Button
          variant="outline"
          size="icon"
          onClick={() => navigate(`/profiles/edit/${profile.slot}`)}
          title={translate("Edit Profile")}
          className="glass-button"
        >
          <Edit className="h-4 w-4" />
        </Button>

        {(profile.status === PlotStatus.IDLE || profile.status === PlotStatus.FINISHED || profile.status === PlotStatus.STOPPED) && (
          <Button
            className="flex-1 gap-1 status-gradient-connected text-white border-0 hover:shadow-lg transition-all duration-300"
            onClick={() => {
              console.log("start profile", profile.slot);
              onCommand(profile.slot, TemperatureProfileCommand.START);
            }}
            title={translate("Start Profile")}
            disabled={false}
          >
            <Play className="h-4 w-4" />
            <T>Start</T>
          </Button>
        )}
        {(profile.status === PlotStatus.RUNNING || profile.status === PlotStatus.INITIALIZING || profile.status === PlotStatus.WAITING) && (
          <Button
            className="flex-1 gap-1 bg-gradient-to-r from-amber-400 to-orange-500 text-white border-0 hover:shadow-lg transition-all duration-300"
            onClick={() => onCommand(profile.slot, TemperatureProfileCommand.PAUSE)}
            title={translate("Pause Profile")}
            disabled={profile.status === PlotStatus.INITIALIZING}
          >
            <Pause className="h-4 w-4" />
            <T>Pause</T>
          </Button>
        )}
        {profile.status === PlotStatus.PAUSED && (
          <Button
            className="flex-1 gap-1 bg-gradient-to-r from-cyan-400 to-blue-500 text-white border-0 hover:shadow-lg transition-all duration-300"
            onClick={() => onCommand(profile.slot, TemperatureProfileCommand.RESUME)}
            title={translate("Resume Profile")}
            disabled={false}
          >
            <Play className="h-4 w-4" />
            <T>Resume</T>
          </Button>
        )}

        {(profile.status === PlotStatus.RUNNING || profile.status === PlotStatus.PAUSED || profile.status === PlotStatus.INITIALIZING || profile.status === PlotStatus.WAITING) && (
          <Button
            variant="outline"
            size="icon"
            className="glass-button border-red-400/50 text-red-300 hover:bg-red-500/20"
            onClick={() => onCommand(profile.slot, TemperatureProfileCommand.STOP)}
            title={translate("Stop Profile")}
            disabled={false}
          >
            <StopCircle className="h-4 w-4" />
          </Button>
        )}

        <Button
          variant="outline"
          size="icon"
          onClick={() => onDuplicate(profile)}
          title={translate("Duplicate Profile")}
          className="glass-button border-cyan-400/50 text-cyan-300 hover:bg-cyan-500/20"
          disabled={!canDuplicate}
        >
          <Copy className="h-4 w-4" />
        </Button>

        <Button
          variant="outline"
          size="icon"
          onClick={() => onCopyTo(profile)}
          title={translate("Copy to existing slot...")}
          className="glass-button border-indigo-400/50 text-indigo-300 hover:bg-indigo-500/20"
        >
          <CopyPlus className="h-4 w-4" />
        </Button>

        <Button
          variant="outline"
          size="icon"
          onClick={() => onDelete(profileId)}
          className="glass-button border-red-400/50 text-red-300 hover:bg-red-500/20"
          title={translate("Delete Profile")}
        >
          <Trash2 className="h-4 w-4" />
        </Button>
      </CardFooter>
    </Card>
  );
};

export default ProfileCard;
