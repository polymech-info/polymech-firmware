import modbusApiService, 
{ 
  type Profile as ServiceProfile, 
  type ProfilesResponse, 
  //type ProfileControlPoint as ServiceControlPoint,
  type ControlPoint as ServiceControlPoint,
} from '@polymech/client-ts';

import { type TemperatureProfile, type ControlPoint } from '@/types';

// Helper to transform service control points to UI control points
// Assumes service time is 0-1 and temperature is absolute. UI wants x (0-1) and y (0-1, relative to max).
// This might need adjustment based on actual service behavior.
const transformServiceControlPointsToUI = (servicePoints: ServiceControlPoint[], profileMaxTemp: number): ControlPoint[] => {
  if (!servicePoints || servicePoints.length === 0) {
    return [{ x: 0, y: 0 }, { x: 1, y: 1 }]; // Default ramp
  }
  return servicePoints.map(p => ({
    x: p.time, // Assuming p.time is already normalized (0-1)
    y: profileMaxTemp > 0 ? p.temperature / profileMaxTemp : 0, // Normalize temperature to 0-1
  }));
};


export const api = {
  getProfiles: async (): Promise<TemperatureProfile[]> => {
    try {
      const response: ProfilesResponse = await modbusApiService.getProfiles();
      // The service's Profile type is missing name, description, and a clear max temperature for normalization.
      // We'll use defaults/placeholders.
      return response.profiles.map((p: ServiceProfile) => {
        const estimatedMaxTemp = p.controlPoints.reduce((max, cp) => Math.max(max, cp.temperature), 100); // Estimate max from points or default
        return {
          id: p.slot.toString(), // Assuming slot is the ID
          name: `Profile ${p.slot}`, // Placeholder name
          description: 'Description from service placeholder', // Placeholder description
          controlPoints: transformServiceControlPointsToUI(p.controlPoints, estimatedMaxTemp),
          duration: p.duration, // Assuming duration unit matches (e.g., ms)
          max: estimatedMaxTemp, // This is an estimate.
        };
      });
    } catch (error) {
      console.error('Error fetching profiles:', error);
      // Return default or empty profiles if the API call fails
      // For now, rethrow or return empty to let useQuery handle error state.
      throw error; 
    }
  },

  createProfile: async (data: Omit<TemperatureProfile, 'id' | 'createdAt' | 'updatedAt'>): Promise<TemperatureProfile> => {
    console.warn('api.createProfile is a stub and does not persist data to the backend.');
    // This is a mock implementation.
    // In a real scenario, this would call modbusApiService.createProfile (if it existed)
    // and transform data structures as needed.
    const newProfile: TemperatureProfile = {
      ...data,
      id: `temp-id-${Date.now()}`, // Temporary local ID
      // createdAt and updatedAt would be set by the backend
    };
    // Simulate backend by returning the created profile
    return newProfile;
  },

  updateProfile: async (id: string, data: Omit<TemperatureProfile, 'id' | 'createdAt' | 'updatedAt'>): Promise<TemperatureProfile> => {
    console.warn('api.updateProfile is a stub and does not persist data to the backend.');
    // Mock implementation
    const updatedProfile: TemperatureProfile = {
      ...data,
      id,
    };
    return updatedProfile;
  },

  deleteProfile: async (id: string): Promise<void> => {
    console.warn('api.deleteProfile is a stub and does not persist data to the backend.');
    // Mock implementation
    return Promise.resolve();
  },
};


// For useQuery, it's good practice for queryFn to return the data directly or throw an error.
// The transformation logic is now within getProfiles. 