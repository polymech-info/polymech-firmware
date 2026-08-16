import { z } from "zod";
import { storageService } from './storageService';
import { TemperatureProfile, ControlPoint } from './types';

// Define enum for control point types
export enum ControlPointType {
  Linear = 0,
  Cubic = 1
}

// Control Point validation schema
export const controlPointSchema = z.object({
  x: z.number().min(0).max(1),
  y: z.number().min(0).max(1),
  handleX: z.number().optional(),
  handleY: z.number().optional(),
  type: z.nativeEnum(ControlPointType).optional(),
});

// Profile validation schema
const profileSchema = z.object({
  name: z.string().min(1),
  description: z.string(),
  controlPoints: z.array(controlPointSchema).min(2),
  duration: z.number().positive(),
  slot: z.number().positive(),
  targetStartAddress: z.number().positive(),
  count: z.number().positive(),
  range: z.number().positive(),
});

export type ProfileInput = z.infer<typeof profileSchema>;

// Profile Service
export const profileService = {
  // Get all profiles
  getProfiles: async (): Promise<TemperatureProfile[]> => {
    return await storageService.getProfiles();
  },

  // Get a single profile by ID
  getProfile: async (id: string): Promise<TemperatureProfile> => {
    const profiles = await storageService.getProfiles();
    const profile = profiles.find(p => p.id === id);
    
    if (!profile) {
      throw new Error('Profile not found');
    }
    
    return profile;
  },

  // Create a new profile
  createProfile: async (data: ProfileInput): Promise<TemperatureProfile> => {
    // Validate input data
    const validated = profileSchema.parse(data);
    
    // Get existing profiles
    const profiles = await storageService.getProfiles();
    
    // Create a UUID for the new profile
    const newId = crypto.randomUUID();
    
    // Create new profile
    const newProfile: TemperatureProfile = {
      id: newId,
      name: validated.name,
      description: validated.description,
      controlPoints: validated.controlPoints.map(point => ({
        x: point.x,
        y: point.y,
        handleX: point.handleX,
        handleY: point.handleY,
        type: point.type ?? ControlPointType.Linear
      })),
      duration: validated.duration,
      minTemp: 0,
      maxTemp: 100,
      createdAt: new Date().toISOString(),
      updatedAt: new Date().toISOString()
    };
    
    // Save updated profiles
    profiles.push(newProfile);
    await storageService.saveProfiles(profiles);
    
    return newProfile;
  },

  // Update an existing profile
  updateProfile: async (id: string, data: ProfileInput): Promise<TemperatureProfile> => {
    // Validate input data
    const validated = profileSchema.parse(data);
    
    // Get existing profiles
    const profiles = await storageService.getProfiles();
    const index = profiles.findIndex(p => p.id === id);
    
    if (index === -1) {
      throw new Error('Profile not found');
    }
    
    // Update profile
    profiles[index] = {
      ...profiles[index],
      name: validated.name,
      description: validated.description,
      controlPoints: validated.controlPoints.map(point => ({
        x: point.x,
        y: point.y,
        handleX: point.handleX,
        handleY: point.handleY,
        type: point.type ?? ControlPointType.Linear
      })),
      duration: validated.duration,
      updatedAt: new Date().toISOString()
    };
    
    // Save updated profiles
    await storageService.saveProfiles(profiles);
    
    return profiles[index];
  },

  // Delete a profile
  deleteProfile: async (id: string): Promise<void> => {
    const profiles = await storageService.getProfiles();
    const newProfiles = profiles.filter(p => p.id !== id);
    
    if (profiles.length === newProfiles.length) {
      throw new Error('Profile not found');
    }
    
    await storageService.saveProfiles(newProfiles);
  }
};
