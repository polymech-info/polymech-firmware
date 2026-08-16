/**
 * Interval in milliseconds for polling REST API endpoints 
 * (Coils, Registers, SystemInfo).
 */
export const REST_POLLING_INTERVAL_MS = 2000;

/**
 * WebSocket reconnect attempt interval in milliseconds.
 */
export const WS_RECONNECT_INTERVAL_MS = 5000;

/**
 * WebSocket register refresh interval in milliseconds.
 */
export const WS_REGISTER_REFRESH_INTERVAL_MS = 1000;

// Add new constant for polling
export const WS_REGISTER_POLL_INTERVAL_MS = 5000; // Interval for polling all registers via WebSocket


// PV and SP register names for PID controllers
export const PV_REGISTER_NAME_SUFFIX = "PV";
export const SP_REGISTER_NAME_SUFFIX = "SP";
export const SP_CMD_COMMAND_REGISTER_PREFIX = "SP CMD";
export const COMMS_WRITE_REGISTER_PREFIX = "Comms Write";
export const STOP_CMD_REGISTER_PREFIX = "Run/Stop";
export const STATUS_UP_PREFIX = "Status High:";
export const STATUS_LOW_PREFIX = "Status Low:";
export const HEATUP_STATUS_PREFIX = "Heatup Status";
export const ENABLED_REGISTER_PREFIX = "Enabled";
export const TOTAL_COST_REGISTER_PREFIX = "Total Cost (Cents)";

// Heating Time Constants
export const HEATING_TIME = {
  MIN_MS: 500,
  MAX_MS: 7200000, // 2 hours
  STEP_MS: 1000,
} as const;

// Heating Time Constants in Seconds
export const HEATING_TIME_S = {
  MIN_S: 0.5,
  MAX_S: 7200, // 2 hours
  STEP_S: 1,
} as const;

// Register Names
export const REGISTER_NAMES = {
  INFO: "Info",
  MAX_TIME: "MaxTime",
  MAX_SIM: "MaxSim",
  OFFSET: "Offset",
  ENABLE: "Enabled",
  START: "StartIndex",
  END: "EndIndex",
  MODE: "Mode",
} as const;

// Register Groups
export const REGISTER_GROUPS = {
  AMPERAGE_BUDGET: "AmperageBudgetManager",
  PIDS: "Omron",
} as const;

// Corrected: PROFILE_REGISTER_NAMES
// The values are the expected 'RegisterData.name' (for registers) or 'CoilData.name' (for coils).
// The 'RegisterData.group' or 'CoilData.group' is assumed to be the profile's unique name (e.g., pService.name).
export const PROFILE_REGISTER_NAMES = {
  STATUS: "Status",
  CURRENT_VALUE: "CurrentValue",
  DURATION: "Duration",
  ELAPSED: "Elapsed",
  REMAINING: "Remaining",
  COMMAND: "Command",
  ENABLED: "Enabled"
} as const;

// Signal Plot Register Names
// The values are the expected 'RegisterData.name' (for registers) or 'CoilData.name' (for coils).
// The 'RegisterData.group' or 'CoilData.group' is assumed to be the signal plot's unique name.
export const SIGNAL_PLOT_REGISTER_NAMES = {
  STATUS: "SigPlot Status",
  DURATION: "SigPlot Duration LW",
  ELAPSED: "SigPlot Elapsed",
  COMMAND: "SigPlot Command",
  ENABLED: "SigPlot Enable"
} as const;

export const PRESS_CYLINDER_GROUP = "PressCylinder";

// Press Cylinder Register Names
export const PRESS_CYLINDER_REGISTER_NAMES = {
  STATE: 'State',
  PRESSURE: 'PV',
  SP: 'Target SP',
  COMMAND: 'Command',
  MODE: 'Mode',
  ERROR: 'Error',
  ENABLED: 'Enabled',
  INTERLOCKED: 'Interlocked',
};

export const LOADCELL_GROUP = "Loadcell";

// Loadcell Register Names
export const LOADCELL_REGISTER_NAMES = {
  PV: 'PV',
  VOLTAGE: 'Voltage',
  ZERO_VOLTAGE: 'Zero Voltage',
  ENABLED: 'Enabled',
  MODE: 'Mode',
  COMMAND: 'Command',
};


export const SOLENOID_GROUP = "Solenoid";

// Solenoid Register Names
export const SOLENOID_REGISTER_NAMES = {
  ENABLED: 'Enabled',
  STATE: 'State'
}

export const OMRON_E5_REGISTER_NAMES = {} as const;

export const PHApp_GROUP = "PHApp";
export const PHApp_REGISTER_NAMES = {
  APP_STATE: "App State",
  SUB_STATE_0: "Sub State 0",
  SUB_STATE_1: "Sub State 1",
  IS_SLAVE: "Is Slave",
  ALL_OMRON_STOP: "All Omron Stop",
  ALL_OMRON_COM_WRITE: "All Omron Com Write",
} as const;


// Status Values
export const STATUS = {
  IDLE: 0,
  HEATING: 1,
  COOLING: 2,
  ERROR: 3,
} as const;

