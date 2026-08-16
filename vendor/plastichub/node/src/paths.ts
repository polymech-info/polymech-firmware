import * as path from 'path';
import { URL } from 'url';
export const STATS_SUFFIX = '_stats.json';
export const TRACE_SUFFIX = '_trace.json';
// utils to create output file name for url, format : hostname_time.json
const _url_short = (url: string) =>
    new URL(url).hostname;

const _date_suffix = () =>
    new Date().toLocaleTimeString().replace(/:/g, '_');

const _default_filename = (url: string) =>
    `${_url_short(url)}_${_date_suffix()}`;

export const default_path = (cwd: string, url: string) =>
    `${path.join(cwd, _default_filename(url))}${STATS_SUFFIX}`;

export const default_trace_path = (cwd: string, url: string) =>
    `${path.join(cwd, _default_filename(url))}${TRACE_SUFFIX}`;