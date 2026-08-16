import React from 'react';
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card';
import { Switch } from '@/components/ui/switch';
import { Label } from '@/components/ui/label';
import { T } from '../i18n';
import { useModbus } from '@/contexts/ModbusContext';
import { toast } from 'sonner';
import { SP_CMD_COMMAND_REGISTER_PREFIX, TOTAL_COST_REGISTER_PREFIX, PHApp_GROUP, PHApp_REGISTER_NAMES } from '@/constants';
import { Input } from './ui/input';
import { Button } from './ui/button';
import { getSlaveIdFromGroup } from '@/lib/controllerUtils';

const Commons: React.FC = () => {
  const { coils, registers, updateCoil, updateMultipleRegisters, isConnected } = useModbus();
  const [spValue, setSpValue] = React.useState('');

  // Helper functions to get current coil states
  const getSlaveMode = React.useMemo(() => {
    const isSlaveCoil = coils.find(coil =>
      coil.name === PHApp_REGISTER_NAMES.IS_SLAVE && coil.group === PHApp_GROUP
    );
    return isSlaveCoil?.value || false;
  }, [coils]);

  const getAllStop = React.useMemo(() => {
    const allOmronStopCoil = coils.find(coil =>
      coil.name === PHApp_REGISTER_NAMES.ALL_OMRON_STOP && coil.group === PHApp_GROUP
    );
    return allOmronStopCoil?.value || false;
  }, [coils]);

  const getComWrite = React.useMemo(() => {
    const allOmronComWriteCoil = coils.find(coil =>
      coil.name === PHApp_REGISTER_NAMES.ALL_OMRON_COM_WRITE && coil.group === PHApp_GROUP
    );
    return allOmronComWriteCoil?.value || false;
  }, [coils]);

  const costData = React.useMemo(() => {
    if (!registers || registers.length === 0) {
      return { totalInEuros: '0.00', cellCosts: [] };
    }

    const costRegisters = registers
      .filter(reg => reg.name.startsWith(TOTAL_COST_REGISTER_PREFIX) && typeof reg.value === 'number')
      .map(reg => ({
        slaveId: getSlaveIdFromGroup(reg.group),
        cost: reg.value as number
      }))
      .filter(item => item.slaveId !== null)
      .sort((a, b) => a.slaveId! - b.slaveId!);

    const totalInCents = costRegisters.reduce((sum, reg) => sum + reg.cost, 0);

    const cellCosts = [];
    for (let i = 0; i < costRegisters.length; i += 2) {
      const cell_1 = costRegisters[i];
      const cell_2 = costRegisters[i + 1];
      const cellCostInCents = cell_1.cost + (cell_2 ? cell_2.cost : 0);
      cellCosts.push(cellCostInCents / 100);
    }

    return {
      totalInEuros: (totalInCents / 100).toFixed(2),
      cellCosts: cellCosts
    };
  }, [registers]);



  const handleSetAllSp = () => {
    if (!isConnected) {
      toast.error("Not connected to Modbus server.");
      return;
    }
    const numericSp = parseFloat(spValue);
    if (isNaN(numericSp)) {
      toast.error("Invalid Set Point value. Please enter a number.");
      return;
    }

    const spCommandRegisters = registers.filter(reg =>
      reg.name.startsWith(SP_CMD_COMMAND_REGISTER_PREFIX)
    );

    if (spCommandRegisters.length === 0) {
      toast.info("No SP command registers found to update.");
      return;
    }

    const updates = spCommandRegisters.map(reg => ({
      address: reg.address,
      value: numericSp
    }));

    updateMultipleRegisters(updates)
      .then(() => {
        toast.success(`Set Point update command sent to ${updates.length} controller(s).`);
      })
      .catch((err) => {
        toast.error("Failed to send Set Point command.", {
          description: err.message,
        });
      });
  };

  const handleAllStopRunToggle = (isStop: boolean) => {
    if (!isConnected) {
      toast.error("Not connected to Modbus server.");
      return;
    }

    const allOmronStopCoil = coils.find(coil =>
      coil.name === PHApp_REGISTER_NAMES.ALL_OMRON_STOP && coil.group === PHApp_GROUP
    );

    if (!allOmronStopCoil) {
      toast.info("All Omron Stop coil not found.");
      return;
    }

    updateCoil(allOmronStopCoil.address, isStop)
      .then(() => {
        toast.success(`${isStop ? 'Stop' : 'Run'} command sent to all Omron controllers.`);
      })
      .catch((err) => {
        toast.error(`Failed to send ${isStop ? 'Stop' : 'Run'} command.`, {
          description: err.message,
        });
      });
  };

  const handleComWriteToggle = (isChecked: boolean) => {
    if (!isConnected) {
      toast.error("Not connected to Modbus server.");
      return;
    }

    const allOmronComWriteCoil = coils.find(coil =>
      coil.name === PHApp_REGISTER_NAMES.ALL_OMRON_COM_WRITE && coil.group === PHApp_GROUP
    );

    if (!allOmronComWriteCoil) {
      toast.info("All Omron Com Write coil not found.");
      return;
    }

    console.log('Updating COM Write coil', allOmronComWriteCoil.address, isChecked);

    updateCoil(allOmronComWriteCoil.address, isChecked)
      .then(() => {
        toast.success(`COM Write ${isChecked ? 'enabled' : 'disabled'} for all Omron controllers.`);
      })
      .catch((err) => {
        toast.error("Failed to update COM Write coil.", {
          description: err.message,
        });
      });
  };

  const handleMasterSlaveToggle = (isSlaveMode: boolean) => {
    if (!isConnected) {
      toast.error("Not connected to Modbus server.");
      return;
    }

    const isSlaveCoil = coils.find(coil =>
      coil.name === PHApp_REGISTER_NAMES.IS_SLAVE && coil.group === PHApp_GROUP
    );

    if (!isSlaveCoil) {
      toast.info("Is Slave coil not found.");
      return;
    }

    updateCoil(isSlaveCoil.address, isSlaveMode)
      .then(() => {
        toast.success(`${isSlaveMode ? 'Slave' : 'Master'} mode activated.`);
      })
      .catch((err) => {
        toast.error("Failed to set slave mode.", {
          description: err.message,
        });
      });
  };



  return (
    <>
      <Card id="dasboard-commons" className="glass-card">
        <CardHeader>
          <CardTitle className="glass-text"><T>Commons</T></CardTitle>
        </CardHeader>
        <CardContent className="space-y-4">
          <div className="flex items-center space-x-2">
            <Switch
              id="master-slave-toggle"
              checked={getSlaveMode}
              onCheckedChange={handleMasterSlaveToggle}
              disabled={!isConnected}
              className="data-[state=checked]:bg-emerald-500"
            />
            <Label htmlFor="master-slave-toggle" className="text-slate-700 dark:text-white"><T>Slave Mode</T></Label>
            <p className="text-sm text-slate-500 dark:text-slate-400">
              <T>When Slave Mode is enabled, all Omron controllers will be disabled for processing.</T>
            </p>
          </div>
          <div className="flex items-center space-x-2">
            <Switch
              id="all-stop-run-toggle"
              checked={getAllStop}
              onCheckedChange={handleAllStopRunToggle}
              disabled={!isConnected}
              className="data-[state=checked]:bg-emerald-500"
            />
            <Label htmlFor="all-stop-run-toggle" className="text-slate-700 dark:text-white"><T>All Stop</T></Label>
          </div>
          <div className="flex items-center space-x-2">
            <Switch
              id="com-write-toggle"
              checked={getComWrite}
              onCheckedChange={handleComWriteToggle}
              disabled={!isConnected}
              className="data-[state=checked]:bg-emerald-500"
            />
            <Label htmlFor="com-write-toggle" className="text-slate-700 dark:text-white"><T>COM Write</T></Label>
          </div>
          <div className="flex w-full max-w-sm items-center space-x-2">
            <Input
              type="number"
              placeholder="Set Point"
              value={spValue}
              onChange={(e) => setSpValue(e.target.value)}
              disabled={!isConnected}
              className="glass-input"
            />
            <Button
              type="submit"
              onClick={handleSetAllSp}
              disabled={!isConnected}
              className="status-gradient-connected text-white border-0"
            >
              <T>Set All SP</T>
            </Button>
          </div>

          <div className="pt-4">
            <h4 className="font-semibold text-slate-700 dark:text-white"><T>Total Cost</T></h4>
            <p className="text-2xl font-bold text-slate-800 dark:text-white">{costData.totalInEuros} €</p>
            <div className="text-xs text-slate-500 dark:text-slate-400">
              {costData.cellCosts.map((cost, index) => (
                <span key={index}>Cell {index + 1}: {cost.toFixed(2)} € | </span>
              ))}
            </div>
          </div>

        </CardContent>
      </Card>
    </>
  );
};

export default Commons; 