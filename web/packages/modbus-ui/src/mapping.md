# Find registers / coils

src\components\CassandraHMIDisplay.tsx

### Retrieve PV/SP values from an Omron controller

const { registers, coils, updateRegister } = useModbus();

```ts
const controllersData: ControllerDisplayData[] = controllerConfigs.map(config => {
        const pvRegister = allModbusRegisters.find((reg: RegisterData) => 
          getSlaveIdFromGroup(reg.group) === config.slaveId && reg.name.endsWith(PV_REGISTER_NAME_SUFFIX)
        );
        const spRegister = allModbusRegisters.find((reg: RegisterData) => 
          getSlaveIdFromGroup(reg.group) === config.slaveId && reg.name.endsWith(SP_REGISTER_NAME_SUFFIX)
        );
        const statusHighRegister = allModbusRegisters.find((reg: RegisterData) =>
          getSlaveIdFromGroup(reg.group) === config.slaveId && reg.name === STATUS_HIGH_REGISTER_NAME
        );
        const statusLowRegister = allModbusRegisters.find((reg: RegisterData) =>
          getSlaveIdFromGroup(reg.group) === config.slaveId && reg.name === STATUS_LOW_REGISTER_NAME
        );
})
```

whereby search strings can be found in src\constants.ts

```
const PV_REGISTER_NAME_SUFFIX = "PV";
const SP_REGISTER_NAME_SUFFIX = "SP";
const STATUS_HIGH_REGISTER_NAME = "Status High";
const STATUS_LOW_REGISTER_NAME = "Status Low";
```

### Get Coil Address

```ts

const { registers, coils, updateRegister } = useModbus();

// Find the enable coil
  const enableCoil = coils.find(coil => 
    coil.group === REGISTER_GROUPS.AMPERAGE_BUDGET && 
    coil.name === REGISTER_NAMES.ENABLE
  );
```
