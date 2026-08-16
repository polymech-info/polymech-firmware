# Enhanced Profile Editor with Multi-Plot Support

## Overview

The Enhanced Profile Editor extends the existing ProfileEditorEx to allow editing pressure profiles alongside temperature profiles within the same interface. This provides a unified editing experience while keeping plot types isolated - the server handles all coordination based on profile associations.

## Key Benefits

- **Unified Editing**: Edit temperature and pressure profiles in the same interface
- **Server-Side Coordination**: Plot coordination handled by server, not client
- **Simplified Association**: Simple slot references link profiles together
- **Isolated Plot Types**: Each plot type remains independent and focused

## Feature Flag

The enhanced profile editor is controlled by the existing constant:

```typescript
// In src/constants.ts
export const ENABLE_TEMPERATURE_PRESSURE_PROFILE = true;
```

This constant enables/disables the pressure profile editing within temperature profile editor.

## Data Structure

The enhanced editor uses a single object literal with simple associations:

```typescript
interface EnhancedProfileData {
  // Main temperature profile (existing structure)
  profile: TemperatureProfile;
  
  // Basic profile metadata
  name: string;
  slot: number;
  description?: string;
  duration: number;
  max: number;
  
  // Simple associations (server-coordinated)
  signalPlot: number;           // 0 = none, >0 = signal plot slot
  pressureProfile: number;      // 0 = none, >0 = pressure profile slot
  
  // Available options for editing
  availableSignalPlots: SignalPlotData[];
  availablePressureProfiles: PressureProfile[];
}
```

## User Interface Enhancement

When `ENABLE_TEMPERATURE_PRESSURE_PROFILE` is enabled, the existing ProfileEditorEx gains an additional section:

### Enhanced Sections

1. **Existing Temperature Section** (unchanged)
   - Bezier curve editor for temperature over time
   - Temperature control points list
   - Duration controls (MASTER duration)
   - All existing temperature functionality

2. **Existing Signal Plot Section** (unchanged)
   - Signal plot selector and editor
   - Signal control points timeline
   - All existing signal plot functionality
   - **Note**: Signal plot inherits temperature profile's duration when associated

3. **New Pressure Profile Section** (conditional)
   - Pressure profile selector dropdown
   - Pressure profile editor (when selected)
   - Independent pressure curve editing
   - **Note**: Pressure profile inherits temperature profile's duration when associated

### Server-Side Coordination

The server automatically coordinates plot execution and duration based on associations:
- When `signalPlot: 0` → No signal plot coordination
- When `signalPlot: N` → Server pauses/stops signal plot N with temperature profile
  - Signal plot runs for temperature profile's duration
- When `pressureProfile: 0` → No pressure profile coordination  
- When `pressureProfile: N` → Server pauses/stops pressure profile N with temperature profile
  - Pressure profile runs for temperature profile's duration

## Technical Implementation

### Minimal Component Changes

The implementation leverages the existing ProfileEditorEx with minimal modifications:

```typescript
// Enhanced props for ProfileEditorEx
interface ProfileEditorProps {
  onSubmit: (data: ProfileSavePayload) => void;
  initialData?: EnhancedProfileData; // Enhanced data structure
  max: number;
  availableControllers: Controller[];
}
```

### State Management Enhancement

Existing state gets enhanced with simple pressure profile selection:

```typescript
// Add to existing ProfileEditorEx state
const [pressureProfileId, setPressureProfileId] = useState<number>(initialData?.pressureProfile || 0);
const [availablePressureProfiles, setAvailablePressureProfiles] = useState<PressureProfile[]>([]);
const [selectedPressureProfileData, setSelectedPressureProfileData] = useState<PressureProfile | null>(null);
```

### Conditional Rendering

```typescript
// In ProfileEditorEx render method - similar to existing signal plot section
{ENABLE_TEMPERATURE_PRESSURE_PROFILE && (
  <div className="border-t border-slate-300/30 dark:border-white/10 pt-4">
    <Label className="text-slate-700 dark:text-white">
      <T>Associated Pressure Profile (Optional)</T>
    </Label>
    <Select
      onValueChange={(value) => {
        const newId = value === "@none" ? 0 : parseInt(value, 10);
        setPressureProfileId(newId);
      }}
      value={pressureProfileId === 0 ? "@none" : pressureProfileId.toString()}
    >
      <SelectTrigger className="glass-input">
        <SelectValue placeholder={translate("Select a pressure profile to associate")} />
      </SelectTrigger>
      <SelectContent className="glass-panel border-0">
        <SelectItem value="@none"><T>None</T></SelectItem>
        {availablePressureProfiles.map((profile) => (
          <SelectItem key={profile.slot} value={profile.slot.toString()}>
            {profile.name} ({translate("Slot:")} {profile.slot})
          </SelectItem>
        ))}
      </SelectContent>
    </Select>
  </div>
)}
```

## User Workflow

### Creating Enhanced Profiles

1. **Temperature Setup**: Configure temperature profile using existing controls
2. **Signal Plot Association** (optional): Select signal plot from dropdown (existing functionality)
3. **Pressure Profile Association** (optional): Select pressure profile from dropdown
4. **Save**: Save profile with associations - server handles coordination

### Object Literal Structure

The enhanced profile editor receives/outputs a single object with simple associations:

