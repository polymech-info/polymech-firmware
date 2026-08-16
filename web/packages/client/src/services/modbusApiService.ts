export type {
  SystemInfo,
  CoilResponse,
  CoilUpdateResponse,
  CoilsArrayResponse,
  RegisterResponse,
  RegistersArrayResponse,
  RegisterUpdateResponse,
  NetworkSettingsResponse,
  NetworkSettingsUpdatePayload,
  PlungerSettingsResponse,
  PlungerSettingsUpdatePayload,
  ProfilesResponse,
  ProfileSavePayload,
  PressureProfilesResponse,
  PressureProfileSavePayload,
  RelayTestResponse,
  SignalPlotData,

  RegisteredMethod,
  SerialCommandPayload,
  SerialCommandResponse,
  Settings,
  ComponentInfo,
  ComponentFlags,
  FileReadResponse,
  FileWriteRequest,
  FileWriteResponse
};

import {
  SystemInfo,
  CoilResponse,
  CoilUpdateResponse,
  CoilsArrayResponse,
  RegisterResponse,
  RegistersArrayResponse,
  RegisterUpdateResponse,
  NetworkSettingsResponse,
  NetworkSettingsUpdatePayload,
  PlungerSettingsResponse,
  PlungerSettingsUpdatePayload,
  ProfilesResponse,
  ProfileSavePayload,
  PressureProfilesResponse,
  PressureProfileSavePayload,
  RelayTestResponse,
  SignalPlotData,

  RegisteredMethod,
  SerialCommandPayload,
  SerialCommandResponse,
  Settings,
  ComponentInfo,
  ComponentFlags,
  Profile,
  PressureProfile,
  FileReadResponse,
  FileWriteRequest,
  FileWriteResponse
} from '../types';

export interface FeatureFlagsResponse {
  features: {
    [key: string]: boolean;
  };
  settings?: {
    [key: string]: any;
  };
}

class ModbusApiService {
  private baseUrl: string = 'http://192.168.1.250/api';
  private isConnected: boolean = false;
  private _cachedSignalPlots: SignalPlotData[] | null = null;
  private _fetchPlotsPromise: Promise<SignalPlotData[]> | null = null;
  private _cachedProfiles: Profile[] | null = null;
  private _fetchProfilesPromise: Promise<Profile[]> | null = null;
  private _cachedPressureProfiles: PressureProfile[] | null = null;
  private _fetchPressureProfilesPromise: Promise<PressureProfile[]> | null = null;
  private _cachedFeatureFlags: FeatureFlagsResponse | null = null;
  private _fetchFeatureFlagsPromise: Promise<FeatureFlagsResponse | null> | null = null;

  constructor() {
    // Initialize with default values
  }

  // Set the base URL for the API
  setBaseUrl(url: string): void {
    // Ensure URL ends with /api
    url = url.endsWith('/') ? url.slice(0, -1) : url;
    if (!url.endsWith('/api')) {
      url = `${url}/api`;
    }
    this.baseUrl = url;
  }

  getBaseUrl(): string {
    return this.baseUrl;
  }

  // Test connection to the API
  async testConnection(): Promise<boolean> {
    try {
      const response = await fetch(`${this.baseUrl}/v1/system/info`);

      if (!response.ok) {
        throw new Error(`HTTP error ${response.status}: ${response.statusText}`);
      }
      this.isConnected = true;
      return true;
    } catch (error) {
      console.error('API connection test failed:', error);
      this.isConnected = false;
      return false;
    }
  }

  // Get system information
  async getSystemInfo(): Promise<SystemInfo> {
    const response = await fetch(`${this.baseUrl}/v1/system/info`);
    if (!response.ok) {
      throw new Error(`Failed to get system info: ${response.statusText}`);
    }

    try {
      return await response.json();
    } catch (jsonError) {
      throw new Error(`Invalid JSON response from system info endpoint: ${jsonError}`);
    }
  }

