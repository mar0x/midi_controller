import './style.css'
import 'bootstrap/dist/css/bootstrap.min.css';
import typescriptLogo from './assets/typescript.svg'
import viteLogo from './assets/vite.svg'
// import heroImg from './assets/hero.png'
import { setupConnect, sendDeviceTitle, sendMIDIChannel,
         sendProgramStart, sendChannelStart } from './connect.ts'
import { onDownloadClick, onUploadClick, onUploadFileChange, processFileText } from './progtable.ts'
import { setupLcd, printLcd } from './vrEmuLcd.ts'
import 'bootstrap';
import { EditableCell } from './editable_cell.ts';
import { settings } from './settings.ts'

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
        <button id="upload-button" type="button" class="btn btn-primary">Upload</a>
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
                <tr><th>In Channel</th><td is="editable-cell" id="device-channel" input-type="channel">1</td></tr>
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

<section id="next-steps">
  <div id="docs">
    <svg class="icon" role="presentation" aria-hidden="true"><use href="/icons.svg#documentation-icon"></use></svg>
    <h2>Documentation</h2>
    <p>Your questions, answered</p>
    <ul>
      <li>
        <a href="https://vite.dev/" target="_blank">
          <img class="logo" src="${viteLogo}" alt="" />
          Explore Vite
        </a>
      </li>
      <li>
        <a href="https://www.typescriptlang.org" target="_blank" id="download-link-">
          <img class="button-icon" src="${typescriptLogo}" alt="">
          Download
        </a>
      </li>
      <li>
        <input type="file" id="upload-file" name="fileUpload">
      </li>
    </ul>
  </div>
  <div id="social">
    <svg class="icon" role="presentation" aria-hidden="true"><use href="/icons.svg#social-icon"></use></svg>
    <h2>Connect with us</h2>
    <p>Join the Vite community</p>
    <ul>
      <li><a href="https://github.com/vitejs/vite" target="_blank"><svg class="button-icon" role="presentation" aria-hidden="true"><use href="/icons.svg#github-icon"></use></svg>GitHub</a></li>
      <li><a href="https://chat.vite.dev/" target="_blank"><svg class="button-icon" role="presentation" aria-hidden="true"><use href="/icons.svg#discord-icon"></use></svg>Discord</a></li>
      <li><a href="https://x.com/vite_js" target="_blank"><svg class="button-icon" role="presentation" aria-hidden="true"><use href="/icons.svg#x-icon"></use></svg>X.com</a></li>
      <li><a href="https://bsky.app/profile/vite.dev" target="_blank"><svg class="button-icon" role="presentation" aria-hidden="true"><use href="/icons.svg#bluesky-icon"></use></svg>Bluesky</a></li>
    </ul>
  </div>
</section>

<div class="ticks"></div>
<section id="spacer"></section>
`

setupConnect(document.querySelector<HTMLButtonElement>('#connect-button')!)

const downloadLink = document.querySelector('#download-link') as HTMLAnchorElement;
downloadLink?.addEventListener('click', onDownloadClick);

const uploadButton = document.querySelector('#upload-button') as HTMLButtonElement;
uploadButton?.addEventListener('click', onUploadClick);

const uploadFile = document.querySelector('#upload-file') as HTMLInputElement;
uploadFile?.addEventListener('change', onUploadFileChange);

setupLcd();
printLcd("Disconnected ...", "");

const dtitle = document.querySelector(`#device-title`) as EditableCell;
const dchannel = document.querySelector(`#device-channel`) as EditableCell;
const dprogstart = document.querySelector(`#device-program-start`) as HTMLSelectElement;
const dchanstart = document.querySelector(`#device-channel-start`) as HTMLSelectElement;

const dropZone = document.querySelector('#app') as HTMLElement | null;
if (dropZone) {
    dropZone.addEventListener('dragover', (event: DragEvent) => {
        event.preventDefault();
        dropZone.classList.add('bg-dark-subtle');
    });

    dropZone.addEventListener('dragleave', (event: DragEvent) => {
        event.preventDefault();
        dropZone.classList.remove('bg-dark-subtle');
    });

    dropZone.addEventListener('drop', (event: DragEvent) => {
        event.preventDefault();
        dropZone.classList.remove('bg-dark-subtle');

        const files: FileList | undefined = event.dataTransfer?.files;

        if (files && files.length > 0) {
            processFileText(files[0]);
        }
    });
}

export function updateDeviceTitle(t: string) {
    settings.title = t;
    dtitle.textContent = t;
}

export function updateMIDIChannel(c: number) {
    settings.channel = c;
    dchannel.textContent = `${settings.channel + settings.channel_start}`;
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
    if (channel < 0) channel = 0;
    if (channel > 15) channel = 15;

    sendMIDIChannel(channel);
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
