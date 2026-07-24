
function onInputKeydown(e: KeyboardEvent) {
    const input = e.target as InplaceEdit;

    if (e.key == 'Escape') {
        input.cancel();
    }

    if (e.key == 'Enter') {
        input.submit();
    }
}

function onFocusOut(e: FocusEvent) {
    const input = e.target as InplaceEdit;
    if (input) {
        input.cancel();
    }
}

export class InplaceEdit extends HTMLInputElement {
    removed: boolean = false;
    orig_value: string = '';

    constructor() {
        super();

        this.type = 'text';
        this.classList.add('form-control')
        this.classList.add('form-control-sm')

        this.addEventListener('keydown', onInputKeydown);
        this.addEventListener('focusout', onFocusOut);
    }

    cancel(): void {
        if (this.removed) return;

        this.removed = true;

        console.log('cancel');

        const pt = this.parentElement;
        if (pt) {
            pt.textContent = this.orig_value;

            this.remove();
        }
    }

    submit(): void {
        if (this.removed) return;

        this.removed = true;

        console.log('submit');

        const target = this.parentElement;

        if (target) {
            const e = new CustomEvent<string>('textUpdate', {
                detail: this.value,
                bubbles: true,
            });

            target.textContent = this.value;
            target.dispatchEvent(e);

            this.remove();
        }
    }
}

customElements.define('inplace-edit', InplaceEdit, { extends: 'input' });
