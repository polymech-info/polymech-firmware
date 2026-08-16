import { capitalize } from "../common/strings";

export const howto_header = (title, category, image, description: string = "", tagline: string = "", config: string = "") => {
   return `---
image: ${image}
category: "${category}"
title: "${title}"
tagline: ${tagline || '""' }
description: ${description || `"Precious Plastic - Howto : ${category} :: ${title} "` }
${config}
---\n`;
}

export const gallery_image = (path, title= "", alt ="") =>{
   return `
 - url: "${path}"
   image_path: "${path}"
   alt: "${alt}"
   title: "${title}"`;
}

export const drawing_image = (path, pdf, title= "", alt ="") =>{
   return `
 - url: "${pdf}"
   image_path: "${path}"
   alt: "${alt}"
   title: "${title}"`;
}


export const machine_header = (title, category, image, slug, rel, description: string = "", tagline: string = "", config: string = "") => {
   return `---
image: ${image}
category: "${category}"
title: "${title}"
product_rel: "/${rel}"
tagline: ${tagline || '""'}
description: ${description || `"Precious Plastic - Machine : ${capitalize(category)} :: ${title}"` }
${config}
sidebar: 
   nav: "machines"
---\n`;
}

export const projects_header = (title, category, image, slug, rel, description: string = "", tagline: string = "", config: string = "") => {
   return `---
image: ${image}
category: "${category}"
title: "${title}"
product_rel: "/${rel}"
tagline: ${tagline || '""'}
description: ${description || `"Precious Plastic - ${capitalize(category)} :: ${title}"` }
${config}
sidebar: 
   nav: "projects"
---\n`;
}