  // Get feature flags
  async getFeatureFlags(forceRefresh: boolean = false): Promise<FeatureFlagsResponse | null> {
    if (this._cachedFeatureFlags && !forceRefresh) {
      return this._cachedFeatureFlags;
    }

    if (this._fetchFeatureFlagsPromise) {
      return this._fetchFeatureFlagsPromise;
    }

    this._fetchFeatureFlagsPromise = (async () => {
      try {
        const response = await fetch(`${this.baseUrl}/v1/features`);
        if (!response.ok) {
          // If endpoint doesn't exist (404) or another error, return null
          return null;
        }
        try {
          const data = await response.json();
          this._cachedFeatureFlags = data;
          return data;
        } catch (jsonError) {
          console.error('Invalid JSON response from feature flags endpoint:', jsonError);
          return null;
        }
      } catch (error) {
        console.error('Failed to fetch feature flags:', error);
        return null;
      } finally {
        this._fetchFeatureFlagsPromise = null;
      }
    })();

    return this._fetchFeatureFlagsPromise;
  }

  // Get components
  async getComponents(): Promise<ComponentInfo[]> {
    const response = await fetch(`${this.baseUrl}/v1/components`);
    if (!response.ok) {
      throw new Error(`Failed to get components: ${response.statusText}`);
    }
    try {
      return await response.json();
    } catch (jsonError) {
      throw new Error(`Invalid JSON response from components endpoint: ${jsonError}`);
    }
  }

  // Get multiple coils
  async getCoils(start: number = 0, count: number = 500): Promise<CoilResponse[]> {
    let allCoils: CoilResponse[] = [];
    let currentPage = 0;
    let totalPages = 1;
    const pageSize = 20; // Sensible default page size
    while (currentPage < totalPages) {
      const response = await fetch(`${this.baseUrl}/v1/coils?page=${currentPage}&pageSize=${pageSize}`);
      if (!response.ok) {
        throw new Error(`Failed to get coils (page ${currentPage}): ${response.statusText}`);
      }
      let data: CoilsArrayResponse;
      try {
        data = await response.json();
      } catch (jsonError) {
        throw new Error(`Invalid JSON response from coils endpoint (page ${currentPage}): ${jsonError}`);
      }

      if (data.coils) {
        allCoils = allCoils.concat(data.coils);
      }
      // Assuming meta object is present in the response
      if (data.meta) {
        totalPages = data.meta.totalPages;
      } else {
        // If no meta, assume single page and break
        break;
      }
      currentPage++;
    }
    return allCoils;
  }

  // Get a specific coil
  async getCoil(address: number): Promise<boolean> {
    const response = await fetch(`${this.baseUrl}/v1/coils?address=${address}`);

    if (!response.ok) {
      throw new Error(`Failed to get coil at address ${address}: ${response.statusText}`);
    }

    const data: CoilResponse = await response.json();
    return data.value;
  }

  // Set a coil value
  async setCoil(address: number, value: boolean): Promise<CoilUpdateResponse> {
    // According to new Swagger spec, value is now a query parameter, not in the request body
    const response = await fetch(`${this.baseUrl}/v1/coils/${address}?value=${value ? 1 : 0}`, {
      method: 'POST',
    });

    if (!response.ok) {
      throw new Error(`Failed to set coil at address ${address}: ${response.statusText}`);
    }

    return await response.json();
  }

  // Get multiple registers
  async getRegisters(start: number = 0, count: number = 1000): Promise<RegisterResponse[]> {
    let allRegisters: RegisterResponse[] = [];
    let currentPage = 0;
    let totalPages = 1;
    const pageSize = 10;

    while (currentPage < totalPages) {
      const response = await fetch(`${this.baseUrl}/v1/registers?page=${currentPage}&pageSize=${pageSize}`);
      if (!response.ok) {
        throw new Error(`Failed to get registers (page ${currentPage}): ${response.statusText}`);
      }
      let data: RegistersArrayResponse;
      try {
        data = await response.json();
      } catch (jsonError) {
        throw new Error(`Invalid JSON response from registers endpoint (page ${currentPage}): ${jsonError}`);
      }
      if (data.registers) {
        allRegisters = allRegisters.concat(data.registers);
      }
      if (data.meta) {
        totalPages = data.meta.totalPages;
      } else {
        break; // No metadata, assume single page
      }
      currentPage++;
      await new Promise(resolve => setTimeout(resolve, 30));
    }

    const seenAddresses = new Map<number, number>();
    allRegisters.forEach(reg => {
      seenAddresses.set(reg.address, (seenAddresses.get(reg.address) || 0) + 1);
    });

    const duplicateAddresses = Array.from(seenAddresses.entries())
      .filter(([, count]) => count > 1)
      .map(([address]) => address);

    if (duplicateAddresses.length > 0) {
      console.error('[modbusApiService] Duplicate register addresses found after fetching all pages!');
      duplicateAddresses.forEach(address => {
        const duplicateRegs = allRegisters.filter(reg => reg.address === address);
        console.error(`[modbusApiService] Address ${address} is duplicated ${duplicateRegs.length} times:`, duplicateRegs);
      });
    }
    return allRegisters;
  }

