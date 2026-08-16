import React, { useState, useEffect, FormEvent } from 'react';
import modbusApiService, { NetworkSettingsUpdatePayload } from '@polymech/client-ts/modbusApiService';
import { Input } from '@/components/ui/input';
import { Button } from '@/components/ui/button';
import { Label } from '@/components/ui/label';
import { toast } from 'sonner';
import { T } from '../i18n';
import { Card, CardContent, CardDescription, CardFooter, CardHeader, CardTitle } from '@/components/ui/card';
import { HelpCircle } from 'lucide-react';

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
  hostname: '',
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
        setSettings({
          ...initialSettingsState,
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

  const handleSave = async (payload: Partial<NetworkSettingsUpdatePayload>) => {
    setIsSaving(true);
    setError(null);
    try {
      await modbusApiService.setNetworkSettings(payload);
      toast.success(<T>Network settings updated successfully!</T>);
      setSettings(prev => ({ ...prev, sta_password: '', ap_password: '' }));
    } catch (err) {
      console.error("Failed to save network settings:", err);
      const errorMessage = err instanceof Error ? err.message : 'An unknown error occurred while saving';
      setError(errorMessage);
      toast.error(<T>Failed to save network settings: {errorMessage}</T>);
    } finally {
      setIsSaving(false);
    }
  };

  const handleStaSubmit = (e: FormEvent<HTMLFormElement>) => {
    e.preventDefault();
    const payload: Partial<NetworkSettingsUpdatePayload> = {
      sta_ssid: settings.sta_ssid,
      sta_local_ip: settings.sta_local_ip,
      sta_gateway: settings.sta_gateway,
      sta_subnet: settings.sta_subnet,
      sta_primary_dns: settings.sta_primary_dns,
      sta_secondary_dns: settings.sta_secondary_dns,
      hostname: settings.hostname,
    };
    if (settings.sta_password) {
      payload.sta_password = settings.sta_password;
    }
    handleSave(payload);
  };

  const handleApSubmit = (e: FormEvent<HTMLFormElement>) => {
    e.preventDefault();
    const payload: Partial<NetworkSettingsUpdatePayload> = {
      ap_ssid: settings.ap_ssid,
      ap_config_ip: settings.ap_config_ip,
      ap_config_gateway: settings.ap_config_gateway,
      ap_config_subnet: settings.ap_config_subnet,
      hostname: settings.hostname,
    };
    if (settings.ap_password) {
      payload.ap_password = settings.ap_password;
    }
    handleSave(payload);
  };

  if (loading) {
    return <p><T>Loading network settings...</T></p>;
  }

  return (
    <div id="network-settings-display" className="space-y-6">
      <div className="flex justify-between items-center">
        <h2 className="text-xl font-semibold"><T>Network Settings</T></h2>
        <Button asChild size="sm" variant="ghost" className="glass-button">
          <a href="https://polymech.info/en/resources/cassandra/network" target="_blank" rel="noopener noreferrer">
            <HelpCircle className="h-4 w-4 mr-2" />
            <T>Help</T>
          </a>
        </Button>
      </div>

      {error && <p className="text-red-500 bg-red-100 p-3 rounded-md"><T>Error</T>: {error}</p>}

      <Card>
        <CardHeader>
          <CardTitle><T>Hostname</T></CardTitle>
        </CardHeader>
        <CardContent>
          <Label htmlFor="hostname"><T>Device Hostname</T></Label>
          <Input id="hostname" name="hostname" value={settings.hostname || ''} onChange={handleChange} />
          <CardDescription className="pt-2">
            <T>This hostname is used for both STA and AP modes. Changes here will be saved with either form.</T>
          </CardDescription>
        </CardContent>
      </Card>

      <form onSubmit={handleStaSubmit}>
        <Card>
          <CardHeader>
            <CardTitle><T>Station (STA) Mode</T></CardTitle>
            <CardDescription><T>Connects to an existing Wi-Fi network.</T></CardDescription>
          </CardHeader>
          <CardContent className="space-y-4">
            <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
              <div>
                <Label htmlFor="sta_ssid"><T>STA SSID</T></Label>
                <Input id="sta_ssid" name="sta_ssid" value={settings.sta_ssid || ''} onChange={handleChange} autoComplete="username" />
              </div>
              <div>
                <Label htmlFor="sta_password"><T>STA Password</T></Label>
                <Input id="sta_password" name="sta_password" type="password" value={settings.sta_password || ''} onChange={handleChange} placeholder="Leave blank if no change" autoComplete="current-password" />
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
          </CardContent>
          <CardFooter>
            <Button type="submit" disabled={isSaving || loading}>
              {isSaving ? <T>Saving...</T> : <T>Save STA Settings</T>}
            </Button>
          </CardFooter>
        </Card>
      </form>

      <form onSubmit={handleApSubmit}>
        <Card>
          <CardHeader>
            <CardTitle><T>Access Point (AP) Mode</T></CardTitle>
            <CardDescription><T>Creates its own Wi-Fi network.</T></CardDescription>
          </CardHeader>
          <CardContent className="space-y-4">
            <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
              <div>
                <Label htmlFor="ap_ssid"><T>AP SSID</T></Label>
                <Input id="ap_ssid" name="ap_ssid" value={settings.ap_ssid || ''} onChange={handleChange} autoComplete="username" />
              </div>
              <div>
                <Label htmlFor="ap_password"><T>AP Password</T></Label>
                <Input id="ap_password" name="ap_password" type="password" value={settings.ap_password || ''} onChange={handleChange} placeholder="Leave blank if no change" autoComplete="new-password" />
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
          </CardContent>
          <CardFooter>
            <Button type="submit" disabled={isSaving || loading}>
              {isSaving ? <T>Saving...</T> : <T>Save AP Settings</T>}
            </Button>
          </CardFooter>
        </Card>
      </form>
    </div>
  );
};

export default NetworkSettingsDisplay; 