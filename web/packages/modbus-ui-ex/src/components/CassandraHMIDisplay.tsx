import React, { useMemo, useState, useEffect } from 'react';
import { useModbus } from '@/contexts/ModbusContext';
import type { RegisterData } from '@/contexts/ModbusContext';
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card';
import { T } from '../i18n';

import CassandraControllerCard from './CassandraControllerCard';
import SequentialHeatingCard from './SequentialHeatingCard';
import ControllerChart from './ControllerChart';
import { Progress } from "@/components/ui/progress";
import { PlotStatus, type Profile, TemperatureProfileCommand } from "@/types";
import { Play, Pause, StopCircle, ChevronDown, ChevronUp } from "lucide-react";
import { Button } from "@/components/ui/button";
import { useToast } from "@/components/ui/use-toast";
import CollapsibleSection from './CollapsibleSection';

import { 
  PV_REGISTER_NAME_SUFFIX, 
  SP_CMD_COMMAND_REGISTER_PREFIX, 
  PROFILE_REGISTER_NAMES
} from '@/constants';

import { 
  PARTITION_CONFIG, 
  getSlaveIdFromGroup, 
  ControllerConfig,
  STATUS_BITS,
  checkStatusBit
} from '@/lib/controllerUtils';

interface ControllerDisplayData {
  slaveId: number;
  name?: string; // Optional name for display
  pv: number | string;
  sp: number | string;
  isRunning: boolean;
  hasAlarm: boolean;
  isAutoTuning: boolean;
  hasHeaterBreak: boolean;
  hasSensorBreak: boolean;
  mode: 'manual' | 'auto' | 'cascade' | 'program' | 'unknown';
  currentProfileName?: string | null; // Added for active profile name
  isHeating: boolean; // Added for heating status
}

interface PartitionDisplayData {
  name: string;
  controllers: ControllerDisplayData[];
}

const STATUS_HIGH_REGISTER_NAME = "Status High";
const STATUS_LOW_REGISTER_NAME = "Status Low";
const HMI_OPEN_PARTITIONS_STORAGE_KEY = 'hmiOpenPartitions'; // localStorage key for partitions

interface ControllerStatus {
  isRunning: boolean;
  hasAlarm: boolean;
  isAutoTuning: boolean;
  hasHeaterBreak: boolean;
  hasSensorBreak: boolean;
  mode: 'manual' | 'auto' | 'cascade' | 'program' | 'unknown';
  isHeating: boolean; // Added for heating status
}

// Helper function to get controller status
const getControllerStatus = (statusHigh: RegisterData | undefined, statusLow: RegisterData | undefined): ControllerStatus => 
{
  if (!statusHigh || !statusLow || typeof statusHigh.value !== 'number' || typeof statusLow.value !== 'number') {
    return {
      isRunning: false,
      hasAlarm: false,
      isAutoTuning: false,
      hasHeaterBreak: false,
      hasSensorBreak: false,
      mode: 'unknown',
      isHeating: false, // Default to false
    };
  }
  const high = statusHigh.value;
  const low = statusLow.value;
  
  // Determine operation mode
  let mode: ControllerStatus['mode'] = 'unknown';
  if (checkStatusBit(high, low, STATUS_BITS.MANUAL)) mode = 'manual';
  else if (checkStatusBit(high, low, STATUS_BITS.AUTO)) mode = 'auto';
  else if (checkStatusBit(high, low, STATUS_BITS.CASCADE)) mode = 'cascade';
  else if (checkStatusBit(high, low, STATUS_BITS.PROGRAM)) mode = 'program';

  return {
    isRunning: !checkStatusBit(high, low, STATUS_BITS.RUN_STOP), // RunStop is inverted
    hasAlarm: checkStatusBit(high, low, STATUS_BITS.ALARM),
    isAutoTuning: checkStatusBit(high, low, STATUS_BITS.AT),
    hasHeaterBreak: checkStatusBit(high, low, STATUS_BITS.HEATER_BREAK),
    hasSensorBreak: checkStatusBit(high, low, STATUS_BITS.SENSOR_BREAK),
    mode,
    isHeating: checkStatusBit(high, low, STATUS_BITS.Control_OutputOpenOutput), // Check heating status
  };
};

