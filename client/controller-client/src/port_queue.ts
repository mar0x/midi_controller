const encoder = new TextEncoder();
const window_size = 128;

export class PortQueue {
    port: SerialPort;
    connected: boolean = true;
    queue: string[] = [];
    sent_queue: string[] = [];
    sent_queue_size: number = 0;

    promise: Promise<void>;
    queue_resolve!: () => void;

    constructor(port: SerialPort) {
        const pq = this;

        this.port = port;
        this.promise = new Promise<void>( (resolve) => {
            pq.queue_resolve = resolve;
        } );
    }

    resolve() {
        if (this.sent_queue_size <= window_size && this.queue.length > 0) {
            this.queue_resolve();
        }
    }

    push(s: string) {
        this.queue.push(s);
        this.resolve();
    }

    disconnect() {
        this.connected = false;
        this.queue_resolve();
    }

    onRead(s: string) {
        if (this.sent_queue.length > 0) {
            const first = this.sent_queue[0];
            if (s.startsWith(first) || s.startsWith("err:" + first)) {
                this.sent_queue.shift();
                this.sent_queue_size -= encoder.encode(first + "\n").length;
                this.resolve();
            }
        }
    }

    async flushLoop(): Promise<void> {
        const pq = this;
        const port = this.port;

        while (this.connected && port.writable) {
            await this.promise;
            this.promise = new Promise<void>( (resolve) => {
                pq.queue_resolve = resolve;
            } );

            while (this.connected &&
                   this.sent_queue_size <= window_size &&
                   this.queue.length > 0) {
                const line = this.queue[0];
                if (!line) {
                    continue;
                }

                const b = encoder.encode(line + "\n");

                try {
                    const writer = port.writable.getWriter();
                    await writer.write(b);
                    writer.releaseLock();
                } catch (err: unknown) {
                    if (err instanceof Error) {
                        console.error(err.stack);
                    }
                    console.log(`failed to send ${line}`);
                    break;
                }

                this.queue.shift();

                this.sent_queue.push(line);
                this.sent_queue_size += b.length;
            }
        }
    }
}
