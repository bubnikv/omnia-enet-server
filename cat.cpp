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

#include "cat.h"

#include <vector>
#include <cfloat>

bool Cat::init(libusb_device_handle *handle)
{
    setFreq(33333333); // Default I/Q ordering

    m_libusb_device_handle = handle;
    
#if 0
    int qtyfound = findPeaberryDevice();

    if (qtyfound == 0) {
        error = "No radio hardware found. "
            "Make sure the Peaberry SDR is connected, powered, "
            "and you have installed the libusb-win32 "
            "device driver.";
        return false;
    }

    if (qtyfound > 1) {
        error = "Found more than one Peaberry SDR. "
            "Unplug all radios except the one you want to use.";
        return false;
    }
#endif

    return error.empty();
}

#if 0
int Cat::findPeaberryDevice()
{
    if (m_libusb_context == nullptr) {
        int err = libusb_init(&m_libusb_context);
        if (err < 0) {
            error = "Failed to init libusb"; // rc = %i\n", err;
            return -1;
        }
#if 0
        err = libusb_set_option(m_libusb_context, LIBUSB_OPTION_USE_USBDK);
        if (err < 0) {
            error = "Failed to set USBDK backend"; // rc = %i\n", err;
            return -1;
        }
#endif
    }

    libusb_device **devs = nullptr;
    ssize_t            cnt  = libusb_get_device_list(m_libusb_context, &devs);
    if (cnt < 0) {
        error = "Failed to get a device list"; // rc = %i\n", cnt;
        return -1;
    }

    libusb_device  *dev         = devs[0];
    int                qtyfound = 0;
    for (int i = 0; dev != nullptr; dev = devs[i++]) {
        struct libusb_device_descriptor desc;
        int err = libusb_get_device_descriptor(dev, &desc);
        if (err < 0) {
            error = "Failed to get a device descriptor"; // rc = %i\n", err;
            libusb_free_device_list(devs, 1);
            return -1;
        }
        if (desc.idProduct != 0 &&
            desc.idVendor  == USBDEV_SHARED_VENDOR  &&   // VOTI  VID
            desc.idProduct == USBDEV_SHARED_PRODUCT) {  // OBDEV PID
            libusb_device_handle *dev_handle = nullptr;
            if (libusb_open(dev, &dev_handle) == 0) {
                unsigned char buffer[2048];
#if 0
                libusb_get_string_descriptor_ascii(dev_handle, desc.iManufacturer, buffer, sizeof(buffer));
                libusb_get_descriptor(dev_handle, LIBUSB_DT_HID, 0, buffer, sizeof(buffer));
                libusb_get_descriptor(dev_handle, LIBUSB_DT_REPORT, 0, buffer, sizeof(buffer));
#endif
                libusb_get_string_descriptor_ascii(dev_handle, desc.iManufacturer, buffer, sizeof(buffer));
                if (strcmp((const char*)buffer, VENDOR_NAME_OBDEV) == 0) {
                    libusb_get_string_descriptor_ascii(dev_handle, desc.iProduct, buffer, sizeof(buffer));
                    if (strcmp((const char*)buffer, "Peaberry SDR") == 0) {
                        if (qtyfound == 0) {
                            buffer[0] = 0;
                            libusb_get_string_descriptor_ascii(dev_handle, desc.iSerialNumber, buffer, sizeof(buffer));
                            this->serialNumber = (const char*)buffer;
                            m_libusb_device = dev;
                            m_libusb_device_handle = dev_handle;
                            dev_handle = nullptr;
                            ++qtyfound;
                        }
                        else {
#if 0
                            libusb_close(m_libusb_device_handle);
                            m_libusb_device_handle = nullptr;
                            ++qtyfound;
#endif
                        }
                    }
                }
            }
            if (dev_handle != nullptr)
                libusb_close(dev_handle);
        }
    }

    return qtyfound;
}
#endif

inline void setLongWord(uint32_t value, char *bytes)
{
    bytes[0] = value & 0xff;
    bytes[1] = ((value & 0xff00) >> 8) & 0xff;
    bytes[2] = ((value & 0xff0000) >> 16) & 0xff;
    bytes[3] = ((value & 0xff000000) >> 24) & 0xff;
}

