export class Splitter {
    str: string;
    delimiter: string;
    line: string;
    line_end: number;
    shift: boolean;

    constructor(s: string = "", d: string = "\n", shift: boolean = false) {
        this.str = s;
        this.delimiter = d;
        this.line = '';
        this.line_end = -1;
        this.shift = shift;
    }

    add(s: string) {
        this.str += s;
    }

    next(): boolean {
        const d_pos = this.str.indexOf(this.delimiter, this.line_end + 1);
        if (d_pos >= 0) {
            this.line = this.str.substring(this.line_end + 1, d_pos);
            if (this.shift) {
                this.str = this.str.substring(d_pos + 1);
                this.line_end = -1;
            } else {
                this.line_end = d_pos;
            }
            return true;
        }

        this.line = this.str;
        return false;
    }
}
