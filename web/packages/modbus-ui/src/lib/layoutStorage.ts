import modbusApiService from '@polymech/client-ts/modbusApiService';
import logger from '@/Logger';
import { RootLayoutData } from './unifiedLayoutManager';

export interface LayoutStorageService {
  load(): Promise<RootLayoutData | null>;
  save(data: RootLayoutData): Promise<boolean>;
  saveToApiOnly(data: RootLayoutData): Promise<boolean>;
}

export class ApiLayoutStorageService implements LayoutStorageService {
  private static readonly FILENAME = 'layout.json';

  async load(): Promise<RootLayoutData | null> {
    try {
      // Load from API only
      const response = await modbusApiService.readFile(ApiLayoutStorageService.FILENAME);
      if (response) {
        return response as unknown as RootLayoutData;
      }
    } catch (apiError) {
      logger.warn('Failed to load layout from API', apiError);
    }

    return null;
  }

  async save(data: RootLayoutData): Promise<boolean> {
    // Save to API only
    try {
      const content = JSON.stringify(data, null, 2);
      const response = await modbusApiService.writeFile({
        filename: ApiLayoutStorageService.FILENAME,
        content
      });
      
      return response.success;
    } catch (apiError) {
      logger.error('Failed to save layout to API', apiError);
      return false;
    }
  }

  async saveToApiOnly(data: RootLayoutData): Promise<boolean> {
    // saveToApiOnly is now the same as save since we removed localStorage fallback
    return this.save(data);
  }
}


// Default storage service instance
export const layoutStorage: LayoutStorageService = new ApiLayoutStorageService();
