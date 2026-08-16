import { Logger } from "tslog";
// Create a shared logger instance with custom configuration
// "we dont need the script path nor linenumbers"
export const log = new Logger({
    prettyLogTemplate: "{{yyyy}}-{{mm}}-{{dd}} {{hh}}:{{MM}}:{{ss}}\t{{logLevelName}}\t",
});
// Optionally export a factory if we need creating separate named instances later,
// but for now a singleton `log` is sufficient for the user requirement.
//# sourceMappingURL=logger.js.map