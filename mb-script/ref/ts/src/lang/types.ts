

/**
 * The block's capabilities. This will be evaluated in the interface but also
 * by the run-time (speed ups).
 *
 */
export enum BLOCK_CAPABILITIES {
    /**
     * No other block includes this one.
     * @constant
     * @type int
     */
    TOPMOST = 0x00004000,
    /**
     * The block's execution context can be changed to another object.
     * @constant
     * @type int
     */
    TARGET = 0x00040000,
    /**
     * The block may create additional input terminals ('reset', 'pause', ...).
     * @constant
     * @type int
     */
    VARIABLE_INPUTS = 0x00000080,
    /**
     * The block may create additional output terminals ('onFinish', 'onError').
     * @constant
     * @type int
     */
    VARIABLE_OUTPUTS = 0x00000100,
    /**
     * The block may create additional ouput parameters ('result', 'error',...).
     * @constant
     * @type int
     */
    VARIABLE_OUTPUT_PARAMETERS = 0x00000200,
    /**
     * The block may create additional input parameters.
     * @constant
     * @type int
     */
    VARIABLE_INPUT_PARAMETERS = 0x00000400,
    /**
     * The block can contain child blocks.
     * @constant
     * @type int
     */
    CHILDREN = 0x00000020,
    /**
     * Block provides standard signals ('paused', 'error').
     * @constant
     * @type int
     */
    SIGNALS = 0x00000080
}
/**
 * Flags to describe a block's execution behavior.
 *
 * @enum {integer} module=xide/types/RUN_FLAGS
 * @memberOf module=xide/types
 */
export enum RUN_FLAGS {
    /**
     * The block can execute child blocks.
     * @constant
     * @type int
     */
    CHILDREN = 0x00000020,
    /**
     * Block is waiting for a message => EXECUTION_STATE==RUNNING
     * @constant
     * @type int
     */
    WAIT = 0x000008000
};

/**
 * Flags to describe a block's execution state.
 *
 * @enum {integer} module=xide/types/EXECUTION_STATE
 * @memberOf module=xide/types
 */
export enum EXECUTION_STATE {
    /**
     * The block is doing nothing and also has done nothing. The is the default state
     * @constant
     * @type int
     */
    NONE = 0x00000000,
    /**
     * The block is running.
     * @constant
     * @type int
     */
    RUNNING = 0x00000001,
    /**
     * The block is an error state.
     * @constant
     * @type int
     */
    ERROR = 0x00000002,
    /**
     * The block is in an paused state.
     * @constant
     * @type int
     */
    PAUSED = 0x00000004,
    /**
     * The block is an finished state, ready to be cleared to "NONE" at the next frame.
     * @constant
     * @type int
     */
    FINISH = 0x00000008,
    /**
     * The block is an stopped state, ready to be cleared to "NONE" at the next frame.
     * @constant
     * @type int
     */
    STOPPED = 0x00000010,
    /**
     * The block has been launched once...
     * @constant
     * @type int
     */
    ONCE = 0x80000000,
    /**
     * Block will be reseted next frame
     * @constant
     * @type int
     */
    RESET_NEXT_FRAME = 0x00800000,
    /**
     * Block is locked and so no further inputs can be activated.
     * @constant
     * @type int
     */
    LOCKED = 0x20000000	// Block is locked for utilisation in xblox
}

export enum BLOCK_MODE {
    NORMAL = 0,
    UPDATE_WIDGET_PROPERTY = 1
};

/**
 * Flags to describe a block's belonging to a standard signal.
 * @enum {integer} module=xblox/types/BLOCK_OUTLET
 * @memberOf module=xblox/types
 */
export enum BLOCK_OUTLET {
    NONE = 0x00000000,
    PROGRESS = 0x00000001,
    ERROR = 0x00000002,
    PAUSED = 0x00000004,
    FINISH = 0x00000008,
    STOPPED = 0x00000010
};
/**
 * Flags to describe flags of the inner state of a block which might change upon the optimization. It also
 * contains some other settings which might be static, default or changed by the UI(debugger, etc...)
 *
 * @enum {integer} module:xide/types/BLOCK_FLAGS
 * @memberOf module:xide/types
 */
