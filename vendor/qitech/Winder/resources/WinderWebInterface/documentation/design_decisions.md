## Design Decisions
Why are we using the less popular Preact instead of React?
- preact is smaller (advertises itself with 3KB only) and ESP is memory constrained
- both libraries are syntax compatible
- preact ships a full PWA with Service Workers out of the Box

Why do we use a custom webSocket implementation and not Socket.IO?
- it is hard to make a 1:1 API compatible C++ version
- socket.io is a little overhead
- our message format is kind of similar to socket.io

Why are we using Zustand and not Preact Context?
- Preact came to its limits when updating nested components via context
- all parent components were updated too although nothing changed
- with Zustand we can keep config in one Javascript object
- Zustand makes mutations easy and allows simple data persistence via local storage

Why are we using JavaScript and not Typescript?
- when the project was started we weren't too aware of typescript
- the project needed to be developed fast so it was best to have no overhead

Why does the webinterface communicate via WS and not HTTP requests?
- Websockets are meant to be faster
- the winder can just send info (like Wifi connection failed) without being asked for it

Why are there no unit tests?
- there just wasn't enough time to do it
- the project would need some restructuring for a good testing basis
- as it will run out of Service with the `Pro App` Unit Tests will never come in here :(

Why are we using a custom icon font?
- we didn't want to pack all FontAwesome icons inside the application
- the FA npm module is kind of ugly to use with an import required for every icon
- Icomoon makes it easy to create a custom font
