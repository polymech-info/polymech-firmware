
// Re-export types
export * from './types';

// Import services
import { profileService } from './profileService';
import { zoneService } from './zoneService';
import { controllerService } from './controllerService';

// Create API client for frontend use
export const api = {
  // Re-export zone methods
  getZones: zoneService.getZones,
  getZone: zoneService.getZone,
  getZoneControllers: zoneService.getZoneControllers,
  deleteZone: zoneService.deleteZone,
  createZone: zoneService.createZone,
  
  // Re-export profile methods
  getProfiles: profileService.getProfiles,
  getProfile: profileService.getProfile,
  createProfile: profileService.createProfile,
  updateProfile: profileService.updateProfile,
  deleteProfile: profileService.deleteProfile,
  
  // Re-export controller methods
  getControllers: controllerService.getControllers,
  getController: controllerService.getController,
  updateController: controllerService.updateController,
  startController: controllerService.startController,
  stopController: controllerService.stopController,
  createController: controllerService.createController,
  deleteController: controllerService.deleteController
};
