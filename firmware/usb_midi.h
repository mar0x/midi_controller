
#pragma once

#include <stdint.h>
#include <PluggableUSB.h>

#include "serial_num.h"
#include "usb_midi_desc.h"

#include "storage.h"

namespace usb_midi {

struct event_t {
    uint8_t header;
    uint8_t byte1;
    uint8_t byte2;
    uint8_t byte3;

    void reset() { header = 0; }
    bool empty() const { return header == 0; }
    uint8_t jack() const { return (header & 0xF0) >> 4; }
    uint8_t size() const {
        static const uint8_t cin2size[16] =
            { 0, 0, 2, 3, 3, 1, 2, 3, 3, 3, 3, 3, 2, 2, 3, 1 };

        return cin2size[header & 0x0F];
    }
    bool sys_ex() const {
        uint8_t cin = header & 0x0F;
        return cin == 4 || cin == 6 || cin == 7 || cin == 15;
    }

    operator bool() const { return header != 0; }
};

/**
   Concrete MIDI implementation of a PluggableUSBModule
   By default, will define one midi in and one midi out enpoints.
 */
class port_t : public PluggableUSBModule
{
private:
    uint8_t rx_ep() const { return pluggedEndpoint; }
    uint8_t tx_ep() const { return pluggedEndpoint + 1; }

    uint8_t epType[2];   ///< Container that defines the two bulk MIDI IN/OUT endpoints types

protected:
    virtual bool setup(USBSetup& /* setup */) override {
        return false;
    }

    virtual int getInterface(uint8_t* interfaceNum) override {
        *interfaceNum += 2; // uses 2 interfaces

        MIDIDescriptor m = {
          D_IAD(pluggedInterface, 2, AUDIO_CLASS_CODE, SUBCLASS_AUDIOCONTROL, 0),
          D_INTERFACE(pluggedInterface, 0, AUDIO_CLASS_CODE, SUBCLASS_AUDIOCONTROL, 0),
          {(uint8_t) (pluggedInterface + 1)},

          D_INTERFACE((uint8_t) (pluggedInterface + 1), 2, AUDIO_CLASS_CODE, SUBCLASS_MIDISTREAMING, 0),
          {},

          {JACK_TYPE_EMBEDDED, 1},
          {JACK_TYPE_EMBEDDED, 2},
          {JACK_TYPE_EXTERNAL, 3},

          {4, 1},

          {JACK_TYPE_EMBEDDED, 5, 4},
          {JACK_TYPE_EMBEDDED, 6, 3},
          {JACK_TYPE_EXTERNAL, 7, 2},

          {USB_ENDPOINT_OUT(pluggedEndpoint)},
          {1, 2},

          {USB_ENDPOINT_IN(pluggedEndpoint + 1)},
          {5, 6},
        };

        m.idh2.wTotalLength = sizeof(MIDIDescriptor) - offsetof(MIDIDescriptor, idh2);

        return USB_SendControl(0, &m, sizeof(m));
    }

    virtual int getDescriptor(USBSetup&) override {
        return 0;
    }

    virtual uint8_t getShortName(char* name) override {
        name[0] = 'P';
        name[1] = 'M';
        name[2] = 'X';

        using storage = patch_mate::storage;
        using serial_num_t = patch_mate::serial_num_t;

        storage::read(* (serial_num_t *) (name + 3));

        return 3 + sizeof(serial_num_t);
    }

public:
    port_t() : PluggableUSBModule(2, 2, epType)
    {
        epType[0] = EP_TYPE_BULK_OUT;   // MIDI_ENDPOINT_OUT
        epType[1] = EP_TYPE_BULK_IN;    // MIDI_ENDPOINT_IN
        PluggableUSB().plug(this);
    }

    int available() const {
        return USB_Available(rx_ep()) / sizeof(event_t);
    }

    int recv(event_t& p) const {
        int c = USB_Recv(rx_ep(), &p, sizeof(p));

        if (c < (int) sizeof(p)) {
            p.reset();
            return 0;
        }

        return 1;
    }

    void flush(void) { USB_Flush(tx_ep()); }

    void send(const event_t& event) {
        USB_Send(tx_ep(), &event, sizeof(event));
        USB_Flush(tx_ep());
    }
};

port_t port;
}
