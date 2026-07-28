export class ProgramDesc {
    profile_id: number;
    prog_id: number;
    title: string;
    seq_id: number;

    constructor(profile_id: number, prog_id: number,
                title: string, seq_id: number) {
        this.profile_id = profile_id;
        this.prog_id = prog_id;
        this.title = title;
        this.seq_id = seq_id;
    }

    clone(): ProgramDesc {
        return new ProgramDesc(this.profile_id, this.prog_id, this.title, this.seq_id);
    }

    static readonly serial_re = /^PR (\d+) (\d+) "([^"]*)" (\d+)/;

    static parseSerialLine(s: string): ProgramDesc | undefined {
        const m = s.match(ProgramDesc.serial_re);
        if (m) {
            return new ProgramDesc(Number(m[1]), Number(m[2]), m[3], Number(m[4]));
        }

        return undefined;
    }

    serialLine(): string {
        return `PR ${this.profile_id} ${this.prog_id} "${this.title}" ${this.seq_id}`;
    }

    static readonly import_re = /^PR;(\d+);(\d+);([^;]*);(\d+);?/;

    static parseImportLine(s: string): ProgramDesc | undefined {
        const m = s.match(ProgramDesc.import_re);
        if (m) {
            return new ProgramDesc(Number(m[1]), Number(m[2]), m[3], Number(m[4]));
        }

        return undefined;
    }

    exportLine(): string {
        return `PR;${this.profile_id};${this.prog_id};${this.title};${this.seq_id}`;
    }

    extraColumnValue(): string {
        if (this.profile_id == 0) {
            const bank_no = Math.trunc(this.prog_id / 2).toString().padStart(2, '0');
            const bank_ab = (this.prog_id % 2) ? 'B' : 'A';
            return `BANK${bank_no} ${bank_ab}, ${this.prog_id + 1}`;
        }
        if (this.profile_id == 1) {
            return `${this.prog_id + 1}`;
        }
        if (this.profile_id == 2) {
            return `${this.prog_id + 1}`;
        }
        return "";
    }
}
