import * as z from "zod";

export enum ControlPointType {
  Linear = 0,
  Cubic = 1
}

// Control Point validation schema
export const controlPointSchema = z.object({
  x: z.number().min(0),
  y: z.number().min(0),
  handleX: z.number().optional(),
  handleY: z.number().optional(),
  type: z.nativeEnum(ControlPointType).optional()
});

// Infer the type from the Zod schema
export type ControlPoint = z.infer<typeof controlPointSchema>;

export type HeatZone = {
  id: string;
  name: string;
  description?: string;
};

export type Controller = {
  id: string;
  name: string;
  slaveId: number;
  currentTemp: number;
  targetTemp: number;
  minTemp: number;
  maxTemp: number;
  updateInterval: number; // in ms
  currentProfile: string | null;
  isRunning: boolean;
  lastUpdated: string;
  zoneId: string; // Added zoneId to associate controllers with zones
};

export enum PlotStatus {
  IDLE = 0,
  INITIALIZING = 1,
  RUNNING = 2,
  PAUSED = 3,
  STOPPED = 4,
  FINISHED = 5,
  WAITING = 6
}

export enum ProfileType {
  Base = 0,
  Temperature = 1,
  Pressure = 2,
  Signal = 3
}

export enum PlotCommand {
  NONE = 0,
  START = 1,
  STOP = 2,
  PAUSE = 3,
  RESUME = 4
}

export const profileSchema = z.object({
  id: z.number(),
  slot: z.number().default(0),
  name: z.string().default("Unnamed Profile"),
  description: z.string().optional(),
  duration: z.number().default(0),
  max: z.number().default(0),
  controlPoints: z.array(controlPointSchema).default([]),
  targetRegisters: z.array(z.number()).default([]),
  signalPlot: z.number().optional(),
  pressureProfile: z.number().optional(),
  status: z.nativeEnum(PlotStatus).default(PlotStatus.IDLE),
  elapsed: z.number().default(0),
  remaining: z.number().default(0),
  currentTemp: z.number().default(0),
  enabled: z.boolean().default(false),
  children: z.array(z.number()).default([]),
  type: z.nativeEnum(ProfileType).default(ProfileType.Temperature),
  associatedControllerNames: z.array(z.string()).optional(),
  overrides: z.object({
    sp: z.array(z.object({
      targetRegister: z.number(),
      offset: z.number()
    })).optional()
  }).optional(),
});

export type Profile = z.infer<typeof profileSchema>;

export interface PressureProfile extends Omit<Profile, 'type'> {
  type: ProfileType.Pressure;
  targetRegisters: number[];
}

export interface ProfilesResponse {
  profiles: Profile[];
}

export enum TemperatureProfileCommand {
  NONE = 0,
  START = 1,
  STOP = 2,
  PAUSE = 3,
  RESUME = 4
}

export enum TemperatureProfileRegisterOffset {
  STATUS = 0,
  CURRENT_TEMP = 1,
  DURATION_LW = 2,
  DURATION_HW = 3,
  ELAPSED_LW = 4,
  ELAPSED_HW = 5,
  REMAIN_LW = 6,
  REMAIN_HW = 7,
  COMMAND = 8,
  ENABLE_CMD = 9,
  _COUNT // Placeholder for the count of registers if needed
}

export type PlotPoint = {
  // ... existing code ...
};

// --- Modbus Register Definitions ---
export enum SignalPlotRegisterOffset {
  STATUS = 0,
  DURATION_LW = 1,
  DURATION_HW = 2,
  ELAPSED_LW = 3,
  ELAPSED_HW = 4,
  COMMAND = 5,
  ENABLE_CMD = 6,
  _COUNT
}

export const SIGNAL_PLOT_REGISTER_COUNT = SignalPlotRegisterOffset._COUNT;

export enum SignalPlotCommand {
  NONE = 0,
  START = 1,
  STOP = 2,
  PAUSE = 3,
  RESUME = 4
}

export enum ESignalType {
  NONE = 0,
  MB_WRITE_COIL = 1,
  MB_WRITE_HOLDING_REGISTER = 2,
  CALL_METHOD = 3,
  CALL_FUNCTION = 4,
  CALL_REST = 5,
  GPIO_WRITE = 6,
  DISPLAY_MESSAGE = 7,
  USER_DEFINED = 8,
  PAUSE_PROFILE = 9,
  STOP_PIDS = 10,
  START_PIDS = 11,
  BUZZER_OFF = 12,
  BUZZER_SOLID = 13,
  BUZZER_SLOW_BLINK = 14,
  BUZZER_FAST_BLINK = 15,
  BUZZER_LONG_BEEP_SHORT_PAUSE = 16,
  IFTTT_WEBHOOK = 17
}


export enum ESignalState {
  STATE_NONE = 0,      // not hit yet
  STATE_ERROR = 1,     // error
  STATE_ON = 2,        // on - has been hit
  STATE_OFF = 3,       // off - disabled by user
  STATE_CUSTOM_1 = 100 // custom
}

export interface SSignalControlPoint {
  id: number;           // Identifier for this specific control point instance (0-255)
  time: number;        // relative to start of profile (scaled to 100%, 500 = 50% of duration)
  state: ESignalState; // Target state active from this time forward
  type: ESignalType;   // Type of signal (e.g., MB_WRITE_COIL, MB_WRITE_HOLDING_REGISTER, CALL_METHOD, CALL_FUNCTION, CALL_REST)

  /**
   * @brief Custom user-defined integer associated with this point (e.g., target register value)
   *
   * For MB_WRITE_COIL, this is the register address
   * For MB_WRITE_HOLDING_REGISTER, this is the register value
   * For CALL_METHOD, this is the component id // TODO: not implemented yet
   * For CALL_FUNCTION, this is the function id // TODO: not implemented yet
   * For CALL_REST, this is the REST API endpoint // TODO: not implemented yet
   * For USER_DEFINED, this is the user-defined value // TODO: not implemented yet
   */
  arg_0?: number; // Custom user-defined integer associated with this point (e.g., target register value)

  /**
   * @brief Custom user-defined integer associated with this point (e.g., target register value)
   * 
   * For MB_WRITE_COIL, this is the coil value
   * For MB_WRITE_HOLDING_REGISTER, this is the register value
   * For CALL_METHOD, this is the method index // TODO: not implemented yet
   * For CALL_FUNCTION, this is the function index // TODO: not implemented yet
   * For CALL_REST, this is the REST API endpoint // TODO: not implemented yet
   * For USER_DEFINED, this is the user-defined value // TODO: not implemented yet
   */
  arg_1?: number; // Custom user-defined integer associated with this point (e.g., target register value)    
  /**
   * @brief Not used yet
   */
  arg_2?: number; // Custom user-defined integer associated with this point (e.g., target register value)
  /**
   * @brief Not used yet
   */
  user?: any; // Custom user-defined integer associated with this point (e.g., target register value)
  /**
   * @brief Label for this control point
   */
  name?: string; // Label for this control point
  /**
   * @brief Description for this control point
   */
  description?: string; // Description for this control point
}

export interface SignalPlotData {
  name: string;
  duration: number; // milliseconds
  slot: number;
  controlPoints: SSignalControlPoint[];
}
