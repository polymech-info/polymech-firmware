import * as CLI from 'yargs';
import { sanitize } from '../argv';
import { Options } from '..';
import { render as output } from '../output';
import * as chokidar from 'chokidar';
import * as path from 'path';
import { debug } from '..';
import { convert, convertFiles } from './markdown';
import * as utils from '../lib/common/strings';
import { sync as read } from '@xblox/fs/read';
import { sync as exists } from '@xblox/fs/exists';
import { sync as dir } from '@xblox/fs/dir';
import { sync as write } from '@xblox/fs/write';
import * as cheerio from 'cheerio';
const pretty = require('pretty');

const defaultOptions = (yargs: CLI.Argv) => {
    return yargs.option('input', {
        default: './',
        describe: 'The sources'
    }).option('debug', {
        default: 'false',
        describe: 'Enable internal debug message'
    }).option('tsx', {
        default: 'true',
        describe: 'Update tsx file'
    })
};

export const parseHTML = (input: string) => {
    const $ = cheerio.load(input as string, {
        xmlMode: true
    });

    $('meta').remove();
    $('templates').remove();

    input = $.html();
    input = pretty(input,{ocd: true});
    return input;
}

export const updateTSX = (mdPath: string) => {
    const parts = path.parse(mdPath);

    let html = read(`${parts.dir}/${parts.name}.html`, 'string');
    const tsxin = read(`${parts.dir}/${parts.name}.tsxin`, 'string');
    html = parseHTML(html as string);

    const output = utils.replace(tsxin as string, null, {
        CONTENT: html
    }, {
        begin: '<%',
        end: '%>'
    });

    write(`${parts.dir}/${parts.name}.tsx`, output);
}

let options = (yargs: CLI.Argv) => defaultOptions(yargs);
// npm run build ; node ./build/main.js watch --input=../pages
export const register = (cli: CLI.Argv) => {
    return cli.command('watch', '', options, async (argv: CLI.Arguments) => {
        if (argv.help) { return; }
        const src = path.resolve('' + argv.input);
        const watcher = chokidar.watch(`${src}/**/*.md`, {
            ignored: /(^|[\/\\])\../, // ignore dotfiles
            persistent: true
        });
        if (argv.debug) {
            debug(`Watching ${src}`);
        }
        watcher
            .on('change', path => {
                convertFiles([path]);
                updateTSX(path);
            })
    });
};
