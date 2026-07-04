import { updateProgram, changeProgram } from './progtable.ts'
import { printLcd } from './vrEmuLcd.ts'
import { Splitter } from './splitter.ts'

let port: SerialPort | undefined;

declare global {
  interface HTMLButtonElement {
    port?: SerialPort;
  }
}

const encoder = new TextEncoder();
const decoder = new TextDecoder();
const prog_re = /^PR (\d+) "([^"]*)"/;
const display_re = /^D (.{16})(.{16})/;
const prog_change_re = /^PC (\d+)/;

export async function writeProgram(n: number, title: string) {
    if (port && port.writable) {
        const writer = port.writable.getWriter();
        let s = 'PR ' + String(n) + ' "' + title + '"\n';
        console.log('write: ' + s);
        writer.write( encoder.encode(s) );
        writer.releaseLock();
    } else {
        updateProgram(n, title);
    }
}

export async function selectProgram(n: number) {
    if (port && port.writable) {
        const writer = port.writable.getWriter();
        let s = 'PC ' + String(n) + '\n';
        console.log('write: ' + s);
        writer.write( encoder.encode(s) );
        writer.releaseLock();
    } else {
        changeProgram(n);
    }
}

async function readLoop() {
    const lines = new Splitter();
    lines.shift = true;

    while (port && port.readable) {
        const reader = port.readable.getReader();
        const res = await reader.read();

        lines.add(decoder.decode(res.value));
        while (lines.next()) {
            const pr = lines.line;
            console.log('read: ' + pr);
            let m = pr.match(prog_re);
            if (m) {
                updateProgram(Number(m[1]), m[2])
            }
            m = pr.match(display_re);
            if (m) {
                printLcd(m[1], m[2]);
            }
            m = pr.match(prog_change_re);
            if (m) {
                changeProgram(Number(m[1]));
            }
        }
        reader.releaseLock();
    }
}

async function writeLoop() {
    if (port && port.writable) {
        const writer = port.writable.getWriter();
        let s = 'D 200\n';
        console.log('write: ' + s);
        writer.write( encoder.encode(s) );
        writer.releaseLock();
    }

    for (let i = 0; i < 100; i++) {
        if (port && port.writable) {
            const writer = port.writable.getWriter();
            let s = 'PR ' + String(i) + '\n';
            console.log('write: ' + s);
            writer.write( encoder.encode(s) );
            writer.releaseLock();
        } else {
            break;
        }
    }

    if (port && port.writable) {
        const writer = port.writable.getWriter();
        let s = 'PC\n';
        console.log('write: ' + s);
        writer.write( encoder.encode(s) );
        writer.releaseLock();
    }
}

export function setupConnect(element: HTMLButtonElement) {
  const doConnect = async () => {
    if (!port) {
        element.innerHTML = `Connecting ...`
        port = await navigator.serial.requestPort({});
        await port.open({ baudRate: 115200 })
        element.innerHTML = `Disconnect`
        element.className = `disconnect`
        element.port = port;

        await Promise.all([readLoop(), writeLoop()]);
        console.log("All operations completed.");
    } else {
        await port.close();
        port = undefined;
        delete element.port;
        element.innerHTML = `Connect`
        element.className = `connect`
    }
  }
  element.addEventListener('click', () => doConnect())
  element.innerHTML = `Connect`
}

