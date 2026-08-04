
//Intel Hex record types
export const DATA = 0,
  END_OF_FILE = 1,
  EXT_SEGMENT_ADDR = 2,
  START_SEGMENT_ADDR = 3,
  EXT_LINEAR_ADDR = 4,
  START_LINEAR_ADDR = 5;

const EMPTY_VALUE = 0xFF;

function decodeHexString(hex: string): Uint8Array {
    // Check for valid length
    if (hex.length % 2 !== 0) {
        throw new Error('Invalid hex string length');
    }

    const bytes = new Uint8Array(hex.length / 2);

    for (let i = 0; i < hex.length; i += 2) {
        // Extract a two-character chunk and convert to a number
        const v = parseInt(hex.substring(i, i + 2), 16);

        if (isNaN(v)) {
            throw new Error('Invalid hexadecimal character encountered');
        }

        bytes[i / 2] = v;
    }

    return bytes;
}

/* intel_hex.parse(data)
  - `data` - Intel Hex file (string in ASCII format or Buffer Object)
  - `bufferSize` - the size of the Buffer containing the data (optional)
    The data exceeding the buffer size will be discarded
  - `addressOffset` - starting address offset (optional)
    The data before the starting address will be discarded

  returns an Object with the following properties:

  - `data` - data as a Buffer Object, **padded with 0xFF
    where data is empty**.
  - `startSegmentAddress` - the address provided by the last
    start segment address record; null, if not given
  - `startLinearAddress` - the address provided by the last
    start linear address record; null, if not given

  Special thanks to: http://en.wikipedia.org/wiki/Intel_HEX
*/
export function parseIntelHex(data: string, bufferSize: number = 32768, addressOffset: number = 0) {
    // Initialization
    var buf = new Uint8Array(bufferSize),
        bufLength = 0, //Length of data in the buffer
        highAddress = 0, //upper address
        startSegmentAddress = null,
        startLinearAddress = null,
        lineNum = 0, //Line number in the Intel Hex string
        pos = 0; //Current position in the Intel Hex string

    const SMALLEST_LINE = 11;

    while (pos + SMALLEST_LINE <= data.length) {
        //Parse an entire line
        if (data.charAt(pos++) != ":") {
            throw new Error(`Line ${lineNum + 1} does not start with a colon (:).`);
        }

        lineNum++;

        // Number of bytes (hex digit pairs) in the data field
        const dataLength = parseInt(data.substr(pos, 2), 16); pos += 2;
        // Get 16-bit address (big-endian)
        const lowAddress = parseInt(data.substr(pos, 4), 16); pos += 4;
        // Record type
        const recordType = parseInt(data.substr(pos, 2), 16); pos += 2;
        // Data field (hex-encoded string)
        const dataField = data.substr(pos, dataLength * 2); pos += dataLength * 2;
        const dataFieldBuf = decodeHexString(dataField);
        // Checksum
        const checksum = parseInt(data.substr(pos, 2), 16); pos += 2;
        // Validate checksum
        var calcChecksum = (dataLength + (lowAddress >> 8) + lowAddress + recordType) & 0xFF;

        for (let i = 0; i < dataLength; i++) {
            calcChecksum = (calcChecksum + dataFieldBuf[i]) & 0xFF;
        }
        calcChecksum = (0x100 - calcChecksum) & 0xFF;

        if (checksum != calcChecksum) {
            throw new Error(`Invalid checksum on line ${lineNum}: got ${checksum}, but expected ${calcChecksum}`);
        }

        // Parse the record based on its recordType
        switch (recordType) {
        case DATA:
            var absoluteAddress = highAddress + lowAddress - addressOffset;
            // Expand buf, if necessary
            if (absoluteAddress + dataLength >= buf.length) {
                const tmp = new Uint8Array((absoluteAddress + dataLength) * 2);
                tmp.set(buf, 0);
                tmp.fill(0, buf.length, tmp.length);
                buf = tmp;
            }

            // Write over skipped bytes with EMPTY_VALUE
            if (absoluteAddress > bufLength) {
                buf.fill(EMPTY_VALUE, bufLength, absoluteAddress);
            }

            // Write the dataFieldBuf to buf
            buf.set(dataFieldBuf, absoluteAddress);
            bufLength = Math.max(bufLength, absoluteAddress + dataLength);
            // Safely abort if the buffer length is already sufficient
            if (bufLength >= bufferSize) {
                return {
                    "data": buf.slice(0, bufLength),
                    "startSegmentAddress": startSegmentAddress,
                    "startLinearAddress": startLinearAddress
                };
            }
            break;
        case END_OF_FILE:
            if (dataLength != 0) {
                throw new Error(`Invalid EOF record on line ${lineNum}.`);
            }
            return {
                "data": buf.slice(0, bufLength),
                "startSegmentAddress": startSegmentAddress,
                "startLinearAddress": startLinearAddress
            };
            break;
        case EXT_SEGMENT_ADDR:
            if (dataLength != 2 || lowAddress != 0) {
                throw new Error(`Invalid extended segment address record on line ${lineNum}.`);
            }
            highAddress = parseInt(dataField, 16) << 4;
            break;
        case START_SEGMENT_ADDR:
            if (dataLength != 4 || lowAddress != 0) {
                throw new Error(`Invalid start segment address record on line ${lineNum}.`);
            }
            startSegmentAddress = parseInt(dataField, 16);
            break;
        case EXT_LINEAR_ADDR:
            if (dataLength != 2 || lowAddress != 0) {
                throw new Error(`Invalid extended linear address record on line ${lineNum}.`);
            }
            highAddress = parseInt(dataField, 16) << 16;
            break;
        case START_LINEAR_ADDR:
            if (dataLength != 4 || lowAddress != 0) {
                throw new Error(`Invalid start linear address record on line ${lineNum}.`);
            }
            startLinearAddress = parseInt(dataField, 16);
            break;
        default:
            throw new Error(`Invalid record type (${recordType}) on line ${lineNum}`);
            break;
        }

        // Advance to the next line
        if (data.charAt(pos) == "\r") pos++;
        if (data.charAt(pos) == "\n") pos++;
    }
    throw new Error("Unexpected end of input: missing or invalid EOF record.");
}
