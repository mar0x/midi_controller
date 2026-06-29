let port: SerialPort | undefined;

declare global {
  interface HTMLButtonElement {
    port?: SerialPort;
  }
}

const encoder = new TextEncoder();
const decoder = new TextDecoder();

export function setupConnect(element: HTMLButtonElement) {
  const doConnect = async () => {
    if (!port) {
        element.innerHTML = `Connecting ...`
        port = await navigator.serial.requestPort({});
        await port.open({ baudRate: 115200 })
        element.innerHTML = `Disconnect`
        element.className = `disconnect`
        element.port = port;

        if (port.writable) {
            const writer = port.writable.getWriter();
            writer.write( encoder.encode('PR\n') );
            writer.releaseLock();
        }

        if (port.readable) {
            const reader = port.readable.getReader();
            const res = await reader.read();
            console.log(decoder.decode(res.value));
            reader.releaseLock();
        }
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

