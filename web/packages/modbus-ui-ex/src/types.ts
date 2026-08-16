import { z } from "zod";

export enum ControlPointType {
    Linear = 0,
    Cubic = 1
  }
  
  // Control Point validation schema
  export const controlPointSchema = z.object({
    x: z.number().min(0),
    y: z.number().min(0)
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
  
export interface TemperatureProfile {
  name: string;
  slot: number;
  description?: string;
  duration: number;
  max: number; 
  controlPoints: ControlPoint[];
  targetRegisters?: number[];
  signalPlot?: number;
}

export interface Profile extends TemperatureProfile {
  id: string;
  slot: number;
  status: PlotStatus;
  elapsed?: number;
  remaining?: number;
  currentTemp?: number;
  enabled: boolean;
  associatedControllerNames?: string[];
}

export interface ProfilesResponse {
  profiles: Profile[];
}

export enum PlotStatus {
  IDLE = 0,
  RUNNING = 1,
  PAUSED = 2,
  FINISHED = 3,
  STOPPED = 4
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
    PAUSE_PROFILE = 9
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
    arg_0: number; // Custom user-defined integer associated with this point (e.g., target register value)

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
    arg_1: number; // Custom user-defined integer associated with this point (e.g., target register value)    
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
