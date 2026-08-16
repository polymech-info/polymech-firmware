/**
 * A 'Configurable Information's ("CI") processing state during post or pre-processing.
 *
 * @enum {int} module:xide/types/CI_STATE
 * @memberOf module:xide/types
 */
export const CI_STATE = {
    /**
     * Nothing done, could also mean there is nothing to do all
     * @constant
     * @type int
     */
    NONE: 0x00000000,
    /**
     * In pending state. At that time the compiler has accepted additional work and ci flag processing is queued
     * but not scheduled yet.
     * @constant
     * @type int
     */
    PENDING: 0x00000001,
    /**
     * The processing state.
     * @constant
     * @type int
     */
    PROCESSING: 0x00000002,
    /**
     * The CI has been processed but it failed.
     * @constant
     * @type int
     */
    FAILED: 0x00000004,
    /**
     * The CI was successfully processed.
     * @constant
     * @type int
     */
    SUCCESSED: 0x00000008,
    /**
     * The CI has been processed.
     * @constant
     * @type int
     */
    PROCESSED: 0x00000010,
    /**
     * The CI left the post/pre processor entirly but has not been accepted by the originating source.
     * This state can happen when the source became invalid and so its sort of orphan.
     * @constant
     * @type int
     */
    DEQUEUED: 0x00000020,
    /**
     * The CI fully resolved and no references except by the source are around.
     * @constant
     * @type int
     */
    SOLVED: 0x00000040,
    /**
     * Flag to mark the core's end of this bitmask, from here its user land
     * @constant
     * @type int
     */
    END: 0x00000080
};
/**
 * A 'Configurable Information's ("CI") type flags for post and pre-processing a value.
 * @enum {string} CIFLAGS
 * @global
 * @memberOf module:xide/types
 */
export const CIFLAG = {
    /**
     * Instruct for no additional extra processing
     * @constant
     * @type int
     */
    NONE: 0x00000000,
    /**
     * Will instruct the pre/post processor to base-64 decode or encode
     * @constant
     * @type int
     */
    BASE_64: 0x00000001,
    /**
     * Post/Pre process the value with a user function
     * @constant
     * @type int
     */
    USE_FUNCTION: 0x00000002,
    /**
     * Replace variables with local scope's variables during the post/pre process
     * @constant
     * @type int
     */
    REPLACE_VARIABLES: 0x00000004,
    /**
     * Replace variables with local scope's variables during the post/pre process but evaluate the whole string
     * as Javascript
     * @constant
     * @type int
     */
    REPLACE_VARIABLES_EVALUATED: 0x00000008,
    /**
     * Will instruct the pre/post processor to escpape evaluated or replaced variables or expressions
     * @constant
     * @type int
     */
    ESCAPE: 0x00000010,
    /**
     * Will instruct the pre/post processor to replace block calls with oridinary vanilla script
     * @constant
     * @type int
     */
    REPLACE_BLOCK_CALLS: 0x00000020,
    /**
     * Will instruct the pre/post processor to remove variable delimitters/placeholders from the final string
     * @constant
     * @type int
     */
    REMOVE_DELIMTTERS: 0x00000040,
    /**
     * Will instruct the pre/post processor to remove   "[" ,"]" , "(" , ")" , "{", "}" , "*" , "+" , "."
     * @constant
     * @type int
     */
    ESCAPE_SPECIAL_CHARS: 0x00000080,
    /**
     * Will instruct the pre/post processor to use regular expressions over string substitution
     * @constant
     * @type int
     */
    USE_REGEX: 0x00000100,
    /**
     * Will instruct the pre/post processor to use Filtrex (custom bison parser, needs xexpression) over string substitution
     * @constant
     * @type int
     */
    USE_FILTREX: 0x00000200,
    /**
     * Cascade entry. There are cases where #USE_FUNCTION is not enough or we'd like to avoid further type checking.
     * @constant
     * @type int
     */
    CASCADE: 0x00000400,
    /**
     * Cascade entry. There are cases where #USE_FUNCTION is not enough or we'd like to avoid further type checking.
     * @constant
     * @type int
     */
    EXPRESSION: 0x00000800,
    /**
     * Dont parse anything
     * @constant
     * @type int
     */
    DONT_PARSE: 0x000001000,
    /**
     * Convert to hex
     * @constant
     * @type int
     */
    TO_HEX: 0x000002000,
    /**
     * Convert to hex
     * @constant
     * @type int
     */
    REPLACE_HEX: 0x000004000,
    /**
     * Wait for finish
     * @constant
     * @type int
     */
    WAIT: 0x000008000,
    /**
     * Wait for finish
     * @constant
     * @type int
     */
    DONT_ESCAPE: 0x000010000,
    /**
     * Flag to mark the maximum core bit mask, after here its user land
     * @constant
     * @type int
     */
    END: 0x000020000
};
/**
 * A 'Configurable Information's ("CI") type information. Every CI has this information. You can
 * re-composite new types with ECIType.STRUCTURE. However all 'beans' (rich objects) in the system all displayed through a set of CIs,
 * also called the CIS (Configurable Information Set). There are many types already :
 *
 * Each ECIType has mapped widgets, BOOL : checkbox, STRING: Text-Areay and so forth.
 *
 * @enum {string} module:xide/types/ECIType
 * @memberOf module:xide/types
 */