export enum BLOCK_FLAGS {
    NONE = 0x00000000,	// Reserved for future use
    ACTIVE = 0x00000001,	// This behavior is active
    SCRIPT = 0x00000002,	// This behavior is a script
    RESERVED1 = 0x00000004,	// Reserved for internal use
    USEFUNCTION = 0x00000008,	// Block uses a function and not a graph
    RESERVED2 = 0x00000010,	// Reserved for internal use
    SINGLE = 0x00000020,	// Only this block will excecuted, child blocks not.
    WAITSFORMESSAGE = 0x00000040,	// Block is waiting for a message to activate one of its outputs
    VARIABLEINPUTS = 0x00000080,	// Block may have its inputs changed by editing them
    VARIABLEOUTPUTS = 0x00000100,	// Block may have its outputs changed by editing them
    VARIABLEPARAMETERINPUTS = 0x00000200,	// Block may have its number of input parameters changed by editing them
    VARIABLEPARAMETEROUTPUTS = 0x00000400,	// Block may have its number of output parameters changed by editing them
    TOPMOST = 0x00004000,	// No other Block includes this one
    BUILDINGBLOCK = 0x00008000,	// This Block is a building block (eg= not a transformer of parameter operation)
    MESSAGESENDER = 0x00010000,	// Block may send messages during its execution
    MESSAGERECEIVER = 0x00020000,	// Block may check messages during its execution
    TARGETABLE = 0x00040000,	// Block may be owned by a different object that the one to which its execution will apply
    CUSTOMEDITDIALOG = 0x00080000,	// This Block have a custom Dialog Box for parameters edition .
    RESERVED0 = 0x00100000,	// Reserved for internal use.
    EXECUTEDLASTFRAME = 0x00200000,	// This behavior has been executed during last process. (Available only in profile mode )
    DEACTIVATENEXTFRAME = 0x00400000,	// Block will be deactivated next frame
    RESETNEXTFRAME = 0x00800000,	// Block will be reseted next frame

    INTERNALLYCREATEDINPUTS = 0x01000000,	// Block execution may create/delete inputs
    INTERNALLYCREATEDOUTPUTS = 0x02000000,	// Block execution may create/delete outputs
    INTERNALLYCREATEDINPUTPARAMS = 0x04000000,	// Block execution may create/delete input parameters or change their type
    INTERNALLYCREATEDOUTPUTPARAMS = 0x08000000,	// Block execution may create/delete output parameters or change their type
    INTERNALLYCREATEDLOCALPARAMS = 0x40000000,	// Block execution may create/delete local parameters or change their type

    ACTIVATENEXTFRAME = 0x10000000,	// Block will be activated next frame
    LOCKED = 0x20000000,	// Block is locked for utilisation in xblox
    LAUNCHEDONCE = 0x80000000	// Block has not yet been launched...
}
/**
 *  Mask for the messages the callback function of a block should be aware of. This goes directly in
 *  the EventedMixin as part of the 'emits' chain (@TODO)
 *
 * @enum module:xide/types/BLOCK_CALLBACKMASK
 * @memberOf module:xide/types
 */
