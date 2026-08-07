import { sendProgram, sendProfile, sendSelectProgram,
         sendDeviceTitle, sendMIDIChannel, sendProgramStart,
         sendChannelStart } from './connect.ts'
import { Splitter } from './splitter.ts'
import { invalidChannelFeedback } from './main.ts'
import Sortable from 'sortablejs';

import { ProgramDesc } from './program.ts'
import { ProfileDesc } from './profile.ts'
import { ProgramRowElement } from './program_row.ts'

import { EditableCell } from './editable_cell';

import { settings } from './settings.ts'

class ProfileTab {
    pfd: ProfileDesc;

    program_by_id: ProgramRowElement[] = [];
    program_by_seq: ProgramRowElement[] = [];

    cur_prog_id: number = 0;

    constructor(pfd: ProfileDesc) {
        this.pfd = pfd;
    }
}

const profiles: ProfileTab[] = [];

function animateRow(currentRow: Element, targetRow: Element) {
  const currentRect = currentRow.getBoundingClientRect();
  const targetRect = targetRow.getBoundingClientRect();

  const currentNewRect = targetRect;
  const currentOffset = currentRect.top - currentNewRect.top;

  currentRow.animate([
    { transform: `translateY(${currentOffset}px)` },
    { transform: 'translateY(0)' }
  ], {
    duration: 300,
    easing: 'ease-in-out'
  });
}

function insertProgramRow(tbody: HTMLTableSectionElement, pr: ProgramRowElement) {
    const pt = profiles[pr.prd.profile_id];
    const seq = pr.prd.seq_id;

    for (let i = seq - 1; i >= 0; i--) {
        const p = pt.program_by_seq[i];
        if (p) {
            p.after(pr);
            return;
        }
    }

    for (let i = seq + 1; i < pt.program_by_seq.length; i++) {
        const p = pt.program_by_seq[i];
        if (p) {
            tbody.insertBefore(pr, p);
            return;
        }
    }

    tbody.appendChild(pr);
}

export function moveProgram(profile_id: number, f: number, t: number, send: boolean = false, animate: boolean = false) {
    const pt = profiles[profile_id];
    if (!pt) {
        console.log(`Profile #${profile_id} not found`);
        return;
    }

    const pr = pt.program_by_seq[f];
    if (animate) {
        const from_row = pt.program_by_seq[f];
        const to_row = pt.program_by_seq[t];

        animateRow(from_row, to_row);
    }

    pt.program_by_seq.splice(f, 1);
    pt.program_by_seq.splice(t, 0, pr);

    const tbody = pr.parentElement as HTMLTableSectionElement;
    pr.remove();

    if (f > t) {
        [f, t] = [t, f];
    }

    for (let i = f; i <= t; i++) {
        const p = pt.program_by_seq[i];
        if (p) {
            p.set_seq(i);
        }
    }

    insertProgramRow(tbody, pr);

    if (send) {
        sendProgram(pr.prd);
    }
}

export function updateProgram(prd: ProgramDesc) {
    const pt = profiles[prd.profile_id];
    if (!pt) {
        console.log(`Profile #${prd.profile_id} not found`);
        return;
    }

    const pr = pt.program_by_id[prd.prog_id];
    if (pr) {
        pr.set_title(prd.title);

        if (prd.seq_id != pr.prd.seq_id) {
            moveProgram(prd.profile_id, pr.prd.seq_id, prd.seq_id);
        }
        return;
    }

    const tbody = document.querySelector(`#profile-${prd.profile_id}-program-tbody`) as HTMLTableSectionElement;
    if (tbody) {
        const pr = document.createElement('tr', { is: 'program-row' }) as ProgramRowElement;
        pr.attach(prd);

        pt.program_by_id[prd.prog_id] = pr;
        pt.program_by_seq[prd.seq_id] = pr;

        insertProgramRow(tbody, pr);
    }
}

export function selectProgram(profile_id: number, prog_id: number) {
    const pt = profiles[profile_id];
    if (pt) {
        const pr = pt.program_by_id[prog_id];
        if (pr) {
            pr.select_btn.checked = true;

            if (pt.program_by_id[pt.cur_prog_id]) {
                pt.program_by_id[pt.cur_prog_id].inactive();
            }
            pt.cur_prog_id = prog_id;
            if (pt.program_by_id[pt.cur_prog_id]) {
                pt.program_by_id[pt.cur_prog_id].active();
            }
        }
    }
}

