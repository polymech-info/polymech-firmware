import { supabase } from "@/integrations/supabase/client";
import { TemperatureProfile, HeatZone, Controller, ControlPoint } from './types';
import type { Database } from '@/integrations/supabase/types';
import { Json } from '@/integrations/supabase/types';
import { defaultProfiles } from './data/defaultData';
import { logger } from '@/logger';

// Supabase Service
class SupabaseService {
  // Profile-specific methods
  async getProfiles(): Promise<TemperatureProfile[]> {
    const { data, error } = await supabase
      .from('temperature_profiles')
      .select('*');
      
    if (error) {
      logger.error('Error fetching profiles:', error);
      
      // If fetching fails, return default profiles
      return defaultProfiles;
    }
    
    // If no profiles are found in the database, initialize with default profiles
    if (data.length === 0) {
      logger.info('No profiles found in database, initializing with defaults');
      
      try {
        await this.initializeDefaultProfiles();
        
        // Re-fetch after initialization
        const { data: newData, error: newError } = await supabase
          .from('temperature_profiles')
          .select('*');
          
        if (newError) {
          logger.error('Error fetching profiles after initialization:', newError);
          return defaultProfiles;
        }
        
        return this.parseProfilesFromDatabase(newData);
      } catch (initError) {
        logger.error('Error initializing default profiles:', initError);
        return defaultProfiles;
      }
    }
    
    return this.parseProfilesFromDatabase(data);
  }
  
  // Helper method to parse profiles from database
  private parseProfilesFromDatabase(data: any[]): TemperatureProfile[] {
    return data.map(profile => ({
      id: profile.id,
      name: profile.name,
      description: profile.description || '',
      controlPoints: this.parseControlPoints(profile.control_points),
      duration: profile.duration,
      createdAt: profile.created_at,
      updatedAt: profile.updated_at
    }));
  }

  // Initialize database with default profiles
  private async initializeDefaultProfiles(): Promise<void> {
    const { error } = await supabase
      .from('temperature_profiles')
      .upsert(
        defaultProfiles.map(profile => ({
          id: profile.id,
          name: profile.name,
          description: profile.description,
          control_points: profile.controlPoints as unknown as Json,
          duration: profile.duration,
          created_at: profile.createdAt,
          updated_at: profile.updatedAt
        }))
      );
      
    if (error) {
      logger.error('Error initializing default profiles:', error);
      throw error;
    }
  }

  // Helper method to parse control points from JSON
  private parseControlPoints(controlPointsJson: Json): ControlPoint[] {
    if (!controlPointsJson) return [];
    
    try {
      if (typeof controlPointsJson === 'string') {
        return JSON.parse(controlPointsJson) as ControlPoint[];
      }
      
      // First cast to unknown, then to the specific type
      if (Array.isArray(controlPointsJson)) {
        return controlPointsJson.map(point => {
          // Safely access properties by checking the type first
          if (typeof point === 'object' && point !== null) {
            // Need to use type assertion after verifying it's an object
            const pointObj = point as Record<string, unknown>;
            
            // Ensure each point has the required properties
            return {
              x: typeof pointObj.x === 'number' ? pointObj.x : 0,
              y: typeof pointObj.y === 'number' ? pointObj.y : 0,
              type: (typeof pointObj.type === 'string' ? pointObj.type : 'linear') as ControlPoint['type'],
              handleX: typeof pointObj.handleX === 'number' ? pointObj.handleX : undefined,
              handleY: typeof pointObj.handleY === 'number' ? pointObj.handleY : undefined
            };
          }
          return { x: 0, y: 0, type: 'linear' };
        });
      }
      
      logger.error('Control points data is not an array:', controlPointsJson);
      return [];
    } catch (error) {
      logger.error('Error parsing control points:', error);
      return [];
    }
  }

  async saveProfiles(profiles: TemperatureProfile[]): Promise<void> {
    // For now, we'll handle this as a full replacement
    // In a production app, you might want more sophisticated sync logic
    const { error } = await supabase
      .from('temperature_profiles')
      .upsert(
        profiles.map(profile => ({
          id: profile.id,
          name: profile.name,
          description: profile.description,
          control_points: profile.controlPoints as unknown as Json,
          duration: profile.duration,
          created_at: profile.createdAt,
          updated_at: new Date().toISOString()
        }))
      );
      
    if (error) {
      logger.error('Error saving profiles:', error);
      throw error;
    }
  }