bool Cat::set_freq(int64_t frequency)
{
    // PE0FKO, Command 0x32:
    // -------------
    // Set the oscillator frequency by value. The frequency is formatted in MHz
    // as 11.21 bits value.
    // The "automatic band pass filter selection", "smooth tune", "one side calibration" and
    // the "frequency subtract multiply" are all done in this function. (if enabled in the firmware)
    char   buffer[4];
    setLongWord(uint32_t(floor((double(frequency) * 4. * 2.097152 + 0.5))), buffer);  //   2097152=2^21
    int retval = libusb_control_transfer(m_libusb_device_handle, 
        LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_OUT,
        0x32 /* REQUEST_SET_FREQ_BY_VALUE */, 0x700 + 0x55, 0,
        (unsigned char*)buffer, sizeof(buffer), 500);
    if (retval < 0) 
	printf("Cat::setfreq error %s\n", libusb_error_name(retval));
    return retval == 4;
}

bool Cat::set_cw_tx_freq(int64_t frequency)
{
    // OK1IAK, Command 0x60:
    // -------------
    // Set the oscillator frequency by value. The frequency is formatted in MHz
    // as 11.21 bits value.
    // The "automatic band pass filter selection", "smooth tune", "one side calibration" and
    // the "frequency subtract multiply" are all done in this function. (if enabled in the firmware)
    char   buffer[4];
    setLongWord(uint32_t(floor((double(frequency) * 4. * 2.097152 + 0.5))), buffer);  //   2097152=2^21
    int retval = libusb_control_transfer(m_libusb_device_handle,
        LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_OUT,
        0x60 /* REQUEST_SET_CW_TX_FREQ */, 0x700 + 0x55, 0,
        (unsigned char*)buffer, sizeof(buffer), 500);
    return retval == 4;
}

bool Cat::set_cw_keyer_speed(int wpm)
{
    // OK1IAK, Command 0x65:
    // Set keyer speed, in ms per dot.
    if (wpm < 5)
        wpm = 5;
    else if (wpm > 45)
        wpm = 45;
    unsigned char ms_per_dot = (unsigned char)(60000.f / (float(wpm) * 50.f) + 0.5f);
    int retval = libusb_control_transfer(m_libusb_device_handle,
        LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_OUT,
        0x65 /* REQUEST_SET_CW_KEYER_SPEED */, 0x700 + 0x55, 0,
        &ms_per_dot, 1, 500);
    return retval == 1;
}

bool Cat::set_cw_keyer_mode(KeyerMode keyer_mode)
{
    // OK1IAK, Command 0x66:
    // Set keyer mode.
    unsigned char umode = 0x80;
    switch (keyer_mode) {
    case KEYER_MODE_SK:            umode += IAMBIC_SKEY;    break;
    case KEYER_MODE_IAMBIC_A:                            break;
    case KEYER_MODE_IAMBIC_B:    umode += IAMBIC_MODE_B; break;
    }
    int retval = libusb_control_transfer(m_libusb_device_handle,
        LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_OUT,
        0x66 /* REQUEST_SET_CW_KEYER_MODE */, 0x700 + 0x55, 0,
        &umode, 1, 500);
    return retval == 1;
}

// Delay of the dit sent after dit played, to avoid hot switching of the AMP relay, in microseconds. Maximum time is 15ms.
// Relay hang after the last dit, in microseconds. Maximum time is 10 seconds.
bool Cat::set_amp_control(bool enabled, int delay, int hang)
{
    // OK1IAK, Command 0x67: CMD_SET_AMP_SEQUENCING
    // Convert delay value to 0.5ms time intervals.
    if (delay < 0 || ! enabled)
        delay = 0;
    else if (delay > 15000)
        delay = 15000;
    delay = (delay + 250) / 500;
    // Convert hang value to 0.5ms time intervals.
    if (hang < 0 || !enabled)
        hang = 0;
    else if (hang > 10000000)
        hang = 10000000;
    hang = (hang + 250) / 500;
    // Form the packet.
    unsigned char buffer[4];
    buffer[0] = enabled;
    buffer[1] = delay;
    buffer[2] = hang >> 8;
    buffer[3] = hang & 0x0ff;
    int retval = libusb_control_transfer(m_libusb_device_handle,
        LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_OUT,
        0x67 /* CMD_SET_AMP_SEQUENCING */, 0x700 + 0x55, 0,
        (unsigned char*)buffer, sizeof(buffer), 500);
    return retval == 4;
}

