import { InplaceEdit } from './inplace_edit.ts'

function onCellKeydown(e: KeyboardEvent) {
    const cell = e.target as EditableCell;

    if (e.key == 'Enter' || e.key == ' ') {
        cell.click();
    }
}

function onCellClick(e: Event) {
    const cell = e.target as EditableCell;
    if (cell) {
        const text = cell.textContent;

        const edit = document.createElement('input', { is: 'inplace-edit' }) as InplaceEdit;
        if (edit) {
            edit.orig_value = text;
            edit.value = text;

            const itype = cell.getAttribute('input-type');
            if (itype == 'channel') {
                console.log(`type = ${itype}`)
                edit.addEventListener('input', () => {
                    edit.value = edit.value.replace(/[^0-9]/, '');
                });
            }
            //const width: number = cell.getBoundingClientRect().width;
            //edit.style.width = `${width - 20}px`;

            cell.textContent = '';
            cell.appendChild(edit);

            edit.focus();
        }
    }
}

export class EditableCell extends HTMLTableCellElement {
    constructor() {
        super();

        this.tabIndex = 0;

        this.addEventListener('keydown', onCellKeydown);
        this.addEventListener('click', onCellClick);
    }
}

customElements.define('editable-cell', EditableCell, { extends: 'td' });

declare global {
  interface HTMLElementTagNameMap {
    'editable-cell': EditableCell;
  }
}

