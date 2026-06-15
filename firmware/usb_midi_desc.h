
#pragma once

#include <stdint.h>
#include <PluggableUSB.h>

namespace usb_midi {

enum {
    MIDI_AUDIO = 0x01,          // USB interfaceClass
    MIDI_AUDIO_CONTROL = 0x01,  // USB interfaceSubClass
    MIDI_STREAMING = 0x03,      // USB interfaceSubClass

    // see "USB Device Class Definition for Audio Devices"
    // A.1 Audio Interface Class Code
    AUDIO_CLASS_CODE = 0x01,

    // A.2 Audio Interface Subclass Codes
    SUBCLASS_UNDEFINED = 0x00,
    SUBCLASS_AUDIOCONTROL = 0x01,
    SUBCLASS_AUDIOSTREAMING = 0x02,
    SUBCLASS_MIDISTREAMING = 0x03,

    // A.4 Audio Class-Specific Descriptor Types
    AUDIO_CS_UNDEFINED = 0x20,
    AUDIO_CS_DEVICE = 0x21,
    AUDIO_CS_CONFIGURATION = 0x22,
    AUDIO_CS_STRING = 0x23,
    AUDIO_CS_INTERFACE = 0x24,
    AUDIO_CS_ENDPOINT = 0x25,

    // A.5 Audio Class-Specific AC Interface Descriptor Subtypes
    AC_DESCRIPTOR_UNDEFINED = 0x00,
    AC_HEADER = 0x01,
    AC_INPUT_TERMINAL = 0x02,
    AC_OUTPUT_TERMINAL = 0x03,
    AC_MIXER_UNIT = 0x04,
    AC_SELECTOR_UNIT = 0x05,
    AC_FEATURE_UNIT = 0x06,
    AC_PROCESSING_UNIT = 0x07,
    AC_EXTENSION_UNIT = 0x08,

    // see "USB Device Class Definition for MIDI Devices"
    // A.1 MS Class-Specific Interface Descriptor Subtypes
    MS_DESCRIPTOR_UNDEFINED = 0x00,
    MS_HEADER = 0x01,
    MS_MIDI_IN_JACK = 0x02,
    MS_MIDI_OUT_JACK = 0x03,
    MS_ELEMENT = 0x04,

    // A.2 MS Class-Specific Endpoint Descriptor Subtypes
    //MS_DESCRIPTOR_UNDEFINED = 0x00,
    MS_GENERAL = 0x01,

    // A.3 MS MIDI IN and OUT Jack types
    JACK_TYPE_UNDEFINED = 0x00,
    JACK_TYPE_EMBEDDED = 0x01,
    JACK_TYPE_EXTERNAL = 0x02,

    MIDI_IN_JACK_SUBTYPE = 0x02, // aka AC_INPUT_TERMINAL
    MIDI_OUT_JACK_SUBTYPE = 0x03, // aka AC_OUTPUT_TERMINAL

    MIDI_CS_INTERFACE = 0x24, // aka AUDIO_CS_INTERFACE
    MIDI_CS_ENDPOINT = 0x25,  // AUDIO_CS_ENDPOINT

    MIDI_BUFFER_SIZE = USB_EP_SIZE, // 64
};

_Pragma("pack(1)")

/// Audio Control Interface Header Descriptor
struct ACIfcDesc {
    uint8_t len               = sizeof(ACIfcDesc);  // 9
    uint8_t dtype             = AUDIO_CS_INTERFACE; // 0x24
    uint8_t dsubType          = AC_HEADER;          // 0x01
    uint16_t bcdADc           = 0x0100;
    uint16_t wTotalLength     = 9;
    uint8_t bInCollection     = 1;
    uint8_t interfaceNumbers;

    ACIfcDesc(uint8_t num) :
      interfaceNumbers(num)
    { }
};

/// MIDI Stream Interface Header Descriptor
struct MSIfcDesc
{
    uint8_t len               = sizeof(MSIfcDesc);  // 7
    uint8_t dtype             = MIDI_CS_INTERFACE;  // 0x24
    uint8_t dsubType          = AC_HEADER;          // 0x01
    uint16_t bcdADc           = 0x0100;
    uint16_t wTotalLength     = sizeof(MSIfcDesc);  // 7
};

struct JackInDesc
{
    uint8_t len               = sizeof(JackInDesc); // 6
    uint8_t dtype             = MIDI_CS_INTERFACE;  // 0x24
    uint8_t dsubType          = MS_MIDI_IN_JACK;    // 0x02
    uint8_t jackType;
    uint8_t jackID;
    uint8_t jackStrIndex      = 0;

