import { SSignalControlPoint, ESignalState, ESignalType, SignalPlotData } from '../../types';

// Define "nice" time intervals in milliseconds for timeline markers
export const NICE_TIME_INTERVALS_MS = [
    1000,      // 1s
    2000,      // 2s
    5000,      // 5s
    10000,     // 10s
    15000,     // 15s
    30000,     // 30s
    60000,     // 1m
    120000,    // 2m
    300000,    // 5m
    600000,    // 10m
    900000,    // 15m
    1800000,   // 30m
    3600000,   // 1h
    7200000,   // 2h
    10800000,  // 3h
    14400000,  // 4h
    18000000,  // 5h
    21600000,  // 6h
  ];
export const sampleSignalPlotData: SignalPlotData[] = [

    {
      "name": "SampleSignalPlot_0",
      "duration": 180000, // 3 minutes
      "slot": 0,
      "controlPoints": [
        { "id": 1, "name": "Start Heater A", "description": "Turn on Coil 30", "time": 100, "state": ESignalState.STATE_ON, "type": ESignalType.MB_WRITE_COIL, "arg_0": 30, "arg_1": 1, "arg_2": 0 },
        { "id": 2, "name": "Stop Heater A", "description": "Turn off Coil 30", "time": 150, "state": ESignalState.STATE_ON, "type": ESignalType.MB_WRITE_COIL, "arg_0": 30, "arg_1": 0, "arg_2": 0 },
        { "id": 3, "name": "Start Heater B", "description": "Turn on Coil 31", "time": 500, "state": ESignalState.STATE_ON, "type": ESignalType.MB_WRITE_COIL, "arg_0": 31, "arg_1": 1, "arg_2": 0 },
        { "id": 4, "name": "Set Temp Target", "description": "Set Register 100 to 1234", "time": 750, "state": ESignalState.STATE_OFF, "type": ESignalType.MB_WRITE_HOLDING_REGISTER, "arg_0": 100, "arg_1": 1234, "arg_2": 0 },
        { "id": 5, "name": "Stop Heater B", "description": "Turn off Coil 31", "time": 900, "state": ESignalState.STATE_ON, "type": ESignalType.MB_WRITE_COIL, "arg_0": 31, "arg_1": 0, "arg_2": 0 },
      ].sort((a,b) => a.time - b.time) // Ensure initial sort
    },
    {
      "name": "ShortPlot_70s",
      "duration": 70000, // 70 seconds
      "slot": 1,
      "controlPoints": [
        { "id": 1, "name": "Valve Open", "description": "Open valve at address 1", "time": 250, "state": ESignalState.STATE_ON, "type": ESignalType.MB_WRITE_COIL, "arg_0": 1, "arg_1": 1},
        { "id": 2, "name": "Valve Close", "description": "Close valve at address 1", "time": 750, "state": ESignalState.STATE_ON, "type": ESignalType.MB_WRITE_COIL, "arg_0": 1, "arg_1": 0}
      ].sort((a,b) => a.time - b.time)
    }
  ];

  export const baseSampleCpTemplates: Omit<SSignalControlPoint, 'id' | 'time'>[] = [
    { name: "Coil On Sample", description: "Turn a sample coil ON", state: ESignalState.STATE_ON, type: ESignalType.MB_WRITE_COIL, arg_0: 10, arg_1: 1, arg_2: undefined },
    { name: "Coil Off Sample", description: "Turn a sample coil OFF", state: ESignalState.STATE_ON, type: ESignalType.MB_WRITE_COIL, arg_0: 11, arg_1: 0, arg_2: undefined },
    { name: "Another Register Sample", description: "Write another value to a sample register", state: ESignalState.STATE_ON, type: ESignalType.MB_WRITE_HOLDING_REGISTER, arg_0: 102, arg_1: 456, arg_2: undefined },
    { name: "Method Call Sample", description: "Example method call event", state: ESignalState.STATE_ERROR, type: ESignalType.CALL_METHOD, arg_0: 1, arg_1: 2, arg_2: undefined },
    { name: "User Event Sample", description: "Custom user defined event trigger", state: ESignalState.STATE_CUSTOM_1, type: ESignalType.USER_DEFINED, arg_0: 77, arg_1: 88, arg_2: 99 },
];