export const ECIType = {
    /**
     * @const
     * @type { int}
     */
    BOOL: 0,
    /**
     * @const
     * @type { int}
     */
    BOX: 1,
    /**
     * @const
     * @type { int}
     */
    COLOUR: 2,
    /**
     * @const
     * @type { int}
     */
    ENUMERATION: 3,
    /**
     * @const
     * @type { int}
     */
    FILE: 4,
    /**
     * @const
     * @type { int}
     */
    FLAGS: 5,
    /**
     * @const
     * @type { int}
     */
    FLOAT: 6,
    /**
     * @const
     * @type { int}
     */
    INTEGER: 7,
    /**
     * @const
     * @type { int}
     */
    MATRIX: 8,
    /**
     * @const
     * @type { int}
     */
    OBJECT: 9,
    /**
     * @const
     * @type { int}
     */
    REFERENCE: 10,
    /**
     * @const
     * @type { int}
     */
    QUATERNION: 11,
    /**
     * @const
     * @type { int}
     */
    RECTANGLE: 12,
    /**
     * @const
     * @type { int}
     */
    STRING: 13,
    /**
     * @const
     * @type { int}
     */
    VECTOR: 14,
    /**
     * @const
     * @type { int}
     */
    VECTOR2D: 15,
    /**
     * @const
     * @type { int}
     */
    VECTOR4D: 16,
    /**
     * @const
     * @type { int}
     */
    ICON: 17,
    /**
     * @const
     * @type { int}
     */
    IMAGE: 18,
    /**
     * @const
     * @type { int}
     */
    STRUCTURE: 21,
    /**
     * @const
     * @type { int}
     */
    BANNER2: 22,
    /**
     * @const
     * @type { int}
     */
    SCRIPT: 24,
    /**
     * @const
     * @type { int}
     */
    EXPRESSION: 25,
    /**
     * @const
     * @type { int}
     */
    ARGUMENT: 27,
    /**
     * @const
     * @type { int}
     */
    JSON_DATA: 28,
    /**
     * @const
     * @type { int}
     */
    EXPRESSION_EDITOR: 29,
    /**
     * @const
     * @type { int}
     */
    WIDGET_REFERENCE: 30,
    /**
     * @const
     * @type { int}
     */
    DOM_PROPERTIES: 31,
    /**
     * @const
     * @type { int}
     */
    BLOCK_REFERENCE: 32,
    /**
     * @const
     * @type { int}
     */
    BLOCK_SETTINGS: 33,
    /**
     * user land from here
     * @const
     * @type { int}
     */
    END: 35,
    /**
     * @const
     * @type { int}
     */
    UNKNOWN: -1
};
