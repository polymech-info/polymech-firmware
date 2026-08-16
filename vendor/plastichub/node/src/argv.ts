import * as CLI from 'yargs';
import {
    warn, error,
    default_path,
    Options, OutputFormat, OutputTarget, inspect
} from './';


const LIGHT = 'http://google.co.uk';
const HEAVY = 'http://0.0.0.0:5555/app/xcf?debug=true&xblox=debug&xgrid=debug&davinci=debug&userDirectory=/PMaster/x4mm/user;'

// default options for all commands
export const defaultOptions = (yargs: CLI.Argv) => {
    return yargs.option('url', {
        default: LIGHT,
        describe: 'The URL to analyze'
    }).option('format', {
        default: 'text',
        describe: 'Normal human readable text or JSON [text|json]'
    }).option('target', {
        default: 'console',
        describe: 'Output target [console|file]'
    }).option('path', {
        default: '',
        describe: 'The target location on the local filesystem for --target=file'
    }).option('debug', {
        default: 'false',
        describe: 'Enable internal debug message'
    })
};

// Sanitizes faulty user argv options for all commands.
export const sanitize = (argv: CLI.Arguments): Options => {
    const args = argv as Options;
    args.cwd = args.cwd || process.cwd();
    if (!args.url) {
        // internal user error, should never happen!
        error('Invalid url, abort');
        return process.exit();
    }
    // path given but target is not file, correct to file
    if (args.path && args.target !== OutputTarget.FILE) {
        args.target = OutputTarget.FILE;
    }
    // target is file but no path given, correct to default file
    if (args.target === OutputTarget.FILE && !args.path) {
        args.path = default_path(args.cwd, args.url);
    }
    // format string not valid
    if (!(argv.format in OutputFormat)) {
        warn(`Unknown output format ${argv.format}! Default to ${OutputFormat.text}`);
        args.format = OutputFormat.text;
    }
    return args;
};
