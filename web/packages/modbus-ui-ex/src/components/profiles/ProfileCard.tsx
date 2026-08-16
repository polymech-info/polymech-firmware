import React, { useState, useEffect } from 'react';
import { Card, CardContent, CardFooter, CardHeader, CardTitle } from '@/components/ui/card';
import { Button } from '@/components/ui/button';
import { Profile, PlotStatus, TemperatureProfileCommand } from '@/types';
import BezierEditor from '@/components/profiles/bezier/BezierEditor';
import { Edit, Trash2, Play, Pause, StopCircle, Copy } from 'lucide-react';
import { Switch } from "@/components/ui/switch";
import { Label } from "@/components/ui/label";
import { T, translate } from '../../i18n';
import { marked } from 'marked';

import {
  Select,
  SelectContent,
  SelectItem,
  SelectTrigger,
  SelectValue,
} from '@/components/ui/select';

interface ProfileCardProps {
  profile: Profile;
  onEdit: (profile: Profile) => void;
  onDelete: (id: string) => void;
  onCommand: (profileSlot: number, command: TemperatureProfileCommand) => void;
  onEnableToggle: (profileSlot: number, profileName: string, newState: boolean) => void;
  zones?: { id: string, name: string }[];
  onApplyToZone?: (profileId: string, zoneId: string) => void;
  onDuplicate: (profileToDuplicate: Profile) => void;
  canDuplicate?: boolean;
}

const formatDuration = (ms: number): string => {
  if (isNaN(ms) || ms < 0) return '00h 00min';
  const totalSeconds = Math.floor(ms / 1000);
  const hours = Math.floor(totalSeconds / 3600);
  const minutes = Math.floor((totalSeconds % 3600) / 60);
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
    case PlotStatus.PAUSED:
      return 'Paused';
    case PlotStatus.FINISHED:
      return 'Finished';
    case PlotStatus.STOPPED:
      return 'Stopped';
    default:
      return 'Unknown';
  }
};

