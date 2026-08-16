# Dependency Injection in Polymech Firmware

This document outlines dependency injection (DI) patterns applicable to the Polymech firmware, with a focus on lightweight ("cheap") techniques suitable for an embedded C++ environment.

## 1. What is Dependency Injection?

Dependency Injection is a design pattern where a component (an object) receives its dependencies from an external source rather than creating them internally. This promotes loose coupling, making the system more modular, easier to test, and more maintainable.

In an embedded context, DI techniques must be "cheap" in terms of:
- **CPU Overhead**: The mechanism should not consume significant processing time.
- **Memory Footprint**: The solution should not require large amounts of RAM or flash.
- **Cognitive Overhead**: The pattern should be simple to understand and use.

## 2. Current Approach: Manual Constructor Injection

The firmware currently uses **Manual Constructor Injection**. The central application class (e.g., `PHApp`) instantiates components and their dependencies, and then "injects" the dependencies into the components by passing them as pointers to the constructor.

### Example: `Plunger` Component

The instantiation of the `Plunger` component in `src/PHApp.cpp` illustrates this pattern perfectly:

```cpp
// In PHApp, dependencies are created first:
vfd_0 = new SAKO_VFD(this, MB_SAKO_VFD_SLAVE_ID);
joystick_0 = new Joystick(this, PIN_JOYSTICK_UP, ...);
pot_0 = new POT(this, MB_ANALOG_0);
pot_1 = new POT(this, MB_ANALOG_1);

// Then, the Plunger is created with its dependencies injected via the constructor:
plunger_0 = new Plunger(this, vfd_0, joystick_0, pot_0, pot_1);
```

The `Plunger` class constructor receives and stores pointers to the objects it depends on:

```cpp
// lib/polymech-base/src/components/Plunger.h
class Plunger : public Component {
public:
    Plunger(Component* owner, SAKO_VFD* vfd, Joystick* joystick, POT* speedPot, POT* torquePot);
    // ...
private:
    SAKO_VFD* _vfd;
    Joystick* _joystick;
    POT* _speedPot;
    POT* _torquePot;
    // ...
};
```

### Advantages

- **Explicit Dependencies**: A component's dependencies are clear from its constructor signature.
- **Compile-Time Safety**: Missing dependencies will result in a compile-time error.
- **Zero Runtime Overhead**: It's just a function call. No lookups or dynamic resolutions are needed.
- **Simplicity**: The pattern is easy to understand and implement without any special libraries.

### Disadvantages

- **Boilerplate**: The main application class can become large and cluttered with object creation logic as the application grows.
- **Initialization Order**: The developer must manually manage the correct order of object creation.

## 3. Alternative "Cheap" DI Options

While the current approach is effective, here are some alternative patterns that can be considered.

### Option 1: Factory Pattern

This pattern encapsulates the creation logic for a set of related objects into a separate `Factory` class. The main application asks the factory for a component, and the factory handles creating the component and its dependencies.

#### Example

```cpp
class ComponentFactory {
public:
    ComponentFactory(App* owner) : _owner(owner) {}

    Plunger* createPlunger() {
        SAKO_VFD* vfd = new SAKO_VFD(_owner, MB_SAKO_VFD_SLAVE_ID);
        Joystick* joystick = new Joystick(_owner, ...);
        // ... create other dependencies
        return new Plunger(_owner, vfd, joystick, ...);
    }
private:
    App* _owner;
};

// In main app:
auto factory = new ComponentFactory(this);
plunger_0 = factory->createPlunger();
```

- **Pros**: Cleans up the main application logic, centralizes creation logic.
- **Cons**: Adds another layer of abstraction and more classes to maintain.

### Option 2: Service Locator

A Service Locator is a central registry where dependencies ("services") are registered and can be requested by components.

#### Example

```cpp
// A simple Service Locator
class ServiceLocator {
public:
    static SAKO_VFD* getVFD() { return _vfd; }
    static void registerVFD(SAKO_VFD* vfd) { _vfd = vfd; }
private:
    static SAKO_VFD* _vfd;
};

// In main app:
vfd_0 = new SAKO_VFD(...);
ServiceLocator::registerVFD(vfd_0);

// In a component's constructor or setup:
_vfd = ServiceLocator::getVFD();
```

- **Pros**: Can simplify constructors, decouples components from the main app.
- **Cons**: **(Anti-Pattern Warning)** Hides dependencies, making the code harder to understand and test. It's essentially a glorified global variable, which can lead to maintenance issues. It also introduces runtime lookup overhead.

### Option 3: Compile-Time DI with Templates

This advanced technique uses C++ templates to inject dependencies at compile-time. The dependency is a type parameter, not an object instance passed to the constructor.

#### Example

```cpp
template<typename VfdType, typename JoystickType>
class Plunger : public Component {
public:
    Plunger(Component* owner) {
        // Here we assume VfdType and JoystickType can be instantiated
        // or accessed somehow. This is a very simplified example.
    }
    // ...
};
```

