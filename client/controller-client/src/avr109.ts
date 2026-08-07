
const encoder = new TextEncoder();
const decoder = new TextDecoder();

const DEBUG_LEVEL = 7;
const INFO_LEVEL = 4;
const log_level = INFO_LEVEL;

function cmd(value: (string | number)[]): Uint8Array {
      const a = new Uint8Array(value.length);

      for (let i = 0; i < value.length; i++) {
          if (typeof value[i] === 'string') {
              const s: string = value[i] as string;
              a[i] = s.charCodeAt(0);
          }
          if (typeof value[i] == 'number') {
              a[i] = value[i] as number;
          }
      }

      return a;
}

export class Flasher {
    options: any = {};
    sp: SerialPort;
    signature: string;
    flashChunkSize: number = 0;

    constructor(sp: SerialPort, options: any = {}) {
        this.options = options;
        this.sp = sp;
        this.signature = this.options.signature || 'CATERIN';
    }

    public async write(b: Uint8Array): Promise<void> {
        if (this.sp.writable) {
            const writer = this.sp.writable.getWriter();
            await writer.write(b);
            writer.releaseLock();
        }
    }

    public async read(): Promise<Uint8Array> {
        if (this.sp.readable) {
            const reader = this.sp.readable.getReader();
            const res = await reader.read();
            reader.releaseLock();

            if (res && res.value) {
                if (log_level >= DEBUG_LEVEL) {
                    console.log('read: ', res.value);
                }
                return res.value;
            }
        }

        throw Error('Port is not readable');
    }

    public async c(value: string | Uint8Array | (string | number)[], expect: number = 1): Promise<Uint8Array> {
        if (typeof value === 'string') {
            value = encoder.encode(value);
        }

        if (Array.isArray(value)) {
            value = cmd(value);
        }

        if (log_level >= DEBUG_LEVEL) {
            const v = decoder.decode(value.slice(0, 1));
            console.log(`cmd: ${v}, write: `, value);
        }

        await this.write(value);

        let read_len = 0;
        const read_bufs = [];
        while (read_len < expect) {
            const res = await this.read();
            read_bufs.push(res);
            read_len += res.length;
        }

        if (read_bufs.length == 1) {
            return read_bufs[0];
        }

        const res = new Uint8Array(read_len);
        let start = 0;
        for (let i = 0; i < read_bufs.length; i++) {
            const buf = read_bufs[i];
            res.set(buf, start);
            start += buf.length;
        }

        return res;
    }

    public async prepare(): Promise<void> {
        let s = decoder.decode(await this.c('S', 7));
        if (log_level >= INFO_LEVEL) {
            console.log(`Signature = ${s}`);
        }
        if (s !== this.signature) {
            throw new Error(`Invalid device signature; expecting: ${this.signature}, received: ${s}`);
        }

        s = decoder.decode(await this.c('V', 2));
        if (log_level >= INFO_LEVEL) {
            console.log(`Software Version = ${s}`);
        }

        s = decoder.decode(await this.c('v'));
        if (log_level >= INFO_LEVEL) {
            console.log(`Hardware Version = ${s}`);
        }

        s = decoder.decode(await this.c('p'));
        if (log_level >= INFO_LEVEL) {
            console.log(`Programmer Type = ${s}`);
        }
        if (s !== 'S') {
            throw new Error(`Invalid programmer type; expecting: S, received: ${s}`);
        }

        s = decoder.decode(await this.c('a'));
        if (log_level >= INFO_LEVEL) {
            console.log(`Auto Increment = ${s}`);
        }
        if (s !== 'Y') {
            throw new Error(`Auto Increment is unsupported; expecting: Y, received: ${s}`);
        }

        let b = await this.c('b', 3);
        if (b[0] != 'Y'.charCodeAt(0)) {
            throw new Error('Buffered memory access not supported.');
        }
        this.flashChunkSize = (b[1] << 8) | b[2];
        if (log_level >= INFO_LEVEL) {
            console.log(`flashChunkSize = ${this.flashChunkSize}`);
        }

        await this.c('t');
        await this.c('TD');
        await this.c('P');

        b = await this.c('s', 3);
        if (log_level >= INFO_LEVEL) {
            console.log('Device signature: ', b);
        }
/*
        await this.c('F');
        await this.c('F');
        await this.c('F');

        await this.c('N');
        await this.c('N');
        await this.c('N');

        await this.c('Q');
        await this.c('Q');
        await this.c('Q');

        await this.c(['A', 0x03, 0xfc]);
        await this.c(['g', 0x00, 0x01, 'E']);

        await this.c(['A', 0x03, 0xff]);
        await this.c(['g', 0x00, 0x01, 'E']);

        await this.c(['A', 0x03, 0xff]);
        await this.c(['g', 0x00, 0x01, 'E']);

        await this.c(['A', 0x03, 0xff]);
        await this.c(['g', 0x00, 0x01, 'E']);
*/
    }

    public async erase(): Promise<void> {
        await this.c('e');
    }

    public async program(data: Uint8Array): Promise<void> {
        await this.c(['A', 0x00, 0x00]);

        for (let i = 0; i < data.length; i += this.flashChunkSize) {
            const chunk = data.slice(i, i + this.flashChunkSize);
            const c = cmd(['B', (chunk.length >> 8) & 0xFF, chunk.length & 0xFF, 'F']);
            const buf = new Uint8Array(c.length + chunk.length);
            buf.set(c, 0);
            buf.set(chunk, c.length);

            await this.c(buf);
        }
    }

    public async readee(): Promise<void> {
        await this.c(['A', 0x00, 0x00]);

        const c = ['g', (this.flashChunkSize >> 8) & 0xFF, this.flashChunkSize & 0xFF, 'E'];
        const vb = await this.c(c, this.flashChunkSize);

        console.log('readee: ', vb);
    }

    public async verify(data: Uint8Array): Promise<void> {
        await this.c(['A', 0x00, 0x00]);

        for (let i = 0; i < data.length; i += this.flashChunkSize) {
            const chunk = data.slice(i, i + this.flashChunkSize);
            // const c = ['g', (chunk.length >> 8) & 0xFF, chunk.length & 0xFF, 'F'];
            const c = ['g', (this.flashChunkSize >> 8) & 0xFF, this.flashChunkSize & 0xFF, 'F'];
            const vb = await this.c(c, this.flashChunkSize);
            for (let n = 0; n < chunk.length; n++) {
                if (vb[n] != chunk[n]) {
                    throw new Error('Firmware on the device does not match local data');
                }
            }
        }
    }

    public async finish(): Promise<void> {
        await this.c('L');
        await this.c('E');
    }

    public async fuseCheck(): Promise<void> {
        await this.c('F');
        await this.c('F');
        await this.c('F');

        await this.c('N');
        await this.c('N');
        await this.c('N');

        await this.c('Q');
        await this.c('Q');
        await this.c('Q');

        await this.c('L');
        await this.c('E');
    }
}
