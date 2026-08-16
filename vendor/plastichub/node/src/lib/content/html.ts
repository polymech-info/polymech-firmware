import { GIT_REPO } from '../../constants';
import { html_beautify } from 'js-beautify';
export const img = (file, label, id = '') => {
    return `<div class="thumb">
            <a href="${file}" _target="_blank" >
                <img id="${id}" src="${file}" width="100%" />
            </a>
            <span class="thumb-label">${label}</span>
        </div>`;
}

export const changelog_entry = (e) => {
    return `<div class="change_log_entry">
        <span><pre>${e.date}&nbsp;</pre></span><span><a href="${GIT_REPO}/commit/${e.hash}">${e.msg}</a></span>
        <ul>
        ${e.files.map((f) => {
            return `<li><a href="${GIT_REPO}/blob/master/${f.path}">${f.path}</a></li>`
        })}
        </ul>
    </div>
    `
}

export const changelog = (log: any[]) => {
    return html_beautify(log.map(changelog_entry).join('<br/>'));
}