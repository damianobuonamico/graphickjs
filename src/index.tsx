/* @refresh reload */
import './index.css';
import './wasm/loader';
import { render } from 'solid-js/web';

import App from './App';

render(() => <App />, document.getElementById('root') as HTMLElement);