// Max Values
export const MAX_VALUES = {
  SIMULTANEOUS: 255,
  OFFSET: 255,
} as const;

// Min Values
export const MIN_VALUES = {
  SIMULTANEOUS: 1,
  OFFSET: 1,
} as const;

export const getControlPointTypeNames = (t: (key: string) => string): Record<number, string> => ({
  0: t('No Operation'),
  1: t('Write Coil'),
  2: t('Write Holding Register'),
  3: t('Call Method'),
  4: t('Call Function'),
  5: t('Call REST API'),
  6: t('Write GPIO'),
  7: t('Display Message'),
  8: t('User Defined'),
  9: t('Pause Profile'),
  10: t('Stop PID Controllers'),
  11: 'Start PID Controllers',
  12: t('Buzzer: Off'),
  13: t('Buzzer: Solid On'),
  14: t('Buzzer: Slow Blink'),
  15: t('Buzzer: Fast Blink'),
  16: t('Buzzer: Long Beep/Short Pause'),
  17: t('Send IFTTT Notification')
});

export const CONTROL_POINT_TYPE_NAMES: Record<number, string> = {
  0: 'No Operation',
  1: 'Write Coil',
  2: 'Write Holding Register',
  3: 'Call Method',
  4: 'Call Function',
  5: 'Call REST API',
  6: 'Write GPIO',
  7: 'Display Message',
  8: 'User Defined',
  9: 'Pause Profile',
  10: 'Stop PID Controllers',
  11: 'Start PID Controllers',
  12: 'Buzzer: Off',
  13: 'Buzzer: Solid On',
  14: 'Buzzer: Slow Blink',
  15: 'Buzzer: Fast Blink',
  16: 'Buzzer: Long Beep/Short Pause',
  17: 'Send IFTTT Notification'
};

export const PROFILE_POLLING_INTERVAL = 3000;
export const PROFILE_TEMPERATURE_COUNT = 4; // Max number of temperature profiles 
////////////////////////////
//
// Plunger Constants
export const PLUNGER_GROUP = "Plunger";

export const PLUNGER_REGISTER_NAMES = {
  STATE: 'Plunger State',
  COMMAND: 'Plunger Command'
} as const;

export const PLUNGER_STATES = {
  IDLE: 0,
  HOMING_MANUAL: 1,
  HOMING_AUTO: 2,
  PLUNGING_MANUAL: 3,
  PLUNGING_AUTO: 4,
  STOPPING: 5,
  JAMMED: 6,
  RESETTING_JAM: 7,
  RECORD: 8,
  REPLAY: 9,
  FILLING: 10,
  POST_FLOW: 11
} as const;

export const PLUNGER_COMMANDS = {
  NONE: 0,
  HOME: 1,
  PLUNGE: 2,
  STOP: 3,
  INFO: 4,
  FILL: 5,
  REPLAY: 6
} as const;

////////////////////////////
//
// Delta VFD Constants
export const DELTA_VFD_GROUP = "DELTA_VFD";
export const DELTA_VFD_REGISTER_NAMES = {
  RUNNING_FREQUENCY: 'DELTA: Run Freq',
  SET_FREQUENCY: 'DELTA: Set Freq',
  OUTPUT_CURRENT: 'DELTA: Current',
  OUTPUT_POWER_KW: 'DELTA: Power kW',
  OUTPUT_TORQUE_PERCENT: 'DELTA: Torque %',
  FAULT_CODE: 'DELTA: Fault (0:none,79-84:OC,62:OV,87+:OL,58+:CE)',
  IS_RUNNING: 'DELTA: Running',
  HAS_FAULT: 'DELTA: Fault?',
  STATE: 'DELTA: State (0:stop,1:run,2:accel,3:decel,4:err)',
  CMD_FREQ: 'DELTA: Set Freq Cmd',
  CMD_DIRECTION: 'DELTA: Direction Cmd (0:stop,1:fwd,2:rev,99:reset)',
  CMD_COMMAND: 'DELTA: Command (1:info,2:reset,3:setup,4:reset_fault)',
  TARGET_REGISTER: 'DELTA: Target Reg',
  TARGET_VALUE: 'DELTA: Target Val'
} as const;

export const DELTA_VFD_STATES = {
  STOPPED: 0,
  RUNNING: 1,
  ACCELERATING: 2,
  DECELERATING: 3,
  ERROR: 4
} as const;

export const DELTA_VFD_DIRECTION_COMMANDS = {
  STOP: 0,
  FORWARD: 1,
  REVERSE: 2,
  RESET: 99
} as const;

export const DELTA_VFD_COMMANDS = {
  NONE: 0,
  INFO: 1,
  RESET: 2,
  SETUP: 3,
  RESET_FAULT: 4
} as const;

////////////////////////////
//
// Enable Features
export const ENABLE_TEMPERATURE_PRESSURE_PROFILE = true;
export const ENABLE_VFD_CONTROLS = true;
export const ENABLE_PRESS_CYLINDER = true;
export const ENABLE_HMI_EDIT = true;
export const ENABLE_MODBUS_DEBUG = true;
