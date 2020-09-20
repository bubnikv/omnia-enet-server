// Peaberry CW - Transceiver for Peaberry SDR
// Copyright (C) 2015 David Turnbull AE9RB
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef CAT_H
#define CAT_H

#include <string>
#include <cstdint>

#include "libusb-1.0/libusb.h"

#include "Config.h"

#define USBDEV_SHARED_VENDOR    0x16C0  // VOTI  VID
#define USBDEV_SHARED_PRODUCT   0x05DC  // OBDEV PID 
#define VENDOR_NAME_OBDEV        "www.obdev.at"

#define IAMBIC_MODE_B       (1 << 0)
#define IAMBIC_SKEY         (1 << 1)
#define IAMBIC_AUTOSPACE    (1 << 2)
#define IAMBIC_RST_N        (1 << 7)

enum class CatCommandID : uint16_t {
    // Set local oscillator frequency in Hz.
    // int64_t frequency
    SetFreq,
    // Set the CW TX frequency in Hz.
    // int64_t frequency
    SetCWTxFreq,
    // Set the CW keyer speed in Words per Minute.
    // Limited to <5, 45>
    // uint8_t
    SetCWKeyerSpeed,
    // KeyerMode mode
    // uint8_t
    SetKeyerMode,
    // Delay of the dit sent after dit played, to avoid hot switching of the AMP relay, in microseconds. Maximum time is 15ms.
    // Relay hang after the last dit, in microseconds. Maximum time is 10 seconds.
    // bool enabled, uint32_t delay, uint32_t hang
    SetAMPControl,
    // CW phase & amplitude balance and output power.
    // double phase_balance_deg, double amplitude_balance, double power
    SetIQBalanceAndPower,
};

class Cat {
public:
    Cat() {}
    ~Cat() {}
    bool init(libusb_device_handle *handle);

    const std::string get_error() const { return error; }
    std::string error;
    std::string serialNumber;

    // Set local oscillator frequency in Hz.
    bool set_freq(int64_t frequency);
    // Set the CW TX frequency in Hz.
    bool set_cw_tx_freq(int64_t frequency);
    // Set the CW keyer speed in Words per Minute.
    // Limited to <5, 45>
    bool set_cw_keyer_speed(int wpm);
    bool set_cw_keyer_mode(KeyerMode mode);
    // Delay of the dit sent after dit played, to avoid hot switching of the AMP relay, in microseconds. Maximum time is 15ms.
    // Relay hang after the last dit, in microseconds. Maximum time is 10 seconds.
    bool set_amp_control(bool enabled, int delay, int hang);

    bool setIQBalanceAndPower(double phase_balance_deg, double amplitude_balance, double power);

private:
//    int findPeaberryDevice();
    std::string readUsbString(struct usb_dev_handle *udh, uint8_t iDesc);

    libusb_context            *m_libusb_context = nullptr;
    libusb_device            *m_libusb_device  = nullptr;
    libusb_device_handle    *m_libusb_device_handle = nullptr;

    int64_t freqChangeMark = 0;
    int64_t currentFreq = 0;
    int64_t requestedFreq = 0;
    int64_t freq = 0;
    int64_t xit = 0;
    int64_t rit = 0;
    bool    transmitOK = false;

    void start();
    void stop();
    void setFreq(int64_t f) { freq = f; requestedFreq = f + rit; approveTransmit(); }
    void setXit(int64_t f) { xit = f; approveTransmit(); }
    void setRit(int64_t f) { rit = f; requestedFreq = f + freq; }

private:
    void doWork();
    void approveTransmit();
};

extern Cat g_Cat;

#endif // CAT_H
