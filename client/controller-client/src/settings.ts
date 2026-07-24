export class Settings {
    title: string = 'MIDI Controller';
    channel: number = 0;
    program_start: number = 1;
    channel_start: number = 1;
}

export const settings = new Settings();
