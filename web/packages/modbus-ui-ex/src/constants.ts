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
export const WS_REGISTER_POLL_INTERVAL_MS = 2000; // Interval for polling all registers via WebSocket

// Add other constants here as needed 

// PV and SP register names for PID controllers
export const PV_REGISTER_NAME_SUFFIX = "PV";
export const SP_REGISTER_NAME_SUFFIX = "SP";
export const SP_CMD_COMMAND_REGISTER_PREFIX = "SP CMD";
export const ENABLE_CMD_REGISTER_PREFIX = "Run/Stop";
export const STATUS_UP_PREFIX = "Status High:";
export const STATUS_LOW_PREFIX = "Status Low:";
export const HEATUP_STATUS_PREFIX = "Heatup Status";

// Heating Time Constants
export const HEATING_TIME = {
  MIN_MS: 500,
  MAX_MS: 7200000, // 2 hours
  STEP_MS: 1000,
} as const;

// Register Names
export const REGISTER_NAMES = {
  INFO: "AmperageBudgetInfo",
  MAX_TIME: "AmperageBudgetMaxTime",
  MAX_SIM: "AmperageBudgetMaxSim",
  OFFSET: "AmperageBudgetOffset",
  ENABLE: "AmperageBudgetEnable",
  START: "AmperageBudgetStartIndex",
  END: "AmperageBudgetEndIndex",
} as const;

// Register Groups
export const REGISTER_GROUPS = {
  AMPERAGE_BUDGET: "AmperageBudget",
  PIDS: "Omron",
} as const;

// Remove the previous PROFILE_REGISTER_GROUPS or PROFILE_MODBUS_CONFIG
// export const PROFILE_REGISTER_GROUPS = { ... };
// export const PROFILE_MODBUS_CONFIG = { ... };

// Corrected: PROFILE_REGISTER_NAMES
// The values are the expected 'RegisterData.name' (for registers) or 'CoilData.name' (for coils).
// The 'RegisterData.group' or 'CoilData.group' is assumed to be the profile's unique name (e.g., pService.name).
export const PROFILE_REGISTER_NAMES = {
  STATUS: "TProf STATUS",         // Register name for status
  CURRENT_TEMP: "TProf CURRENT_TEMP",  // Register name for current temperature
  DURATION_LW: "TProf Duration LW", // Register name for duration low word
  DURATION_HW: "TProf Duration HW", // Register name for duration high word
  ELAPSED_LW: "TProf ELAPSED_LW",   // Register name for elapsed time low word
  ELAPSED_HW: "TProf ELAPSED_HW",   // Register name for elapsed time high word
  COMMAND: "TProf Command",        // Register name for command
  ENABLE_CMD: "TProf ENABLE_CMD"        // Coil name for enable/disable
} as const;

// Signal Plot Register Names
// The values are the expected 'RegisterData.name' (for registers) or 'CoilData.name' (for coils).
// The 'RegisterData.group' or 'CoilData.group' is assumed to be the signal plot's unique name.
export const SIGNAL_PLOT_REGISTER_NAMES = {
  STATUS: "SigPlot Status",
  DURATION_LW: "SigPlot Duration LW",
  DURATION_HW: "SigPlot Duration HW",
  ELAPSED_LW: "SigPlot Elapsed",
  ELAPSED_HW: "SigPlot Elapsed HW",
  COMMAND: "SigPlot Command",
  ENABLE_CMD: "SigPlot Enable"
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

export const PROFILE_POLLING_INTERVAL = 3000; 
export const PROFILE_TEMPERATURE_COUNT = 4; // Max number of temperature profiles 