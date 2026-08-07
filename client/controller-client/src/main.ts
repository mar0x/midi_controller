import './style.css'
import 'bootstrap/dist/css/bootstrap.min.css';
import { setupConnect, sendDeviceTitle, sendMIDIChannel,
         sendProgramStart, sendChannelStart, flashHex } from './connect.ts'
import { onDownloadClick, onUploadClick, processFileText } from './progtable.ts'
import type { HTMLUploadButtonElement } from './progtable.ts'
import { setupLcd, printLcd } from './vrEmuLcd.ts'
import 'bootstrap';
import { EditableCell } from './editable_cell.ts';
import { settings } from './settings.ts'
import { parseIntelHex } from './intel_hex.ts'

export function invalidChannelFeedback() {
    return `Channel expected to be ${settings.channel_start}-${settings.channel_start + 15}`;
}

document.querySelector<HTMLDivElement>('#app')!.innerHTML = `
<nav class="navbar navbar-expand-lg sticky-top bg-body-tertiary">
  <div class="container-fluid">
    <a class="navbar-brand" href="#">MIDI Controller</a>
    <button class="navbar-toggler" type="button" data-bs-toggle="collapse" data-bs-target="#navbarNavAltMarkup" aria-controls="navbarNavAltMarkup" aria-expanded="false" aria-label="Toggle navigation">
      <span class="navbar-toggler-icon"></span>
    </button>

    <div class="collapse navbar-collapse" id="navbarNavAltMarkup">
      <div class="navbar-nav">
        <button id="demo-button" type="button" class="btn btn-primary me-1" disabled>Demo</button>
        <button id="connect-button" type="button" class="btn btn-primary me-1">Connect</button>
        <a class="btn btn-primary me-1" target="_blank" href="#" id="download-link">Download</a>
        <button id="upload-button" type="button" class="btn btn-primary me-1">Upload</a>
        <button id="update-button" type="button" class="btn btn-primary me-1">Update</a>
      </div>
    </div>
  </div>
</nav>

<div id="settings" class="row">
  <div class="col">
    <canvas id="lcdCanvas" width="400" height="100"></canvas>
  </div>
  <div class="col">

    <div class="accordion accordion-flush" id="device-settings-accordion">
      <div class="accordion-item">
        <h2 class="accordion-header">
          <button class="accordion-button collapsed" type="button" data-bs-toggle="collapse" data-bs-target="#device-settings-collapse" aria-expanded="false" aria-controls="flush-collapseOne">
            Device Settings
          </button>
        </h2>

        <div id="device-settings-collapse" class="accordion-collapse collapse" data-bs-parent="#device-settings-accordion">
          <div class="accordion-body">

            <table id="device-settings-table" class="table table-dark table-hover">
              <tbody>
                <tr><th>Title</th><td is="editable-cell" id="device-title">MIDI Controller</td></tr>
                <tr><th>In Channel</th>
                    <td is="editable-cell" id="device-channel" input-type="number">
                        <span class="editable-value">${settings.channel + settings.channel_start}</span>
                        <div class="invalid-feedback">${invalidChannelFeedback()}</div>
                    </td>
                </tr>
                <tr><th>MIDI Forward</th><td><input type="checkbox" checked/></td></tr>
                <tr><th>Channel Start</th><td>
                    <select class="form-select form-select-sm" id="device-channel-start">
                        <option value="0">0</option>
                        <option value="1" selected>1</option>
                    </select>
                    </td></tr>
                <tr><th>Program Start</th><td>
                    <select class="form-select form-select-sm" id="device-program-start">
                        <option value="0">0</option>
                        <option value="1" selected>1</option>
                    </select>
                </td></tr>
              </tbody>
            </table>

          </div>
        </div>

      </div>
    </div>
  </div>
</div>

<div class="ticks"></div>

<section id="programs-tabs" class="bg-dark" data-bs-theme="dark">
  <nav>
    <div class="nav nav-tabs" id="profile-tab" role="tablist"></div>
  </nav>
  <div class="tab-content" id="profile-tabContent"></div>
</section>
`

setupConnect(document.querySelector<HTMLButtonElement>('#connect-button')!)

const downloadLink = document.querySelector('#download-link') as HTMLAnchorElement;
downloadLink?.addEventListener('click', onDownloadClick);

const uploadButton = document.querySelector('#upload-button') as HTMLButtonElement;
uploadButton?.addEventListener('click', onUploadClick);

const updateButton = document.querySelector('#update-button') as HTMLButtonElement;
updateButton?.addEventListener('click', onUpdateClick);

setupLcd();
printLcd("Disconnected ...", "");

