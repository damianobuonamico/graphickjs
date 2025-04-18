# Graphick

[Graphick](http://graphickjs.vercel.app) is a demo graphic design software written in C++ for the web using WebAssembly. The UI is written in JS using SolidJS.

## Features

Some of the features already built are:

- Fast custom vector renderer (fills, strokes, images).
- Full history system.
- Basic editor tools.
- Zooming and panning infinite canvas.
- WASM to SolidJS UI synchronization.
- Basic SVG loading.
- Math and geometry utilities.

Some other features are buried deep into secondary branches:

- Freehand writing
- Text support
- Animation support

## Usage

```bash
$ npm install # or pnpm install or yarn install
```

In the project directory, you can run:

### `npm dev` or `npm start`

Runs the app in the development mode.<br>
Open [http://localhost:3000](http://localhost:3000) to view it in the browser.

The page will reload if you make edits.<br>

### `npm run build`

Builds the app for production to the `dist` folder.<br>

## Development

Run wasm-src/debug/setup.bat script to set up a development Visual Studio solution.

To compile WASM binaries run wasm-src/compile.py (make sure to have emscripten installed).

## Contributing

The project was built in my free time and it is not under current development. If someone is interested in evolving this project into opensource or something, just open a GitHub issue or discussion.
