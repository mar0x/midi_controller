export class ProfileDesc {
    id: number;
    title: string;
    channel: number;
    port_mask: number;

    constructor(id: number, title: string,
                channel: number, port_mask: number) {
        this.id = id;
        this.title = title;
        this.channel = channel;
        this.port_mask = port_mask;
    }

    clone(): ProfileDesc {
        return new ProfileDesc(this.id, this.title, this.channel, this.port_mask);
    }

    static readonly serial_re = /^PF (\d+) "([^"]*)" (\d+) (\d+)/;

    static parseSerialLine(s: string): ProfileDesc | undefined {
        const m = s.match(ProfileDesc.serial_re);
        if (m) {
            return new ProfileDesc(Number(m[1]), m[2], Number(m[3]), Number(m[4]));
        }

        return undefined;
    }

    serialLine(): string {
        return `PF ${this.id} "${this.title}" ${this.channel} ${this.port_mask}`;
    }

    static readonly import_re = /^PF;(\d+);([^;]*);(\d+);(\d+);?/;

    static parseImportLine(s: string): ProfileDesc | undefined {
        const m = s.match(ProfileDesc.import_re);
        if (m) {
            return new ProfileDesc(Number(m[1]), m[2], Number(m[3]), Number(m[4]));
        }

        return undefined;
    }

    exportLine(): string {
        return `PF;${this.id};${this.title};${this.channel};${this.port_mask};`;
    }

    extraColumnTitle(): string {
        if (this.id == 0) {
            return "Bank";
        }
        if (this.id == 1) {
            return "Patch #";
        }
        if (this.id == 2) {
            return "Preset #";
        }
        return "";
    }
}
