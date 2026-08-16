import React, { useState, useEffect, useCallback, useRef } from 'react';
import { useNavigate } from 'react-router-dom';

import { useModbus } from '@/contexts/ModbusContext';
import {type ProfileSavePayload } from '@polymech/client-ts';
import { type Profile as UIProfile, TemperatureProfileCommand, PlotStatus } from '@/types';
import { PROFILE_TEMPERATURE_COUNT, PROFILE_REGISTER_NAMES } from '@/constants';
import { PARTITION_CONFIG } from '@/lib/controllerUtils.js';
import { transformServiceProfileToUI, transformUIControlPointsToService } from '@/lib/profile-transformers';

import ProfileCard from '@/components/profiles/ProfileCard';
import { Button } from '@/components/ui/button';
import { Plus, Download, Upload, HelpCircle } from 'lucide-react';
import { useToast } from '@/components/ui/use-toast';
import { AlertDialog, AlertDialogAction, AlertDialogCancel, AlertDialogContent, AlertDialogDescription, AlertDialogFooter, AlertDialogHeader, AlertDialogTitle } from '@/components/ui/alert-dialog';

import { T, translate } from '@/i18n';


const Profiles = () => {
  const { toast } = useToast();
  const navigate = useNavigate();
  const { 
    profiles: serviceProfiles, 
    getProfiles: fetchServiceProfiles, 
    registers: allModbusRegisters, 
    coils: allModbusCoils, 
    updateRegister,
    updateCoil, 
    saveProfile,
    saveMultipleProfiles,
    isConnected 
  } = useModbus();
  
  const [uiProfiles, setUiProfiles] = useState<UIProfile[]>([]);
  const [isLoading, setIsLoading] = useState(true);
  const [deleteDialogOpen, setDeleteDialogOpen] = useState(false);
  const [profileToDelete, setProfileToDelete] = useState<UIProfile | null>(null);
  const [notifiedFinishedProfiles, setNotifiedFinishedProfiles] = useState<Set<number>>(new Set());
  const fileInputRef = useRef<HTMLInputElement>(null);
  
  const loadAndTransformProfiles = useCallback(async () => { 
    if (!isConnected) {
      toast({ title: translate("Not Connected"), description: translate("Cannot fetch profiles. Modbus not connected.") });
      setIsLoading(false);
      setUiProfiles([]);
      return;
    }
    setIsLoading(true);
    try {
      await fetchServiceProfiles();
    } catch (error) {
      toast({
        title: translate("Error Fetching Profiles"),
        description: error instanceof Error ? error.message : translate("Could not fetch profiles from context."),
        variant: "destructive"
      });
      setUiProfiles([]);
    } finally {
      setIsLoading(false);
    }
  }, [isConnected, fetchServiceProfiles, toast]);

  useEffect(() => {
    loadAndTransformProfiles();
  }, [loadAndTransformProfiles]);

  useEffect(() => {
    if (serviceProfiles && allModbusRegisters && allModbusCoils) { 
      const newUiProfiles: UIProfile[] = serviceProfiles
        .map(p => transformServiceProfileToUI(p, allModbusRegisters, allModbusCoils, PARTITION_CONFIG))
        .sort((a, b) => a.slot - b.slot);

      setUiProfiles(currentUiProfiles => {
        newUiProfiles.forEach(newProfile => {
          if (newProfile.status === PlotStatus.FINISHED && newProfile.slot !== undefined) {
            const oldProfile = currentUiProfiles.find(op => op.slot === newProfile.slot);
            if ((!oldProfile || oldProfile.status !== PlotStatus.FINISHED) && !notifiedFinishedProfiles.has(newProfile.slot)) {
              if (Notification.permission === "granted") {
                new Notification(translate("Profile Finished"), {
                  body: translate('Profile "{profileName}" (Slot {slot}) has finished.').replace('{profileName}', newProfile.name).replace('{slot}', String(newProfile.slot)),
                });
                setNotifiedFinishedProfiles(prev => new Set(prev).add(newProfile.slot));
              } else if (Notification.permission !== "denied") {
                Notification.requestPermission().then(permission => {
                  if (permission === "granted") {
                    new Notification(translate("Profile Finished"), {
                      body: translate('Profile "{profileName}" (Slot {slot}) has finished.').replace('{profileName}', newProfile.name).replace('{slot}', String(newProfile.slot)),
                    });
                    setNotifiedFinishedProfiles(prev => new Set(prev).add(newProfile.slot));
                  }
                });
              }
            }
          } else if (newProfile.status !== PlotStatus.FINISHED && newProfile.slot !== undefined && notifiedFinishedProfiles.has(newProfile.slot)) {
            setNotifiedFinishedProfiles(prev => {
              const updatedSet = new Set(prev);
              updatedSet.delete(newProfile.slot);
              return updatedSet;
            });
          }
        });
        return newUiProfiles;
      });
    }
  }, [serviceProfiles, allModbusRegisters, allModbusCoils, notifiedFinishedProfiles]);
  
  useEffect(() => {
    if ('Notification' in window && Notification.permission !== "granted" && Notification.permission !== "denied") {
      Notification.requestPermission();
    }
  }, []);

  const canCreateNewProfile = uiProfiles.filter(p => p.name && p.name.length > 0).length < PROFILE_TEMPERATURE_COUNT;
  
  const handleDownloadAllProfilesJson = () => {
    if (uiProfiles.length === 0) {
      toast({ title: translate("No Profiles"), description: translate("There are no profiles to download.") });
      return;
    }
    const profilesToSave = uiProfiles
      .filter(p => p.name && p.name.length > 0)
      .map(p => ({
        slot: p.slot,
        name: p.name,
        description: p.description,
        duration: p.duration,
        max: p.max,
        controlPoints: transformUIControlPointsToService(p.controlPoints),
        targetRegisters: p.targetRegisters,
        signalPlot: p.signalPlot,
      }));

    const jsonString = JSON.stringify(profilesToSave, null, 2);
    const blob = new Blob([jsonString], { type: 'application/json' });
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = 'temperature-profiles.json';
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);
    URL.revokeObjectURL(url);
    toast({ title: translate("Profiles Downloaded"), description: translate("All temperature profiles have been downloaded.") });
  };

  const handleUploadProfilesJsonSelected = async (event: React.ChangeEvent<HTMLInputElement>) => {
    const file = event.target.files?.[0];
    if (!file) return;

    if (!saveMultipleProfiles) {
      toast({ title: translate("Feature Not Available"), description: translate("The 'saveMultipleProfiles' function is not available in the current context."), variant: "destructive" });
      return;
    }

    const reader = new FileReader();
    reader.onload = async (e) => {
      try {
        const text = e.target?.result;
        if (typeof text !== 'string') throw new Error(translate("Failed to read file content."));
        const uploadedProfiles = JSON.parse(text) as ProfileSavePayload[];
        
        if (!Array.isArray(uploadedProfiles)) throw new Error(translate("Invalid JSON format: Expected an array of profiles."));
        // Basic validation for the first profile object
        if (uploadedProfiles.length > 0) {
            const firstProfile = uploadedProfiles[0];
            const requiredKeys: (keyof ProfileSavePayload)[] = ['slot', 'name', 'controlPoints', 'duration', 'max'];
            for (const key of requiredKeys) {
                if (!(key in firstProfile)) {
                    throw new Error(translate("Invalid profile format: Missing required key '{key}' in first profile.").replace('{key}', key));
                }
            }
        }
        
        await saveMultipleProfiles(uploadedProfiles);
        
        toast({ title: translate("Upload Successful"), description: translate("{count} profiles have been uploaded and saved.").replace('{count}', String(uploadedProfiles.length)) });

        await fetchServiceProfiles();

      } catch (error) {
        toast({
          title: translate("Upload Failed"),
          description: error instanceof Error ? error.message : translate("An unknown error occurred during upload."),
          variant: "destructive"
        });
      } finally {
        if (fileInputRef.current) {
          fileInputRef.current.value = "";
        }
      }
    };
    reader.readAsText(file);
  };
  
  const triggerUploadJson = () => {
    fileInputRef.current?.click();
  };
  
  const handleDeleteProfile = async () => {
    if (!profileToDelete) return;
    
    const blankProfile: ProfileSavePayload = {
      slot: profileToDelete.slot,
      name: "",
      description: "",
      duration: 0,
      max: 0,
      controlPoints: [],
      targetRegisters: [],
    };

    try {
      if (!saveProfile) throw new Error("saveProfile not available");
      await saveProfile(blankProfile);
      toast({ title: translate("Profile Cleared"), description: translate('Profile "{profileName}" has been cleared on the server.').replace('{profileName}', profileToDelete.name) });
      await fetchServiceProfiles();
    } catch (error) {
      toast({ title: translate("Delete Error"), description: translate("Could not clear profile on the server."), variant: "destructive" });
    } finally {
      setDeleteDialogOpen(false);
      setProfileToDelete(null);
    }
  };
  
  const openEditPage = (profile: UIProfile) => {
    navigate(`/profiles/edit/${profile.slot}`);
  };
  
  const openDeleteDialog = (profile: UIProfile) => {
    setProfileToDelete(profile);
    setDeleteDialogOpen(true);
  };
  
  const handleProfileCommand = async (profileSlot: number, command: TemperatureProfileCommand) => {
    if (!isConnected) {
      toast({ title: translate("Error"), description: translate("Not connected to Modbus server."), variant: "destructive" });
      return;
    }
    const currentProfile = uiProfiles.find(p => p.slot === profileSlot);
    if (!currentProfile || !currentProfile.name) { 
      toast({ title: translate("Command Error"), description: translate("Profile with slot {slot} not found or missing name.").replace('{slot}', String(profileSlot)), variant: "destructive" });
      return;
    }

    const commandRegisterEntry = allModbusRegisters.find(
      reg => reg.group === currentProfile.name && reg.name === PROFILE_REGISTER_NAMES.COMMAND
    );

    if (!commandRegisterEntry) {
        toast({ 
            title: translate("Command Error"), 
            description: translate("Command register (Group: {group}, Name: {name}) not found.").replace('{group}', currentProfile.name).replace('{name}', PROFILE_REGISTER_NAMES.COMMAND), 
            variant: "destructive",
            duration: 10000
        });
        return;
    }

    try {
      await updateRegister(commandRegisterEntry.address, command);
      toast({ 
        title: translate("Profile Command Sent"), 
        description: translate("{command} command sent to profile '{profileName}' (Register: {address}).").replace('{command}', TemperatureProfileCommand[command]).replace('{profileName}', currentProfile.name).replace('{address}', String(commandRegisterEntry.address))
      });
    } catch (error) {
      toast({ 
        title: translate("Command Failed"), 
        description: translate("Failed to send command to profile {slot}: {error}").replace('{slot}', String(profileSlot)).replace('{error}', error instanceof Error ? error.message : String(error)), 
        variant: "destructive" 
      });
    }
  };
  
  const handleProfileEnableToggle = async (profileSlot: number, currentProfileName: string, newEnabledState: boolean) => {
    if (!isConnected || !allModbusCoils) {
      toast({ title: translate("Error"), description: translate("Not connected or coils not loaded."), variant: "destructive" });
      return;
    }
    const enableCoil = allModbusCoils.find(coil => 
      coil.group === currentProfileName && 
      coil.name === PROFILE_REGISTER_NAMES.ENABLE_CMD
    );

    if (!enableCoil) {
      toast({ 
        title: translate("Toggle Error"), 
        description: translate("Enable/Disable coil (Group: {group}, Name: {name}) not found.").replace('{group}', currentProfileName).replace('{name}', PROFILE_REGISTER_NAMES.ENABLE_CMD), 
        variant: "destructive"
      });
      return;
    }

    try {
      await updateCoil(enableCoil.address, newEnabledState);
      toast({ 
        title: translate("Profile Update"), 
        description: translate("Profile '{profileName}' {state}.").replace('{profileName}', currentProfileName).replace('{state}', newEnabledState ? translate("enabled") : translate("disabled"))
      });
    } catch (error) {
      toast({ 
        title: translate("Toggle Failed"), 
        description: translate("Failed to {action} profile '{profileName}': {error}").replace('{action}', newEnabledState ? translate("enable") : translate("disable")).replace('{profileName}', currentProfileName).replace('{error}', error instanceof Error ? error.message : String(error)), 
        variant: "destructive" 
      });
    }
  };
  
  const handleDuplicateProfile = async (profileToDuplicate: UIProfile) => {
    if (!canCreateNewProfile) {
      toast({ title: translate("Limit Reached"), description: translate("Cannot duplicate profile. Maximum of {count} profiles allowed.").replace('{count}', String(PROFILE_TEMPERATURE_COUNT)), variant: "destructive" });
      return;
    }
    if (!saveProfile) {
      toast({ title: translate("Duplicate Error"), description: translate("saveProfile function not available from context."), variant: "destructive" });
      return;
    }
    const existingSlots = new Set(uiProfiles.map(p => p.slot));
    let nextSlot = -1;
    for (let i = 0; i < PROFILE_TEMPERATURE_COUNT; i++) {
      if (!existingSlots.has(i)) {
        nextSlot = i;
        break;
      }
    }

    if (nextSlot === -1) {
      toast({ title: translate("Limit Reached"), description: translate("No available slots to duplicate profile."), variant: "destructive" });
      return;
    }

    const duplicatedProfileData: ProfileSavePayload = {
      slot: nextSlot, 
      name: `${profileToDuplicate.name} (Copy)`,
      description: profileToDuplicate.description || "",
      max: profileToDuplicate.max,
      controlPoints: transformUIControlPointsToService(profileToDuplicate.controlPoints) as any, 
      targetRegisters: profileToDuplicate.targetRegisters || [],
      duration: profileToDuplicate.duration,
      signalPlot: profileToDuplicate.signalPlot,
    };
    try {
      await saveProfile(duplicatedProfileData);
      toast({ 
        title: translate("Profile Duplicated"), 
        description: translate('Profile "{profileName}" created with slot {slot}.').replace('{profileName}', duplicatedProfileData.name).replace('{slot}', String(nextSlot))
      });
      await fetchServiceProfiles();
    } catch (error) {
      console.error("Failed to duplicate profile:", error);
    }
  };
  
  if (isLoading && uiProfiles.length === 0) {
    return (
      <div className="container py-6 space-y-6">
        <div className="flex justify-center items-center h-64">
          <div className="flex flex-col items-center gap-2">
            <div className="h-8 w-8 rounded-full border-4 border-t-primary border-r-transparent border-b-transparent border-l-transparent animate-spin" />
            <p className="text-muted-foreground"><T>Loading profiles from Modbus...</T></p>
          </div>
        </div>
      </div>
    );
  }
  
  return (
    <div className="container py-6 space-y-6 px-0 sm:px-2 md:px-0">
      <div className="flex flex-col md:flex-row justify-between items-center gap-4 md:gap-2">
        <h1 className="text-2xl font-bold text-center md:text-left"><T>Temperature Profiles</T></h1>
        <div className="flex flex-col sm:flex-row gap-2 w-full md:w-auto">
          <Button asChild variant="outline" className="w-full sm:w-auto">
            <a href="https://polymech.io/docs/cassandra/profiles/" target="_blank" rel="noopener noreferrer">
              <HelpCircle className="h-4 w-4 mr-2" />
              <T>Help</T>
            </a>
          </Button>
          <Button onClick={handleDownloadAllProfilesJson} variant="outline" className="w-full sm:w-auto">
            <Download className="h-4 w-4 mr-2" />
            <T>Download</T>
          </Button>
          <Button onClick={triggerUploadJson} variant="outline" className="w-full sm:w-auto">
            <Upload className="h-4 w-4 mr-2" />
            <T>Upload</T>
          </Button>
          <Button onClick={() => navigate('/profiles/new')} className="w-full sm:w-auto" disabled={!canCreateNewProfile}>
            <Plus className="h-4 w-4 mr-2" />
            <T>New Profile</T>
            {!canCreateNewProfile && <span className="ml-2 text-xs">({PROFILE_TEMPERATURE_COUNT} <T>max</T>)</span>}
          </Button>
        </div>
      </div>
      
      <input
        type="file"
        ref={fileInputRef}
        onChange={handleUploadProfilesJsonSelected}
        accept=".json"
        style={{ display: 'none' }}
      />

      {uiProfiles.filter(p => p.name && p.name.length > 0).length === 0 && !isLoading ? (
        <div className="flex flex-col items-center justify-center h-64 border rounded-lg bg-muted/10">
          <h3 className="text-lg font-medium"><T>No profiles found on server</T></h3>
          <p className="text-muted-foreground mb-4"><T>Create a new profile to get started.</T></p>
          <div className="flex gap-2">
            <Button onClick={() => navigate('/profiles/new')} disabled={!canCreateNewProfile}>
              <Plus className="h-4 w-4 mr-2" />
              <T>Create Profile</T>
              {!canCreateNewProfile && <span className="ml-2 text-xs">({PROFILE_TEMPERATURE_COUNT} <T>max</T>)</span>}
            </Button>
          </div>
        </div>
      ) : (
        <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-2 gap-6">
          {uiProfiles.filter(p => p.name && p.name.length > 0).map(profile => (
            <ProfileCard
              key={profile.slot}
              profile={profile}
              onEdit={openEditPage}
              onDelete={() => openDeleteDialog(profile)}
              onCommand={handleProfileCommand}
              onEnableToggle={handleProfileEnableToggle}
              onDuplicate={handleDuplicateProfile}
              canDuplicate={canCreateNewProfile}
            />
          ))}
        </div>
      )}
      
      <AlertDialog open={deleteDialogOpen} onOpenChange={setDeleteDialogOpen}>
        <AlertDialogContent>
          <AlertDialogHeader>
            <AlertDialogTitle><T>Are you sure?</T></AlertDialogTitle>
            <AlertDialogDescription>
              {translate('This will permanently clear the profile "{profileName}" from the server. This action cannot be undone.').replace('{profileName}', profileToDelete?.name || '')}
            </AlertDialogDescription>
          </AlertDialogHeader>
          <AlertDialogFooter>
            <AlertDialogCancel><T>Cancel</T></AlertDialogCancel>
            <AlertDialogAction onClick={handleDeleteProfile} className="bg-destructive">
              <T>Clear on Server</T>
            </AlertDialogAction>
          </AlertDialogFooter>
        </AlertDialogContent>
      </AlertDialog>
    </div>
  );
};

export default Profiles;