  // Get a specific register
  async getRegister(address: number): Promise<number> {
    const response = await fetch(`${this.baseUrl}/v1/registers?address=${address}`);

    if (!response.ok) {
      throw new Error(`Failed to get register at address ${address}: ${response.statusText}`);
    }

    const data: RegisterResponse = await response.json();
    return data.value;
  }

  // Get system logs
  async getLogs(): Promise<string[]> {
    const response = await fetch(`${this.baseUrl}/v1/system/logs`);

    if (!response.ok) {
      throw new Error(`Failed to get system logs: ${response.statusText}`);
    }

    const data = await response.json();
    // Assuming the API returns an array of strings directly
    return Array.isArray(data) ? data : [];
  }

  // Set a register value
  async setRegister(address: number, value: number): Promise<RegisterUpdateResponse> {
    // According to new Swagger spec, value is now a query parameter, not in the request body
    const response = await fetch(`${this.baseUrl}/v1/registers/${address}?value=${value}`, {
      method: 'POST',
    });

    if (!response.ok) {
      throw new Error(`Failed to set register at address ${address}: ${response.statusText}`);
    }

    return await response.json();
  }
  getConnectionStatus(): boolean {
    return this.isConnected;
  }
  // Get Network Settings
  async getNetworkSettings(): Promise<NetworkSettingsResponse> {
    const response = await fetch(`${this.baseUrl}/v1/network/settings`);
    if (!response.ok) {
      throw new Error(`Failed to get network settings: ${response.statusText}`);
    }
    return await response.json();
  }
  // Set Network Settings
  async setNetworkSettings(settings: NetworkSettingsUpdatePayload): Promise<any> { // Assuming the response might vary or be minimal
    const response = await fetch(`${this.baseUrl}/v1/network/settings`, {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
      },
      body: JSON.stringify(settings),
    });

