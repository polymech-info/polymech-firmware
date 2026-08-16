import React, { useState, useEffect, useCallback, useRef } from 'react';

import { useModbus } from '@/contexts/ModbusContext';
import { type ProfileSavePayload } from '@polymech/client-ts';
import { type Profile as UIProfile, TemperatureProfileCommand } from '@/types';
import { PROFILE_TEMPERATURE_COUNT, PROFILE_REGISTER_NAMES } from '@/constants';
import { findRegisterForProfile } from '@/lib/controllerUtils.js';
import { transformServiceProfileToUI, transformUIControlPointsToService } from '@/lib/profile-transformers';

import ProfileCard from '@/components/profiles/ProfileCard';
import { Button } from '@/components/ui/button';
import { Download, Upload, HelpCircle } from 'lucide-react';
import { useToast } from '@/components/ui/use-toast';
import { AlertDialog, AlertDialogAction, AlertDialogCancel, AlertDialogContent, AlertDialogDescription, AlertDialogFooter, AlertDialogHeader, AlertDialogTitle } from '@/components/ui/alert-dialog';
import { Select, SelectContent, SelectItem, SelectTrigger, SelectValue } from '@/components/ui/select';

import { T, translate } from '@/i18n';

const Profiles = () => {
  const { toast } = useToast();
  const {
    profiles: serviceProfiles,
    getProfiles: fetchServiceProfiles,
    registers: allModbusRegisters,
    coils: allModbusCoils,
    updateRegister,
    saveProfile,
    uploadProfiles,
    isConnected,
    pressureProfiles,
    settings
  } = useModbus();

  const [uiProfiles, setUiProfiles] = useState<UIProfile[]>([]);
  const [isLoading, setIsLoading] = useState(true);
  const [deleteDialogOpen, setDeleteDialogOpen] = useState(false);
  const [profileToDelete, setProfileToDelete] = useState<UIProfile | null>(null);
  const [copyDialogOpen, setCopyDialogOpen] = useState(false);
  const [profileToCopy, setProfileToCopy] = useState<UIProfile | null>(null);
  const [targetProfileSlot, setTargetProfileSlot] = useState<string | null>(null);
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
    if (serviceProfiles && allModbusRegisters && allModbusCoils && settings) {
      const newUiProfiles: UIProfile[] = serviceProfiles
        .map(p => transformServiceProfileToUI(p, allModbusRegisters, allModbusCoils, settings.partitions))
        .sort((a, b) => a.slot - b.slot);

      setUiProfiles(newUiProfiles);
    }
  }, [serviceProfiles, allModbusRegisters, allModbusCoils, settings]);

  const canCreateNewProfile = uiProfiles.filter(p => p.name && p.name.length > 0).length < PROFILE_TEMPERATURE_COUNT;

  const handleDownloadAllProfilesJson = () => {
    if (uiProfiles.length === 0) {
      toast({ title: translate("No Profiles"), description: translate("There are no profiles to download.") });
      return;
    }
    const profilesToSave = uiProfiles
      .filter(p => p.name && p.name.length > 0)
      .map(p => {
        // Find linked pressure profile to get its target registers
        let pressureTargetRegisters: number[] = [];
        if (p.pressureProfile !== undefined && p.pressureProfile >= 0) {
          const linkedPressureProfile = pressureProfiles.find(pp => pp.slot === p.pressureProfile);
          if (linkedPressureProfile && linkedPressureProfile.targetRegisters) {
            // Ensure we filter out zeros and non-numbers just in case, similar to import logic or robust handling
            pressureTargetRegisters = Array.isArray(linkedPressureProfile.targetRegisters)
              ? linkedPressureProfile.targetRegisters.filter((reg: any) => typeof reg === 'number' && reg > 0)
              : [];
          }
        }

        return {
          slot: p.slot,
          name: p.name,
          description: p.description,
          duration: p.duration,
          max: p.max,
          controlPoints: transformUIControlPointsToService(p.controlPoints),
          targetRegisters: p.targetRegisters,
          signalPlot: p.signalPlot,
          children: p.children,
          enabled: p.enabled,
          pressureProfile: p.pressureProfile,
          pressureTargetRegisters: pressureTargetRegisters,
        };
      });

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

  const handleUploadButtonClick = () => {
    if (!isConnected) {
      toast({ title: translate("Not Connected"), description: translate("Cannot upload profiles. Modbus not connected."), variant: "destructive" });
      return;
    }
    fileInputRef.current?.click();
  };

  const handleUploadProfilesJsonSelected = async (event: React.ChangeEvent<HTMLInputElement>) => {
    const file = event.target.files?.[0];
    if (!file) return;

    try {
      const fileContent = await file.text();
      const profilesData = JSON.parse(fileContent);

      if (!Array.isArray(profilesData)) {
        toast({ title: translate("Invalid File"), description: translate("File must contain an array of profiles."), variant: "destructive" });
        return;
      }

      if (profilesData.length === 0) {
        toast({ title: translate("Empty File"), description: translate("File contains no profiles."), variant: "destructive" });
        return;
      }

      // Validate that each profile has required fields
      for (const profile of profilesData) {
        if (typeof profile.slot !== 'number') {
          toast({ title: translate("Invalid Profile"), description: translate("Each profile must have a valid slot number."), variant: "destructive" });
          return;
        }
      }

      if (!uploadProfiles) {
        toast({ title: translate("Upload Error"), description: translate("Upload function not available."), variant: "destructive" });
        return;
      }

      await uploadProfiles(profilesData);
      toast({
        title: translate("Profiles Uploaded"),
        description: translate("{count} profiles uploaded successfully.").replace('{count}', String(profilesData.length))
      });

    } catch (error) {
      toast({
        title: translate("Upload Failed"),
        description: error instanceof Error ? error.message : translate("Failed to upload profiles."),
        variant: "destructive"
      });
    } finally {
      // Reset the input value so the same file can be selected again
      if (fileInputRef.current) {
        fileInputRef.current.value = '';
      }
    }
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

  const openDeleteDialog = (profile: UIProfile) => {
    setProfileToDelete(profile);
    setDeleteDialogOpen(true);
  };

  const openCopyToDialog = (profile: UIProfile) => {
    setProfileToCopy(profile);
    setTargetProfileSlot(null);
    setCopyDialogOpen(true);
  };

  const handleCopyToConfirm = async () => {
    if (!profileToCopy || targetProfileSlot === null) {
      toast({ title: translate("Copy Error"), description: translate("Source or destination profile not selected."), variant: "destructive" });
      return;
    }

    const targetSlot = Number(targetProfileSlot);
    const targetProfile = uiProfiles.find(p => p.slot === targetSlot);
    if (!targetProfile) {
      toast({ title: translate("Copy Error"), description: translate("Target profile not found."), variant: "destructive" });
      return;
    }

    const payload: ProfileSavePayload = {
      slot: targetSlot,
      name: profileToCopy.name,
      description: profileToCopy.description,
      duration: profileToCopy.duration,
      max: profileToCopy.max,
      controlPoints: transformUIControlPointsToService(profileToCopy.controlPoints) as any,
      targetRegisters: profileToCopy.targetRegisters,
      signalPlot: profileToCopy.signalPlot,
    };

    try {
      if (!saveProfile) throw new Error("saveProfile function is not available.");

      await saveProfile(payload);

      toast({
        title: translate("Profile Copied"),
        description: translate('Successfully copied "{sourceName}" to "{targetName}" (Slot {slot}).').replace('{sourceName}', profileToCopy.name).replace('{targetName}', targetProfile.name || `Slot ${targetSlot}`).replace('{slot}', String(targetSlot))
      });

      await fetchServiceProfiles();
    } catch (error) {
      toast({
        title: translate("Copy Failed"),
        description: error instanceof Error ? error.message : translate("An unknown error occurred."),
        variant: "destructive"
      });
    } finally {
      setCopyDialogOpen(false);
      setProfileToCopy(null);
      setTargetProfileSlot(null);
    }
  };

  const handleProfileCommand = async (profileSlot: number, command: TemperatureProfileCommand) => {
    /// console.log("handleProfileCommand", profileSlot, command);
    if (!isConnected) {
      toast({ title: translate("Error"), description: translate("Not connected to Modbus server."), variant: "destructive" });
      return;
    }
    const currentProfile = uiProfiles.find(p => p.slot === profileSlot);
    if (!currentProfile || !currentProfile.name) {
      toast({ title: translate("Command Error"), description: translate("Profile with slot {slot} not found or missing name.").replace('{slot}', String(profileSlot)), variant: "destructive" });
      return;
    }

    const commandRegisterEntry = findRegisterForProfile(
      allModbusRegisters,
      currentProfile.name,
      profileSlot,
      PROFILE_REGISTER_NAMES.COMMAND
    );

    if (!commandRegisterEntry) {
      toast({
        title: translate("Command Error"),
        description: translate("Command register for profile '{profileName}' (Slot: {slot}) not found.").replace('{profileName}', currentProfile.name).replace('{slot}', String(profileSlot)),
        variant: "destructive",
        duration: 10000
      });
      return;
    }

    try {
      console.log("Sending command to profile", currentProfile.name, commandRegisterEntry.address, command);
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
        <div className="glass-card p-8 flex justify-center items-center h-64">
          <div className="flex flex-col items-center gap-4">
            <div className="h-8 w-8 rounded-full border-4 border-t-cyan-400 border-r-transparent border-b-transparent border-l-transparent animate-spin" />
            <p className="glass-text text-lg"><T>Loading profiles from Modbus...</T></p>
          </div>
        </div>
      </div>
    );
  }

  return (
    <div className="container py-6 space-y-6 px-0 sm:px-2 md:px-0" id="profiles-display">
      <div className="glass-panel p-6 glass-shimmer">
        <div className="flex flex-col md:flex-row justify-between items-center gap-4 md:gap-2">
          <h1 className="text-2xl font-bold text-center md:text-left accent-text"><T>Temperature Profiles</T></h1>
          <div className="flex flex-col sm:flex-row gap-3 w-full md:w-auto">
            <Button asChild variant="outline" className="glass-button w-full sm:w-auto">
              <a href="https://polymech.info/en/resources/cassandra/profiles/" target="_blank" rel="noopener noreferrer">
                <HelpCircle className="h-4 w-4 mr-2" />
                <T>Help</T>
              </a>
            </Button>
            <Button onClick={handleDownloadAllProfilesJson} variant="outline" className="glass-button w-full sm:w-auto">
              <Download className="h-4 w-4 mr-2" />
              <T>Download</T>
            </Button>
            <Button onClick={handleUploadButtonClick} variant="outline" className="glass-button w-full sm:w-auto">
              <Upload className="h-4 w-4 mr-2" />
              <T>Upload</T>
            </Button>
          </div>
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
        <div className="glass-card p-8 flex flex-col items-center justify-center h-64">
          <h3 className="text-lg font-medium glass-text"><T>Waiting for profiles...</T></h3>
          <div className="flex gap-2 mt-4">
            <div className="status-indicator status-disconnected"></div>
          </div>
        </div>
      ) : (
        <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-2 gap-6">
          {uiProfiles.filter(p => p.name && p.name.length > 0).map(profile => (
            <ProfileCard
              key={profile.id}
              profile={profile}
              onDelete={() => openDeleteDialog(profile)}
              onCommand={handleProfileCommand}
              onDuplicate={handleDuplicateProfile}
              onCopyTo={openCopyToDialog}
              canDuplicate={canCreateNewProfile}
            />
          ))}
        </div>
      )}

      <AlertDialog open={deleteDialogOpen} onOpenChange={setDeleteDialogOpen}>
        <AlertDialogContent className="glass-panel border-0">
          <AlertDialogHeader>
            <AlertDialogTitle className="glass-text"><T>Are you sure?</T></AlertDialogTitle>
            <AlertDialogDescription className="text-slate-600 dark:text-white/70">
              {translate('This will permanently clear the profile "{profileName}" from the server. This action cannot be undone.').replace('{profileName}', profileToDelete?.name || '')}
            </AlertDialogDescription>
          </AlertDialogHeader>
          <AlertDialogFooter>
            <AlertDialogCancel className="glass-button"><T>Cancel</T></AlertDialogCancel>
            <AlertDialogAction onClick={handleDeleteProfile} className="status-gradient-error text-white border-0 hover:shadow-lg transition-all duration-300">
              <T>Clear on Server</T>
            </AlertDialogAction>
          </AlertDialogFooter>
        </AlertDialogContent>
      </AlertDialog>

      <AlertDialog open={copyDialogOpen} onOpenChange={setCopyDialogOpen}>
        <AlertDialogContent className="glass-panel border-0">
          <AlertDialogHeader>
            <AlertDialogTitle className="glass-text">{translate('Copy "{profileName}" to...').replace('{profileName}', profileToCopy?.name || '')}</AlertDialogTitle>
            <AlertDialogDescription className="text-slate-600 dark:text-white/70">
              {translate('Select a destination profile. The content of "{profileName}" will overwrite the selected profile. This action cannot be undone.').replace('{profileName}', profileToCopy?.name || '')}
            </AlertDialogDescription>
          </AlertDialogHeader>

          <div className="py-4">
            <Select onValueChange={setTargetProfileSlot} value={targetProfileSlot ?? undefined}>
              <SelectTrigger className="glass-input">
                <SelectValue placeholder={translate("Select a profile to overwrite")} />
              </SelectTrigger>
              <SelectContent className="glass-panel border-0">
                {uiProfiles
                  .filter(p => p.slot !== profileToCopy?.slot)
                  .map(p => (
                    <SelectItem key={p.id} value={String(p.slot)} className="text-slate-700 dark:text-white/90 hover:bg-slate-100 dark:hover:bg-white/10">
                      {p.name || `${translate('Empty Slot')} ${p.slot}`} ({translate('Slot')}: {p.slot})
                    </SelectItem>
                  ))
                }
              </SelectContent>
            </Select>
          </div>

          <AlertDialogFooter>
            <AlertDialogCancel className="glass-button"><T>Cancel</T></AlertDialogCancel>
            <AlertDialogAction onClick={handleCopyToConfirm} disabled={targetProfileSlot === null} className="status-gradient-connected text-white border-0 hover:shadow-lg transition-all duration-300 disabled:opacity-50">
              <T>Copy and Overwrite</T>
            </AlertDialogAction>
          </AlertDialogFooter>
        </AlertDialogContent>
      </AlertDialog>
    </div>
  );
};

export default Profiles;
