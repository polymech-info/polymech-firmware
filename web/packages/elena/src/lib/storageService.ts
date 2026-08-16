
import { profileStorage } from './storage/profileStorage';
import { controllerStorage } from './storage/controllerStorage';
import { zoneStorage } from './storage/zoneStorage';
import { TemperatureProfile, HeatZone, Controller } from './types';

// Combined Storage Service (for backward compatibility)
class StorageService {
  // Profile-specific methods
  async getProfiles(): Promise<TemperatureProfile[]> {
    return profileStorage.getProfiles();
  }

  async saveProfiles(profiles: TemperatureProfile[]): Promise<void> {
    return profileStorage.saveProfiles(profiles);
  }

  // Controller-specific methods
  async getControllers(): Promise<Controller[]> {
    return controllerStorage.getControllers();
  }

  async saveControllers(controllers: Controller[]): Promise<void> {
    return controllerStorage.saveControllers(controllers);
  }
  
  async deleteController(id: string): Promise<void> {
    return controllerStorage.deleteController(id);
  }

  // Zone-specific methods
  async getZones(): Promise<HeatZone[]> {
    return zoneStorage.getZones();
  }

  async saveZones(zones: HeatZone[]): Promise<void> {
    return zoneStorage.saveZones(zones);
  }
  
  async deleteZone(id: string): Promise<void> {
    return zoneStorage.deleteZone(id);
  }
  
  async createZone(zone: Omit<HeatZone, 'id'>): Promise<HeatZone> {
    return zoneStorage.createZone(zone);
  }
}

export const storageService = new StorageService();
