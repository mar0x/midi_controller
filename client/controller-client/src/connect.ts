import { updateProgram, updateProfile, selectProgram } from './progtable.ts'
import { updateDeviceTitle, updateMIDIChannel, updateProgramStart,
         updateChannelStart, updateDeviceVersion,
         updateMIDIForward } from './main.ts'
import { printLcd } from './vrEmuLcd.ts'
import { Splitter } from './splitter.ts'
import { ProgramDesc } from './program.ts'
import { ProfileDesc } from './profile.ts'
import { PortQueue } from './port_queue.ts'
import { Flasher } from './avr109.ts'
import { settings } from './settings.ts'

let connect_btn: HTMLButtonElement | undefined;
let port: SerialPort | undefined;
let reader: ReadableStreamDefaultReader<Uint8Array> | undefined;

let port_queue: PortQueue | undefined;

const decoder = new TextDecoder();
const display_re = /^D (\d+) (.{16})(.{16})/;
const prog_change_re = /^PC (\d+) (\d+)/;
const midi_channel_re = /^MC (\d+)/;
const midi_forward_re = /^MF (\d+)/;
const prog_start_re = /^PS (\d+)/;
const chan_start_re = /^CS (\d+)/;
const device_title_re = /^DT "([^"]*)"/;
const version_re = /^V "BD ([^"]*)" "BT ([^"]*)" "([^"]*)" "([^"]*)" "([^"]*)"/;

async function disconnect() {
    if (port) {
        try {
            await port.close();
        } catch (err: unknown) {
            console.log("Port close", err);
        }

        port = undefined;
        if (port_queue) {
            port_queue.disconnect();
            port_queue = undefined;
        }
    }

    if (connect_btn) {
        connect_btn.textContent = 'Connect'
    }
}

function sendLine(s: string): boolean {
    if (port_queue) {
        port_queue.push(s);
        return true;
    }

    return false;
}

export function sendProgram(prd: ProgramDesc) {
    if (!sendLine(prd.serialLine())) {
        updateProgram(prd);
    }
}

export function sendProfile(pfd: ProfileDesc) {
    if (!sendLine(pfd.serialLine())) {
        updateProfile(pfd);
    }
}

export function sendSelectProgram(profile_id: number, prog_id: number) {
    if (!sendLine(`PC ${profile_id} ${prog_id}`)) {
        selectProgram(profile_id, prog_id);
    }
}

export function sendDeviceTitle(t: string) {
    if (!sendLine(`DT "${t}"`)) {
        updateDeviceTitle(t);
    }
}

export function sendMIDIChannel(c: number) {
    if (!sendLine(`MC ${c}`)) {
        updateMIDIChannel(c);
    }
}

export function sendMIDIForward(f: boolean) {
    if (!(settings.firmware_version >= '2026.08.09' &&
          sendLine(`MF ${f ? 1 : 0}`))) {
        updateMIDIForward(f);
    }
}

export function sendProgramStart(s: number) {
    if (!sendLine(`PS ${s}`)) {
        updateProgramStart(s);
    }
}

export function sendChannelStart(s: number) {
    if (!sendLine(`CS ${s}`)) {
        updateChannelStart(s);
    }
}