export function onDownloadClick(e: MouseEvent) {
  const downloadLink = e.target as HTMLAnchorElement;

  if (downloadLink) {
    //const entries = Object.entries(programs) as [string, ProgramType][];
    let content: string = `
DT;${settings.title};
MC;${settings.channel};
PS;${settings.program_start};
CS;${settings.channel_start};

`;

    for (let f = 0; f < profiles.length; f++) {
        const pt = profiles[f];
        if (!pt) continue;

        content += `${pt.pfd.exportLine()}\n`;
        const prs = pt.program_by_seq;
        for (let s = 0; s < prs.length; s++) {
            const pr = prs[s];
            if (pr) {
                content += `${pr.prd.exportLine()}\n`;
            }
        }

        content += `PC;${f};${pt.cur_prog_id};

`;
    }

    const contentType: string = 'text/csv;charset=utf-8;';
    const filename: string = `backup-${settings.title}.csv`;

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

export interface HTMLUploadButtonElement extends HTMLButtonElement {
  fileInputElement: HTMLInputElement;
}

export function onUploadClick(e: MouseEvent) {
    const uploadButton = e.target as HTMLUploadButtonElement;

    if (!uploadButton) return;

    let uploadFile = uploadButton.fileInputElement;
    if (!uploadFile) {
        uploadFile = document.createElement('input') as HTMLInputElement;
        uploadFile.type = 'file';
        uploadFile.accept = 'text/csv';
        uploadFile.multiple = false;
        uploadFile.addEventListener('change', onUploadFileChange);

        uploadButton.fileInputElement = uploadFile;
    }

    uploadFile.click();
}

const prog_change_re = /^PC;(\d+);(\d+);/;
const midi_channel_re = /^MC;(\d+);/;
const prog_start_re = /^PS;(\d+);/;
const chan_start_re = /^CS;(\d+);/;
const device_title_re = /^DT;([^;]*);/;

export async function processFileText(file: File): Promise<void> {
    try {
        const textContent: string = await file.text();

        if (file.type === "text/csv") {
            const lines = new Splitter(textContent);
            while (lines.next()) {
                const prd = ProgramDesc.parseImportLine(lines.line);
                if (prd) {
                    sendProgram(prd);
                    continue;
                }

                const prf = ProfileDesc.parseImportLine(lines.line);
                if (prf) {
                    sendProfile(prf);
                    continue;
                }

                let m = lines.line.match(prog_change_re);
                if (m) {
                    sendSelectProgram(Number(m[1]), Number(m[2]));
                    continue;
                }

                m = lines.line.match(midi_channel_re);
                if (m) {
                    sendMIDIChannel(Number(m[1]));
                    continue;
                }

                m = lines.line.match(prog_start_re);
                if (m) {
                    sendProgramStart(Number(m[1]));
                    continue;
                }

                m = lines.line.match(chan_start_re);
                if (m) {
                    sendChannelStart(Number(m[1]));
                    continue;
                }

                m = lines.line.match(device_title_re);
                if (m) {
                    sendDeviceTitle(m[1]);
                    continue;
                }
            }
        }
    } catch (error) {
        console.error("Error reading file:", error);
    }
}

export function onUploadFileChange(e: Event) {
    const uploadFile = e.target as HTMLInputElement;

    if (!uploadFile || !uploadFile.files || uploadFile.files.length === 0) {
        console.warn("No file selected.");
        return;
    }

    const selectedFile: File = uploadFile.files[0];

    console.log(`File Name: ${selectedFile.name}`);
    console.log(`File Size: ${selectedFile.size} bytes`);
    console.log(`File Type: ${selectedFile.type}`);

    processFileText(selectedFile);
}

function onProfileTitleUpdate(e: Event, pfd: ProfileDesc) {
    const ce = e as CustomEvent<string>;

    const pfd_clone = pfd.clone();
    pfd_clone.title = ce.detail;

    sendProfile(pfd_clone);
}

function onProfileChannelUpdate(e: Event, pfd: ProfileDesc) {
    const ce = e as CustomEvent<string>;

    let channel = Number(ce.detail) - settings.channel_start;
    if (channel >= 0 && channel < 16) {
        const pfd_clone = pfd.clone();
        pfd_clone.channel = channel;

        sendProfile(pfd_clone);
    } else {
        const cell = e.target as EditableCell;
        if (cell) {
            cell.feedbackTextContent = invalidChannelFeedback();
        }
        e.preventDefault();
    }
}

function profileSettingsRow(id: number): string {
    const pt = profiles[id];

    const res = `
    <div id="profile-${id}-settings-row" class="row">
      <div class="col"></div>
      <div class="col">

        <div class="accordion accordion-flush"
             id="profile-${id}-settings-accordion">
          <div class="accordion-item">
            <h2 class="accordion-header">
              <button class="accordion-button collapsed" type="button"
                      data-bs-toggle="collapse"
                      data-bs-target="#profile-${id}-settings-collapse"
                      aria-expanded="false"
                      aria-controls="profile-${id}-settings-collapse">
                Profile Settings
              </button>
            </h2>

            <div id="profile-${id}-settings-collapse"
                 class="accordion-collapse collapse"
                 data-bs-parent="#profile-${id}-settings-accordion">
              <div class="accordion-body">
                <table id="profile-${id}-settings-table"
                       class="table table-dark table-hover">
                  <tbody>
                    <tr><th class="w10em">Title</th><td is="editable-cell" id="profile-${id}-title">${pt.pfd.title}</td></tr>
                    <tr><th>Out Channel</th>
                        <td is="editable-cell" id="profile-${id}-channel">
                            <span class="editable-value">${pt.pfd.channel + settings.channel_start}</span>
                            <div class="invalid-feedback">${invalidChannelFeedback()}</div>
                        </td>
                    </tr>
                    <tr><th>MIDI Forward</th><td><input type="checkbox" checked/></td></tr>
                  </tbody>
                </table>
              </div>
            </div>

          </div>
        </div>

      </div>
    </div>`;

    return res;
}

export function updateProfile(pfd: ProfileDesc) {
    const id = pfd.id;
    const title = pfd.title;

    let pt = profiles[id];
    if (pt) {
        pt.pfd.title = title;
        const button = document.querySelector<HTMLElement>(`#profile-${id}-tab`);
        if (button) {
            button.textContent = title;
        }
        const ptitle = document.querySelector(`#profile-${id}-title`) as EditableCell;
        if (ptitle) {
            ptitle.value = title;
        }
        pt.pfd.channel = pfd.channel;
        const pchannel = document.querySelector(`#profile-${id}-channel`) as EditableCell;
        if (pchannel) {
            pchannel.value = `${pfd.channel + settings.channel_start}`;
        }

        // TODO: update profile settings: port_mask etc.
        return;
    }

    const tab_class = (profiles.length == 0) ? 'active' : '';
    const content_class = (profiles.length == 0) ? 'show active' : '';

    profiles[id] = new ProfileTab(pfd);

    const profileTab = document.querySelector<HTMLDivElement>('#profile-tab');
    const profileTabContent = document.querySelector<HTMLDivElement>('#profile-tabContent');

    if (profileTab) {
        const tab_html = `
            <button class="nav-link fw-bold ${tab_class}" id="profile-${id}-tab"
                data-bs-toggle="tab"
                data-bs-target="#profile-${id}" type="button" role="tab"
                aria-controls="profile-${id}"
                aria-selected="false">${title}</button>`;

        profileTab.insertAdjacentHTML('beforeend', tab_html);
    }

    if (profileTabContent) {
        const extraColumnTitle = pfd.extraColumnTitle();
        const extraColumn = extraColumnTitle ? `<th scope="col" class="w10em">${extraColumnTitle}</th>` : '';
        const settings_row_html = profileSettingsRow(id);
        const content_html = `
            <div class="tab-pane fade ${content_class}" id="profile-${id}" role="tabpanel"
                aria-labelledby="profile-${id}-tab">
              ${settings_row_html}

              <div class="table-scrollable" id="profile-${id}-program-table-container">
                <table id="profile-${id}-program-table"
                       class="table table-hover">
                  <thead>
                    <tr>
                      <th scope="col" class="w5em">v</th>
                      <th scope="col" class="w5em">#</th>
                      ${extraColumn}
                      <th scope="col">Title</th>
                    </tr>
                  </thead>
                  <tbody id="profile-${id}-program-tbody">
                  </tbody>
                </table>
              </div>

            </div>`;

        profileTabContent.insertAdjacentHTML('beforeend', content_html);

        const ptitle = document.querySelector(`#profile-${id}-title`) as EditableCell;
        ptitle.addEventListener('textUpdate',
            (e) => {
                onProfileTitleUpdate(e, pfd);
            });

        const pchannel = document.querySelector(`#profile-${id}-channel`) as EditableCell;
        pchannel.addEventListener('textUpdate',
            (e) => {
                onProfileChannelUpdate(e, pfd);
            });

        const ptablecontainer = document.querySelector(`#profile-${id}-program-table-container`) as HTMLDivElement;
        const tbody = document.querySelector(`#profile-${id}-program-tbody`) as HTMLTableSectionElement;
        new Sortable(tbody, {
            animation: 150,
            scroll: ptablecontainer,
            scrollSensitivity: 50,
            // forceFallback: true,
            onEnd: (e) => {
                const de = (e as any).originalEvent?.dataTransfer?.dropEffect;
                const oldIndex = Number(e.oldIndex);
                const newIndex = Number(e.newIndex);

                if (newIndex == oldIndex) {
                    return;
                }

                if (de == 'move') {
                    moveProgram(id, oldIndex, newIndex, true, false);
                }
                else
                {
                    const tr = tbody.children[newIndex];
                    tr.remove();
                    tbody.insertBefore(tr, tbody.children[oldIndex]);
                }
            }
        });
    }
}
