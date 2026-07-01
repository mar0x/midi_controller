declare namespace vrEmuLcd {
    export enum CharacterRom {
        Eurpoean,
        Japanese
    }

    export interface LcdInstance {
        sendCommand(command: number): void;
        writeByte(data: number): void;
        writeString(str: string): void;
        render(ctx: CanvasRenderingContext2D, x: number, y: number, width: number, height: number): void;
        // Add other methods as needed: readByte, readAddress, pixelState, etc.
    }

    export function setLoadedCallback(callback: () => void): void;
    export function newLCD(cols: number, rows: number, rom: CharacterRom): LcdInstance;
}

// Global lcd command constants
declare const LCD_CMD_CLEAR: number;
declare const LCD_CMD_HOME: number;
declare const LCD_CMD_ENTRY: number;
declare const LCD_CMD_DISPLAY: number;
declare const LCD_CMD_CURSOR: number;
declare const LCD_CMD_FUNCTION: number;
declare const LCD_CMD_CGRAM: number;
declare const LCD_CMD_DDRAM: number;
declare const LCD_CMD_DISPLAY_ON: number;
declare const LCD_CMD_DISPLAY_OFF: number;

export function setupLcd() {
  const canvas = document.getElementById('lcdCanvas') as HTMLCanvasElement;
  const ctx = canvas.getContext('2d');

  if (ctx) {
      vrEmuLcd.setLoadedCallback(() => {
          // Initialize 16x2 LCD
          const lcd = vrEmuLcd.newLCD(16, 2, vrEmuLcd.CharacterRom.Eurpoean);

          // Turn on display
          lcd.sendCommand(LCD_CMD_DISPLAY | LCD_CMD_DISPLAY_ON);

          // Write text to LCD
          lcd.writeString("Hello, World!");

          // Render to canvas
          lcd.render(ctx, 0, 0, 400, 200);
      });
  }
}
