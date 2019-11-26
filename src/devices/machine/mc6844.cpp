// license:BSD-3-Clause
// copyright-holders: Joakim Larsson Edström
/**********************************************************************

  Motorola 6844 emulation. 

 "MC6844 — Direct Memory Access Controller

  This DMAC works with an M6800 MPU Clock Pulse Generator and an I/O Peripheral Controller,
  such as the units described here, to facilitate  direct access to the computer memory by
  the peripheral, thus by passing MPU interactive time delay.

  General Description

  The MC6844 is operable in three modes: HALT Burst, Cycle Steal and TSC Steal. 
  In the Burst Mode, the MPU is halted by the first transfer request (TxRQ) input and
  is restarted when the Byte Count Register (BCR) is zero. Each data transfer is synchronized 
  by a pulse input of TxRQ. In the Cycle Steal Mode, the MPU is halted by each TxRQ and 
  is restarted after each one byte of data transferred. In the TSC Steal Mode, DMAC uses the 
  three-state control function of the MPU to control the system bus. One byte of data is
  transferred during each DMA cycle.

  The DMAC has four channels. A Priority Control Register determines which of the channels 
  is enabled. While data is being transferred on one channel, the other channels are inhibited.
  When one channel completes transferring, the next will become valid for DMA transfer. The PCR 
  also utilizes a Rotate Control bit. Priority of DMA transfer is normally fixed in sequential 
  order. The highest priority is in #0 Channel and the lowest is in #3. When this bit is in high
  level, channel priority is rotated such that the just-serviced channel has the lowest priority 
  in the next DMA transfer."   

  Source: https://en.wikipedia.org/wiki/File:Motorola_Microcomputer_Components_1978_pg13.jpg

  TODO:

**********************************************************************/

#include "emu.h"
#include "mc6844.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************
#define LOG_SETUP   (1U << 1)
#define LOG_INT     (1U << 2)

#define VERBOSE (LOG_SETUP | LOG_INT)
//#define LOG_OUTPUT_STREAM std::cout

#include "logmacro.h"

#define LOGSETUP(...) LOGMASKED(LOG_SETUP, __VA_ARGS__)
#define LOGINT(...)   LOGMASKED(LOG_INT,   __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

// device type definition
DEFINE_DEVICE_TYPE(MC6844, mc6844_device, "mc6844", "MC6844 DMA")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mc6844_device - constructor
//-------------------------------------------------
mc6844_device::mc6844_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MC6844, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_add_mconfig - device-specific machine
//  configuration addiitons
//-------------------------------------------------
void mc6844_device::device_add_mconfig(machine_config &config)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mc6844_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mc6844_device::device_reset()
{
}

//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

READ8_MEMBER( mc6844_device::read )
{
	return 0;
}

WRITE8_MEMBER( mc6844_device::write )
{
}
