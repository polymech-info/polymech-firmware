import * as CLI from 'yargs';
import { debug } from '..';
import * as utils from '../lib/common/strings';
import { read as readSheet } from '../lib/net/sheets';
import * as path from 'path';
import { sync as read } from '@xblox/fs/read';
import { sync as exists } from '@xblox/fs/exists';
import { sync as dir } from '@xblox/fs/dir';
import { sync as write } from '@xblox/fs/write';
import { Converter } from 'showdown';


const defaultOptions = (yargs: CLI.Argv) => {
    return yargs.option('input', {
        default: './',
        describe: 'The sources'
    }).option('output', {
        default: './',
        describe: 'The output'
    }).option('debug', {
        default: 'false',
        describe: 'Enable internal debug message'
    })
};

let options = (yargs: CLI.Argv) => defaultOptions(yargs);

// npm run build ; node ./build/main.js test
export const register = (cli: CLI.Argv) => {
    return cli.command('test', 'test command', options, async (argv: CLI.Arguments) => {
        if (argv.help) { return; }

        readSheet('1oVEiGH4o3SV-mAA3Mb-WNVJMyYl4VMxLjWjrSw_ipJY', 'ElenaMargin');

    });
};
