import { storageService } from './storageService';
import { HeatZone, Controller } from './types';
import { controllerService } from './controllerService';
import { logger } from '@/logger';

// Zone Service
export const zoneService = {
  // Get all zones
  getZones: async (): Promise<HeatZone[]> => {
    return await storageService.getZones();
  },

  // Get a single zone by ID
  getZone: async (id: string): Promise<HeatZone> => {
    const zones = await storageService.getZones();
    const zone = zones.find(z => z.id === id);
    
    if (!zone) {
      throw new Error('Zone not found');
    }
    
    return zone;
  },

  // Get all controllers in a zone
  getZoneControllers: async (zoneId: string): Promise<Controller[]> => {
    const controllers = await controllerService.getControllers();
    return controllers.filter(controller => controller.zoneId === zoneId);
  },
  
  // Create a new zone
  createZone: async (zone: Omit<HeatZone, 'id'>): Promise<HeatZone> => {
    try {
      return await storageService.createZone(zone);
    } catch (error) {
      logger.error('Error creating zone:', error);
      throw new Error('Failed to create zone');
    }
  },
  
  // Delete a zone
  deleteZone: async (id: string): Promise<void> => {
    // Check if zone has any controllers
    const controllers = await controllerService.getControllers();
    const zoneControllers = controllers.filter(c => c.zoneId === id);
    
    if (zoneControllers.length > 0) {
      throw new Error('Cannot delete zone with associated controllers. Please remove all controllers from this zone first.');
    }
    
    try {
      await storageService.deleteZone(id);
    } catch (error) {
      logger.error('Error deleting zone:', error);
      throw new Error('Failed to delete zone');
    }
  }
};