- **Pros**: Maximum performance (zero runtime overhead), strong type safety.
- **Cons**: Can be complex to implement and lead to verbose compiler errors ("template vomit"). Can increase compile times.

## 4. Recommendation

The current **Manual Constructor Injection** pattern is robust, simple, and highly efficient. It is well-suited for the project and should remain the primary DI pattern.

- **Stick with Manual Constructor Injection** for its clarity and performance.
- Consider using the **Factory Pattern** if the object creation logic in `PHApp` becomes overly complex and difficult to manage. This can help organize the setup phase without adding significant runtime overhead.
- **Avoid the Service Locator** pattern. While it may seem convenient, it obscures dependencies and can lead to less maintainable code in the long run.
- **Template-based DI** is a powerful but complex option. It could be considered for highly performance-critical sections where every CPU cycle counts, but it's likely overkill for general component wiring.

## 5. Compile-Time DI via Feature Flags

In addition to runtime object injection, the firmware uses a powerful form of compile-time dependency management using preprocessor feature flags.

### How It Works

1.  **Configuration (`src/config.h`)**: The central `config.h` file contains a series of `#define` statements that act as feature flags (e.g., `#define ENABLE_PLUNGER`).
2.  **Conditional Compilation**: Throughout the codebase, these flags are used in `#ifdef` blocks to conditionally compile code. This includes object instantiation, method calls, and even member variable declarations. If a flag is not defined, the corresponding code is completely excluded from the final binary.
3.  **Dependency Validation (`src/config-validate.h`)**: A validation header checks for logical inconsistencies in the configuration at compile time. For example, it ensures that if `ENABLE_PLUNGER` is active, its own dependencies like `ENABLE_SAKO_VFD` are also active, throwing a compiler error if they are not.

This mechanism ensures that only the necessary components are compiled into the firmware, which is a highly effective way to manage dependencies and resource usage on an embedded system.

### The `menu.tsx` Configuration Tool

To simplify the management of these feature flags, the project includes a command-line tool located at `cli-ts/src/commands/menu.tsx`.

- **Functionality**: This tool provides a terminal-based interactive menu for editing `src/config.h`. It parses the header file to find feature flags and their associated settings.
- **Technology**: It uses the `tree-sitter-cpp` library to accurately parse the C++ header file syntax and identify `#define` directives, including those that are commented out.
- **Role in DI**: The `menu.tsx` script acts as a user-friendly interface for the compile-time DI system. It allows a developer to easily enable or disable high-level features.

It is important to note that **the `menu.tsx` script is a configuration editor, not a dependency resolver**. It does not automatically enable `ENABLE_SAKO_VFD` when you enable `ENABLE_PLUNGER`. The developer is still responsible for managing these dependencies, with `config-validate.h` serving as the safety net during the build process.

## 6. Potential Enhancement: Automated Dependency Analysis

The `menu.tsx` script could be evolved from a simple configuration editor into a dependency-aware provisioning tool. By leveraging `tree-sitter-cpp` more deeply, it could automatically analyze and resolve dependencies between components.

### Conceptual Workflow

1.  **Build a Component Map**: The script would first parse `src/features.h` to build a comprehensive map linking feature flags (e.g., `ENABLE_PLUNGER`) to the header files they include (e.g., `components/Plunger.h`) and the primary class names defined within them (e.g., `Plunger`).

2.  **Analyze Component Dependencies**: When a feature is selected in the menu, the script would parse the corresponding header file. It would traverse the Abstract Syntax Tree to identify dependency clues, such as:
    - The types of pointers passed into the class constructor (`SAKO_VFD* vfd`).
    - The types of member variables.
    - Local `#include` directives.

3.  **Resolve and Report**: The script would then cross-reference these discovered dependency class names (e.g., `SAKO_VFD`) with the component map from step 1 to find their associated feature flags (`ENABLE_SAKO_VFD`).

This would allow the tool to generate a dependency object for any given component. For example, analyzing `Plunger` could produce:

```json
{
  "feature": "ENABLE_PLUNGER",
  "header": "src/components/Plunger.h",
  "dependencies": [
    {
      "className": "SAKO_VFD",
      "feature": "ENABLE_SAKO_VFD",
      "header": "src/components/SAKO_VFD.h"
    },
    {
      "className": "Joystick",
      "feature": "ENABLE_JOYSTICK",
      "header": "src/components/Joystick.h"
    }
  ]
}
```

### Benefits

With this capability, the `menu.tsx` script could proactively assist the developer. For instance, upon enabling `ENABLE_PLUNGER`, it could automatically detect that `ENABLE_SAKO_VFD` is also required and either enable it automatically or prompt the user to do so. This would prevent many `config-validate.h` errors before the compilation stage, streamlining the development process significantly. 