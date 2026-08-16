// src/lib/controllerUtils.ts

export interface ControllerConfig {
  slaveId: number;
  name?: string; 
}

export interface PartitionConfig {
  name: string;
  startSlaveId?: number; // Optional if controllers array is provided
  numControllers?: number; // Optional if controllers array is provided
  controllers?: ControllerConfig[]; // Optional array for specific controller configs
}

// Configuration for partitions (sourced from CassandraHMIDisplay.tsx):
export const PARTITION_CONFIG: PartitionConfig[] = [
  {
    name: "Cassandra Bank Alpha",
    controllers: [
      { slaveId: 10, name: "Carina"}, 
      { slaveId: 11, name: "Castor"},
      { slaveId: 12, name: "Cetus"},
      { slaveId: 13, name: "Corona"},
    ]
  },
  {
    name: "Cassandra Bank Beta (Auto-generated)",
    startSlaveId: 14, 
    numControllers: 4,
    controllers: [
      { slaveId: 14, name: "Coma B"},
      { slaveId: 15, name: "Corvus"},
      { slaveId: 16, name: "Crater"},
      { slaveId: 17, name: "Crux"},
    ]
  }
  // Removed the "SP Target Controllers" partition with high slaveId numbers
];


export const PARTITION_CONFIG_: PartitionConfig[] = [
  
  {
    name: "Cassandra Bank Beta (Auto-generated)",
    startSlaveId: 14, 
    numControllers: 2,
    controllers: [
      { slaveId: 14, name: "Beta Main"},
      { slaveId: 15, name: "Beta Aux"}
    ]
  }
  // Removed the "SP Target Controllers" partition with high slaveId numbers
];

export const getSlaveIdFromGroup = (group: string | undefined): number | null => {
  if (!group) return null;
  // Regex to find a number in square brackets at the end of the string
  const match = group.match(/\[(\d+)\]$/); 
  return match && match[1] ? parseInt(match[1], 10) : null;
};

// Status bit definitions
export const STATUS_BITS = {
  // Status High bits (bits 16-31)
  RUN_STOP: 24,        // Bit 24: Run/Stop status
  ALARM: 25,           // Bit 25: Alarm status
  AT: 26,              // Bit 26: Auto-tuning status
  HEATER_BREAK: 27,    // Bit 27: Heater break alarm
  SENSOR_BREAK: 28,    // Bit 28: Sensor break alarm
  // Status Low bits (bits 0-15)
  MANUAL: 0,           // Bit 0: Manual mode
  AUTO: 1,             // Bit 1: Auto mode
  CASCADE: 2,          // Bit 2: Cascade mode
  PROGRAM: 3,          // Bit 3: Program mode
  Control_OutputOpenOutput: 8, // Bit 8: Control Output Open (Heating ON)
} as const;

// Helper function to check status bits
export const checkStatusBit = (high: number, low: number, bit: number): boolean => {
  if (bit <= 15) {
    return (low & (1 << bit)) !== 0;
  } else {
    return (high & (1 << (bit - 16))) !== 0;
  }
}; 