import * as CLI from 'yargs';
import { defaultOptions, sanitize } from '../argv';
import { Options } from '../';
import { render as output } from '../output';
// no extra options, using defaults
const options = (yargs: CLI.Argv) => defaultOptions(yargs);

export const register = (cli: CLI.Argv) => {
    return cli.command('clean', 'Clean all trace and data files', options, async (argv: CLI.Arguments) => {
        //@TODO: this guard might not be necessary
        if (argv.help) { return; }
        const args = sanitize(argv) as Options;
        output({}, args);
    });
};