const ProfileCard: React.FC<ProfileCardProps> = ({
  profile,
  onEdit,
  onDelete,
  onCommand,
  onEnableToggle,
  zones,
  onApplyToZone,
  onDuplicate,
  canDuplicate
}) => {
  const profileId = String(profile.slot);
  const [plainTextDescription, setPlainTextDescription] = useState('');

  useEffect(() => {
    const getPlainTextFromMarkdown = async (markdown: string = '') => {
      const html = await marked.parse(markdown);
      if (typeof DOMParser === 'undefined') {
        setPlainTextDescription(html.replace(/<[^>]+>/g, ''));
        return;
      }
      const doc = new DOMParser().parseFromString(html, 'text/html');
      setPlainTextDescription(doc.body.textContent || '');
    };

    if (profile.description) {
      getPlainTextFromMarkdown(profile.description);
    } else {
      setPlainTextDescription('');
    }
  }, [profile.description]);

  return (
    <Card className="w-full flex flex-col">
      <CardHeader className="pb-2">
        <div className="flex justify-between items-start">
          <CardTitle className="text-lg font-semibold">
            {profile.name}
          </CardTitle>
          <div className="flex items-center space-x-2">
            <Switch 
              id={`enable-profile-${profile.slot}`}
              checked={profile.enabled}
              onCheckedChange={(newState) => onEnableToggle(profile.slot, profile.name, newState)}
              disabled={profile.status === PlotStatus.RUNNING || profile.status === PlotStatus.PAUSED}
            />
            <Label htmlFor={`enable-profile-${profile.slot}`} className="text-xs text-muted-foreground">
              {profile.enabled ? <T>Enabled</T> : <T>Disabled</T>}
            </Label>
          </div>
        </div>
        <div className="text-xs text-muted-foreground pt-1">
          <div className="grid grid-cols-2 gap-x-4 gap-y-1">
            <div>
              <span>{formatDuration(profile.duration)} <T>Total</T></span>
            </div>
            <div>
              <span className="font-semibold"><T>{getStatusText(profile.status)}</T></span>
            </div>
            <div>
              <span>{profile.max}°C <T>Max</T></span>
            </div>
            {(profile.status === PlotStatus.RUNNING || profile.status === PlotStatus.PAUSED) && (
              <>
                {profile.currentTemp !== undefined && (
                  <div>
                    <span className="font-semibold text-sky-600">{profile.currentTemp}°C <T>Now</T></span>
                  </div>
                )}
                {profile.elapsed !== undefined && (
                  <div>
                    <span className="font-semibold text-green-600">{formatDuration(profile.elapsed)} <T>Elapsed</T></span>
                  </div>
                )}
                {profile.remaining !== undefined && (
                  <div>
                    <span className="font-semibold text-amber-600">{formatDuration(profile.remaining)} <T>Remaining</T></span>
                  </div>
                )}
              </>
            )}
          </div>
        </div>
      </CardHeader>
      <CardContent className="space-y-3 pt-2 flex-grow">
        {plainTextDescription && (
          <p className="text-sm text-muted-foreground mb-3">
            {plainTextDescription}
          </p>
        )}
        
        {profile.associatedControllerNames && profile.associatedControllerNames.length > 0 && (
          <div className="pt-2">
            <p className="text-xs font-medium text-muted-foreground">{translate("Associated Controllers:")}</p>
            <ul className="list-disc list-inside text-xs text-muted-foreground">
              {profile.associatedControllerNames.map((name, index) => (
                <li key={index}>{name}</li>
              ))}
            </ul>
          </div>
        )}

        <div className="flex-grow">
          <BezierEditor
            controlPoints={profile.controlPoints}
            onChange={() => {}}
            max={profile.max}
            duration={profile.duration}
            readonly
            showGridLabels={false}
            className="h-40 w-full"
            elapsedTime={profile.elapsed}
            isRunning={profile.status === PlotStatus.RUNNING || profile.status === PlotStatus.PAUSED}
            currentTemp={profile.currentTemp}
          />
        </div>
        
        {zones && zones.length > 0 && onApplyToZone && (
          <div className="space-y-2 pt-3">
            <p className="text-sm font-medium">Apply to zone:</p>
            <Select onValueChange={(zoneId) => onApplyToZone(profileId, zoneId)}>
              <SelectTrigger className="w-full">
                <SelectValue placeholder={<T>Select zone</T>} />
              </SelectTrigger>
              <SelectContent>
                {zones.map((zone) => (
                  <SelectItem key={zone.id} value={zone.id}>
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
          onClick={() => onEdit(profile)}
          title={translate("Edit Profile")}
        >
          <Edit className="h-4 w-4" />
        </Button>
        
        {(profile.status === PlotStatus.IDLE || profile.status === PlotStatus.FINISHED || profile.status === PlotStatus.STOPPED) && (
          <Button 
            className="flex-1 gap-1 bg-green-600 hover:bg-green-700"
            onClick={() => onCommand(profile.slot, TemperatureProfileCommand.START)}
            title={translate("Start Profile")}
            disabled={!profile.enabled}
          >
            <Play className="h-4 w-4" />
            <T>Start</T>
          </Button>
        )}
        {profile.status === PlotStatus.RUNNING && (
          <Button 
            className="flex-1 gap-1 bg-amber-500 hover:bg-amber-600"
            onClick={() => onCommand(profile.slot, TemperatureProfileCommand.PAUSE)}
            title={translate("Pause Profile")}
            disabled={!profile.enabled}
          >
            <Pause className="h-4 w-4" />
            <T>Pause</T>
          </Button>
        )}
        {profile.status === PlotStatus.PAUSED && (
          <Button 
            className="flex-1 gap-1 bg-sky-500 hover:bg-sky-600"
            onClick={() => onCommand(profile.slot, TemperatureProfileCommand.RESUME)}
            title={translate("Resume Profile")}
            disabled={!profile.enabled}
          >
            <Play className="h-4 w-4" />
            <T>Resume</T>
          </Button>
        )}

        {(profile.status === PlotStatus.RUNNING || profile.status === PlotStatus.PAUSED) && (
          <Button 
            variant="outline" 
            size="icon" 
            className="text-destructive border-destructive hover:bg-destructive/10"
            onClick={() => onCommand(profile.slot, TemperatureProfileCommand.STOP)}
            title={translate("Stop Profile")}
            disabled={!profile.enabled}
          >
            <StopCircle className="h-4 w-4" />
          </Button>
        )}

        <Button
          variant="outline"
          size="icon"
          onClick={() => onDuplicate(profile)}
          title={translate("Duplicate Profile")}
          className="text-sky-600 border-sky-600 hover:bg-sky-600/10"
          disabled={!canDuplicate}
        >
          <Copy className="h-4 w-4" />
        </Button>

        <Button 
          variant="outline" 
          size="icon"
          onClick={() => onDelete(profileId)}
          className="text-destructive border-destructive hover:bg-destructive/10"
          title={translate("Delete Profile")}
        >
          <Trash2 className="h-4 w-4" />
        </Button>
      </CardFooter>
    </Card>
  );
};

export default ProfileCard;
