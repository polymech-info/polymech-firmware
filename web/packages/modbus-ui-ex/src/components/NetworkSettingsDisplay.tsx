import React, { useState, useEffect, FormEvent } from 'react';
import modbusApiService, { NetworkSettingsResponse, NetworkSettingsUpdatePayload } from '@polymech/client-ts/modbusApiService';
import { Input } from '@/components/ui/input';
import { Button } from '@/components/ui/button';
import { Label } from '@/components/ui/label';
import { toast } from 'sonner';
import { T } from '../i18n';

const initialSettingsState: NetworkSettingsUpdatePayload = {
  sta_ssid: '',
  sta_password: '', 
  sta_local_ip: '',
  sta_gateway: '',
  sta_subnet: '',
  sta_primary_dns: '',
  sta_secondary_dns: '',
  ap_ssid: '',
  ap_password: '', 
  ap_config_ip: '',
  ap_config_gateway: '',
  ap_config_subnet: '',
};

const NetworkSettingsDisplay = () => {
  const [settings, setSettings] = useState<NetworkSettingsUpdatePayload>(initialSettingsState);
  const [loading, setLoading] = useState<boolean>(true);
  const [error, setError] = useState<string | null>(null);
  const [isSaving, setIsSaving] = useState<boolean>(false);

  useEffect(() => {
    const fetchSettings = async () => {
      try {
        setLoading(true);
        setError(null);
        const currentSettings = await modbusApiService.getNetworkSettings();
        // Passwords are not typically sent from GET, so initialize them as empty or handle appropriately
        setSettings({ 
            ...initialSettingsState, // Ensures all fields are present, especially passwords
            ...currentSettings 
        });
      } catch (err) {
        console.error("Failed to fetch network settings:", err);
        const errorMessage = err instanceof Error ? err.message : 'An unknown error occurred';
        setError(errorMessage);
        toast.error(<T>Failed to load network settings: {errorMessage}</T>);
      } finally {
        setLoading(false);
      }
    };

    fetchSettings();
  }, []);

  const handleChange = (e: React.ChangeEvent<HTMLInputElement>) => {
    const { name, value } = e.target;
    setSettings(prevSettings => ({
      ...prevSettings,
      [name]: value,
    }));
  };

  const handleSubmit = async (e: FormEvent<HTMLFormElement>) => {
    e.preventDefault();
    setIsSaving(true);
    setError(null);
    try {
      // Filter out empty password fields before sending if they are not meant to be updated
      const payload: NetworkSettingsUpdatePayload = { ...settings };
      if (payload.sta_password === '') delete payload.sta_password;
      if (payload.ap_password === '') delete payload.ap_password;

      await modbusApiService.setNetworkSettings(payload);
      toast.success(<T>Network settings updated successfully!</T>);
      // Optionally, re-fetch settings to confirm update or rely on optimistic update
      // For now, we clear password fields after successful save for security
      setSettings(prev => ({...prev, sta_password: '', ap_password: ''}));
    } catch (err) {
      console.error("Failed to save network settings:", err);
      const errorMessage = err instanceof Error ? err.message : 'An unknown error occurred while saving';
      setError(errorMessage);
      toast.error(<T>Failed to save network settings: {errorMessage}</T>);
    } finally {
      setIsSaving(false);
    }
  };

  if (loading) {
    return <p><T>Loading network settings...</T></p>;
  }

  return (
    <form onSubmit={handleSubmit} className="p-4 border rounded-md shadow-sm bg-card text-card-foreground space-y-6">
      <h2 className="text-xl font-semibold mb-4"><T>Network Settings</T></h2>
      
      {error && <p className="text-red-500 bg-red-100 p-3 rounded-md"><T>Error</T>: {error}</p>}

      {/* STA Settings */} 
      <div className="space-y-4 p-4 border rounded-md">
        <h3 className="text-lg font-medium"><T>Station (STA) Mode</T></h3>
        <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
          <div>
            <Label htmlFor="sta_ssid"><T>STA SSID</T></Label>
            <Input id="sta_ssid" name="sta_ssid" value={settings.sta_ssid || ''} onChange={handleChange} />
          </div>
          <div>
            <Label htmlFor="sta_password"><T>STA Password (leave blank to keep unchanged)</T></Label>
            <Input id="sta_password" name="sta_password" type="password" value={settings.sta_password || ''} onChange={handleChange} placeholder="Leave blank if no change"/>
          </div>
          <div>
            <Label htmlFor="sta_local_ip"><T>STA IP Address</T></Label>
            <Input id="sta_local_ip" name="sta_local_ip" value={settings.sta_local_ip || ''} onChange={handleChange} />
          </div>
          <div>
            <Label htmlFor="sta_gateway"><T>STA Gateway</T></Label>
            <Input id="sta_gateway" name="sta_gateway" value={settings.sta_gateway || ''} onChange={handleChange} />
          </div>
          <div>
            <Label htmlFor="sta_subnet"><T>STA Subnet Mask</T></Label>
            <Input id="sta_subnet" name="sta_subnet" value={settings.sta_subnet || ''} onChange={handleChange} />
          </div>
          <div>
            <Label htmlFor="sta_primary_dns"><T>STA Primary DNS</T></Label>
            <Input id="sta_primary_dns" name="sta_primary_dns" value={settings.sta_primary_dns || ''} onChange={handleChange} />
          </div>
          <div>
            <Label htmlFor="sta_secondary_dns"><T>STA Secondary DNS</T></Label>
            <Input id="sta_secondary_dns" name="sta_secondary_dns" value={settings.sta_secondary_dns || ''} onChange={handleChange} />
          </div>
        </div>
      </div>

      {/* AP Settings - Conditionally render if ENABLE_AP_STA is expected or always show */} 
      <div className="space-y-4 p-4 border rounded-md">
        <h3 className="text-lg font-medium"><T>Access Point (AP) Mode</T></h3>
         <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
          <div>
            <Label htmlFor="ap_ssid"><T>AP SSID</T></Label>
            <Input id="ap_ssid" name="ap_ssid" value={settings.ap_ssid || ''} onChange={handleChange} />
          </div>
          <div>
            <Label htmlFor="ap_password"><T>AP Password (leave blank to keep unchanged)</T></Label>
            <Input id="ap_password" name="ap_password" type="password" value={settings.ap_password || ''} onChange={handleChange} placeholder="Leave blank if no change" />
          </div>
          <div>
            <Label htmlFor="ap_config_ip"><T>AP IP Address</T></Label>
            <Input id="ap_config_ip" name="ap_config_ip" value={settings.ap_config_ip || ''} onChange={handleChange} />
          </div>
          <div>
            <Label htmlFor="ap_config_gateway"><T>AP Gateway</T></Label>
            <Input id="ap_config_gateway" name="ap_config_gateway" value={settings.ap_config_gateway || ''} onChange={handleChange} />
          </div>
          <div>
            <Label htmlFor="ap_config_subnet"><T>AP Subnet Mask</T></Label>
            <Input id="ap_config_subnet" name="ap_config_subnet" value={settings.ap_config_subnet || ''} onChange={handleChange} />
          </div>
        </div>
      </div>

      <div className="flex justify-end">
        <Button type="submit" disabled={isSaving || loading}>
          {isSaving ? <T>Saving...</T> : <T>Save Network Settings</T>}
        </Button>
      </div>
    </form>
  );
};

export default NetworkSettingsDisplay; 