const CassandraHMIDisplay = () => {
  const { 
    registers: allModbusRegisters, 
    profiles: contextProfiles, 
    updateRegister,
    isConnected 
  } = useModbus(); 
  const { toast } = useToast();

  const [openPartitions, setOpenPartitions] = useState<Record<string, boolean>>(() => {
    try {
      const storedValue = localStorage.getItem(HMI_OPEN_PARTITIONS_STORAGE_KEY);
      if (storedValue) {
        return JSON.parse(storedValue);
      }
    } catch (error) {
      console.error("Error reading open partitions from localStorage:", error);
    }
    return {}; // Default to all closed if nothing in storage or error
  });

  useEffect(() => {
    try {
      localStorage.setItem(HMI_OPEN_PARTITIONS_STORAGE_KEY, JSON.stringify(openPartitions));
    } catch (error) {
      console.error("Error saving open partitions to localStorage:", error);
    }
  }, [openPartitions]);

  const togglePartition = (partitionName: string) => {
    setOpenPartitions(prev => ({
      ...prev,
      [partitionName]: !prev[partitionName]
    }));
  };

  const liveUiProfiles = useMemo((): Profile[] => {
    if (!contextProfiles || !allModbusRegisters) return [];

    return contextProfiles.map(pService => {
      let liveStatus: PlotStatus | undefined = pService.status; 
      let liveElapsed: number | undefined = pService.elapsed;

      // Find Status Register using profile name as group and constant as register name
      const statusRegister = allModbusRegisters.find(
        r => r.group === pService.name && r.name === PROFILE_REGISTER_NAMES.STATUS
      );
      if (statusRegister && typeof statusRegister.value === 'number' && statusRegister.value in PlotStatus) {
        liveStatus = statusRegister.value as PlotStatus;
      }

      // Find Elapsed Time Registers (LW and HW)
      const elapsedLwRegister = allModbusRegisters.find(
        r => r.group === pService.name && r.name === PROFILE_REGISTER_NAMES.ELAPSED_LW
      );
      const elapsedHwRegister = allModbusRegisters.find(
        r => r.group === pService.name && r.name === PROFILE_REGISTER_NAMES.ELAPSED_HW
      );

      if (elapsedLwRegister && typeof elapsedLwRegister.value === 'number' && 
          elapsedHwRegister && typeof elapsedHwRegister.value === 'number') {
        liveElapsed = (elapsedHwRegister.value << 16) | elapsedLwRegister.value;
      }
      
      return {
        ...pService,
        status: liveStatus,
        elapsed: liveElapsed,
      };
    });
  }, [contextProfiles, allModbusRegisters]);

  const activeUiProfiles = useMemo((): Profile[] => {
    if (!liveUiProfiles) return [];
    return liveUiProfiles.filter(
      profile => profile.status === PlotStatus.RUNNING || profile.status === PlotStatus.PAUSED
    );
  }, [liveUiProfiles]);

  const handleHmiProfileCommand = async (profile: Profile | null, command: TemperatureProfileCommand) => {
    if (!profile || !profile.name) { 
      toast({ title: "Command Error", description: "Invalid profile data for command (missing name).", variant: "destructive" });
      return;
    }
    if (!isConnected) {
      toast({ title: "Error", description: "Not connected to Modbus server.", variant: "destructive" });
      return;
    }

    // Find the command register using profile name as group and constant as register name
    const commandRegisterEntry = allModbusRegisters.find(
      reg => reg.group === profile.name && reg.name === PROFILE_REGISTER_NAMES.COMMAND
    );

    if (!commandRegisterEntry) {
      toast({ 
        title: "Command Error", 
        description: `Command register (Group: ${profile.name}, Name: ${PROFILE_REGISTER_NAMES.COMMAND}) not found.`, 
        variant: "destructive"
      });
      return;
    }

    try {
      await updateRegister(commandRegisterEntry.address, command);
      toast({ 
        title: "Profile Command Sent", 
        description: `${TemperatureProfileCommand[command]} command sent to profile '${profile.name}'.`
      });
    } catch (error) {
      toast({ 
        title: "Command Failed", 
        description: `Failed to send command to profile '${profile.name}': ${error instanceof Error ? error.message : String(error)}`, 
        variant: "destructive" 
      });
    }
  };

  const processedData = useMemo((): PartitionDisplayData[] => {
    return PARTITION_CONFIG.map(partition => {
      let controllerConfigs: ControllerConfig[] = [];
      if (partition.controllers && partition.controllers.length > 0) {
        controllerConfigs = partition.controllers;
      } else if (partition.startSlaveId !== undefined && partition.numControllers !== undefined) {
        for (let i = 0; i < partition.numControllers; i++) {
          controllerConfigs.push({ 
            slaveId: partition.startSlaveId + i, 
            name: `Controller ${partition.startSlaveId + i}`
          });
        }
      }

      const controllersData: ControllerDisplayData[] = controllerConfigs.map(config => {
        const pvRegister = allModbusRegisters.find((reg: RegisterData) => 
          getSlaveIdFromGroup(reg.group) === config.slaveId && reg.name.endsWith(PV_REGISTER_NAME_SUFFIX)
        );
        const statusHighRegister = allModbusRegisters.find((reg: RegisterData) =>
          getSlaveIdFromGroup(reg.group) === config.slaveId && reg.name === STATUS_HIGH_REGISTER_NAME
        );
        const statusLowRegister = allModbusRegisters.find((reg: RegisterData) =>
          getSlaveIdFromGroup(reg.group) === config.slaveId && reg.name === STATUS_LOW_REGISTER_NAME
        );

        const status = getControllerStatus(statusHighRegister, statusLowRegister);
        let activeProfileName: string | null = null;

        const spCmdRegisterForController = allModbusRegisters.find(
          reg => getSlaveIdFromGroup(reg.group) === config.slaveId && 
                 reg.name.startsWith(SP_CMD_COMMAND_REGISTER_PREFIX)
        );

        if (spCmdRegisterForController && liveUiProfiles) {
          const controllerSpCmdAddress = spCmdRegisterForController.address;
          for (const profile of liveUiProfiles) {
            if ((profile.status === PlotStatus.RUNNING || profile.status === PlotStatus.PAUSED) &&
                profile.targetRegisters &&
                profile.targetRegisters.includes(controllerSpCmdAddress)) {
              activeProfileName = profile.name;
              break;
            }
          }
        }

        return {
          slaveId: config.slaveId,
          name: config.name,
          pv: pvRegister ? pvRegister.value : "N/A",
          sp: "N/A", 
          ...status,
          currentProfileName: activeProfileName,
          isHeating: status.isHeating, // Pass heating status
        };
      });

      return {
        name: partition.name,
        controllers: controllersData,
      };
    });
  }, [allModbusRegisters, liveUiProfiles, SP_CMD_COMMAND_REGISTER_PREFIX]);

  if (!isConnected && (!allModbusRegisters || allModbusRegisters.length === 0)) {
    return (
      <div className="p-4 text-center">
        <p className="text-muted-foreground"><T>Connect to a Modbus server to see controller data.</T></p>
      </div>
    );
  }
  
  if (allModbusRegisters.length === 0 && isConnected) {
    return (
        <div className="p-4 text-center">
            <p className="text-muted-foreground"><T>Connected, but no register data received yet. Waiting for data...</T></p>
        </div>
    );
  }

  return (
    <div className="space-y-3 md:space-y-6">
      {/* Display ALL Active (Running or Paused) Profile Information */}
      {activeUiProfiles.length > 0 && (
        <CollapsibleSection 
          title={<T>Active Temperature Profiles</T>} 
          initiallyOpen={true}
          storageKey="hmiActiveProfilesOpen"
          className="mb-4"
          titleClassName="text-lg font-semibold text-primary"
        >
          <div className="space-y-3 pt-2">
            {activeUiProfiles.map(profile => (
              <Card key={profile.slot} className="shadow-md w-full">
                <CardHeader className="pb-2 pt-3 flex flex-row justify-between items-center">
                  <CardTitle className="text-md font-semibold">
                    {profile.status === PlotStatus.RUNNING && (
                      <>Running Profile: <span className="text-primary">{profile.name}</span> (Slot: {profile.slot})</>
                    )}
                    {profile.status === PlotStatus.PAUSED && (
                      <>Paused Profile: <span className="text-primary">{profile.name}</span> (Slot: {profile.slot})</>
                    )}
                    {profile.status !== PlotStatus.RUNNING && profile.status !== PlotStatus.PAUSED && (
                      <>Profile: <span className="text-primary">{profile.name}</span> (Slot: {profile.slot})</>
                    )}
                  </CardTitle>
                  <div className="flex items-center space-x-2">
                    {profile.status === PlotStatus.RUNNING && (
                      <Button 
                        variant="outline"
                        size="sm"
                        onClick={() => handleHmiProfileCommand(profile, TemperatureProfileCommand.PAUSE)}
                        title="Pause Profile"
                        className="px-2 py-1 h-auto"
                        disabled={!profile.enabled} 
                      >
                        <Pause className="h-4 w-4 mr-1" /> Pause
                      </Button>
                    )}
                    {profile.status === PlotStatus.PAUSED && (
                      <Button 
                        variant="outline"
                        size="sm"
                        onClick={() => handleHmiProfileCommand(profile, TemperatureProfileCommand.RESUME)}
                        title="Resume Profile"
                        className="px-2 py-1 h-auto"
                        disabled={!profile.enabled}
                      >
                        <Play className="h-4 w-4 mr-1" /> Resume
                      </Button>
                    )}
                    {(profile.status === PlotStatus.RUNNING || profile.status === PlotStatus.PAUSED) && (
                      <Button 
                        variant="destructive"
                        size="sm" 
                        onClick={() => handleHmiProfileCommand(profile, TemperatureProfileCommand.STOP)}
                        title="Stop Profile"
                        className="px-2 py-1 h-auto"
                        disabled={!profile.enabled} 
                      >
                        <StopCircle className="h-4 w-4 mr-1" /> Stop
                      </Button>
                    )}
                  </div>
                </CardHeader>
                <CardContent className="pt-1 pb-3">
                  {(profile.status === PlotStatus.RUNNING || profile.status === PlotStatus.PAUSED) && 
                   profile.duration > 0 && profile.elapsed !== undefined && (
                    <div className="space-y-1">
                      <Progress 
                        value={(profile.elapsed / profile.duration) * 100} 
                        className="w-full h-3"
                      />
                      <div className="text-xs text-muted-foreground flex justify-between">
                        <span>
                          <T>Elapsed</T>: {Math.floor(profile.elapsed / 60000)}m {Math.floor((profile.elapsed % 60000) / 1000)}s
                        </span>
                        <span>
                          <T>Total</T>: {Math.floor(profile.duration / 60000)}m {Math.floor((profile.duration % 60000) / 1000)}s
                        </span>
                      </div>
                    </div>
                  )}
                   {profile.status === PlotStatus.PAUSED && profile.elapsed === undefined && (
                      <div className="text-xs text-muted-foreground">
                          <span><T>Total Duration</T>: {Math.floor(profile.duration / 60000)}m {Math.floor((profile.duration % 60000) / 1000)}s - <T>Paused</T></span>
                      </div>
                   )}
                </CardContent>
              </Card>
            ))}
          </div>
        </CollapsibleSection>
      )}

      <CollapsibleSection 
        title={<T>Controller Chart</T>} 
        initiallyOpen={false} 
        storageKey="hmiControllerChartOpen"
        titleClassName="text-lg font-semibold"
      >
        <ControllerChart />
      </CollapsibleSection>

      <CollapsibleSection 
        title={<T>Sequential Heating Control</T>} 
        initiallyOpen={false}
        storageKey="hmiSequentialHeatingOpen"
        titleClassName="text-lg font-semibold"
      >
        <SequentialHeatingCard slaveId={4101} />
      </CollapsibleSection>
      
      {processedData.map(partition => {
        const isPartitionOpen = openPartitions[partition.name] ?? false;
        return (
          <Card key={partition.name} className="shadow-lg">
            <CardHeader 
              className="pb-3 pt-4 cursor-pointer flex flex-row justify-between items-center" 
              onClick={() => togglePartition(partition.name)}
            >
              <CardTitle className="text-lg"><T>{partition.name}</T></CardTitle>
              <Button variant="ghost" size="sm" className="p-1 h-auto">
                {isPartitionOpen ? <ChevronUp className="h-5 w-5" /> : <ChevronDown className="h-5 w-5" />}
              </Button>
            </CardHeader>
            {isPartitionOpen && (
              <CardContent className="grid grid-cols-1 md:grid-cols-2 gap-3 pt-3">
                {partition.controllers.length === 0 && (
                  <p className="text-sm text-muted-foreground md:col-span-2"><T>No controllers configured or found for this partition.</T></p>
                )}
                {partition.controllers.map(controller => (
                  <CassandraControllerCard 
                    key={controller.slaveId} 
                    slaveId={controller.slaveId} 
                    name={controller.name} 
                    pv={controller.pv} 
                    isRunning={controller.isRunning}
                    hasAlarm={controller.hasAlarm}
                    isAutoTuning={controller.isAutoTuning}
                    hasHeaterBreak={controller.hasHeaterBreak}
                    hasSensorBreak={controller.hasSensorBreak}
                    mode={controller.mode}
                    currentProfile={controller.currentProfileName}
                    isHeating={controller.isHeating}
                  />
                ))}
              </CardContent>
            )}
          </Card>
        );
      })}
    </div>
  );
};

export default CassandraHMIDisplay; 