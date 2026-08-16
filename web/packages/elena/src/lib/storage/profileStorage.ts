
import { TemperatureProfile } from '../types';
import { supabaseService } from '../supabaseService';

// Profile-specific storage methods
class ProfileStorage {
  async getProfiles(): Promise<TemperatureProfile[]> {
    return supabaseService.getProfiles();
  }

  async saveProfiles(profiles: TemperatureProfile[]): Promise<void> {
    return supabaseService.saveProfiles(profiles);
  }
}

export const profileStorage = new ProfileStorage();
