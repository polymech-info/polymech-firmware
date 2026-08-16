import { useCallback } from 'react';
import modbusApiService from '@polymech/client-ts/modbusApiService';
import {
  type ProfileSavePayload,
  type PressureProfileSavePayload,
} from '@polymech/client-ts';
import { type Profile, type ControlPoint } from '@/types';
import logger from '@/Logger';

export interface WSProfilesHooks {
  getProfiles: () => Promise<void>;
  saveProfile: (profileData: ProfileSavePayload) => Promise<any>;
  uploadProfiles: (profilesData: ProfileSavePayload[]) => Promise<any>;
  getPressureProfiles: () => Promise<void>;
  savePressureProfile: (profileData: PressureProfileSavePayload) => Promise<any>;
}

export const useWSProfiles = (
  apiUrl: string,
  setProfiles: React.Dispatch<React.SetStateAction<Profile[]>>,
  setPressureProfiles: React.Dispatch<React.SetStateAction<any[]>>
): WSProfilesHooks => {

  const getProfiles = useCallback(async (): Promise<void> => {
    try {
      const response = await modbusApiService.getProfiles();
      const mappedProfiles: Profile[] = response.map((p: any) => ({
        ...p,
        enabled: p.enabled !== undefined ? p.enabled : false,
        elapsed: p.elapsed !== undefined ? p.elapsed : 0,
        remaining: p.remaining !== undefined ? p.remaining : 0,
        slot: p.slot !== undefined ? p.slot : 0,
        name: p.name || "Unnamed Profile",
        duration: p.duration !== undefined ? p.duration : 0,
        status: p.status !== undefined ? p.status : 0,
        currentTemp: p.currentTemp !== undefined ? p.currentTemp : 0,
        max: p.max !== undefined ? p.max : 0,
        controlPoints: (p.controlPoints || []).map((cp: any) => ({ x: cp.x || 0, y: cp.y || 0 })) as ControlPoint[],
        targetRegisters: p.targetRegisters || [],
      }));
      setProfiles(mappedProfiles);
    } catch (error) {
      logger.logError(error, 'Error fetching profiles');
    }
  }, [apiUrl, setProfiles]);

  const getPressureProfiles = useCallback(async (): Promise<void> => {
    try {
      const response = await modbusApiService.getPressureProfiles();

      // Map pressure profiles similar to temperature profiles
      const mappedPressureProfiles = response.map((p: any) => ({
        ...p,
        enabled: p.enabled !== undefined ? p.enabled : false,
        slot: p.slot !== undefined ? p.slot : 0,
        name: p.name || "Unnamed Pressure Profile",
        duration: p.duration !== undefined ? p.duration : 0,
        max: p.max !== undefined ? p.max : 0,
        controlPoints: (p.controlPoints || []).map((cp: any) => ({ x: cp.x || 0, y: cp.y || 0 })),
        targetRegisters: p.targetRegisters || [],
      }));

      setPressureProfiles(mappedPressureProfiles);
    } catch (error) {
      logger.logError(error, 'Error fetching pressure profiles');
    }
  }, [setPressureProfiles]);

  const saveProfile = useCallback(async (profileData: ProfileSavePayload): Promise<any> => {
    try {
      const response = await modbusApiService.saveProfile(profileData);
      logger.success(`Profile "${profileData.name}" saved successfully to server.`);
      await getProfiles();
      return response;
    } catch (error) {
      logger.logError(error, 'Error saving profile via context');
      throw error;
    }
  }, [getProfiles]);

  const uploadProfiles = useCallback(async (profilesData: ProfileSavePayload[]): Promise<any> => {
    try {
      const response = await modbusApiService.uploadProfiles(profilesData);
      logger.success(`${profilesData.length} profiles uploaded successfully to server.`);
      await getProfiles();
      return response;
    } catch (error) {
      logger.logError(error, 'Error uploading profiles via context');
      throw error;
    }
  }, [getProfiles]);

  const savePressureProfile = useCallback(async (profileData: PressureProfileSavePayload): Promise<any> => {
    try {
      const response = await modbusApiService.savePressureProfile(profileData);
      logger.success(`Pressure profile "${profileData.name}" saved successfully to server.`);
      await getPressureProfiles();
      return response;
    } catch (error) {
      logger.logError(error, 'Error saving pressure profile via context');
      throw error;
    }
  }, [getPressureProfiles]);

  return {
    getProfiles,
    saveProfile,
    uploadProfiles,
    getPressureProfiles,
    savePressureProfile,
  };
}; 