bool Cat::setIQBalanceAndPower(double phase_balance_deg, double amplitude_balance, double power)
{
    // Allocate 9ms of 96x IQ samples. This buffer represents 4ms of raise, 1ms of steady and 4ms of fall.
    std::vector<int16_t> buffer(96 * 2 * 9, 0);

    // Generate the IQ waveforms.
    const double        freq_kHz = 1.;
    const double        phase_balance = phase_balance_deg * M_PI / 180.;
    const double        i_amp_corr = (amplitude_balance > 1.) ?  1.                         : amplitude_balance;
    const double        q_amp_corr = (amplitude_balance > 1.) ? (1. / amplitude_balance) : 1.;
    for (size_t i = 0; i < 96 * 9; ++i) {
        // Time, from 0ms to 9ms.
        double seq = (0.5 + double(i)) / double(96);
        // Amplitude shape, shaped by the Student aka Error function.
        double shape = 1.;
        if (i < 96 * 4)
            shape = erf(seq - 2.) * 0.5 + 0.5;
        else if (i >= 96 * 5)
            shape = erf((9. - seq) - 2.) * 0.5 + 0.5;
        buffer[i * 2    ] = int16_t(floor(32767. * power * shape * i_amp_corr * sin(2. * M_PI * freq_kHz * seq + phase_balance) + 0.5));
        buffer[i * 2 + 1] = int16_t(floor(32767. * power * shape * q_amp_corr * cos(2. * M_PI * freq_kHz * seq                ) + 0.5));
    }

    // OK1IAK, Command 0x69: CMD_SET_CW_IQ_WAVEFORM
    uint16_t  len = buffer.size() * sizeof(int16_t);
    // Swap endianness, the PSoC3 Keil compiler works with big endian.
    // Use char* to avoid compiler aliasing.
    char     *data = (char*)buffer.data();
    for (size_t i = 0; i < len; i += 2)
        std::swap(data[i], data[i + 1]);
    int retval = libusb_control_transfer(m_libusb_device_handle,
        LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_OUT,
        0x69 /* CMD_SET_CW_IQ_WAVEFORM */, 0x700 + 0x55, 0,
        (unsigned char*)data, len, 500);
    return retval == len;
}

/*
void Cat::start()
{
    udh = usb_open(dev);
    // The libusb-win32 docs say this is necessary.
    // On OSX it will cause the audio device to momentatily vanish.
    usb_set_configuration(udh, dev->config->bConfigurationValue);
    timer = new QTimer(this);
    timer->setInterval(0);
    connect(timer, SIGNAL(timeout()), this, SLOT(doWork()));
    timer->start();
    etimer = new QElapsedTimer();
    etimer->start();
}

void Cat::stop()
{
    // Ensure we stop transmitting.
    libusb_control_transfer(m_libusb_device_handle,
        LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_IN,
        0x50, 0, 0, nullptr, 0, 10);
    libusb_close(m_libusb_device_handle);
}

void Cat::doWork()
{
    const int TIMEOUT = 3;
    union {
        uint32_t freq;
        uint8_t  key;
    } buffer;
    int ret, request;
    int64_t mark;

    mark = etimer->nsecsElapsed();

    request = 0;

    int ret = libusb_control_transfer(m_libusb_device_handle,
        LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_IN,
        0x50, request, 0, (char*)&buffer, sizeof(buffer), TIMEOUT);
    if (ret != 1) {
        static bool waswarned = false;
        if (! waswarned) {
            waswarned = true;
            // qWarning() << "cat TIMEOUT too short";
        }
        // Sleep keeps from consuming all CPU when radio unplugged.
        // QThread::msleep(1);
    }
    else {
        // keyer->keyUpdate(!(buffer.key & 0x20), !(buffer.key & 0x02));
    }


    if (currentFreq != requestedFreq && mark > freqChangeMark) {
        buffer.freq = (double)(requestedFreq + 24000) / 1000000 * (1UL << 23);
        ret = libusb_control_transfer(m_libusb_device_handle,
            LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_OUT,
            0x32, 0, 0, (char*)&buffer, sizeof(buffer), TIMEOUT);
        if (ret == 4) {
            freqChangeMark = mark + 10000000;
            currentFreq = requestedFreq;
        }
    }

}
*/


// Disable transmit outside ham bands
// Note 30m is 10.1-10.1573 for ITU 3
void Cat::approveTransmit()
{
    static int64_t bands[] = {
        1800000, 2000000,
        3500000, 4000000,
        5000000, 5500000,
        7000000, 7300000,
        10100000, 10157300,
        14000000, 14350000,
        18068000, 18168000,
        21000000, 21450000,
        24890000, 24990000,
        28000000, 29700000
    };
    int64_t f = freq + xit;
    size_t n = sizeof(bands) / sizeof(bands[0]);
    size_t i = 0;
    for (; i < n; i += 2)
        if (f >= bands[i] && f <= bands[i + 1])
            break;
    transmitOK = i < n;
}

Cat    g_Cat;
