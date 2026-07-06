export class ProgramDesc {
    title: string;
    in_id: number;
    out_id: number;

    static parseSerialLine(s: string): ProgramDesc | undefined {
    }

    static parseImportLine(s: string): ProgramDesc | undefined {
    }

    serialLine(): string {
        return `PR ${this.out_id} "${this.title}"`;
    }

    exportLine(): string {
        return `PR;${this.out_id};"${this.title}"`;
    }
}
