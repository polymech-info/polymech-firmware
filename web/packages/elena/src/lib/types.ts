// Types for the temperature control system
import { z } from 'zod';
import { controlPointSchema } from './profileService'; // Import the schema and enum

export type TemperatureProfile = {
  id: string;
  name: string;
  description: string;
  controlPoints: ControlPoint[]; // Keep using ControlPoint here
  duration: number; // in minutes
  minTemp: number;
  maxTemp: number;
  createdAt: string;
  updatedAt: string;
};

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
  currentTemp: number;
  targetTemp: number;
  minTemp: number;
  maxTemp: number;
  slaveId: number;
  updateInterval: number; // in ms
  currentProfile: string | null;
  isRunning: boolean;
  lastUpdated: string;
  zoneId: string; // Added zoneId to associate controllers with zones
};
