import { write, exists, read, thumbs, images, resize_images, tail_image, howto_header, substitute, toHTML, parse_config, read_fragments } from '../../lib/';
import * as debug from '../..';
import { GIT_CHANGELOG_MESSAGE_PREFIX } from '../../constants';
import * as path from 'path';

import * as simpleGit from 'simple-git/promise';
import { SimpleGit, ListLogSummary } from 'simple-git';
import * as moment from 'moment';

export async function git_status(cwd, dir) {

    const git: SimpleGit = simpleGit(cwd);
    let statusSummary: ListLogSummary = null;
    try {
        statusSummary = await git.log(['--stat', path.resolve(dir)]);
    }
    catch (e) {
        debug.error('Error Git', e);
    }
    return statusSummary;
}

export async function git_log(cwd, dir) {
    const stats = await git_status(cwd, dir);
    let changelogs = stats.all.filter((e) => e.message.trim().toLowerCase().startsWith(GIT_CHANGELOG_MESSAGE_PREFIX.toLowerCase()));
    if (!changelogs.length) {
        return [];
    }
    let pretty = changelogs.map((e) => 
    {
        return {
            files: e.diff.files.map((f)=>{ return {path:f.file}}),
            msg: e.message.toLowerCase().replace(GIT_CHANGELOG_MESSAGE_PREFIX.toLowerCase(), '').trim(),
            hash: e.hash,
            date: moment(e.date).format('LLLL')
        }
    });
    return pretty;
};