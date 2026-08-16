import React from 'react';
import { Button } from '@/components/ui/button';
import { useNavigate } from 'react-router-dom';
import { Play, Pause, StopCircle, Pencil } from 'lucide-react';
import { translate } from '../../i18n';
import { PressureProfile, PlotStatus, PlotCommand } from '@/types';
import { useModbus } from '@/contexts/ModbusContext';
import { toast } from 'sonner';
import { PROFILE_REGISTER_NAMES } from '@/constants';

interface PressureProfileRowProps {
    profile: PressureProfile;
}

const formatDuration = (ms: number): string => {
    if (isNaN(ms) || ms < 0) return '00:00';
    const totalSeconds = Math.floor(ms / 1000);
    const minutes = Math.floor(totalSeconds / 60);
    const seconds = totalSeconds % 60;
    return `${minutes}:${seconds.toString().padStart(2, '0')}`;
};

const getStatusColor = (status: PlotStatus) => {
    switch (status) {
        case PlotStatus.RUNNING: return 'text-green-500';
        case PlotStatus.PAUSED: return 'text-amber-500';
        case PlotStatus.FINISHED: return 'text-blue-500';
        case PlotStatus.STOPPED: return 'text-red-500';
        default: return 'text-slate-500';
    }
};

const getStatusText = (status: PlotStatus) => {
    switch (status) {
        case PlotStatus.RUNNING: return 'Running';
        case PlotStatus.PAUSED: return 'Paused';
        case PlotStatus.FINISHED: return 'Finished';
        case PlotStatus.STOPPED: return 'Stopped';
        case PlotStatus.INITIALIZING: return 'Init';
        default: return 'Idle';
    }
};

const PressureProfileRow: React.FC<PressureProfileRowProps> = ({ profile }) => {
    const { updateRegister, registers } = useModbus();
    const navigate = useNavigate();

    // The profile object has an 'id' that corresponds to the component ID of the PressureProfile instance on the server.
    // Modbus registers also have an 'id' field that corresponds to the component ID they belong to.
    // We should match registers based on this 'id'.

    const allProfileRegisters = registers.filter(r => r.id === String(profile.id));
    const statusReg = allProfileRegisters.find(r => r.name.startsWith(PROFILE_REGISTER_NAMES.STATUS));
    const elapsedReg = allProfileRegisters.find(r => r.name === PROFILE_REGISTER_NAMES.ELAPSED);
    const durationReg = allProfileRegisters.find(r => r.name === PROFILE_REGISTER_NAMES.DURATION);
    const commandReg = allProfileRegisters.find(r => r.name === PROFILE_REGISTER_NAMES.COMMAND);

    const currentStatus = statusReg?.value !== undefined ? (statusReg.value as PlotStatus) : profile.status;

    // Elapsed and Duration are in seconds from the server (Modbus registers)
    // formatDuration expects milliseconds, so we need to multiply by 1000
    const currentElapsed = elapsedReg?.value !== undefined ? (elapsedReg.value as number) * 1000 : profile.elapsed;
    const currentDuration = durationReg?.value !== undefined ? (durationReg.value as number) * 1000 : profile.duration;


    const handleCommand = async (command: PlotCommand) => {
        if (!commandReg) {
            toast.error(translate('Command register not found for profile {name}').replace('{name}', profile.name));
            return;
        }

        try {
            await updateRegister(commandReg.address, command);
            toast.success(translate('Command sent to {name}').replace('{name}', profile.name));
        } catch (error) {
            toast.error(translate('Failed to send command: {error}').replace('{error}', String(error)));
        }
    };

    const isRunning = currentStatus === PlotStatus.RUNNING;
    const isPaused = currentStatus === PlotStatus.PAUSED;
    const isActive = isRunning || isPaused || currentStatus === PlotStatus.INITIALIZING;

    return (
        <div className="relative flex items-center justify-between p-2 rounded-lg border border-slate-200 dark:border-slate-700 bg-white/50 dark:bg-slate-800/50 hover:bg-white/80 dark:hover:bg-slate-800/80 transition-colors">
            <div className="flex flex-col min-w-0 flex-1 mr-2">
                <div className="font-medium text-sm truncate" title={profile.name}>{profile.name}</div>
                <div className="flex items-center gap-2 text-xs text-muted-foreground">
                    <span className={getStatusColor(currentStatus)}>{translate(getStatusText(currentStatus))}</span>
                    {isActive && (
                        <span>
                            {formatDuration(currentElapsed)} / {formatDuration(currentDuration)}
                        </span>
                    )}
                </div>
            </div>

            <div className="flex items-center gap-1">
                <Button
                    size="icon"
                    variant="ghost"
                    className="h-8 w-8 text-slate-600 hover:text-slate-700 hover:bg-slate-100 dark:hover:bg-slate-800/50"
                    onClick={() => navigate(`/pressure-profiles/edit/${profile.slot}`)}
                    title={translate("Edit Profile")}
                >
                    <Pencil className="h-4 w-4" />
                </Button>
                {!isRunning && (
                    <Button
                        size="icon"
                        variant="ghost"
                        className="h-8 w-8 text-green-600 hover:text-green-700 hover:bg-green-100 dark:hover:bg-green-900/30"
                        onClick={() => handleCommand(PlotCommand.START)}
                        title={translate(isPaused ? "Resume" : "Start")}
                    >
                        <Play className="h-4 w-4" />
                    </Button>
                )}

                {isRunning && (
                    <Button
                        size="icon"
                        variant="ghost"
                        className="h-8 w-8 text-amber-600 hover:text-amber-700 hover:bg-amber-100 dark:hover:bg-amber-900/30"
                        onClick={() => handleCommand(PlotCommand.PAUSE)}
                        title={translate("Pause")}
                    >
                        <Pause className="h-4 w-4" />
                    </Button>
                )}

                <Button
                    size="icon"
                    variant="ghost"
                    className="h-8 w-8 text-red-600 hover:text-red-700 hover:bg-red-100 dark:hover:bg-red-900/30"
                    onClick={() => handleCommand(PlotCommand.STOP)}
                    disabled={!isActive && currentStatus !== PlotStatus.FINISHED}
                    title={translate("Stop")}
                >
                    <StopCircle className="h-4 w-4" />
                </Button>
            </div>

            {/* Progress bar for active profiles */}
            {isActive && (
                <div className="absolute bottom-0 left-0 h-1 bg-green-500/20 w-full rounded-b-lg overflow-hidden">
                    <div
                        className="h-full bg-green-500 transition-all duration-1000 ease-linear"
                        style={{ width: `${Math.min(100, Math.max(0, (currentElapsed / currentDuration) * 100))}%` }}
                    />
                </div>
            )}
        </div>
    );
};

export default PressureProfileRow;
