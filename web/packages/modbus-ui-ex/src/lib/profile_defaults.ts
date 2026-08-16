import { TemperatureProfile, HeatZone, Controller, Profile, PlotStatus } from '../types';

// Default profiles
export const defaultProfiles: Profile[] = [
  {
    id: 'default-quick-ramp',
    slot: 0,
    name: 'Quick Ramp Up',
    description: 'Quickly ramps to maximum temperature',
    controlPoints: [
      { x: 0, y: 0 },
      { x: 0.3, y: 1},
      { x: 1, y: 1 }
    ],
    duration: 1000 * 60 * 10,
    max: 150,
    targetRegisters: [],
    status: PlotStatus.IDLE,
    enabled: false,
    currentTemp: 0,
    elapsed: 0,
    remaining: 1000 * 60 * 10,
    associatedControllerNames: []
  },
  {
    id: 'default-slow-bake',
    slot: 1,
    name: 'Slow Bake',
    description: 'A gentle ramp and long hold for baking processes.',
    controlPoints: [
      { x: 0, y: 0.1 },
      { x: 0.25, y: 0.6 },
      { x: 0.75, y: 0.6 },
      { x: 1, y: 0.1 }
    ],
    duration: 1000 * 60 * 120,
    max: 120,
    targetRegisters: [],
    status: PlotStatus.IDLE,
    enabled: false,
    currentTemp: 0,
    elapsed: 0,
    remaining: 1000 * 60 * 120,
    associatedControllerNames: []
  }
];

// Default heat zones
export const defaultZones: HeatZone[] = [
  {
    id: 'zone-up',
    name: 'Up Zone',
    description: 'Top heat zone array'
  },
  {
    id: 'zone-down',
    name: 'Down Zone',
    description: 'Bottom heat zone array'
  }
];

// Default controllers
export const defaultControllers: Controller[] = [
  // Up Zone Controllers (4)
  {
    id: 'controller-1',
    name: 'Main Chamber',
    currentTemp: 25,
    targetTemp: 75,
    minTemp: 0,
    maxTemp: 100,
    slaveId: 1,
    updateInterval: 250,
    currentProfile: null,
    isRunning: false,
    lastUpdated: new Date().toISOString(),
    zoneId: 'zone-up'
  },
  {
    id: 'controller-2',
    name: 'Secondary Chamber',
    currentTemp: 22,
    targetTemp: 65,
    minTemp: 0,
    maxTemp: 150,
    slaveId: 2,
    updateInterval: 500,
    currentProfile: null,
    isRunning: false,
    lastUpdated: new Date().toISOString(),
    zoneId: 'zone-up'
  },
  {
    id: 'controller-3',
    name: 'Tertiary Chamber',
    currentTemp: 30,
    targetTemp: 80,
    minTemp: 10,
    maxTemp: 120,
    slaveId: 3,
    updateInterval: 300,
    currentProfile: null,
    isRunning: false,
    lastUpdated: new Date().toISOString(),
    zoneId: 'zone-up'
  },
  {
    id: 'controller-4',
    name: 'Auxiliary Up',
    currentTemp: 28,
    targetTemp: 70,
    minTemp: 5,
    maxTemp: 110,
    slaveId: 4,
    updateInterval: 750,
    currentProfile: null,
    isRunning: false,
    lastUpdated: new Date().toISOString(),
    zoneId: 'zone-up'
  },
  
  // Down Zone Controllers (4)
  {
    id: 'controller-5',
    name: 'Primary Lower',
    currentTemp: 18,
    targetTemp: 50,
    minTemp: 0,
    maxTemp: 120,
    slaveId: 5,
    updateInterval: 1000,
    currentProfile: null,
    isRunning: false,
    lastUpdated: new Date().toISOString(),
    zoneId: 'zone-down'
  },
  {
    id: 'controller-6',
    name: 'Secondary Lower',
    currentTemp: 20,
    targetTemp: 55,
    minTemp: 5,
    maxTemp: 90,
    slaveId: 6,
    updateInterval: 400,
    currentProfile: null,
    isRunning: false,
    lastUpdated: new Date().toISOString(),
    zoneId: 'zone-down'
  },
  {
    id: 'controller-7',
    name: 'Tertiary Lower',
    currentTemp: 15,
    targetTemp: 45,
    minTemp: 0,
    maxTemp: 80,
    slaveId: 7,
    updateInterval: 600,
    currentProfile: null,
    isRunning: false,
    lastUpdated: new Date().toISOString(),
    zoneId: 'zone-down'
  },
  {
    id: 'controller-8',
    name: 'Auxiliary Down',
    currentTemp: 23,
    targetTemp: 60,
    minTemp: 10,
    maxTemp: 100,
    slaveId: 8,
    updateInterval: 350,
    currentProfile: null,
    isRunning: false,
    lastUpdated: new Date().toISOString(),
    zoneId: 'zone-down'
  }
];
