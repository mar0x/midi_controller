declare namespace vrEmuLcd {
    export enum CharacterRom {
        Eurpoean,
        Japanese
    }

    export type ColorScheme = {
      BackColor: string;
      PixelOnColor: string;
      PixelOffColor: string;
    };

    export interface LcdInstance {
        sendCommand(command: number): void;
        writeByte(data: number): void;
        writeString(str: string): void;
        render(ctx: CanvasRenderingContext2D, x: number, y: number, width: number, height: number): void;
        readAddress(): number;
        // Add other methods as needed: readByte, readAddress, pixelState, etc.

        colorScheme: ColorScheme;
    }

    export function setLoadedCallback(callback: () => void): void;
    export function newLCD(cols: number, rows: number, rom: CharacterRom): LcdInstance;
}

let lcd: vrEmuLcd.LcdInstance;
let ctx: CanvasRenderingContext2D;

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
declare const LCD_CMD_SET_DRAM_ADDR: number;
declare const LCD_CMD_SHIFT: number;
declare const LCD_CMD_SHIFT_RIGHT: number;
declare const LCD_CMD_SHIFT_CURSOR: number;


const WhiteOnBlack: vrEmuLcd.ColorScheme = {
    BackColor: "#212225",
    PixelOnColor: "#f0f0f0",
    PixelOffColor: "#303030"
  };

export function setupLcd() {
  const canvas = document.getElementById('lcdCanvas') as HTMLCanvasElement;
  ctx = canvas.getContext('2d') as CanvasRenderingContext2D;

  if (ctx) {
      vrEmuLcd.setLoadedCallback(() => {
          // Initialize 16x2 LCD
          lcd = vrEmuLcd.newLCD(16, 2, vrEmuLcd.CharacterRom.Eurpoean);
          lcd.colorScheme = WhiteOnBlack;

          // Turn on display
          lcd.sendCommand(LCD_CMD_DISPLAY | LCD_CMD_DISPLAY_ON);

          // Render to canvas
          lcd.render(ctx, 0, 0, 400, 100);
      });
  }
}

export function printLcd(l1: string, l2: string) {
  if (lcd) {
      lcd.sendCommand(LCD_CMD_SET_DRAM_ADDR);
      lcd.writeString(l1);

      lcd.sendCommand(LCD_CMD_SET_DRAM_ADDR | 40);
      lcd.writeString(l2);

      if (ctx) {
          // Render to canvas
          lcd.render(ctx, 0, 0, 400, 100);
      }
  }
}
