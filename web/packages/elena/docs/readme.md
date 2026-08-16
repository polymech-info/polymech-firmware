# Cassandra RC2 Temperature Control System

The Cassandra RC2 is a temperature control system designed for precise thermal management across multiple heat zones. This web interface allows operators to monitor and control various heating elements arranged in zones with support for temperature profiles.

## System Architecture

The following diagram shows the high-level architecture of the Cassandra RC2 system:

```mermaid
flowchart TD
    UI[Web Interface] --> API[API Client]
    API --> CS[Controller Service]
    API --> PS[Profile Service]
    API --> ZS[Zone Service]
    CS --> SS[Storage Service]
    PS --> SS
    ZS --> SS
    SS --> DB[Supabase Database]
```

## Data Model

The core data entities in the system are:

```mermaid
erDiagram
    HeatZone ||--o{ Controller : contains
    Controller }|--o| TemperatureProfile : runs
    TemperatureProfile ||--|{ ControlPoint : contains

    HeatZone {
        string id
        string name
        string description
    }

    Controller {
        string id
        string name
        number currentTemp
        number targetTemp
        number minTemp
        number maxTemp
        number slaveId
        number updateInterval
        string currentProfile
        boolean isRunning
        string lastUpdated
        string zoneId
    }

    TemperatureProfile {
        string id
        string name
        string description
        array controlPoints
        number duration
        string createdAt
        string updatedAt
    }

    ControlPoint {
        number x
        number y
        number handleX
        number handleY
        string type
    }
```

## Component Hierarchy

The React component hierarchy for the main interface:

```mermaid
flowchart TD
    Index[Index Page] --> MainNav[Main Navigation]
    Index --> ZoneCard[Zone Cards]
    ZoneCard --> ZMC[Zone Master Control]
    ZoneCard --> TC[Temperature Controllers]
    Index --> Dialogs[Dialog Components]
    Dialogs --> PD[Profile Dialog]
    Dialogs --> ACD[Add Controller Dialog]
    Dialogs --> ECD[Edit Controller Dialog]
    Dialogs --> AZD[Add Zone Dialog]

    TC --> Info[Controller Info]
    TC --> TempDisplay[Temperature Display]
    TC --> Controls[Control Buttons]

    ZMC --> Visualization[Heat Press Visualization]
    ZMC --> ZoneControls[Zone-wide Controls]
    ZMC --> ProfileProgress[Profile Progress]
```

## API Structure

The API client provides these main function groups:

```mermaid
flowchart LR
    API[API Client] --> ZoneFunctions[Zone Functions]
    API --> ProfileFunctions[Profile Functions]
    API --> ControllerFunctions[Controller Functions]

    ZoneFunctions --> getZones[getZones]
    ZoneFunctions --> getZone[getZone]
    ZoneFunctions --> createZone[createZone]
    ZoneFunctions --> deleteZone[deleteZone]
    
    ProfileFunctions --> getProfiles[getProfiles]
    ProfileFunctions --> getProfile[getProfile]
    ProfileFunctions --> createProfile[createProfile]
    ProfileFunctions --> updateProfile[updateProfile]
    ProfileFunctions --> deleteProfile[deleteProfile]
    
    ControllerFunctions --> getControllers[getControllers]
    ControllerFunctions --> getController[getController]
    ControllerFunctions --> updateController[updateController]
    ControllerFunctions --> startController[startController]
    ControllerFunctions --> stopController[stopController]
    ControllerFunctions --> createController[createController]
    ControllerFunctions --> deleteController[deleteController]
```

## Temperature Control Flow

The temperature control system functions as follows:

```mermaid
sequenceDiagram
    actor User
    participant UI as Web Interface
    participant API as API Client
    participant Storage as Storage Service
    
    User->>UI: Set target temperature
    UI->>API: updateController(id, {targetTemp})
    API->>Storage: Save updated controller
    Storage-->>API: Confirmation
    API-->>UI: Updated controller object
    UI-->>User: Display updated temperature
    
    User->>UI: Apply temperature profile
    UI->>API: startController(id, profileId)
    API->>Storage: Update controller with profile
    Storage-->>API: Confirmation
    API-->>UI: Updated controller with profile
    UI-->>User: Display profile progress
    
    loop While profile is running
        UI->>UI: Calculate temperature based on profile
        UI->>UI: Update controller display
    end
```

## Zone Management

Heat zones organize controllers into logical groups:

```mermaid
flowchart TD
    Zone1[Zone: Top Platen] --> TC1[Controller 1: Top-Front]
    Zone1 --> TC2[Controller 2: Top-Back]
    Zone2[Zone: Bottom Platen] --> TC3[Controller 3: Bottom-Front]
    Zone2 --> TC4[Controller 4: Bottom-Back]

    subgraph MasterControl[Zone Master Controls]
        SetAll[Set All Temperatures]
        ApplyProfile[Apply Profile to Zone]
        StopAll[Stop All Controllers]
    end

    Zone1 --- MasterControl
    Zone2 --- MasterControl
```

## Temperature Profiles

Temperature profiles define heating patterns over time:

```mermaid
graph LR
    Profile[Temperature Profile] --> Points[Control Points]
    Points --> Bezier[Bezier Curve Calculator]
    Bezier --> Temp[Temperature at Time t]
    
    subgraph Timeline
        Start[Start] --> CP1[Control Point 1]
        CP1 --> CP2[Control Point 2]
        CP2 --> CP3[Control Point 3]
        CP3 --> End[End]
    end
```

## User Interface Workflow

```mermaid
stateDiagram
    [*] --> Dashboard
    Dashboard --> ViewZones
    Dashboard --> AddZone
    
    ViewZones --> ManageControllers
    ViewZones --> ZoneMasterControl
    
    AddZone --> Dashboard
    
    ManageControllers --> AddController
    ManageControllers --> EditController
    ManageControllers --> DeleteController
    ManageControllers --> StartController
    ManageControllers --> StopController
    
    StartController --> SelectProfile
    SelectProfile --> ProfileRunning
    
    ProfileRunning --> StopController
    StopController --> ManageControllers
    
    AddController --> ManageControllers
    EditController --> ManageControllers
    DeleteController --> ManageControllers
    
    ZoneMasterControl --> SetAllTemperatures
    ZoneMasterControl --> ApplyProfileToAll
    ZoneMasterControl --> StopAllControllers
    
    SetAllTemperatures --> ViewZones
    ApplyProfileToAll --> ViewZones
    StopAllControllers --> ViewZones
```

## Getting Started

To run the Cassandra RC2 web interface:

1. Install dependencies:
   ```
   npm install
   ```

2. Start the development server:
   ```
   npm run dev
   ```

3. Access the web interface at http://localhost:5173

## Technologies Used

- React with TypeScript
- TanStack Query for data fetching and caching
- Shadcn UI components
- Supabase for data storage
- Bezier curve calculations for temperature profiles 