const dtitle = document.querySelector(`#device-title`) as EditableCell;
const dchannel = document.querySelector(`#device-channel`) as EditableCell;
const dprogstart = document.querySelector(`#device-program-start`) as HTMLSelectElement;
const dchanstart = document.querySelector(`#device-channel-start`) as HTMLSelectElement;

let hex: any = undefined;

const dropZone = document.querySelector('#app') as HTMLElement | null;
if (dropZone) {
    dropZone.addEventListener('dragover', (event: DragEvent) => {
        if (!event.dataTransfer) return;

        const items = event.dataTransfer.items;

        for (let i = 0; i < items.length; i++) {
            const item = items[i];

            if (item.kind == 'file' && item.type == 'text/csv') {
                event.preventDefault();
                dropZone.classList.add('bg-dark-subtle');
                return;
            } else if (item.kind == 'file' && item.type == 'text/x-hex') {
                event.preventDefault();
                dropZone.classList.add('bg-dark-subtle');
                return;
            }
        }

        event.dataTransfer.dropEffect = "none";
    });

    dropZone.addEventListener('dragleave', (event: DragEvent) => {
        event.preventDefault();
        dropZone.classList.remove('bg-dark-subtle');
    });

    dropZone.addEventListener('drop', async (event: DragEvent) => {
        event.preventDefault();
        dropZone.classList.remove('bg-dark-subtle');

        const files: FileList | undefined = event.dataTransfer?.files;

        if (files && files.length > 0) {
            if (files[0].type == "text/csv") {
                processFileText(files[0]);
            } else if (files[0].type == "text/x-hex") {
                const textContent: string = await files[0].text();
                hex = parseIntelHex(textContent);

                await flashHex(hex);
            }
        }
    });
}

async function onUpdateClick(e: MouseEvent) {
    const updateButton = e.target as HTMLUploadButtonElement;

    if (!updateButton) return;

    if (hex) {
        await flashHex(hex);
        return;
    }

    let updateFile = updateButton.fileInputElement;
    if (!updateFile) {
        updateFile = document.createElement('input') as HTMLInputElement;
        updateFile.type = 'file';
        updateFile.accept = 'text/x-hex';
        updateFile.multiple = false;
        updateFile.addEventListener('change', onUpdateFileChange);

        updateButton.fileInputElement = updateFile;
    }

    updateFile.click();
}

async function onUpdateFileChange(e: Event) {
    const updateFile = e.target as HTMLInputElement;

    if (!updateFile || !updateFile.files || updateFile.files.length === 0) {
        console.warn("No file selected.");
        return;
    }

    const selectedFile: File = updateFile.files[0];

    console.log(`File Name: ${selectedFile.name}`);
    console.log(`File Size: ${selectedFile.size} bytes`);
    console.log(`File Type: ${selectedFile.type}`);

    const textContent: string = await selectedFile.text();
    hex = parseIntelHex(textContent);

    await flashHex(hex);
}

export function updateDeviceTitle(t: string) {
    settings.title = t;
    dtitle.value = t;
}

export function updateMIDIChannel(c: number) {
    settings.channel = c;
    dchannel.value = `${settings.channel + settings.channel_start}`;
}

export function updateProgramStart(start: number) {
    if (start != settings.program_start) {
        settings.program_start = start;
        dprogstart.value = String(start);

        // TODO: update progtables
    }
}

export function updateChannelStart(start: number) {
    if (start != settings.channel_start) {
        settings.channel_start = start;
        dchanstart.value = String(start);

        updateMIDIChannel(settings.channel);
    }
}

function onDeviceTitleUpdate(e: Event) {
    const ce = e as CustomEvent<string>;

    console.log(`deviceTitleUpdate: ${ce.detail}`);

    sendDeviceTitle(ce.detail);
}
dtitle.addEventListener('textUpdate', onDeviceTitleUpdate);

function onDeviceChannelUpdate(e: Event) {
    const ce = e as CustomEvent<string>;

    console.log(`deviceChannelUpdate: ${ce.detail}`);

    let channel = Number(ce.detail) - settings.channel_start;
    if (channel >= 0 && channel < 16) {
        sendMIDIChannel(channel);
    } else {
        const cell = e.target as EditableCell;
        if (cell) {
            cell.feedbackTextContent = invalidChannelFeedback();
        }
        e.preventDefault();
    }
}
dchannel.addEventListener('textUpdate', onDeviceChannelUpdate);

function onDeviceProgramStartUpdate(e: Event) {
    const target = e.target as HTMLSelectElement;
    sendProgramStart(Number(target.value));
}
dprogstart.addEventListener('change', onDeviceProgramStartUpdate);

function onDeviceChannelStartUpdate(e: Event) {
    const target = e.target as HTMLSelectElement;
    sendChannelStart(Number(target.value));
}
dchanstart.addEventListener('change', onDeviceChannelStartUpdate);
