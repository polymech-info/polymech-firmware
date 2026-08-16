import React, { useEffect, useState, useCallback } from 'react';
import { useParams, useNavigate } from 'react-router-dom';
import { useModbus } from '@/contexts/ModbusContext';
import PressureProfileEditor from '@/components/profiles/PressureProfileEditor';
import { useToast } from '@/components/ui/use-toast';
import { PressureProfile } from '@/types';
import { T } from '@/i18n';
import { Button } from '@/components/ui/button';
import { ArrowLeft } from 'lucide-react';
import { PressureProfileSavePayload } from '@polymech/client-ts';

const PressureProfilePage = () => {
    const { slot } = useParams<{ slot: string }>();
    const navigate = useNavigate();
    const { toast } = useToast();
    const {
        pressureProfiles,
        getPressureProfiles,
        savePressureProfile,
        isConnected,
    } = useModbus();

    const [initialData, setInitialData] = useState<PressureProfile | undefined>(undefined);
    const [isLoading, setIsLoading] = useState(true);

    const isEditMode = slot !== undefined;

    const loadProfiles = useCallback(async () => {
        if (!isConnected) {
            toast({ title: 'Not Connected', description: 'Cannot fetch profiles. Modbus not connected.' });
            return;
        }
        if (pressureProfiles.length === 0) {
            try {
                await getPressureProfiles();
            } catch (error) {
                toast({
                    title: 'Error Fetching Profiles',
                    description: error instanceof Error ? error.message : 'Could not fetch profiles from context.',
                    variant: 'destructive',
                });
            }
        }
    }, [isConnected, getPressureProfiles, toast, pressureProfiles.length]);

    useEffect(() => {
        loadProfiles();
    }, [loadProfiles]);

    useEffect(() => {
        if (!initialData && isEditMode && pressureProfiles.length > 0) {
            const profileSlot = parseInt(slot, 10);
            const profile = pressureProfiles.find(p => p.slot === profileSlot);
            if (profile) {
                setInitialData(profile);
            } else {
                toast({
                    title: 'Profile not found',
                    description: `Profile with slot ${profileSlot} could not be found.`,
                    variant: 'destructive',
                });
                navigate('/#');
            }
        }
        if (pressureProfiles.length > 0 || !isEditMode) {
            setIsLoading(false);
        }
    }, [slot, isEditMode, pressureProfiles, navigate, toast, initialData]);

    const handleSubmit = async (data: PressureProfileSavePayload) => {
        if (!savePressureProfile) {
            toast({ title: 'Save Error', description: 'savePressureProfile function not available from context.', variant: 'destructive' });
            return;
        }

        try {
            await savePressureProfile(data);
            toast({
                title: isEditMode ? 'Profile Updated' : 'Profile Created',
                description: `Pressure Profile "${data.name}" has been saved successfully.`,
            });
            navigate('/'); // Or back to list?
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
                <Button variant="outline" size="icon" onClick={() => navigate('/')}>
                    <ArrowLeft className="h-4 w-4" />
                </Button>
                <h1 className="text-2xl font-bold">
                    {isEditMode ? <T>Edit Pressure Profile</T> : <T>Create New Pressure Profile</T>}
                </h1>
            </div>
            <div className="max-w-[800px] mx-auto p-4">
                <PressureProfileEditor
                    onSubmit={handleSubmit}
                    initialData={initialData}
                />
            </div>
        </div>
    );
};

export default PressureProfilePage;
