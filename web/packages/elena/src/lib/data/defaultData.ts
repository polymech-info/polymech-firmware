
import { TemperatureProfile, HeatZone, Controller } from '../types';

// Default profiles
export const defaultProfiles: TemperatureProfile[] = [
  {
    id: 'profile-1',
    name: 'Quick Ramp Up',
    description: 'Quickly ramps to maximum temperature',
    controlPoints: [
      { x: 0, y: 0 },
      { x: 0.3, y: 1, handleX: 0.1, handleY: 0.8 },
      { x: 1, y: 1 }
    ],
    duration: 30,
    createdAt: new Date().toISOString(),
    updatedAt: new Date().toISOString()
  },
  {
    id: 'profile-2',
    name: 'Gradual Heat',
    description: 'Slowly increases temperature over time',
    controlPoints: [
      { x: 0, y: 0 },
      { x: 0.5, y: 0.5, handleX: 0.25, handleY: 0.25 },
      { x: 1, y: 1, handleX: 0.75, handleY: 0.75 }
    ],
    duration: 60,
    createdAt: new Date().toISOString(),
    updatedAt: new Date().toISOString()
  },
  {
    id: 'profile-3',
    name: 'Heat and Hold',
    description: 'Quickly heats and maintains temperature',
    controlPoints: [
      { x: 0, y: 0 },
      { x: 0.2, y: 0.8, handleX: 0.1, handleY: 0.6 },
      { x: 1, y: 0.8 }
    ],
    duration: 45,
    createdAt: new Date().toISOString(),
    updatedAt: new Date().toISOString()
  },
  {
    id: 'profile-4',
    name: 'HDPE-10mm',
    description: 'Optimized for 10mm HDPE material processing',
    controlPoints: [
      { x: 0, y: 0 },
      { x: 0.2, y: 0.6, handleX: 0.1, handleY: 0.4 },
      { x: 0.7, y: 0.9, handleX: 0.5, handleY: 0.85 },
      { x: 1, y: 0.7 }
    ],
    duration: 90,
    createdAt: new Date().toISOString(),
    updatedAt: new Date().toISOString()
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
