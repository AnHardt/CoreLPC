//Author: sdavi
//Implement the SharedSpi as in RRF

#include "Core.h"
#include "SharedSpi.h"

#include "SoftwareSPI.h"
#include "HardwareSPI.h"

static SoftwareSPI swspi0;


static SPI *selectedSPIDevice = nullptr;
SPI *getSSPDevice(SSPChannel channel) noexcept
{
    switch(channel)
    {
        case SSP0: return &HardwareSPI::SSP0; break;
        case SSP1: return &HardwareSPI::SSP1; break;
        case SWSPI0: return &swspi0; break;
        default: return nullptr;
    }
}

void sspi_setPinsForChannel(SSPChannel channel, Pin sck, Pin miso, Pin mosi, Pin cs) noexcept
{
    switch(channel)
    {
        case SSP0: HardwareSPI::SSP0.InitPins(sck, miso, mosi, cs); break;
        case SSP1: break; // SSP1 pins are fixed
        case SWSPI0: swspi0.InitPins(sck, miso, mosi); break;
        default: break;
    }
}


// Set up the Shared SPI subsystem
void sspi_master_init(struct sspi_device *device, uint32_t bits) noexcept
{
    GPIO_PinFunction(device->csPin, 0); //set pin to GPIO
    pinMode(device->csPin, (device->csPolarity) ? OUTPUT_LOW : OUTPUT_HIGH);

    //Only support 8 or 16bit
    if(bits == 8 || bits == 16){
        device->bitsPerTransferControl = bits;
    } else {
        device->bitsPerTransferControl = 8; // default
    }
}


//setup the master device.
void sspi_master_setup_device(const struct sspi_device *device) noexcept
{
    SPI *spi = getSSPDevice(device->sspChannel);
    if(spi != nullptr)
    {
        spi->setup_device(device);
    }
}


// select device to use the SSP/SPI bus
void sspi_select_device(const struct sspi_device *device) noexcept
{
	// Enable the CS line
    if(device->csPolarity == false)
    {
        digitalWrite(device->csPin, LOW); //active low
    }
    else
    {
        digitalWrite(device->csPin, HIGH);// active high
    }
    
    //since transreceive doesnt pass the device struct, we track it here
    //We are assuming only 1 SPI connection is used at a time
    selectedSPIDevice = getSSPDevice(device->sspChannel);
}

// deselect
void sspi_deselect_device(const struct sspi_device *device) noexcept
{
	// Disable the CS line
    if(device->csPolarity == false)
    {
        digitalWrite(device->csPin, HIGH);
    }
    else
    {
        digitalWrite(device->csPin, LOW);
    }
    selectedSPIDevice = nullptr;
}


spi_status_t sspi_transceive_packet(const uint8_t *tx_data, uint8_t *rx_data, size_t len) noexcept
{
    if(selectedSPIDevice == nullptr) return SPI_ERROR_TIMEOUT;//todo: just return timeout error if null
    return selectedSPIDevice->sspi_transceive_packet(tx_data, rx_data, len);
}

//transceive a packet for SDCard
uint8_t sspi_transceive_a_packet(uint8_t buf) noexcept
{
    uint8_t tx[1], rx[1];
    tx[0] = (uint8_t)buf;
    sspi_transceive_packet(tx, rx, 1);
    return rx[0];
}
