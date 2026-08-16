
import React, { useState, useEffect } from 'react';
import { api, TemperatureProfile } from '@/lib/api';
import MainNav from '@/components/MainNav';
import ProfileCard from '@/components/ProfileCard';
import { Button } from '@/components/ui/button';
import { Plus, RefreshCw } from 'lucide-react';
import { useToast } from '@/components/ui/use-toast';
import { Dialog, DialogContent, DialogHeader, DialogTitle } from '@/components/ui/dialog';
import ProfileForm from '@/components/ProfileForm';
import { AlertDialog, AlertDialogAction, AlertDialogCancel, AlertDialogContent, AlertDialogDescription, AlertDialogFooter, AlertDialogHeader, AlertDialogTitle } from '@/components/ui/alert-dialog';
import { useQuery } from '@tanstack/react-query';
import { defaultProfiles } from '@/lib/data/defaultData';

const Profiles = () => {
  const { toast } = useToast();
  const [profiles, setProfiles] = useState<TemperatureProfile[]>([]);
  const [createDialogOpen, setCreateDialogOpen] = useState(false);
  const [editDialogOpen, setEditDialogOpen] = useState(false);
  const [deleteDialogOpen, setDeleteDialogOpen] = useState(false);
  const [selectedProfile, setSelectedProfile] = useState<TemperatureProfile | null>(null);
  
  // Default min/max temperatures for the editor
  const minTemp = 0;
  const maxTemp = 100;
  
  // Fetch profiles
  const { data: profilesData, isLoading, refetch } = useQuery({
    queryKey: ['profiles'],
    queryFn: api.getProfiles
  });
  
  // Update state when data is loaded
  useEffect(() => {
    if (profilesData) setProfiles(profilesData);
  }, [profilesData]);
  
  // Handle create profile
  const handleCreateProfile = async (data: Omit<TemperatureProfile, 'id' | 'createdAt' | 'updatedAt'>) => {
    try {
      await api.createProfile(data);
      setCreateDialogOpen(false);
      refetch();
      
      toast({
        title: "Profile Created",
        description: `${data.name} has been created successfully`
      });
    } catch (error) {
      toast({
        title: "Error",
        description: "Failed to create profile",
        variant: "destructive"
      });
    }
  };
  
  // Handle edit profile
  const handleEditProfile = async (data: Omit<TemperatureProfile, 'id' | 'createdAt' | 'updatedAt'>) => {
    if (!selectedProfile) return;
    
    try {
      await api.updateProfile(selectedProfile.id, data);
      setEditDialogOpen(false);
      refetch();
      
      toast({
        title: "Profile Updated",
        description: `${data.name} has been updated successfully`
      });
    } catch (error) {
      toast({
        title: "Error",
        description: "Failed to update profile",
        variant: "destructive"
      });
    }
  };
  
  // Handle delete profile
  const handleDeleteProfile = async () => {
    if (!selectedProfile) return;
    
    try {
      await api.deleteProfile(selectedProfile.id);
      setDeleteDialogOpen(false);
      refetch();
      
      toast({
        title: "Profile Deleted",
        description: `${selectedProfile.name} has been deleted`
      });
    } catch (error) {
      toast({
        title: "Error",
        description: "Failed to delete profile",
        variant: "destructive"
      });
    }
  };
  
  // Open edit dialog
  const openEditDialog = (profile: TemperatureProfile) => {
    setSelectedProfile(profile);
    setEditDialogOpen(true);
  };
  
  // Open delete dialog
  const openDeleteDialog = (profile: TemperatureProfile) => {
    setSelectedProfile(profile);
    setDeleteDialogOpen(true);
  };
  
  // Prompt to apply a profile
  const handleApplyProfile = (profileId: string) => {
    toast({
      title: "Profile Ready",
      description: "Go to Controllers page to apply this profile",
      action: (
        <Button asChild variant="secondary" size="sm">
          <a href="/">View Controllers</a>
        </Button>
      )
    });
  };
  
  // Force restore default profiles
  const handleRestoreDefaults = async () => {
    try {
      // Get current profiles
      const currentProfiles = await api.getProfiles();
      
      // Find which default profiles aren't in the current profiles
      const missingProfiles = defaultProfiles.filter(
        defaultProfile => !currentProfiles.some(profile => profile.name === defaultProfile.name)
      );
      
      // Add missing profiles
      for (const profile of missingProfiles) {
        await api.createProfile({
          name: profile.name,
          description: profile.description,
          controlPoints: profile.controlPoints,
          duration: profile.duration
        });
      }
      
      refetch();
      
      toast({
        title: "Profiles Restored",
        description: `${missingProfiles.length} default profiles have been restored`
      });
    } catch (error) {
      toast({
        title: "Error",
        description: "Failed to restore default profiles",
        variant: "destructive"
      });
    }
  };
  
  // Loading state
  if (isLoading) {
    return (
      <div className="container py-6 space-y-6">
        <MainNav />
        <div className="flex justify-center items-center h-64">
          <div className="flex flex-col items-center gap-2">
            <div className="h-8 w-8 rounded-full border-4 border-t-primary border-r-transparent border-b-transparent border-l-transparent animate-spin" />
            <p className="text-muted-foreground">Loading profiles...</p>
          </div>
        </div>
      </div>
    );
  }
  
  return (
    <div className="container py-6 space-y-6">
      <MainNav />
      
      <div className="flex justify-between items-center">
        <h1 className="text-2xl font-bold">Temperature Profiles</h1>
        <div className="flex gap-2">
          <Button variant="outline" onClick={handleRestoreDefaults}>
            <RefreshCw className="h-4 w-4 mr-2" />
            Restore Defaults
          </Button>
          <Button onClick={() => setCreateDialogOpen(true)}>
            <Plus className="h-4 w-4 mr-2" />
            New Profile
          </Button>
        </div>
      </div>
      
      {profiles.length === 0 ? (
        <div className="flex flex-col items-center justify-center h-64 border rounded-lg bg-muted/10">
          <h3 className="text-lg font-medium">No profiles yet</h3>
          <p className="text-muted-foreground mb-4">Create your first temperature profile</p>
          <div className="flex gap-2">
            <Button variant="outline" onClick={handleRestoreDefaults}>
              <RefreshCw className="h-4 w-4 mr-2" />
              Load Default Profiles
            </Button>
            <Button onClick={() => setCreateDialogOpen(true)}>
              <Plus className="h-4 w-4 mr-2" />
              Create Profile
            </Button>
          </div>
        </div>
      ) : (
        <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-6">
          {profiles.map(profile => (
            <ProfileCard
              key={profile.id}
              profile={profile}
              minTemp={minTemp}
              maxTemp={maxTemp}
              onEdit={openEditDialog}
              onDelete={(id) => openDeleteDialog(profiles.find(p => p.id === id)!)}
              onApply={handleApplyProfile}
            />
          ))}
        </div>
      )}
      
      {/* Create Profile Dialog */}
      <Dialog open={createDialogOpen} onOpenChange={setCreateDialogOpen}>
        <DialogContent className="sm:max-w-[600px]">
          <DialogHeader>
            <DialogTitle>Create Temperature Profile</DialogTitle>
          </DialogHeader>
          <ProfileForm
            onSubmit={handleCreateProfile}
            minTemp={minTemp}
            maxTemp={maxTemp}
          />
        </DialogContent>
      </Dialog>
      
      {/* Edit Profile Dialog */}
      <Dialog open={editDialogOpen} onOpenChange={setEditDialogOpen}>
        <DialogContent className="sm:max-w-[600px]">
          <DialogHeader>
            <DialogTitle>Edit Temperature Profile</DialogTitle>
          </DialogHeader>
          {selectedProfile && (
            <ProfileForm
              onSubmit={handleEditProfile}
              initialData={selectedProfile}
              minTemp={minTemp}
              maxTemp={maxTemp}
            />
          )}
        </DialogContent>
      </Dialog>
      
      {/* Delete Confirmation Dialog */}
      <AlertDialog open={deleteDialogOpen} onOpenChange={setDeleteDialogOpen}>
        <AlertDialogContent>
          <AlertDialogHeader>
            <AlertDialogTitle>Are you sure?</AlertDialogTitle>
            <AlertDialogDescription>
              This will permanently delete the profile "{selectedProfile?.name}".
              This action cannot be undone.
            </AlertDialogDescription>
          </AlertDialogHeader>
          <AlertDialogFooter>
            <AlertDialogCancel>Cancel</AlertDialogCancel>
            <AlertDialogAction onClick={handleDeleteProfile} className="bg-destructive">
              Delete
            </AlertDialogAction>
          </AlertDialogFooter>
        </AlertDialogContent>
      </AlertDialog>
    </div>
  );
};

export default Profiles;
