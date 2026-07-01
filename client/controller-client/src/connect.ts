import { updateProgram } from './progtable.ts'

let port: SerialPort | undefined;

declare global {
  interface HTMLButtonElement {
    port?: SerialPort;
  }
}

const encoder = new TextEncoder();
const decoder = new TextDecoder();
const prog_re = /PR (\d+) "([^"]*)"/;

export async function writeProgram(n: number, title: string) {
    if (port && port.writable) {
        const writer = port.writable.getWriter();
        let s = 'PR ' + String(n) + ' "' + title + '"\n';
        console.log('write: ' + s);
        writer.write( encoder.encode(s) );
        writer.releaseLock();
    }
}

async function readLoop() {
    let s = ``;
    while (port && port.readable) {
        const reader = port.readable.getReader();
        const res = await reader.read();
        s += decoder.decode(res.value);
        let nl_pos = s.indexOf('\n');
        while (nl_pos >= 0) {
            const pr = s.substring(0, nl_pos);
            console.log('read: ' + pr);
            let m = pr.match(prog_re);
            if (m) {
                updateProgram(Number(m[1]), m[2])
            }
            s = s.substring(nl_pos + 1);
            nl_pos = s.indexOf('\n');
        }
        reader.releaseLock();
    }
}

async function writeLoop() {
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

