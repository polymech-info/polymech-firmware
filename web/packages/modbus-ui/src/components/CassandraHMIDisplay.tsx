import React, { useMemo } from 'react';
import { useModbus } from '@/contexts/ModbusContext';
import type { RegisterData } from '@/contexts/ModbusContext';
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card';
import { T } from '../i18n';

import CassandraControllerCard from './CassandraControllerCard';
import SequentialHeatingCard from './SequentialHeatingCard';
import { PlotStatus, type Profile, TemperatureProfileCommand } from "@/types";
import { useToast } from "@/components/ui/use-toast";
import CollapsibleSection from './CollapsibleSection';
import Commons from './Commons';
import VFDControls from './VFDControls';

import {
  PV_REGISTER_NAME_SUFFIX,
  SP_CMD_COMMAND_REGISTER_PREFIX,
  PROFILE_REGISTER_NAMES
} from '@/constants';

import {
  getSlaveIdFromGroup,
  ControllerConfig,
  getControllerStatus
} from '@/lib/controllerUtils';

interface ControllerDisplayData {
  slaveid: number;
  name: string;
  pv: number | string;
  sp: number | string;
  isRunning: boolean;
  // hasAlarm: boolean;
  isAutoTuning: boolean;
  hasHeaterBreak: boolean;
  hasSensorBreak: boolean;
  mode: 'manual' | 'auto' | 'cascade' | 'program' | 'unknown';
  currentProfileName?: string | null;
  isHeating: boolean;
}

interface PartitionDisplayData {
  name: string;
  controllers: ControllerDisplayData[];
}

const STATUS_HIGH_REGISTER_NAME = "Status High";
const STATUS_LOW_REGISTER_NAME = "Status Low";



