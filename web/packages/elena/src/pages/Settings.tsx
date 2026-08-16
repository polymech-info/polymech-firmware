import React, { useState, useEffect } from 'react';
import MainNav from '@/components/MainNav';
import { Button } from '@/components/ui/button';
import { Input } from '@/components/ui/input';
import { Label } from '@/components/ui/label';
import { Switch } from '@/components/ui/switch';
import { Slider } from '@/components/ui/slider';
import { Card, CardContent, CardDescription, CardFooter, CardHeader, CardTitle } from '@/components/ui/card';
import { Separator } from '@/components/ui/separator';
import { Trash2, Save, RefreshCw } from 'lucide-react';
import { useToast } from '@/components/ui/use-toast';
import { useTheme } from '@/components/ThemeProvider';
import { 
  AlertDialog,
  AlertDialogAction,
  AlertDialogCancel,
  AlertDialogContent,
  AlertDialogDescription,
  AlertDialogFooter,
  AlertDialogHeader,
  AlertDialogTitle,
} from '@/components/ui/alert-dialog';
import { api } from '@/lib/api';

const Settings = () => {
  const { toast } = useToast();
  const { theme, setTheme } = useTheme();
  const [resetDialogOpen, setResetDialogOpen] = useState(false);
  const [deleteControllerId, setDeleteControllerId] = useState<string | null>(null);
  const [refreshRate, setRefreshRate] = useState(1000);
  const [darkMode, setDarkMode] = useState(theme === 'dark');
  const [temperatureUnit, setTemperatureUnit] = useState<'celsius' | 'fahrenheit'>('celsius');
  const [soundEnabled, setSoundEnabled] = useState(true);
  
  useEffect(() => {
    setTheme(darkMode ? 'dark' : 'light');
  }, [darkMode, setTheme]);
  
  const handleSaveSettings = () => {
    toast({
      title: "Settings Saved",
      description: "Your preferences have been updated"
    });
  };
  
  const handleResetData = () => {
    localStorage.clear();
    
    toast({
      title: "Data Reset",
      description: "All data has been reset to default values",
      variant: "destructive"
    });
    
    window.location.reload();
  };

  const handleDeleteController = async () => {
    if (deleteControllerId) {
      try {
        await api.deleteController(deleteControllerId);
        
        toast({
          title: "Controller Deleted",
          description: "The controller has been successfully removed",
          variant: "default"
        });
        
        setDeleteControllerId(null);
      } catch (error) {
        toast({
          title: "Error",
          description: "Failed to delete controller",
          variant: "destructive"
        });
      }
    }
  };
  
  return (
    <div className="container py-6 space-y-6">
      <MainNav />
      
      <div>
        <h1 className="text-2xl font-bold">Settings</h1>
        <p className="text-muted-foreground">Configure your temperature controller application</p>
      </div>
      
      <div className="grid gap-6">
        <Card>
          <CardHeader>
            <CardTitle>General Settings</CardTitle>
            <CardDescription>Configure application preferences</CardDescription>
          </CardHeader>
          <CardContent className="space-y-4">
            <div className="flex items-center justify-between">
              <div>
                <Label htmlFor="dark-mode">Dark Mode</Label>
                <p className="text-sm text-muted-foreground">
                  Enable dark theme for the application
                </p>
              </div>
              <Switch
                id="dark-mode"
                checked={darkMode}
                onCheckedChange={setDarkMode}
              />
            </div>
            
            <Separator />
            
            <div className="flex items-center justify-between">
              <div>
                <Label htmlFor="sound">Sound Effects</Label>
                <p className="text-sm text-muted-foreground">
                  Play sounds for alerts and notifications
                </p>
              </div>
              <Switch
                id="sound"
                checked={soundEnabled}
                onCheckedChange={setSoundEnabled}
              />
            </div>
            
            <Separator />
            
            <div className="space-y-2">
              <Label>Temperature Unit</Label>
              <div className="flex gap-4">
                <div className="flex items-center gap-2">
                  <input
                    type="radio"
                    id="celsius"
                    name="temp-unit"
                    checked={temperatureUnit === 'celsius'}
                    onChange={() => setTemperatureUnit('celsius')}
                    className="rounded-full"
                  />
                  <Label htmlFor="celsius" className="font-normal">
                    Celsius (°C)
                  </Label>
                </div>
                <div className="flex items-center gap-2">
                  <input
                    type="radio"
                    id="fahrenheit"
                    name="temp-unit"
                    checked={temperatureUnit === 'fahrenheit'}
                    onChange={() => setTemperatureUnit('fahrenheit')}
                    className="rounded-full"
                  />
                  <Label htmlFor="fahrenheit" className="font-normal">
                    Fahrenheit (°F)
                  </Label>
                </div>
              </div>
            </div>
          </CardContent>
          <CardFooter>
            <Button 
              onClick={handleSaveSettings}
              className="ml-auto gap-2"
            >
              <Save className="h-4 w-4" />
              Save Settings
            </Button>
          </CardFooter>
        </Card>
        
        <Card>
          <CardHeader>
            <CardTitle>Performance Settings</CardTitle>
            <CardDescription>Configure simulation and display settings</CardDescription>
          </CardHeader>
          <CardContent className="space-y-4">
            <div className="space-y-2">
              <div className="flex justify-between">
                <Label htmlFor="refresh-rate">Refresh Rate (ms)</Label>
                <span className="text-sm font-mono">{refreshRate} ms</span>
              </div>
              <Slider
                id="refresh-rate"
                min={100}
                max={5000}
                step={100}
                value={[refreshRate]}
                onValueChange={(value) => setRefreshRate(value[0])}
              />
              <p className="text-xs text-muted-foreground">
                Adjust how frequently temperature values are updated
              </p>
            </div>
          </CardContent>
          <CardFooter>
            <Button 
              onClick={handleSaveSettings}
              className="ml-auto gap-2"
            >
              <Save className="h-4 w-4" />
              Save Settings
            </Button>
          </CardFooter>
        </Card>
        
        <Card>
          <CardHeader>
            <CardTitle>Data Management</CardTitle>
            <CardDescription>Manage your application data</CardDescription>
          </CardHeader>
          <CardContent className="space-y-4">
            <div className="space-y-2">
              <Label>Reset All Data</Label>
              <p className="text-sm text-muted-foreground">
                This will delete all profiles and reset controllers to default values
              </p>
            </div>
            
            <Button 
              variant="destructive" 
              onClick={() => setResetDialogOpen(true)}
              className="gap-2"
            >
              <Trash2 className="h-4 w-4" />
              Reset All Data
            </Button>
            
            <Separator />
            
            <div className="space-y-2">
              <Label>Export/Import Data</Label>
              <p className="text-sm text-muted-foreground">
                Transfer your profiles between devices
              </p>
              
              <div className="flex gap-2">
                <Button variant="outline" className="gap-2">
                  Export Data
                </Button>
                <Button variant="outline" className="gap-2">
                  Import Data
                </Button>
              </div>
            </div>
          </CardContent>
        </Card>
        
        <Card>
          <CardHeader>
            <CardTitle>About</CardTitle>
            <CardDescription>Temperature Controller Information</CardDescription>
          </CardHeader>
          <CardContent className="space-y-4">
            <div>
              <h3 className="font-medium">Temperature Controller</h3>
              <p className="text-sm text-muted-foreground">
                Version 1.0.0
              </p>
            </div>
            
            <div>
              <h3 className="font-medium">Local Storage</h3>
              <p className="text-sm text-muted-foreground">
                All data is currently stored in your browser's local storage.
              </p>
            </div>
            
            <Button variant="outline" className="gap-2 w-full">
              <RefreshCw className="h-4 w-4" />
              Check for Updates
            </Button>
          </CardContent>
        </Card>
      </div>
      
      <AlertDialog open={resetDialogOpen} onOpenChange={setResetDialogOpen}>
        <AlertDialogContent>
          <AlertDialogHeader>
            <AlertDialogTitle>Are you absolutely sure?</AlertDialogTitle>
            <AlertDialogDescription>
              This will permanently delete all your profiles and reset all controller settings
              to their default values. This action cannot be undone.
            </AlertDialogDescription>
          </AlertDialogHeader>
          <AlertDialogFooter>
            <AlertDialogCancel>Cancel</AlertDialogCancel>
            <AlertDialogAction 
              onClick={handleResetData}
              className="bg-destructive text-destructive-foreground"
            >
              Reset Everything
            </AlertDialogAction>
          </AlertDialogFooter>
        </AlertDialogContent>
      </AlertDialog>

      <AlertDialog open={!!deleteControllerId} onOpenChange={(open) => !open && setDeleteControllerId(null)}>
        <AlertDialogContent>
          <AlertDialogHeader>
            <AlertDialogTitle>Delete Controller?</AlertDialogTitle>
            <AlertDialogDescription>
              This will permanently delete this controller. This action cannot be undone.
            </AlertDialogDescription>
          </AlertDialogHeader>
          <AlertDialogFooter>
            <AlertDialogCancel>Cancel</AlertDialogCancel>
            <AlertDialogAction 
              onClick={handleDeleteController}
              className="bg-destructive text-destructive-foreground"
            >
              Delete Controller
            </AlertDialogAction>
          </AlertDialogFooter>
        </AlertDialogContent>
      </AlertDialog>
    </div>
  );
};

export default Settings;
