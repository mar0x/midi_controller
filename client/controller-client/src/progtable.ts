
declare global {
  interface HTMLInputElement {
    originalValue: string;

    cancel(): void;
    submit(): void;
  }

  interface HTMLTableCellElement {
    programId: number;
  }
}

HTMLInputElement.prototype.cancel = function (this: HTMLInputElement) {
  const parent = this.parentElement as HTMLTableCellElement;
  this.remove();

  if (parent) {
    parent.textContent = this.originalValue;
  }

  active_input = undefined;
}

HTMLInputElement.prototype.submit = function (this: HTMLInputElement) {
  const parent = this.parentElement as HTMLTableCellElement;
  const new_text = this.value;
  this.remove();

  if (parent) {
    parent.textContent = new_text;
  }

  active_input = undefined;
}

let active_input: HTMLInputElement | undefined;

function onInputKeydown(e: KeyboardEvent) {
  const input = e.target as HTMLInputElement;

  if (e.key == 'Escape') {
    input.cancel();
  }

  if (e.key == 'Enter') {
    input.submit();
  }
}

function onCellClick(e: Event) {
  const target = e.target as HTMLTableCellElement;
  if (target) {
    const text = target.textContent;

    if (active_input) {
      active_input.cancel();
    }

    const input = document.createElement('input') as HTMLInputElement;

    if (input) {
      input.type = 'text';
      input.classList.add('form-control')
      input.classList.add('form-control-sm')
      input.value = text;
      input.originalValue = text;

      input.addEventListener('keydown', onInputKeydown);

      target.textContent = '';
      target.classList.add('table-sm');
      target.appendChild(input);

      input.focus();

      active_input = input;
    }
  }
}

export function addProgram(n: number, title: string) {
  // 1. Fetch the table element and cast it to HTMLTableElement
  const table = document.getElementById("program-table") as HTMLTableElement;

  if (table) {
    // 2. Access the first <tbody> in the collection
    const tbody: HTMLTableSectionElement | undefined = table.tBodies[0];

    if (tbody) {
      // 2. Insert a new row at the end of the table (-1 appends to the end)
      const newRow: HTMLTableRowElement = tbody.insertRow(-1);

      // 3. Insert new cells into the newly created row
      const cell0: HTMLTableCellElement = newRow.insertCell(0);
      const cell1: HTMLTableCellElement = newRow.insertCell(1);
      const cell2: HTMLTableCellElement = newRow.insertCell(2);

      const input = document.createElement('input') as HTMLInputElement;
      if (input) {
        input.type = 'radio';
        input.name = 'program';
        input.value = String(n);
      }

      // 4. Assign text or HTML content to the cells
      cell0.appendChild(input);
      cell1.textContent = String(n);
      cell2.textContent = title;
      cell2.programId = n;

      cell2.addEventListener('click', onCellClick);
    }
  }
}

export function onDownloadClick(e: MouseEvent) {
  const downloadLink = e.target as HTMLAnchorElement;

  if (downloadLink) {
    const content: string = 'Test text content';
    const contentType: string = 'text/plain;charset=utf-8;';
    const filename: string = 'backup.txt';

    // 1. Create a Blob object from your data
    const blob: Blob = new Blob([content], { type: contentType });

    // 2. Generate a temporary, unique local URL pointing to the Blob
    const blobUrl: string = URL.createObjectURL(blob);

    // 3. Create a hidden <a> element
    downloadLink.href = blobUrl;
    downloadLink.download = filename;

    // 4. Append to DOM, click it to trigger download, and remove it immediately
    //document.body.appendChild(anchor);
    //anchor.click();
    //document.body.removeChild(anchor);

    // 5. Free up memory by revoking the Object URL
    //URL.revokeObjectURL(blobUrl);
  }
}

async function processFileText(file: File): Promise<void> {
    try {
        // Reads raw content into a string variable
        const textContent: string = await file.text();

        // If processing JSON data:
        if (file.type === "application/json") {
            const parsedData: Record<string, unknown> = JSON.parse(textContent);
            console.log("Parsed JSON:", parsedData);
        } else {
            console.log("Plain Text:", textContent);
        }
    } catch (error) {
        console.error("Error reading file:", error);
    }
}

export function onUploadFileChange(e: Event) {
// 3. Cast event.target to HTMLInputElement to access the FileList
    const uploadFile = e.target as HTMLInputElement;

    // 4. Implement strict null checks for safety
    if (!uploadFile || !uploadFile.files || uploadFile.files.length === 0) {
        console.warn("No file selected.");
        return;
    }

    // 5. Extract the single canonical File object
    const selectedFile: File = uploadFile.files[0];

    console.log(`File Name: ${selectedFile.name}`);
    console.log(`File Size: ${selectedFile.size} bytes`);
    console.log(`File Type: ${selectedFile.type}`);

    processFileText(selectedFile);
}