const CassandraHMIDisplay = () => {
  const {
    registers: allModbusRegisters,
    profiles: contextProfiles,
    updateRegister,
    isConnected,
    settings,
    featureFlags
  } = useModbus();
  const { toast } = useToast();

  const liveUiProfiles = useMemo((): Profile[] => {
    if (!contextProfiles || !allModbusRegisters) return [];

    return contextProfiles.map(pService => {
      let liveStatus: PlotStatus | undefined = pService.status;
      let liveElapsed: number | undefined = pService.elapsed;
      let liveCurrentTemp: number | undefined = pService.currentTemp;

      const statusRegister = allModbusRegisters.find(
        r => r.group === pService.name && r.name.startsWith(PROFILE_REGISTER_NAMES.STATUS)
      );
      if (statusRegister && typeof statusRegister.value === 'number' && statusRegister.value in PlotStatus) {
        liveStatus = statusRegister.value as PlotStatus;
      }

      const currentTempRegister = allModbusRegisters.find(
        r => r.group === pService.name && r.name === PROFILE_REGISTER_NAMES.CURRENT_VALUE
      );
      if (currentTempRegister && typeof currentTempRegister.value === 'number') {
        liveCurrentTemp = currentTempRegister.value;
      }

      const elapsedRegister = allModbusRegisters.find(
        r => r.group === pService.name && r.name === PROFILE_REGISTER_NAMES.ELAPSED
      );

      if (elapsedRegister && typeof elapsedRegister.value === 'number') {
        liveElapsed = elapsedRegister.value * 1000; // Convert seconds to milliseconds
      }

      return {
        ...pService,
        status: liveStatus,
        elapsed: liveElapsed,
        currentTemp: liveCurrentTemp,
      };
    });
  }, [contextProfiles, allModbusRegisters]);

  const activeUiProfiles = useMemo((): Profile[] => {
    if (!liveUiProfiles) return [];
    return liveUiProfiles.filter(
      profile => profile.enabled || profile.status === PlotStatus.RUNNING || profile.status === PlotStatus.PAUSED || profile.status === PlotStatus.INITIALIZING
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
    if (!settings) return [];
    return settings.partitions.map(partition => {
      let controllerConfigs: ControllerConfig[] = [];
      if (partition.controllers && partition.controllers.length > 0) {
        controllerConfigs = partition.controllers.filter(c => c.enabled);
      } else if (partition.startslaveid !== undefined && partition.numcontrollers !== undefined) {
        for (let i = 0; i < partition.numcontrollers; i++) {
          controllerConfigs.push({
            slaveid: partition.startslaveid + i,
            name: `Controller ${partition.startslaveid + i}`,
            enabled: true // Assume auto-generated are enabled
          });
        }
      }

      const controllersData: ControllerDisplayData[] = controllerConfigs.map(config => {
        const pvRegister = allModbusRegisters.find((reg: RegisterData) =>
          getSlaveIdFromGroup(reg.group) === config.slaveid && reg.name.endsWith(PV_REGISTER_NAME_SUFFIX)
        );
        const statusHighRegister = allModbusRegisters.find((reg: RegisterData) =>
          getSlaveIdFromGroup(reg.group) === config.slaveid && reg.name === STATUS_HIGH_REGISTER_NAME
        );
        const statusLowRegister = allModbusRegisters.find((reg: RegisterData) =>
          getSlaveIdFromGroup(reg.group) === config.slaveid && reg.name === STATUS_LOW_REGISTER_NAME
        );
        const status = getControllerStatus(statusHighRegister, statusLowRegister);
        let activeProfileName: string | null = null;

        const spCmdRegisterForController = allModbusRegisters.find(
          reg => getSlaveIdFromGroup(reg.group) === config.slaveid &&
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
          slaveid: config.slaveid,
          name: config.name,
          pv: pvRegister ? pvRegister.value : "N/A",
          sp: "N/A",
          ...status,
          currentProfileName: activeProfileName,
          isHeating: status.isHeating,
        };
      });

      return {
        name: partition.name,
        controllers: controllersData,
      };
    });
  }, [allModbusRegisters, liveUiProfiles, settings]);

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
    <div className="space-y-3 md:space-y-6" id="cassandra-hmi-display">
      <CollapsibleSection
        title={<T>Commons</T>}
        storageKey="hmi-commons-collapsible"
        initiallyOpen={false}
        className="glass-panel"
        headerClassName="flex justify-between items-center p-3 rounded-t-lg"
        contentClassName="p-3 glass-card rounded-b-lg"
        titleClassName="text-lg font-semibold glass-text"
        buttonClassName="text-slate-600 dark:text-white/80 hover:text-slate-800 dark:hover:text-white"
      >
        <Commons />
      </CollapsibleSection>

      {featureFlags.ENABLE_SAKO_VFD && (
        <CollapsibleSection
          title={<T>VFD Control</T>}
          storageKey="hmi-vfd-collapsible"
          className="glass-panel"
          headerClassName="flex justify-between items-center p-3 rounded-t-lg"
          contentClassName="p-3 glass-card rounded-b-lg"
          titleClassName="text-lg font-semibold glass-text"
          buttonClassName="text-slate-600 dark:text-white/80 hover:text-slate-800 dark:hover:text-white"
        >
          <VFDControls />
        </CollapsibleSection>
      )}

      {featureFlags.ENABLE_AMPERAGE_BUDGET_MANAGER && (
        <CollapsibleSection
          title={<T>Sequential Heating Control</T>}
          storageKey="hmi-sequential-heating-collapsible"
          id="hmi-sequential-heating-collapsible"
          className="glass-panel"
          headerClassName="flex justify-between items-center p-3 rounded-t-lg"
          contentClassName="p-3 glass-card rounded-b-lg"
          titleClassName="text-lg font-semibold glass-text"
          buttonClassName="text-slate-600 dark:text-white/80 hover:text-slate-800 dark:hover:text-white"
        >
          <SequentialHeatingCard />
        </CollapsibleSection>
      )}

      {featureFlags.ENABLE_OMRON_E5 && (
        <CollapsibleSection
          title={<T>Controller Partitions</T>}
          storageKey="hmi-partitions-collapsible"
          className="glass-panel"
          headerClassName="flex justify-between items-center p-3 rounded-t-lg"
          contentClassName="p-3 glass-card rounded-b-lg"
          titleClassName="text-lg font-semibold glass-text"
          buttonClassName="text-slate-600 dark:text-white/80 hover:text-slate-800 dark:hover:text-white"
        >
          <div className="grid grid-cols-1 md:grid-cols-2 gap-4 pt-2" id="hmi-partitions-container">
            {processedData.map(partition => (
              <Card key={partition.name} className="glass-card shadow-xl flex flex-col" id={`hmi-partition-${partition.name.replace(/\s+/g, '-')}`}>
                <CardHeader>
                  <CardTitle className="text-lg glass-text"><T>{partition.name}</T></CardTitle>
                </CardHeader>
                <CardContent className="space-y-3 flex-grow">
                  {partition.controllers.length === 0 ? (
                    <p className="text-sm text-slate-500 dark:text-slate-400"><T>No controllers configured or found for this partition.</T></p>
                  ) : (
                    partition.controllers.map((controller, index) => (
                      <React.Fragment key={controller.slaveid}>
                        <CassandraControllerCard
                          slaveId={controller.slaveid}
                          name={controller.name}
                          pv={controller.pv}
                          isRunning={controller.isRunning}
                          //hasAlarm={controller.hasAlarm}
                          isAutoTuning={controller.isAutoTuning}
                          hasHeaterBreak={controller.hasHeaterBreak}
                          hasSensorBreak={controller.hasSensorBreak}
                          mode={controller.mode}
                          currentProfile={controller.currentProfileName}
                          isHeating={controller.isHeating}
                        />
                        {(index + 1) % 2 === 0 && index < partition.controllers.length - 1 && (
                          <div className="border-t border-slate-300/30 dark:border-white/10 my-3" />
                        )}
                      </React.Fragment>
                    ))
                  )}
                </CardContent>
              </Card>
            ))}
          </div>
        </CollapsibleSection>
      )}
    </div>
  );
};

export default CassandraHMIDisplay; 