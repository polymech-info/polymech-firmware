import { debug } from "../../log";

/**
 * This example demonstrates how to use [`PuppeteerCrawler`](../api/puppeteercrawler)
 * in combination with [`RequestQueue`](../api/requestqueue) to recursively scrape the
 * <a href="https://news.ycombinator.com" target="_blank">Hacker News website</a> using headless Chrome / Puppeteer.
 * The crawler starts with a single URL, finds links to next pages,
 * enqueues them and continues until no more desired links are available.
 * The results are stored to the default dataset. In local configuration, the results are stored as JSON files in `./apify_storage/datasets/default`
 *
 * To run this example on the Apify Platform, select the `Node.js 10 + Chrome on Debian (apify/actor-node-chrome)` base image
 * on the source tab of your actor configuration.
 */

const Apify = require('apify');
// https://davehakkens.nl/community/forums/topic/arbor-press-v14/
// post with pics & videos : https://davehakkens.nl/community/forums/topic/launching-kickstarter-campaign-rwristwatches/
export async function crawler(url: string = 'https://davehakkens.nl/community/forums/topic/the-big-electronics-topic/') {
    // Apify.openRequestQueue() is a factory to get a preconfigured RequestQueue instance.
    // We add our first request to it - the initial page the crawler will visit.
    const requestQueue = await Apify.openRequestQueue();
    await requestQueue.addRequest({ url: url });

    // Create an instance of the PuppeteerCrawler class - a crawler
    // that automatically loads the URLs in headless Chrome / Puppeteer.
    const crawler = new Apify.PuppeteerCrawler({
        requestQueue,

        // Here you can set options that are passed to the Apify.launchPuppeteer() function.
        launchPuppeteerOptions: {
            // For example, by adding "slowMo" you'll slow down Puppeteer operations to simplify debugging
            slowMo: 500,
            headless: false,
            devtools: true
        },

        // Stop crawling after several pages
        maxRequestsPerCrawl: 2,

        // This function will be called for each URL to crawl.
        // Here you can write the Puppeteer scripts you are familiar with,
        // with the exception that browsers and pages are automatically managed by the Apify SDK.
        // The function accepts a single parameter, which is an object with the following fields:
        // - request: an instance of the Request class with information such as URL and HTTP method
        // - page: Puppeteer's Page object (see https://pptr.dev/#show=api-class-page)
        handlePageFunction: async ({ request, page }) => {
            console.log(`Processing ${request.url}...`);

            // A function to be evaluated by Puppeteer within the browser context.

            const t = new Promise((resolve) => {
                const pageFunction = ($posts) => {
                    const data = [];
                    const $ = window['jQuery'];
                    const jQuery = $;
                    const otherReplies = $('.bbp-reply-topic-title');
                    let title;
                    let authorLink;
                    let postDate;
                    let postBody;
                    let authorName;
                    try {
                        title = $('#bbpress-forums > div.topic-lead > div.author > h1')[0].innerText;
                    } catch (e) {
                        debugger;
                    }
                    try {

                        authorLink = $('#bbpress-forums > div.topic-lead > div.author > a:nth-child(5)')[0];
                        authorName = authorLink.innerText;
                    } catch (e) {
                        authorName = "fucking G";
                        authorLink = "ban G"

                    }

                    postDate = $('#bbpress-forums > div.topic-lead > div.author > div.date')[0].innerText.split(' at')[0];
                    postBody = $('#bbpress-forums > div.topic-lead > div.content').html();
                    const likes = parseInt(jQuery('#bbpress-forums > div.topic-lead > div.actions > div > div.dav_topic_like')[0].innerText.split(' ')[0]);
                    const saved = parseInt(jQuery('#bbpress-forums > div.topic-lead > div.actions > div > div.dav_topic_favorit > span')[0].innerText.split(' ')[0]);
                    const nbReplies = parseInt(jQuery('#bbpress-forums > div.topic-lead > div.actions > div > div.dav_reply_topic > span')[0].innerText.split(' ')[0]);
                    const pics = [];
                    jQuery('.d4p-bbp-attachment > a').each((i, a) => {
                        pics.push(jQuery(a).attr('href').replace('?ssl=1', ''));
                    });
                    console.log('page function');

                    const replies = [];

                    jQuery('#bbpress-forums > div.list-replies-container > div.list-replies > div.topic-reply').each((i, e) => {
                        try {
                            const authorLogo = jQuery('.author > a > img').attr('srcset').replace(' 2x');
                            const authorName = jQuery('.content .replyheader .smallusername', e)[0].innerText || 'anonymous';
                            const replyDate = jQuery('.content .replyheader .reply-date', e)[0].innerText;
                            const nbLikes = parseInt(jQuery('.content > div.wpulike.wpulike-heart > div > span', e)[0].innerText) || 0;
                            jQuery('.content > div.wpulike.wpulike-heart',e).remove();
                            jQuery('.content .replyheader',e).remove();
                            let replyBody = jQuery('.content',e).html();
                            const replyPics = [];
                            jQuery('.d4p-bbp-attachment > a',e).each((i, a) => {
                                replyPics.push(jQuery(a).attr('href').replace('?ssl=1', ''));
                            });
                            jQuery('.content .bbp-attachments',e).remove();
                            replyBody = jQuery('.content',e).html();
                            replies.push({
                                authorLogo,
                                authorName,
                                replyDate,
                                nbLikes,
                                replyBody,
                                replyPics
                            })
                        } catch (e) {
                            console.error('mah',e);
                            debugger;
                        }
                    })


                    debugger;
                    /*
    
                    // We're getting the title, rank and URL of each post on Hacker News.
                    $posts.forEach(($post) => {
                        data.push({
                            title: $post.querySelector('.title a').innerText,
                            rank: $post.querySelector('.rank').innerText,
                            href: $post.querySelector('.title a').href,
                        });
                    });
                    */
                    return data;
                };
                page.$$eval('return window', pageFunction);
            });
            return t;

            // Store the results to the default dataset.
            // await Apify.pushData(data);

            // Find a link to the next page and enqueue it if it exists.
            /*
            const infos = await Apify.utils.enqueueLinks({
                page,
                requestQueue,
                selector: '.morelink',
            });

            if (infos.length === 0) {
                console.log(`${request.url} is the last page!`);
            }*/

        },

        // This function is called if the page processing failed more than maxRequestRetries+1 times.
        handleFailedRequestFunction: async ({ request }) => {
            console.log(`Request ${request.url} failed too many times`);
            await Apify.pushData({
                '#debug': Apify.utils.createRequestDebugInfo(request),
            });
        },
        gotoFunction: async ({ session, page, request }) => {
            console.log('goto ' + request.url);
            return page.goto(request.url, {
                waitUntil: 'domcontentloaded'
            });
        }
    });

    // Run the crawler and wait for it to finish.
    await crawler.run();
};