```typescript
const enhancedProfileData = {
  // Core temperature profile data (existing)
  profile: temperatureProfile,
  name: "Heat Treatment Process",
  slot: 0,
  description: "Combined heating and pressure cycle",
  duration: 1800000, // 30 minutes - MASTER DURATION for all associated plots
  max: 250, // Max temperature
  
  // Simple associations (server-coordinated)
  signalPlot: 2,           // Associate with signal plot slot 2 (inherits duration)
  pressureProfile: 1,      // Associate with pressure profile slot 1 (inherits duration)
  
  // Available options for dropdowns
  availableSignalPlots: [...],
  availablePressureProfiles: [...]
};
```

### Server Coordination Logic

When the temperature profile runs:
- If `signalPlot: 0` → No signal coordination
- If `signalPlot: N` → Server automatically pauses/stops/resumes signal plot N
  - **Duration**: Signal plot uses temperature profile's duration
- If `pressureProfile: 0` → No pressure coordination  
- If `pressureProfile: N` → Server automatically pauses/stops/resumes pressure profile N
  - **Duration**: Pressure profile uses temperature profile's duration

**Key Point**: The temperature profile is the master timeline - associated signal plots and pressure profiles inherit its duration and execution timeline.

All plot types remain isolated in editing - only the server manages their lifecycle coordination and duration synchronization.

## Process Control Benefits

### Server-Managed Coordination

- Temperature profile controls associated plot lifecycles and duration
- Temperature profile acts as master timeline for all associated plots
- Independent plot editing with automatic server coordination
- Simplified client-side logic - no complex coordination code needed
- Reliable execution managed by server state machine

### Clear Separation of Concerns

- Temperature profiles remain focused on temperature control
- Pressure profiles remain focused on pressure control  
- Signal plots remain focused on discrete actions
- Server handles all inter-plot coordination logic

## Implementation Approach

### Single Component Enhancement
- Extend existing ProfileEditorEx component
- Add conditional pressure profile sections
- Enhance BezierEditor for dual-curve display
- Reuse existing CollapsibleSection patterns

### Minimal Changes Required
- Add pressure profile selector to ProfileEditorEx (similar to signal plot selector)
- Add pressure profile state management (simple slot ID)
- Optional: Add pressure profile editing section when profile selected
- Conditional rendering based on `ENABLE_TEMPERATURE_PRESSURE_PROFILE`

## Configuration

### Feature Detection

```typescript
import { ENABLE_TEMPERATURE_PRESSURE_PROFILE } from '@/constants';

const shouldShowPressureFeatures = () => {
  return ENABLE_TEMPERATURE_PRESSURE_PROFILE && 
         featureFlags.ENABLE_PROFILE_TEMPERATURE && 
         featureFlags.ENABLE_PROFILE_PRESSURE_PROFILE;
};
```

### Usage in ProfileEditorEx

```typescript
// In ProfileEditorEx component
import { ENABLE_TEMPERATURE_PRESSURE_PROFILE } from '@/constants';

// Conditional rendering for pressure profile selector (similar to signal plot selector)
{ENABLE_TEMPERATURE_PRESSURE_PROFILE && (
  <div className="border-t border-slate-300/30 dark:border-white/10 pt-4">
    <Label><T>Associated Pressure Profile (Optional)</T></Label>
    <Select value={pressureProfileId.toString()} onValueChange={setPressureProfileId}>
      <SelectItem value="0"><T>None</T></SelectItem>
      {availablePressureProfiles.map(profile => (
        <SelectItem key={profile.slot} value={profile.slot.toString()}>
          {profile.name}
        </SelectItem>
      ))}
    </Select>
  </div>
)}
```

## Integration with Existing Code

### ProfilePage Integration

```typescript
// In ProfilePage.tsx - pass enhanced data to ProfileEditorEx
const enhancedInitialData = {
  ...initialData,                    // Existing temperature profile data
  signalPlot: initialData?.signalPlot || 0,     // Existing signal plot association
  pressureProfile: initialData?.pressureProfile || 0,  // New pressure profile association
  availableSignalPlots,              // Existing available signal plots
  availablePressureProfiles          // New available pressure profiles from context
};

<ProfileEditor
  onSubmit={handleEnhancedSubmit}    // Enhanced submit includes pressureProfile field
  initialData={enhancedInitialData}
  max={maxTemp}
  availableControllers={availableControllers}
/>
```

### No New Routes Required

The existing routes continue to work:
- `/profiles/new` - Create new enhanced profile
- `/profiles/:slot` - Edit existing profile with enhancements

## Future Enhancements

### Advanced Features

1. **Process Templates**: Pre-defined temperature/pressure coordination patterns
2. **Predictive Modeling**: Suggest pressure curves based on temperature profiles
3. **Process Validation**: Automated checks for process safety and efficiency
4. **Historical Analysis**: Compare actual vs. planned temperature/pressure execution
5. **Multi-Controller Coordination**: Coordinate multiple temperature and pressure controllers

### Integration Opportunities

1. **Recipe Management**: Integration with broader recipe/process management
2. **Quality Control**: Link profile execution to quality metrics
3. **Energy Optimization**: Optimize profiles for energy efficiency
4. **Maintenance Scheduling**: Coordinate profiles with maintenance windows

## Conclusion

The Enhanced Profile Editor with Multi-Plot Support provides a clean, server-coordinated approach to associating different plot types. By extending the existing ProfileEditorEx component with simple associations, this design:

- **Minimizes Code Changes**: Adds simple pressure profile selector similar to existing signal plot selector
- **Maintains Plot Isolation**: Each plot type remains independent and focused
- **Server-Side Coordination**: All plot lifecycle coordination handled by server logic
- **Simplified Client Logic**: No complex linkage modes or client-side coordination needed
- **Backward Compatible**: Existing temperature and signal plot functionality unchanged

The implementation provides powerful multi-plot coordination while keeping the client-side architecture simple and maintainable.