    if (!response.ok) {
      // Try to parse error response if available
      let errorBody = '';
      try {
        errorBody = await response.text();
      } catch (e) {
        // Ignore if error body cannot be read
      }
      throw new Error(`Failed to set network settings: ${response.statusText} ${errorBody ? `- ${errorBody}` : ''}`);
    }
    // Assuming a successful POST might return a confirmation or the updated settings
    // For now, returning the parsed JSON if available, or a simple success indicator
    try {
      return await response.json();
    } catch (e) {
      return { success: true, message: 'Network settings updated.' };
    }
  }

  // Get Plunger Settings
  async getPlungerSettings(): Promise<PlungerSettingsResponse> {
    const response = await fetch(`${this.baseUrl}/v1/plunger/settings`);
    if (!response.ok) {
      throw new Error(`Failed to get plunger settings: ${response.statusText}`);
    }
    return await response.json();
  }

  // Set Plunger Settings
  async setPlungerSettings(settings: PlungerSettingsUpdatePayload): Promise<any> {
    const response = await fetch(`${this.baseUrl}/v1/plunger/settings`, {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
      },
      body: JSON.stringify(settings),
    });

    if (!response.ok) {
      let errorBody = '';
      try {
        errorBody = await response.text();
      } catch (e) { /* Ignore */ }
      throw new Error(`Failed to set plunger settings: ${response.statusText} ${errorBody ? `- ${errorBody}` : ''}`);
    }
    try {
      return await response.json();
    } catch (e) {
      return { success: true, message: 'Plunger settings updated.' };
    }
  }

  // Load Plunger Defaults
  async loadPlungerDefaults(): Promise<any> { // Assuming a minimal success/fail response
    const response = await fetch(`${this.baseUrl}/v1/plunger/settings/load-defaults`, {
      method: 'POST', // Assuming POST, could be PUT
      headers: {
        'Content-Type': 'application/json', // May not be needed if no body is sent
      },
      body: JSON.stringify({}), // Send empty body if required by server, otherwise omit
    });

    if (!response.ok) {
      let errorBody = '';
      try {
        errorBody = await response.text();
      } catch (e) { /* Ignore */ }
      throw new Error(`Failed to load plunger defaults: ${response.statusText} ${errorBody ? `- ${errorBody}` : ''}`);
    }
    // Check if server sends back a meaningful JSON response or just success
    try {
      return await response.json();
    } catch (e) {
      return { success: true, message: 'Plunger defaults loaded successfully.' };
    }
  }

  // Get Settings
  async getSettings(): Promise<Settings> {
    const response = await fetch(`${this.baseUrl}/v1/settings`);
    if (!response.ok) {
      throw new Error(`Failed to get settings: ${response.statusText}`);
    }
    return await response.json();
  }

  // Set Settings
  async setSettings(settings: Settings): Promise<any> { // Assuming the response might vary or be minimal
    const response = await fetch(`${this.baseUrl}/v1/settings`, {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
      },
      body: JSON.stringify(settings),
    });

    if (!response.ok) {
      // Try to parse error response if available
      let errorBody = '';
      try {
        errorBody = await response.text();
      } catch (e) {
        // Ignore if error body cannot be read
      }
      throw new Error(`Failed to set settings: ${response.statusText} ${errorBody ? `- ${errorBody}` : ''}`);
    }
    // Assuming a successful POST might return a confirmation or the updated settings
    // For now, returning the parsed JSON if available, or a simple success indicator
    try {
      return await response.json();
    } catch (e) {
      return { success: true, message: 'Settings updated.' };
    }
  }

  async getProfiles(forceRefresh: boolean = false): Promise<Profile[]> {
    if (this._cachedProfiles && !forceRefresh) {
      return this._cachedProfiles;
    }

    if (this._fetchProfilesPromise) {
      return this._fetchProfilesPromise;
    }

    this._fetchProfilesPromise = (async () => {
      try {
        const response = await fetch(`${this.baseUrl}/v1/profiles`);
        if (!response.ok) {
          throw new Error(`Failed to get profiles: ${response.statusText}`);
        }
        const data = await response.json() as Profile[];
        this._cachedProfiles = data;
        return data;
      } finally {
        this._fetchProfilesPromise = null;
      }
    })();

    return this._fetchProfilesPromise;
  }
  async saveProfile(profileData: ProfileSavePayload): Promise<any> { // Adjust 'any' to a more specific server response type
    const response = await fetch(`${this.baseUrl}/v1/profiles`, {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
      },
      body: JSON.stringify(profileData),
    });

    if (!response.ok) {
      let errorBody = '';
      try {
        errorBody = await response.text();
      } catch (e) { /* Ignore */ }
      throw new Error(`Failed to save profile: ${response.statusText} ${errorBody ? `- ${errorBody}` : ''}`);
    }

    // Invalidate cache on save
    this._cachedProfiles = null;

    try {
      return await response.json(); // Or handle non-JSON success response
    } catch (e) {
      return { success: true, message: `Profile '${profileData.name}' operation successful.` };
    }
  }

  async uploadProfiles(profilesData: ProfileSavePayload[]): Promise<any> {
    const response = await fetch(`${this.baseUrl}/v1/profiles`, {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
      },
      body: JSON.stringify(profilesData),
    });

    if (!response.ok) {
      let errorBody = '';
      try {
        errorBody = await response.text();
      } catch (e) { /* Ignore */ }
      throw new Error(`Failed to upload profiles: ${response.statusText} ${errorBody ? `- ${errorBody}` : ''}`);
    }

    // Invalidate cache on upload
    this._cachedProfiles = null;

    try {
      return await response.json();
    } catch (e) {
      return { success: true, message: 'Profiles uploaded successfully.' };
    }
  }

  async getPressureProfiles(forceRefresh: boolean = false): Promise<PressureProfile[]> {
    if (this._cachedPressureProfiles && !forceRefresh) {
      return this._cachedPressureProfiles;
    }

    if (this._fetchPressureProfilesPromise) {
      return this._fetchPressureProfilesPromise;
    }

    this._fetchPressureProfilesPromise = (async () => {
      try {
        const response = await fetch(`${this.baseUrl}/v1/pressure-profiles`);
        if (!response.ok) {
          throw new Error(`Failed to get pressure profiles: ${response.statusText}`);
        }
        const data = await response.json() as PressureProfile[];
        this._cachedPressureProfiles = data;
        return data;
      } finally {
        this._fetchPressureProfilesPromise = null;
      }
    })();

    return this._fetchPressureProfilesPromise;
  }
  async savePressureProfile(profileData: PressureProfileSavePayload): Promise<any> {
    const response = await fetch(`${this.baseUrl}/v1/pressure-profiles`, {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
      },
      body: JSON.stringify(profileData),
    });

    if (!response.ok) {
      let errorBody = '';
      try {
        errorBody = await response.text();
      } catch (e) { /* Ignore */ }
      throw new Error(`Failed to save pressure profile: ${response.statusText} ${errorBody ? `- ${errorBody}` : ''}`);
    }

    // Invalidate cache on save
    this._cachedPressureProfiles = null;

    try {
      return await response.json();
    } catch (e) {
      return { success: true, message: `Pressure profile '${profileData.name}' operation successful.` };
    }
  }

  // Get all signal plots
  async getSignalPlots(forceRefresh: boolean = false): Promise<SignalPlotData[]> {
    if (this._cachedSignalPlots && !forceRefresh) {
      return this._cachedSignalPlots;
    }

    // Return existing promise if a request is already in flight
    if (this._fetchPlotsPromise) {
      return this._fetchPlotsPromise;
    }

    this._fetchPlotsPromise = (async () => {
      try {
        const response = await fetch(`${this.baseUrl}/v1/signalplots`);
        if (!response.ok) {
          throw new Error(`Failed to get signal plots: ${response.statusText}`);
        }
        // The API now returns an array of SignalPlotData directly
        const data = await response.json();
        this._cachedSignalPlots = data;
        return data;
      } finally {
        // Clear the promise so future calls can fetch again if needed (e.g. forceRefresh)
        this._fetchPlotsPromise = null;
      }
    })();

    return this._fetchPlotsPromise;
  }

  // Save or update a signal plot
  async saveSignalPlot(signalPlotData: SignalPlotData): Promise<any> { // Adjust 'any' to a more specific server response type
    const response = await fetch(`${this.baseUrl}/v1/signalplots`, { // Assuming POST for create, PUT for update might be /v1/signalplots/{slot}
      method: 'POST', // Or PUT if the slot exists and you are updating
      headers: {
        'Content-Type': 'application/json',
      },
      body: JSON.stringify(signalPlotData),
    });

    if (!response.ok) {
      let errorBody = '';
      try {
        errorBody = await response.text();
      } catch (e) { /* Ignore */ }
      throw new Error(`Failed to save signal plot: ${response.statusText} ${errorBody ? `- ${errorBody}` : ''}`);
    }

    // Invalidate or update cache
    if (this._cachedSignalPlots) {
      const savedPlot = signalPlotData; // Simplification, ideally we use the response if it contains the full object
      const index = this._cachedSignalPlots.findIndex(p => p.slot === savedPlot.slot);
      if (index >= 0) {
        this._cachedSignalPlots[index] = savedPlot;
      } else {
        this._cachedSignalPlots.push(savedPlot);
      }
    }

    try {
      return await response.json();
    } catch (e) {
      return { success: true, message: `Signal plot '${signalPlotData.name}' operation successful.` };
    }
  }

  async deleteSignalPlot(slot: number): Promise<any> { // Adjust 'any' to a more specific server response type
    const response = await fetch(`${this.baseUrl}/v1/signalplots/${slot}`, {
      method: 'DELETE',
    });

    if (!response.ok) {
      let errorBody = '';
      try {
        errorBody = await response.text();
      } catch (e) { /* Ignore */ }
      throw new Error(`Failed to delete signal plot at slot ${slot}: ${response.statusText} ${errorBody ? `- ${errorBody}` : ''}`);
    }

    // Update cache
    if (this._cachedSignalPlots) {
      this._cachedSignalPlots = this._cachedSignalPlots.filter(p => p.slot !== slot);
    }

    return { success: true, message: `Signal plot at slot ${slot} deleted successfully.` };
  }

  // Get registered methods
  async getRegisteredMethods(): Promise<RegisteredMethod[]> {
    const response = await fetch(`${this.baseUrl}/v1/methods`);
    if (!response.ok) {
      throw new Error(`Failed to get registered methods: ${response.statusText}`);
    }
    return await response.json();
  }

  /**
   * Sends a serial command to the device using the /v1/methods endpoint.
   * The command follows the format: <<component_id;call_type;flags;method:arg1:arg2>>
   * 
   * @param payload The command payload containing component ID, call type, flags, method and arguments
   * @returns Promise<SerialCommandResponse> The response from the command execution
   * 
   * @example
   * // Call reset method on component 1 (PHApp)
   * await modbusApiService.sendSerialCommand({
   *   componentId: 1,
   *   callType: ECalls.METHOD,
   *   flags: EMessageFlags.RECEIPT,
   *   method: 'reset',
   *   arg1: 0,
   *   arg2: 0
   * });
   */
  async sendSerialCommand(payload: SerialCommandPayload): Promise<SerialCommandResponse> {
    // Construct the command string according to the format
    const command = `<<${payload.componentId};${payload.callType};${payload.flags};${payload.method}:${payload.arg1 ?? 0}:${payload.arg2 ?? 0}>>`;

    const response = await fetch(`${this.baseUrl}/v1/methods`, {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
      },
      body: JSON.stringify({ command }),
    });

    if (!response.ok) {
      let errorBody = '';
      try {
        errorBody = await response.text();
      } catch (e) { /* Ignore */ }
      throw new Error(`Failed to send serial command: ${response.statusText} ${errorBody ? `- ${errorBody}` : ''}`);
    }

    try {
      return await response.json();
    } catch (e) {
      return {
        success: true,
        message: `Command '${command}' sent successfully.`
      };
    }
  }

  /**
   * Reads a JSON file from the device filesystem using the /v1/fs endpoint.
   * 
   * @param filename The name of the file to read (with or without leading slash)
   * @returns Promise<FileReadResponse> The file content and metadata
   * 
   * @example
   * // Read settings file
   * const result = await modbusApiService.readFile('settings.json');
   * if (result.success) {
   *   const settings = JSON.parse(result.content);
   * }
   */
  async readFile(filename: string): Promise<FileReadResponse> {
    if (!filename || filename.includes('..')) {
      throw new Error('Invalid filename provided');
    }

    const response = await fetch(`${this.baseUrl}/v1/fs?file=${encodeURIComponent(filename)}`);

    if (!response.ok) {
      let errorBody = '';
      try {
        errorBody = await response.text();
      } catch (e) { /* Ignore */ }
      throw new Error(`Failed to read file: ${response.statusText} ${errorBody ? `- ${errorBody}` : ''}`);
    }

    try {
      return await response.json();
    } catch (e) {
      throw new Error('Invalid JSON response from server when reading file');
    }
  }

  /**
   * Writes a JSON file to the device filesystem using the /v1/fs endpoint.
   * 
   * @param request The file write request containing filename and content
   * @returns Promise<FileWriteResponse> The write operation result
   * 
   * @example
   * // Write settings file
   * const result = await modbusApiService.writeFile({
   *   filename: 'settings.json',
   *   content: JSON.stringify(settingsData, null, 2)
   * });
   */
  async writeFile(request: FileWriteRequest): Promise<FileWriteResponse> {
    if (!request.filename || request.filename.includes('..')) {
      throw new Error('Invalid filename provided');
    }

    if (!request.content) {
      throw new Error('File content cannot be empty');
    }

    const response = await fetch(`${this.baseUrl}/v1/fs`, {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
      },
      body: JSON.stringify(request),
    });

    if (!response.ok) {
      let errorBody = '';
      try {
        errorBody = await response.text();
      } catch (e) { /* Ignore */ }
      throw new Error(`Failed to write file: ${response.statusText} ${errorBody ? `- ${errorBody}` : ''}`);
    }

    try {
      return await response.json();
    } catch (e) {
      return {
        success: true,
        message: 'File written successfully.'
      };
    }
  }
}
const modbusApiService = new ModbusApiService();
export default modbusApiService;