  // Helper method to abstract fetching and mapping data
  private async _fetchAndMap<DbRow extends { id: string }, OutputType>(
    tableName: keyof Database['public']['Tables'],
    errorMessagePrefix: string,
    mapper: (row: DbRow) => OutputType,
    emptyResultValue: OutputType[] = []
  ): Promise<OutputType[]> {
    const { data, error } = await supabase
      .from(tableName)
      .select('*');

    if (error) {
      logger.error(`${errorMessagePrefix}:`, error);
      return emptyResultValue;
    }

    // Using 'any' for casting simplicity, assuming Supabase returns array or null
    const typedData = data as any[] | null;

    if (!typedData) {
      logger.error(`${errorMessagePrefix}: No data returned.`);
      return emptyResultValue;
    }

    // Use type assertion for the mapper argument
    return typedData.map(row => mapper(row as DbRow));
  }

  // Zone-specific methods
  async getZones(): Promise<HeatZone[]> {
    // Define the specific mapper for zones
    const mapZone = (zone: any): HeatZone => ({
      id: zone.id,
      name: zone.name,
      description: zone.description || ''
    });

    return this._fetchAndMap('heat_zones', 'Error fetching zones', mapZone);
  }

  async createZone(zone: Omit<HeatZone, 'id'>): Promise<HeatZone> {
    const { data, error } = await supabase
      .from('heat_zones')
      .insert([
        {
          name: zone.name,
          description: zone.description || ''
        }
      ])
      .select()
      .single();
      
    if (error) {
      logger.error('Error creating zone:', error);
      throw new Error(`Failed to create zone: ${error.message}`);
    }
    
    return {
      id: data.id,
      name: data.name,
      description: data.description || ''
    };
  }

  async saveZones(zones: HeatZone[]): Promise<void> {
    const { error } = await supabase
      .from('heat_zones')
      .upsert(
        zones.map(zone => ({
          id: zone.id,
          name: zone.name,
          description: zone.description
        }))
      );
      
    if (error) {
      logger.error('Error saving zones:', error);
      throw error;
    }
  }

  async deleteZone(id: string): Promise<void> {
    const { error } = await supabase
      .from('heat_zones')
      .delete()
      .eq('id', id);
      
    if (error) {
      logger.error('Error deleting zone:', error);
      throw new Error(`Failed to delete zone: ${error.message}`);
    }
  }

  // Controller-specific methods
  async getControllers(): Promise<Controller[]> {
    // Define the specific mapper for controllers
    const mapController = (controller: any): Controller => ({
      id: controller.id,
      name: controller.name,
      currentTemp: controller.current_temp,
      targetTemp: controller.target_temp,
      minTemp: controller.min_temp,
      maxTemp: controller.max_temp,
      slaveId: controller.slave_id,
      updateInterval: controller.update_interval,
      currentProfile: controller.current_profile,
      isRunning: controller.is_running,
      lastUpdated: controller.last_updated,
      zoneId: controller.zone_id
    });

    return this._fetchAndMap('controllers', 'Error fetching controllers', mapController);
  }

  async saveControllers(controllers: Controller[]): Promise<void> {
    const { error } = await supabase
      .from('controllers')
      .upsert(
        controllers.map(controller => ({
          id: controller.id,
          name: controller.name,
          current_temp: controller.currentTemp,
          target_temp: controller.targetTemp,
          min_temp: controller.minTemp,
          max_temp: controller.maxTemp,
          slave_id: controller.slaveId,
          update_interval: controller.updateInterval,
          current_profile: controller.currentProfile,
          is_running: controller.isRunning,
          last_updated: new Date().toISOString(),
          zone_id: controller.zoneId
        }))
      );
      
    if (error) {
      logger.error('Error saving controllers:', error);
      throw error;
    }
  }
  
  async deleteController(id: string): Promise<void> {
    const { error } = await supabase
      .from('controllers')
      .delete()
      .eq('id', id);
      
    if (error) {
      logger.error('Error deleting controller:', error);
      throw new Error(`Failed to delete controller: ${error.message}`);
    }
  }
}

export const supabaseService = new SupabaseService();
