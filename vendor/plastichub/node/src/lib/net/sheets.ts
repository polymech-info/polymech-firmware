import * as readline from 'readline';
import { google } from 'googleapis';
import * as path from 'path';
import { sync as readFile } from '@xblox/fs/read';
import { sync as writeFile } from '@xblox/fs/write';

// https://developers.google.com/sheets/api/quickstart/nodejs
// If modifying these scopes, delete token.json.

const SCOPES = ['https://www.googleapis.com/auth/spreadsheets.readonly'];
// The file token.json stores the user's access and refresh tokens, and is
// created automatically when the authorization flow completes for the first
// time.
const TOKEN_PATH = 'token.json';

/**
 * Get and store new token after prompting for user authorization, and then
 * execute the given callback with the authorized OAuth2 client.
 * @param {google.auth.OAuth2} oAuth2Client The OAuth2 client to get token for.
 * @param {getEventsCallback} callback The callback for the authorized client.
 */
const getNewToken = async (oAuth2Client) => {
    const authUrl = oAuth2Client.generateAuthUrl({
        access_type: 'offline',
        scope: SCOPES,
    });
    console.log('Authorize this app by visiting this url:', authUrl);
    const rl = readline.createInterface({
        input: process.stdin,
        output: process.stdout,
    });
    rl.question('Enter the code from that page here: ', (code) => {
        rl.close();
        oAuth2Client.getToken(code, (err, token) => {
            if (err) return console.error('Error while trying to retrieve access token', err);
            oAuth2Client.setCredentials(token);
            // Store the token to disk for later program executions
            writeFile(TOKEN_PATH, JSON.stringify(token, null, 2));
            return oAuth2Client;
        });
    });
}

const readSheet = async (auth, sheet, range) => {
    const sheets = google.sheets({ version: 'v4', auth });

    const res = await sheets.spreadsheets.values.get({
        spreadsheetId: sheet,
        range: range,
    });
    return res.data.values;
}

/**
 * Create an OAuth2 client with the given credentials, and then execute the
 * given callback function.
 * @param {Object} credentials The authorization client credentials.
 * @param {function} callback The callback to call with the authorized client.
 */
const authorize = async (credentials: any) => {
    const { client_secret, client_id, redirect_uris } = credentials.installed;
    const oAuth2Client = new google.auth.OAuth2(client_id, client_secret, redirect_uris[0]);

    const token = readFile(TOKEN_PATH, 'string') as string;
    if (!token) {
        return getNewToken(oAuth2Client);
    }

    oAuth2Client.setCredentials(JSON.parse(token));
    return oAuth2Client;
}

export const read = async (sheet: string, range: string) => {
    const creds = readFile(path.resolve('credentials.json'), 'json');
    const client = await authorize(creds);
    return await readSheet(client, sheet, range);
}