    JackInDesc(uint8_t t, uint8_t id) :
      jackType(t), jackID(id)
    { }
};

struct JackOutDesc
{
    uint8_t len               = sizeof(JackOutDesc);  // 9
    uint8_t dtype             = MIDI_CS_INTERFACE;    // 0x24
    uint8_t dsubType          = MS_MIDI_OUT_JACK;     // 0x03
    uint8_t jackType;
    uint8_t jackID;
    uint8_t nPins             = 1;

    uint8_t srcJackID;
    uint8_t srcPinID          = 1;

    uint8_t jackStrIndex      = 0;

    JackOutDesc(uint8_t t, uint8_t id, uint8_t srcID) :
        jackType(t), jackID(id), srcJackID(srcID)
    { }
};

/// Table 6-5: MIDI Element Descriptor

struct ElemDesc
{
    uint8_t len               = sizeof(ElemDesc);   // 10 + 2 + 2
    uint8_t dtype             = MIDI_CS_INTERFACE;  // 0x24
    uint8_t subtype           = MS_ELEMENT;         // 0x04
    uint8_t elemID;           // Constant uniquely identifying the MIDI OUT Jack within the USB-MIDI function.
    uint8_t inPins            = 1; // Number of Input Pins of this Element

    uint8_t srcJackID;        // ID of the Entity to which the first Input Pin of this Element is connected.
    uint8_t srcPinID          = 1;  // Output Pin number of the Entity to which the first Input Pin of this Element is connected.

    uint8_t outPins           = 1; // Number of Output Pins of this Element

    uint8_t inTerminal        = 0; // The Terminal ID of the Input Terminal to which this Element is connected
    uint8_t outTerminal       = 0; // The Terminal ID of the Output Terminal to which this Element is connected

    uint8_t capsSize          = 2;
    uint16_t caps             = 0x0101; // 'Custom Undefined Type' + 'MIDI Patch Bay'

    uint8_t elemStrIndex      = 0;

    ElemDesc(uint8_t id, uint8_t srcID) :
        elemID(id), srcJackID(srcID)
    { }
};

/// MIDI Jack EndPoint Descriptor, common to midi in and out jacks.
struct JackEPDesc
{
    EndpointDescriptor d;
    uint8_t refresh     = 0;
    uint8_t sync        = 0;;

    JackEPDesc(uint8_t addr) :
        d{
          sizeof(JackEPDesc),           // 9
          USB_ENDPOINT_DESCRIPTOR_TYPE, // 0x05 
          addr,
          USB_ENDPOINT_TYPE_BULK,       // 0x03
          MIDI_BUFFER_SIZE,             // 64
          0}
    { }
};

/// MIDI Jack  EndPoint AudioControl Descriptor, common to midi in and out ac jacks.
template<int n>
struct EPACDesc
{
    uint8_t len                 = sizeof(EPACDesc); // 5
    uint8_t dtype               = MIDI_CS_ENDPOINT; // 0x25
    uint8_t subtype             = MS_GENERAL;       // 0x01
    uint8_t embJacks            = 2;
    uint8_t jackID[2];

    EPACDesc(uint8_t id1, uint8_t id2) : jackID{id1, id2} { }
};

struct MIDIDescriptor
{
    IADDescriptor iad;
    InterfaceDescriptor id1;
    ACIfcDesc idh1;

    InterfaceDescriptor id2;
    MSIfcDesc idh2;

    JackInDesc jin1;
    JackInDesc jin2;
    JackInDesc jin3;

    ElemDesc el1;

    JackOutDesc jout1;
    JackOutDesc jout2;
    JackOutDesc jout3;

    JackEPDesc ep1;
    EPACDesc<2> ep_desc1;

    JackEPDesc ep2;
    EPACDesc<2> ep_desc2;
};

}
