import { writeProgram, selectProgram } from './connect.ts'
import { Splitter } from './splitter.ts'

type ProgramType = {
  id: number;
  title: string;
  radio_btn: HTMLInputElement;
  cell: HTMLTableCellElement;
};

function programRow(p: ProgramType) {
  return p.cell.parentElement as HTMLTableRowElement;
}

const programs: Record<number, ProgramType> = { };
let cur_prog_id: number = 0;

declare global {
  interface HTMLInputElement {
    program: ProgramType;

    cancel(): void;
    submit(): void;
  }

  interface HTMLTableCellElement {
    program: ProgramType;
  }
}

HTMLInputElement.prototype.cancel = function (this: HTMLInputElement) {
  console.log('cancel');

  const parent = this.parentElement as HTMLTableCellElement;
  this.remove();

  if (parent) {
    parent.textContent = this.program.title;
  }

  active_input = undefined;
}

HTMLInputElement.prototype.submit = function (this: HTMLInputElement) {
  const parent = this.parentElement as HTMLTableCellElement;
  const new_text = this.value;
  this.remove();

  active_input = undefined;

  if (parent) {
    parent.textContent = new_text;
    parent.program.title = new_text;
    writeProgram(parent.program.id, new_text);
  }
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
  const cell = e.target as HTMLTableCellElement;
  if (cell) {
    const text = cell.textContent;

    if (active_input) {
      if (active_input === cell as HTMLElement) {
          return;
      }

      const active_cell = active_input.parentElement as HTMLTableCellElement;
      if (active_cell === cell) {
          return;
      }

      active_input.cancel();
    }

    const input = document.createElement('input') as HTMLInputElement;

    if (input) {
      input.type = 'text';
      input.classList.add('form-control')
      input.classList.add('form-control-sm')
      input.value = text;
      input.program = cell.program;

      input.addEventListener('keydown', onInputKeydown);

      cell.textContent = '';
      cell.appendChild(input);

      input.focus();

      active_input = input;
    }
  }
}

function onRadioBtnClick(e: Event) {
  const radio_btn = e.target as HTMLInputElement;

  if (radio_btn) {
    selectProgram(Number(radio_btn.value));
  }
}

export function updateProgram(n: number, title: string) {
  console.log('updateProgram: ' + String(n) + ' ' + title);
  let p: ProgramType = programs[n];
  if (p) {
    p.title = title;
    p.cell.textContent = title;

    // TODO: update edit

    return;
  }

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

      const radio_btn = document.createElement('input') as HTMLInputElement;
      if (radio_btn) {
        radio_btn.type = 'radio';
        radio_btn.name = 'program';
        radio_btn.value = String(n);
      }

      p = { id: n, title: title, radio_btn: radio_btn, cell: cell2 };
      programs[n] = p;

      radio_btn.addEventListener('click', onRadioBtnClick);

      // 4. Assign text or HTML content to the cells
      cell0.appendChild(radio_btn);
      cell1.textContent = String(n);
      cell2.textContent = title;
      cell2.program = p;

      cell2.addEventListener('click', onCellClick);
    }
  }
}

export function changeProgram(n: number) {
  const p: ProgramType = programs[n];
  if (p) {
    p.radio_btn.checked = true;

    programRow(programs[cur_prog_id]).classList.remove("table-active");

    // TODO: uncheck current
    cur_prog_id = n;

    programRow(programs[cur_prog_id]).classList.add("table-active");
  }
}

export function onDownloadClick(e: MouseEvent) {
  const downloadLink = e.target as HTMLAnchorElement;

  if (downloadLink) {
    const entries = Object.entries(programs) as [string, ProgramType][];
    let content: string = '';

    entries.forEach(([n, p]) => {
      content += n + ";" + p.title + ";\n";
    });

    //const content: string = 'Test text content';
    const contentType: string = 'text/csv;charset=utf-8;';
    const filename: string = 'backup.csv';

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

const csv_line_re = /^\s*(\d+)\s*;([^;]*);?/

async function processFileText(file: File): Promise<void> {
    try {
        // Reads raw content into a string variable
        const textContent: string = await file.text();

        // If processing JSON data:
        if (file.type === "application/json") {
            const parsedData: Record<string, unknown> = JSON.parse(textContent);
            console.log("Parsed JSON:", parsedData);
        } else if (file.type === "text/csv") {
            console.log("CSV Text:", textContent);
            const lines = new Splitter(textContent);
            while (lines.next()) {
                const m = lines.line.match(csv_line_re);
                if (m) {
                    writeProgram(Number(m[1]), m[2]);
                }
            }
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
