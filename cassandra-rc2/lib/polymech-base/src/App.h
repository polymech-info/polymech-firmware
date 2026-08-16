#ifndef APP_H
#define APP_H

#include <vector>
#include <xtypes.h>
#include <Component.h>
#include <Bridge.h>
#include <xtimer.h>

class Bridge;

/**
 * @brief The App class represents the main application component.
 *
 * This class inherits from the Component class and provides functionality
 * for setting up, running the main loop, debugging, and providing information
 * about the application.
 */
class App : public Component
{
private:
    float _heapFragmentationPercent = 0.0;
    millis_t _lastHeapCheck = 0;

public:
    /**
     * @brief Default constructor for the App class.
     */
    App();

    App(String _name, short _id, int _flags) : Component(_name, _id, _flags) {}

    /**
     * @brief Retrieves a component by its ID.
     *
     * @param id The ID of the component to retrieve.
     * @return A pointer to the component with the specified ID, or nullptr if not found.
     */
    virtual Component *byId(ushort id);

    /**
     * @brief Performs the setup operations for the application and it's components.
     *
     * @return A status code indicating the result of the setup operation.
     */
    virtual short setup();

    /**
     * @brief Virtual function being called after all components have been setup.
     * @return The error code indicating the success or failure of the operation.
     */
    virtual short onRun();
    
    /**
     * @brief Runs the main loop of the application.
     *
     * @return A status code indicating the result of the loop operation.
     */
    virtual short loop();

    /**
     * @brief Performs debugging operations for the application.
     *
     * @return A status code indicating the result of the debug operation.
     */
    virtual short debug();

    /**
     * @brief Retrieves information about the application.
     *
     * @return A status code indicating the result of the info operation.
     */
    virtual short info();
    float getHeapFragmentation() const;

    /**
     * @brief Retrieves the number of components with a specific flag.
     *
     * @param flag The flag to filter the components by.
     * @return The number of components with the specified flag.
     */
    virtual short numByFlag(ushort flag);

    /**
     * @brief The list of components in the application.
     */
    std::vector<Component *> components;

    /**
     * @brief The timestamp for the last debug operation.
     */
    millis_t debugTS;

    /**
     * @brief The timestamp for the last loop operation.
     */
    millis_t loopTS;

    /**
     * @brief The wait time between loop iterations.
     */
    millis_t wait;

    /**
     * @brief The timestamp for the last wait operation.
     */
    millis_t waitTS;

    /**
     * @brief The interval for debugging operations.
     */
    millis_t DEBUG_INTERVAL;

    //////////////////////////////////////////
    //
    //  Messaging / Binding
    //
    /**
     * @brief Register all components in `Bridge`
     *
     * @return A status code indicating the result of the setup operation.
     */
    virtual short registerComponents(Bridge *bridge);

    Bridge *bridge;

    virtual void printRegisters() { }

    //////////////////////////////////////////
    //
    //  Debugging
    //
    virtual short setDebugParams(short val0, short val1);
};

#endif