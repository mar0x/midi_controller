export class Settings {
    title: string = 'MIDI Controller';
    channel: number = 0;
    program_start: number = 1;
    channel_start: number = 1;
    midi_forward: boolean = false;
    firmware_version: string = '0';
    hardware_version: string = '0';
    serial_num: string = '';
}

export const settings = new Settings();
