export const EVENTS = {
    
};

export enum COMMANDS {
    RUN_FILE = 'Run_File',
    RUN_CLASS = 'Run_Class',
    RUN_APP_SERVER_CLASS = 'Run_App_Server_Class',
    RUN_APP_SERVER_CLASS_METHOD = 'Run_App_Server_Class_Method',
    RUN_APP_SERVER_COMPONENT_METHOD = 'Run_App_Server_Component_Method',
    CANCEL_APP_SERVER_COMPONENT_METHOD = 'Cancel_App_Server_Component_Method',
    ANSWER_APP_SERVER_COMPONENT_METHOD_INTERRUPT = 'Answer_App_Server_Component_Method_Interrupt'    
}

export enum LOGGING_SIGNAL {
};

export enum LOGGING_FLAGS {
    /**
     * No logging
     * @constant
     * @type int
     */
    NONE = 0x00000000,
    /**
     * Log in the IDE's global console
     * @constant
     * @type int
     */
    GLOBAL_CONSOLE = 0x00000001,
    /**
     * Log in the IDE's status bar
     * @constant
     * @type int
     */
    STATUS_BAR = 0x00000002,
    /**
     * Create notification popup in the IDE
     * @constant
     * @type int
     */
    POPUP = 0x00000004,
    /**
     * Log to file
     * @constant
     * @type int
     */
    FILE = 0x00000008,
    /**
     * Log into the IDE's dev tool's console
     * @constant
     * @type int
     */
    DEV_CONSOLE = 0x00000010    
};
