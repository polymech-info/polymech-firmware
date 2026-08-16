import { default as chalk } from 'chalk';
import * as util from 'util';
import * as _ora from 'ora';
// tslint:disable-next-line:no-var-requires
const jsome = require('jsome');
jsome.level.show = true;
const glog = console.log;
export const log = (msg: string, ...rest: any[]) => glog(chalk.magenta(msg), ...rest);
export const info = (msg: string, ...rest) => glog(chalk.green(msg), ...rest);
export const error = (msg: string, ...rest: any[]) => glog(chalk.red(msg), ...rest);
export const warn = (msg: string, ...rest) => glog(chalk.yellow(msg), ...rest);
export const debug = (msg: string, ...rest) => glog(chalk.blue(msg), ...rest);
export const stack = (msg: string, ...rest) => glog(chalk.red(msg), new Error().stack);
export const inspect = (msg: string, d: any = null) => {
    glog(chalk.blue(msg));
    d && jsome(d);
};
export const spinner = (msg: string): any => _ora(msg);