async function readLoop() {
    const lines = new Splitter();
    lines.shift = true;
    let profile_no = 0;
    let max_prog_id = 0;

    while (port && port.readable) {
        reader = port.readable.getReader();
        let res;
        try {
            res = await reader.read();
        } catch (err: unknown) {
            await disconnect();

            if (!reader) {
                break;
            }
        }
        reader.releaseLock();
        reader = undefined;

        if (res) {
            lines.add(decoder.decode(res.value));
        }

        while (lines.next()) {
            const pr = lines.line;

            if (port_queue) {
                port_queue.onRead(pr);
            }

            //console.log('read: ' + pr);
            const prd = ProgramDesc.parseSerialLine(pr);
            if (prd) {
                updateProgram(prd);
                if (max_prog_id < prd.prog_id) {
                    max_prog_id = prd.prog_id;
                }
                continue;
            }

            const pfd = ProfileDesc.parseSerialLine(pr);
            if (pfd) {
                updateProfile(pfd);
                if (profile_no == 1) {
                    if (pfd.v0) {
                        if (max_prog_id >= 199) {
                            updateDeviceVersion('0', '2026.05.29');
                        } else {
                            updateDeviceVersion('0', '0');
                        }
                    } else {
                        sendLine('V');
                    }
                }
                ++profile_no;
                continue;
            }

            let m = pr.match(display_re);
            if (m) {
                printLcd(m[2], m[3]);
                continue;
            }

            m = pr.match(prog_change_re);
            if (m) {
                selectProgram(Number(m[1]), Number(m[2]));
                continue;
            }

            m = pr.match(midi_channel_re);
            if (m) {
                updateMIDIChannel(Number(m[1]));
                continue;
            }

            m = pr.match(midi_forward_re);
            if (m) {
                updateMIDIForward(Number(m[1]) != 0);
                continue;
            }

            m = pr.match(prog_start_re);
            if (m) {
                updateProgramStart(Number(m[1]));
                continue;
            }

            m = pr.match(chan_start_re);
            if (m) {
                updateChannelStart(Number(m[1]));
                continue;
            }

            m = pr.match(device_title_re);
            if (m) {
                updateDeviceTitle(m[1]);
                continue;
            }

            m = pr.match(version_re);
            if (m) {
                settings.serial_num = m[5];
                updateDeviceVersion(m[3], m[4]);
                if (settings.firmware_version >= '2026.08.09') {
                    sendLine('MF');
                }
                continue;
            }
        }
    }
}

function writeLoop() {
    sendLine('D 200');
    sendLine('DT');
    sendLine('MC')
    sendLine('PS');
    sendLine('CS');

    for (let profile_id = 0; profile_id < 3; profile_id++) {
        sendLine(`PF ${profile_id}`);

        for (let i = 0; i < 200; i++) {
            sendLine(`PR ${profile_id} ${i}`);
        }

        sendLine(`PC ${profile_id}`);
    }
}

async function connect(e: MouseEvent) {
    const btn = e.target as HTMLButtonElement;

    if (!port) {
        btn.textContent = `Connecting ...`
        try {
            port = await navigator.serial.requestPort(
                  { filters: [{ usbVendorId: 0x2341 }] });
            await port.open({ baudRate: 115200 })
        } catch (err: unknown) {
            port = undefined;
            btn.textContent = `Connect`
            return;
        }

        btn.textContent = `Disconnect`
        port_queue = new PortQueue(port);

        port_queue.flushLoop();
        readLoop();
        writeLoop();

        //await Promise.all([readLoop(), writeLoop()]);
        //console.log("All operations completed.");
    } else {
        if (reader) {
            reader.releaseLock();
            reader = undefined;
        } else {
            await disconnect();
        }
    }
}

export function setupConnect(element: HTMLButtonElement) {
    connect_btn = element;
    element.addEventListener('click', connect);
    element.textContent = `Connect`;
}

export async function flashHex(hex: any) {
    console.log("hex: ", hex);

    if (reader) {
        reader.releaseLock();
        reader = undefined;
    } else {
        await disconnect();
    }

    let p: SerialPort | undefined;

    try {
        p = await navigator.serial.requestPort(
            { filters: [{ usbVendorId: 0x2341 }] });
        const info = p.getInfo();
        console.log('port #1: ', p, info);
        await p.open({ baudRate: 1200 })
    } catch (err: unknown) {
        console.log("Error: ", err);
        return;
    }

    await p.close();

    try {
        p = await navigator.serial.requestPort(
            { filters: [{ usbProductId: 0x37, usbVendorId: 0x2341 }] });
        const info = p.getInfo();
        console.log('port #2: ', p, info);
        await p.open({ baudRate: 57600 })
    } catch (err: unknown) {
        console.log("Error: ", err);
        return;
    }

    const f = new Flasher(p);
    await f.prepare();
    console.log('prepare Ok');

    //await f.readee();
    //console.log('readee Ok');

    await f.program(hex.data);
    console.log('program Ok');

    //await f.verify(hex.data);
    //console.log('verify Ok');
    await f.finish();
    console.log('Ok');

    await p.close();
}

