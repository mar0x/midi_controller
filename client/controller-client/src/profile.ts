export class ProfileDesc {
    id: number;
    title: string;
    channel: number;
    port_mask: number;
    v0: boolean;
    program_count: number;

    constructor(id: number, title: string,
                channel: number, port_mask: number,
                program_count: number = 0) {
        this.id = id;
        this.title = title;
        this.channel = channel;
        this.port_mask = port_mask;
        this.v0 = program_count == 0;
        this.program_count = program_count;
    }

    clone(): ProfileDesc {
        return new ProfileDesc(this.id, this.title, this.channel, this.port_mask);
    }

    static readonly serial_v0_re = /^PF (\d+) "([^"]*)" (\d+) (\d+)/;
    static readonly serial_re = /^PF (\d+) "([^"]*)" (\d+) (\d+) (\d+)/;

    static parseSerialLineV0(s: string): ProfileDesc | undefined {
        const m = s.match(ProfileDesc.serial_v0_re);
        if (m) {
            return new ProfileDesc(Number(m[1]), m[2], Number(m[3]), Number(m[4]));
        }

        return undefined;
    }

    static parseSerialLine(s: string): ProfileDesc | undefined {
        const m = s.match(ProfileDesc.serial_re);
        if (m) {
            return new ProfileDesc(Number(m[1]), m[2], Number(m[3]), Number(m[4]), Number(m[5]));
        }

        const m0 = s.match(ProfileDesc.serial_v0_re);
        if (m0) {
            return new ProfileDesc(Number(m0[1]), m0[2], Number(m0[3]), Number(m0[4]));
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
