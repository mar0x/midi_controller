
const encoder = new TextEncoder();
const decoder = new TextDecoder();

const debug = false;

function cmd(value: (string | number)[]): Uint8Array {
      const a = new Uint8Array(value.length);

      for (let i = 0; i < value.length; i++) {
          if (typeof value[i] === 'string') {
              const s: string = value[i] as string;
              if (debug && i == 0) {
                  console.log(`cmd: ${value}`);
              }
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
    signature: string = "CATERIN";
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
                if (debug) {
                    console.log('read: ', res.value);
                }
                return res.value;
            }
        }

        throw Error('Port is not readable');
    }

    public async c(value: string | Uint8Array | (string | number)[], expect: number = 1): Promise<Uint8Array> {
        if (typeof value === 'string') {
            if (debug) {
                console.log(`cmd: ${value}`);
            }
            value = encoder.encode(value);
        }

        if (Array.isArray(value)) {
            value = cmd(value);
        }

        if (debug) {
            console.log('write: ', value);
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
        let s = decoder.decode(await this.c('S'));
        if (s !== this.signature) {
            throw new Error(`Invalid device signature; expecting: ${this.signature}, received: ${s}`);
        }
        await this.c('V');
        await this.c('v');
        await this.c('p');
        await this.c('a');

        let b = await this.c('b', 3);
        if (b[0] != 'Y'.charCodeAt(0)) {
            throw new Error('Buffered memory access not supported.');
        }
        this.flashChunkSize = (b[1] << 8) | b[2];
        if (debug) {
            console.log(`flashChunkSize = ${this.flashChunkSize}`);
        }

        await this.c('t');
        await this.c('ID');
        await this.c('P');

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

    public async verify(data: Uint8Array): Promise<void> {
        await this.c(['A', 0x00, 0x00]);

        for (let i = 0; i < data.length; i += this.flashChunkSize) {
            const chunk = data.slice(i, i + this.flashChunkSize);
            const c = ['g', (chunk.length >> 8) & 0xFF, chunk.length & 0xFF, 'F'];

            const vb = await this.c(c, chunk.length);
            for (let n = 0; n < chunk.length; n++) {
                if (vb[n] != chunk[n]) {
                    throw new Error('Firmware on the device does not match local data');
                }
            }
        }
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
