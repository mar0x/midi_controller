import { ProgramDesc } from './program.ts'
import { EditableCell } from './editable_cell.ts'
import { sendSelectProgram, sendProgram } from './connect.ts'

const radio_btn_name_re = /^program-(\d+)-select/;

function onRadioBtnClick(e: Event) {
    const radio_btn = e.target as HTMLInputElement;

    if (radio_btn) {
        const m = radio_btn.name.match(radio_btn_name_re);
        if (m) {
            sendSelectProgram(Number(m[1]), Number(radio_btn.value));
        }
    }
}

function onTextUpdate(e: Event) {
    const ptitle = e.target as EditableCell;
    const ce = e as CustomEvent<string>;

    const pr = ptitle.parentElement as ProgramRowElement;

    console.log(`textUpdate: ${ce.detail}`);

    const prd = pr.prd.clone();
    prd.title = ce.detail;

    sendProgram(prd);
}

export class ProgramRowElement extends HTMLTableRowElement {
    prd!: ProgramDesc;
    select_btn!: HTMLInputElement;
    ptitle!: EditableCell;
    sequence_cell!: HTMLTableCellElement;

    attach(prd: ProgramDesc): void {
        this.prd = prd;

        const select_cell: HTMLTableCellElement = this.insertCell(0);
        this.sequence_cell = this.insertCell(1);
        this.sequence_cell.textContent = String(prd.seq_id + 1);

        const select_btn = document.createElement('input') as HTMLInputElement;
        if (select_btn) {
            select_btn.type = 'radio';
            select_btn.name = `program-${prd.profile_id}-select`;
            select_btn.value = `${prd.prog_id}`;

            select_cell.appendChild(select_btn);
            select_btn.addEventListener('click', onRadioBtnClick);

            this.select_btn = select_btn;
        }

        const extraValue = prd.extraColumnValue();
        if (extraValue) {
            const td = this.insertCell(2);
            td.textContent = extraValue;
        }

        const ptitle = document.createElement('td', { is: 'editable-cell' }) as EditableCell;
        if (ptitle) {
            ptitle.textContent = prd.title;
            this.appendChild(ptitle);

            this.ptitle = ptitle;
            ptitle.addEventListener('textUpdate', onTextUpdate);
        }
    }

    set_title(title: string) {
        this.prd.title = title;
        this.ptitle.textContent = title;
    }

    set_seq(seq_id: number) {
        if (this.prd.seq_id != seq_id) {
            this.prd.seq_id = seq_id;
            this.sequence_cell.textContent = String(seq_id + 1);
        }
    }

    active(): void {
        this.classList.add("table-active");
    }

    inactive(): void {
        this.classList.remove("table-active");
    }
}

customElements.define('program-row', ProgramRowElement, { extends: 'tr' });
