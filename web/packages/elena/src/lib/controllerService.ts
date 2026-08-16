import { z } from "zod";
import { storageService } from './storageService';
import { Controller } from './types';
import { logger } from '@/logger';

// Controller validation schema
const controllerSchema = z.object({
  name: z.string().min(1),
  minTemp: z.number(),
  maxTemp: z.number(),
  targetTemp: z.number(),
  slaveId: z.number().int().positive(),
  updateInterval: z.number().int().positive(),
  zoneId: z.string().min(1),
});

export type ControllerInput = z.infer<typeof controllerSchema>;

// Controller Service
export const controllerService = {
  // Get all controllers
  getControllers: async (): Promise<Controller[]> => {
    return await storageService.getControllers();
  },

  // Get a single controller by ID
  getController: async (id: string): Promise<Controller> => {
    const controllers = await storageService.getControllers();
    const controller = controllers.find(c => c.id === id);
    
    if (!controller) {
      throw new Error('Controller not found');
    }
    
    return controller;
  },

  // Update controller properties
  updateController: async (id: string, data: Partial<Controller>): Promise<Controller> => {
    const controllers = await storageService.getControllers();
    const index = controllers.findIndex(c => c.id === id);
    
    if (index === -1) {
      throw new Error('Controller not found');
    }
    
    controllers[index] = {
      ...controllers[index],
      ...data,
      lastUpdated: new Date().toISOString()
    };
    
    await storageService.saveControllers(controllers);
    
    return controllers[index];
  },

  // Start a controller (optionally with a profile)
  startController: async (id: string, profileId?: string): Promise<Controller> => {
    const controllers = await storageService.getControllers();
    const index = controllers.findIndex(c => c.id === id);
    
    if (index === -1) {
      throw new Error('Controller not found');
    }
    
    controllers[index] = {
      ...controllers[index],
      currentProfile: profileId || controllers[index].currentProfile,
      isRunning: true,
      lastUpdated: new Date().toISOString()
    };
    
    await storageService.saveControllers(controllers);
    
    return controllers[index];
  },

  // Stop a controller
  stopController: async (id: string): Promise<Controller> => {
    const controllers = await storageService.getControllers();
    const index = controllers.findIndex(c => c.id === id);
    
    if (index === -1) {
      throw new Error('Controller not found');
    }
    
    controllers[index] = {
      ...controllers[index],
      isRunning: false,
      lastUpdated: new Date().toISOString()
    };
    
    await storageService.saveControllers(controllers);
    
    return controllers[index];
  },

  // Create a new controller
  createController: async (data: ControllerInput): Promise<Controller> => {
    const validated = controllerSchema.parse(data);
    
    const controllers = await storageService.getControllers();
    
    const newId = crypto.randomUUID();
    
    const newController: Controller = {
      id: newId,
      name: validated.name,
      minTemp: validated.minTemp,
      maxTemp: validated.maxTemp,
      targetTemp: validated.targetTemp,
      slaveId: validated.slaveId,
      updateInterval: validated.updateInterval,
      currentTemp: validated.targetTemp - 5 + Math.random() * 10,
      currentProfile: null,
      isRunning: false,
      lastUpdated: new Date().toISOString(),
      zoneId: validated.zoneId
    };
    
    controllers.push(newController);
    await storageService.saveControllers(controllers);
    
    return newController;
  },

  // Delete a controller
  deleteController: async (id: string): Promise<void> => {
    try {
      await storageService.deleteController(id);
    } catch (error) {
      logger.error('Error deleting controller:', error);
      throw new Error('Failed to delete controller');
    }
  }
};
