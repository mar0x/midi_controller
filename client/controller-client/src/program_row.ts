import { ProgramDesc } from './program.ts'
import { EditableCell } from './editable_cell.ts'
import { sendSelectProgram, sendProgram } from './connect.ts'
import { moveProgram } from './progtable.ts'

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

function onSequenceUpdate(e: Event) {
    const sequence_cell = e.target as EditableCell;
    const ce = e as CustomEvent<string>;

    const pr = sequence_cell.parentElement as ProgramRowElement;

    console.log(`sequenceUpdate: ${ce.detail}`);

    let seq_id = Number(ce.detail) - 1;
    const prd = pr.prd;

    if (seq_id >= 0 && seq_id < 200) {
        moveProgram(prd.profile_id, prd.seq_id, seq_id, true, true);
    } else {
        e.preventDefault();
    }
}

function onTitleUpdate(e: Event) {
    const ptitle = e.target as EditableCell;
    const ce = e as CustomEvent<string>;

    const pr = ptitle.parentElement as ProgramRowElement;

    console.log(`titleUpdate: ${ce.detail}`);

    const prd = pr.prd.clone();
    prd.title = ce.detail;

    sendProgram(prd);
}

export class ProgramRowElement extends HTMLTableRowElement {
    prd!: ProgramDesc;
    select_btn!: HTMLInputElement;
    ptitle!: EditableCell;
    sequence_cell!: EditableCell;

    attach(prd: ProgramDesc): void {
        this.prd = prd;

        const select_cell: HTMLTableCellElement = this.insertCell(0);

        const sequence_cell = document.createElement('td', { is: 'editable-cell' }) as EditableCell;
        if (sequence_cell) {
            this.sequence_cell = sequence_cell;
            sequence_cell.value = String(prd.seq_id + 1);
            sequence_cell.setAttribute('input-type', 'number');
            this.appendChild(sequence_cell);
            sequence_cell.addEventListener('textUpdate', onSequenceUpdate);
        }

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
            ptitle.value = prd.title;
            this.appendChild(ptitle);

            this.ptitle = ptitle;
            ptitle.addEventListener('textUpdate', onTitleUpdate);
        }

        this.addEventListener('dragend', (e: DragEvent) => {
            console.log(`dragend: ${e.dataTransfer?.dropEffect}`, e);
        });
    }

    set_title(title: string) {
        this.prd.title = title;
        this.ptitle.value = title;
    }

    set_seq(seq_id: number) {
        if (this.prd.seq_id != seq_id) {
            this.prd.seq_id = seq_id;
            this.sequence_cell.value = String(seq_id + 1);
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
