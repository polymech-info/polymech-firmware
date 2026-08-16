# build environment
FROM node:16 as builder
WORKDIR /usr/src/app
COPY frontend/package*.json ./
RUN npm install
# layers should be cached till here, no npm install on rebuild

COPY frontend .
RUN npm run build


# production environment
FROM node:16
WORKDIR /usr/src/app
COPY mock/package*.json ./
RUN npm install
# layers should be cached till here, no npm install on rebuild

COPY mock .
COPY --from=builder /usr/src/app/build public
EXPOSE 80
CMD [ "node", "MockServer.js" ]
