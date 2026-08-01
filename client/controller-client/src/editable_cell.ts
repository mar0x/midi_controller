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
        if (cell.tagName == 'INPUT' && cell.classList.contains('inplace-edit')) {
            e.preventDefault();
            return;
        }

        const text = cell.value;

        const edit = document.createElement('input', { is: 'inplace-edit' }) as InplaceEdit;
        if (edit) {
            edit.orig_value = text;
            edit.value = text;
            edit.classList.add("inplace-edit");

            const itype = cell.getAttribute('input-type');
            if (itype == 'number') {
                edit.addEventListener('input', () => {
                    edit.value = edit.value.replace(/[^0-9]/, '');
                });
            }

            cell.value = '';

            const feedback = cell.querySelector<HTMLElement>('.invalid-feedback');
            if (feedback) {
                cell.insertBefore(edit, feedback);
            } else {
                cell.appendChild(edit);
            }

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

    get value(): string {
        const editable = this.querySelector<HTMLElement>('.editable-value');
        if (editable) {
            return editable.textContent;
        } else {
            return this.textContent;
        }
    }

    set value(v: string) {
        const editable = this.querySelector<HTMLElement>('.editable-value');
        if (editable) {
            editable.textContent = v;
        } else {
            this.textContent = v;
        }
    }

    get feedbackTextContent(): string {
        const feedback = this.querySelector<HTMLElement>('.invalid-feedback');
        if (feedback) {
            return feedback.textContent;
        }
        return '';
    }

    set feedbackTextContent(v: string) {
        const feedback = this.querySelector<HTMLElement>('.invalid-feedback');
        if (feedback) {
            feedback.textContent = v;
        }
    }
}

customElements.define('editable-cell', EditableCell, { extends: 'td' });

declare global {
  interface HTMLElementTagNameMap {
    'editable-cell': EditableCell;
  }
}