export enum BLOCK_CALLBACKMASK {
    PRESAVE = 0x00000001,	// Emits PRESAVE messages
    DELETE = 0x00000002,	// Emits DELETE messages
    ATTACH = 0x00000004,	// Emits ATTACH messages
    DETACH = 0x00000008,	// Emits DETACH messages
    PAUSE = 0x00000010,	// Emits PAUSE messages
    RESUME = 0x00000020,	// Emits RESUME messages
    CREATE = 0x00000040,	// Emits CREATE messages
    RESET = 0x00001000,	// Emits RESET messages
    POSTSAVE = 0x00000100,	// Emits POSTSAVE messages
    LOAD = 0x00000200,	// Emits LOAD messages
    EDITED = 0x00000400,	// Emits EDITED messages
    SETTINGSEDITED = 0x00000800,	// Emits SETTINGSEDITED messages
    READSTATE = 0x00001000,	// Emits READSTATE messages
    NEWSCENE = 0x00002000,	// Emits NEWSCENE messages
    ACTIVATESCRIPT = 0x00004000,	// Emits ACTIVATESCRIPT messages
    DEACTIVATESCRIPT = 0x00008000,	// Emits DEACTIVATESCRIPT messages
    RESETINBREAKPOINT = 0x00010000,	// Emits RESETINBREAKPOINT messages
    RENAME = 0x00020000,	// Emits RENAME messages
    BASE = 0x0000000E,	// Base flags =attach /detach /delete
    SAVELOAD = 0x00000301,	// Base flags for load and save
    PPR = 0x00000130,	// Base flags for play/pause/reset
    EDITIONS = 0x00000C00,	// Base flags for editions of settings or parameters
    ALL = 0xFFFFFFFF	// All flags
}

export enum EVENTS {
    ON_RUN_BLOCK = <any>'onRunBlock',
    ON_RUN_BLOCK_FAILED = <any>'onRunBlockFailed',
    ON_RUN_BLOCK_SUCCESS = <any>'onRunBlockSuccess',
    ON_BLOCK_SELECTED = <any>'onItemSelected',
    ON_BLOCK_UNSELECTED = <any>'onBlockUnSelected',
    ON_BLOCK_EXPRESSION_FAILED = <any>'onExpressionFailed',
    ON_BUILD_BLOCK_INFO_LIST = <any>'onBuildBlockInfoList',
    ON_BUILD_BLOCK_INFO_LIST_END = <any>'onBuildBlockInfoListEnd',
    ON_BLOCK_PROPERTY_CHANGED = <any>'onBlockPropertyChanged',
    ON_SCOPE_CREATED = <any>'onScopeCreated',
    ON_VARIABLE_CHANGED = <any>'onVariableChanged',
    ON_CREATE_VARIABLE_CI = <any>'onCreateVariableCI'
}


export enum Type {
    AssignmentExpression = <any>'AssignmentExpression',
    ArrayExpression = <any>'ArrayExpression',
    BlockStatement = <any>'BlockStatement',
    BinaryExpression = <any>'BinaryExpression',
    BreakStatement = <any>'BreakStatement',
    CallExpression = <any>'CallExpression',
    CatchClause = <any>'CatchClause',
    ConditionalExpression = <any>'ConditionalExpression',
    ContinueStatement = <any>'ContinueStatement',
    DoWhileStatement = <any>'DoWhileStatement',
    DebuggerStatement = <any>'DebuggerStatement',
    EmptyStatement = <any>'EmptyStatement',
    ExpressionStatement = <any>'ExpressionStatement',
    ForStatement = <any>'ForStatement',
    ForInStatement = <any>'ForInStatement',
    FunctionDeclaration = <any>'FunctionDeclaration',
    FunctionExpression = <any>'FunctionExpression',
    Identifier = <any>'Identifier',
    IfStatement = <any>'IfStatement',
    Literal = <any>'Literal',
    LabeledStatement = <any>'LabeledStatement',
    LogicalExpression = <any>'LogicalExpression',
    MemberExpression = <any>'MemberExpression',
    NewExpression = <any>'NewExpression',
    ObjectExpression = <any>'ObjectExpression',
    Program = <any>'Program',
    Property = <any>'Property',
    ReturnStatement = <any>'ReturnStatement',
    SequenceExpression = <any>'SequenceExpression',
    SwitchStatement = <any>'SwitchStatement',
    SwitchCase = <any>'SwitchCase',
    ThisExpression = <any>'ThisExpression',
    ThrowStatement = <any>'ThrowStatement',
    TryStatement = <any>'TryStatement',
    UnaryExpression = <any>'UnaryExpression',
    UpdateExpression = <any>'UpdateExpression',
    VariableDeclaration = <any>'VariableDeclaration',
    VariableDeclarator = <any>'VariableDeclarator',
    WhileStatement = <any>'WhileStatement',
    WithStatement = <any>'WithStatement'
};