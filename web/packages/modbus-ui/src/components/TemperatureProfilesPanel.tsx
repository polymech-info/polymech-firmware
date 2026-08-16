import React, { useMemo } from 'react';
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card';
import { Progress } from '@/components/ui/progress';
import { Button } from '@/components/ui/button';
import { T } from '../i18n';
import { useToast } from '@/components/ui/use-toast';
import { useModbus } from '@/contexts/ModbusContext';
import { PlotStatus, type Profile, TemperatureProfileCommand } from '@/types';
import { PROFILE_REGISTER_NAMES } from '@/constants';
import { Play, Pause, StopCircle } from 'lucide-react';

const TemperatureProfilesPanel: React.FC = () => {
  const { registers, profiles, updateRegister, isConnected } = useModbus();
  const { toast } = useToast();

  const liveUiProfiles = useMemo((): Profile[] => {
    if (!profiles || !registers) return [];
    return profiles.map(pService => {
      let liveStatus: PlotStatus | undefined = pService.status;
      let liveElapsed: number | undefined = pService.elapsed;
      let liveCurrentTemp: number | undefined = pService.currentTemp;

      const statusRegister = registers.find(r => r.group === pService.name && r.name.startsWith(PROFILE_REGISTER_NAMES.STATUS));
      if (statusRegister && typeof statusRegister.value === 'number' && statusRegister.value in PlotStatus) {
        liveStatus = statusRegister.value as PlotStatus;
      }

      const currentTempRegister = registers.find(r => r.group === pService.name && r.name === PROFILE_REGISTER_NAMES.CURRENT_VALUE);
      if (currentTempRegister && typeof currentTempRegister.value === 'number') {
        liveCurrentTemp = currentTempRegister.value;
      }

      const elapsedRegister = registers.find(r => r.group === pService.name && r.name === PROFILE_REGISTER_NAMES.ELAPSED);
      if (elapsedRegister && typeof elapsedRegister.value === 'number') {
        liveElapsed = elapsedRegister.value * 1000;
      }

      return {
        ...pService,
        status: liveStatus,
        elapsed: liveElapsed,
        currentTemp: liveCurrentTemp,
      };
    });
  }, [profiles, registers]);

  const activeUiProfiles = useMemo((): Profile[] => {
    if (!liveUiProfiles) return [];
    return liveUiProfiles.filter(
      profile => profile.enabled || profile.status === PlotStatus.RUNNING || profile.status === PlotStatus.PAUSED || profile.status === PlotStatus.INITIALIZING
    );
  }, [liveUiProfiles]);

  const handleHmiProfileCommand = async (profile: Profile | null, command: TemperatureProfileCommand) => {
    if (!profile || !profile.name) {
      toast({ title: 'Command Error', description: 'Invalid profile data for command (missing name).', variant: 'destructive' });
      return;
    }
    if (!isConnected) {
      toast({ title: 'Error', description: 'Not connected to Modbus server.', variant: 'destructive' });
      return;
    }

    const commandRegisterEntry = registers.find(reg => reg.group === profile.name && reg.name === PROFILE_REGISTER_NAMES.COMMAND);
    if (!commandRegisterEntry) {
      toast({ title: 'Command Error', description: `Command register (Group: ${profile.name}, Name: ${PROFILE_REGISTER_NAMES.COMMAND}) not found.`, variant: 'destructive' });
      return;
    }

    try {
      await updateRegister(commandRegisterEntry.address, command);
      toast({ title: 'Profile Command Sent', description: `${TemperatureProfileCommand[command]} command sent to profile '${profile.name}'.` });
    } catch (error) {
      toast({ title: 'Command Failed', description: `Failed to send command to profile '${profile.name}': ${error instanceof Error ? error.message : String(error)}`, variant: 'destructive' });
    }
  };

  if (activeUiProfiles.length === 0) return null;

  return (
    <div>
      <h2 className="text-lg font-semibold mb-3 text-slate-700 dark:text-slate-200"><T>Temperature Profiles</T></h2>
      <div className="space-y-3">
        {activeUiProfiles.map(profile => (
          <Card key={profile.slot} className="glass-card shadow-xl w-full">
            <CardHeader className="pb-2 pt-3 flex flex-row justify-between items-center">
              <CardTitle className="text-md font-semibold text-slate-700 dark:text-white">
                {profile.status === PlotStatus.RUNNING ? 'Running Profile: ' : profile.status === PlotStatus.PAUSED ? 'Paused Profile: ' : 'Profile: '}
                <span className="text-indigo-600 dark:text-cyan-400 font-bold">{profile.name}</span>
                <span> (Slot: {profile.slot})</span>
              </CardTitle>
              <div className="flex items-center space-x-2">
                {profile.enabled && profile.status !== PlotStatus.RUNNING && profile.status !== PlotStatus.PAUSED && (
                  <Button onClick={() => handleHmiProfileCommand(profile, TemperatureProfileCommand.START)} title="Start Profile" className="px-2 py-1 h-auto glass-button status-gradient-connected text-white border-0">
                    <Play className="h-4 w-4 mr-1" /> Start
                  </Button>
                )}
                {profile.status === PlotStatus.RUNNING && (
                  <Button onClick={() => handleHmiProfileCommand(profile, TemperatureProfileCommand.PAUSE)} title="Pause Profile" className="px-2 py-1 h-auto glass-button bg-gradient-to-r from-amber-400 to-orange-500 text-white border-0" disabled={!profile.enabled}>
                    <Pause className="h-4 w-4 mr-1" /> Pause
                  </Button>
                )}
                {profile.status === PlotStatus.PAUSED && (
                  <Button onClick={() => handleHmiProfileCommand(profile, TemperatureProfileCommand.RESUME)} title="Resume Profile" className="px-2 py-1 h-auto glass-button status-gradient-connected text-white border-0" disabled={!profile.enabled}>
                    <Play className="h-4 w-4 mr-1" /> Resume
                  </Button>
                )}
                {(profile.status === PlotStatus.RUNNING || profile.status === PlotStatus.PAUSED) && (
                  <Button onClick={() => handleHmiProfileCommand(profile, TemperatureProfileCommand.STOP)} title="Stop Profile" className="px-2 py-1 h-auto glass-button status-gradient-error text-white border-0" disabled={!profile.enabled}>
                    <StopCircle className="h-4 w-4 mr-1" /> Stop
                  </Button>
                )}
              </div>
            </CardHeader>
            <CardContent className="pt-1 pb-3">
              {(profile.status === PlotStatus.RUNNING || profile.status === PlotStatus.PAUSED) && profile.duration > 0 && (
                <div className="space-y-2 pt-1">
                  {profile.currentTemp !== undefined && (
                    <div className="text-center">
                      <span className="font-bold text-lg text-indigo-600 dark:text-cyan-400">{profile.currentTemp.toFixed(1)}°C</span>
                    </div>
                  )}
                  {profile.elapsed !== undefined && (
                    <>
                      <Progress value={(profile.elapsed / profile.duration) * 100} className="w-full h-3 glass-progress" />
                      <div className="text-xs text-slate-500 dark:text-slate-400 flex justify-between">
                        <span>
                          <T>Elapsed</T>: {Math.floor(profile.elapsed / 60000)}m {Math.floor((profile.elapsed % 60000) / 1000)}s
                        </span>
                        <span>
                          <T>Total</T>: {Math.floor(profile.duration / 60000)}m {Math.floor((profile.duration % 60000) / 1000)}s
                        </span>
                      </div>
                    </>
                  )}
                </div>
              )}
              {profile.status === PlotStatus.PAUSED && profile.elapsed === undefined && (
                <div className="text-xs text-slate-500 dark:text-slate-400">
                  <span>
                    <T>Total Duration</T>: {Math.floor(profile.duration / 60000)}m {Math.floor((profile.duration % 60000) / 1000)}s - <T>Paused</T>
                  </span>
                </div>
              )}
            </CardContent>
          </Card>
        ))}
      </div>
    </div>
  );
};

export default TemperatureProfilesPanel;


