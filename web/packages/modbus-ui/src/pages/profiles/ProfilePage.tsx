import React, { useEffect, useState, useCallback } from 'react';
import { useParams, useNavigate } from 'react-router-dom';
import { useModbus } from '@/contexts/ModbusContext';
import ProfileEditor from '@/components/profiles/ProfileEditorEx';
import { useToast } from '@/components/ui/use-toast';
import { type Profile as UIProfile, Controller } from '@/types';
import { transformControllerConfigsToProfileFormFormat, transformServiceProfileToUI } from '../../lib/profile-transformers';
import { T } from '@/i18n';
import { Button } from '@/components/ui/button';
import { ArrowLeft } from 'lucide-react';
import { type ProfileSavePayload } from '@polymech/client-ts';

const ProfilePage = () => {
  const { slot } = useParams<{ slot: string }>();
  const navigate = useNavigate();
  const { toast } = useToast();
  const {
    profiles: serviceProfiles,
    pressureProfiles,
    getProfiles: fetchServiceProfiles,
    getPressureProfiles,
    saveProfile,
    isConnected,
    registers: allModbusRegisters,
    coils: allModbusCoils,
    settings,
  } = useModbus();

  const [initialData, setInitialData] = useState<UIProfile | undefined>(undefined);
  const [availableControllers, setAvailableControllers] = useState<Controller[]>([]);
  const [isLoading, setIsLoading] = useState(true);

  const isEditMode = slot !== undefined;
  const maxTemp = 100; // This could be dynamic

  useEffect(() => {
    if (settings) {
      const transformed = transformControllerConfigsToProfileFormFormat(settings.partitions);
      setAvailableControllers(transformed);
    }
  }, [settings]);

  const loadProfiles = useCallback(async () => {
    if (!isConnected) {
      toast({ title: 'Not Connected', description: 'Cannot fetch profiles. Modbus not connected.' });
      return;
    }
    if (serviceProfiles.length === 0) {
      try {
        await fetchServiceProfiles();
        await getPressureProfiles();
      } catch (error) {
        toast({
          title: 'Error Fetching Profiles',
          description: error instanceof Error ? error.message : 'Could not fetch profiles from context.',
          variant: 'destructive',
        });
      }
    }
  }, [isConnected, fetchServiceProfiles, getPressureProfiles, toast, serviceProfiles]);

  useEffect(() => {
    loadProfiles();
  }, [loadProfiles]);

  useEffect(() => {
    if (!initialData && isEditMode && serviceProfiles.length > 0 && allModbusRegisters && allModbusCoils && settings) {
      const profileSlot = parseInt(slot, 10);
      const serviceProfile = serviceProfiles.find(p => p.slot === profileSlot);
      if (serviceProfile) {
        const uiProfile = transformServiceProfileToUI(serviceProfile, allModbusRegisters, allModbusCoils, settings.partitions);
        setInitialData(uiProfile);
      } else {
        toast({
          title: 'Profile not found',
          description: `Profile with slot ${profileSlot} could not be found.`,
          variant: 'destructive',
        });
        navigate('/#');
      }
    }
    if (serviceProfiles.length > 0 || !isEditMode) {
      setIsLoading(false);
    }
  }, [slot, isEditMode, serviceProfiles, allModbusRegisters, allModbusCoils, navigate, toast, initialData, settings]);

  const handleSubmit = async (data: ProfileSavePayload) => {
    if (!saveProfile) {
      toast({ title: 'Save Error', description: 'saveProfile function not available from context.', variant: 'destructive' });
      return;
    }

    const nextAvailableSlot = () => {
      const existingSlots = new Set(serviceProfiles.map(p => p.slot));
      for (let i = 0; i < 10; i++) { // Assuming max 10 profiles
        if (!existingSlots.has(i)) return i;
      }
      return -1; // Indicate no slot available
    };

    const profileSlot = isEditMode ? parseInt(slot!, 10) : nextAvailableSlot();

    if (profileSlot === -1) {
      toast({ title: 'Error', description: 'No available profile slots.', variant: 'destructive' });
      return;
    }

    const profilePayload: ProfileSavePayload = {
      name: data.name,
      description: data.description,
      duration: data.duration,
      max: data.max,
      controlPoints: data.controlPoints,
      targetRegisters: data.targetRegisters,
      signalPlot: data.signalPlot,
      pressureProfile: data.pressureProfile,
      children: data.children,
      overrides: data.overrides,
      slot: profileSlot,

    };

    try {
      await saveProfile(profilePayload);
      toast({
        title: isEditMode ? 'Profile Updated' : 'Profile Created',
        description: `Profile "${profilePayload.name}" has been saved successfully.`,
      });
      navigate('/profiles');
    } catch (error) {
      console.error('Failed to save profile:', error);
      toast({
        title: 'Save Failed',
        description: `Failed to save profile: ${error instanceof Error ? error.message : 'Unknown error'}`,
        variant: 'destructive',
      });
    }
  };

  if (isLoading) {
    return (
      <div className="flex justify-center items-center h-64">
        <div className="flex flex-col items-center gap-2">
          <div className="h-8 w-8 rounded-full border-4 border-t-primary border-r-transparent border-b-transparent border-l-transparent animate-spin" />
          <p className="text-muted-foreground">Loading profile data...</p>
        </div>
      </div>
    );
  }

  return (
    <div className="container py-6 space-y-6 px-0 sm:px-2 md:px-4">
      <div className="max-w-[800px] mx-auto flex items-center gap-4">
        <Button variant="outline" size="icon" onClick={() => navigate('/profiles')}>
          <ArrowLeft className="h-4 w-4" />
        </Button>
        <h1 className="text-2xl font-bold">
          {isEditMode ? <T>Edit Profile</T> : <T>Create New Profile</T>}
        </h1>
      </div>
      <div className="max-w-[800px] mx-auto p-4">
        <ProfileEditor
          onSubmit={handleSubmit}
          initialData={initialData}
          max={initialData?.max || maxTemp}
          availableControllers={availableControllers}
        />
      </div>
    </div>
  );
};

export default ProfilePage; 