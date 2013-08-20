#ifndef __SPI_INIT_H__
#define __SPI_INIT_H__

/*********************************************************************
 * Function:        void	SpiInitDevice(SpiChannel chn, int isMaster, int frmEn, int frmMaster)
 *
 * PreCondition:    None
 *
 * Input:           chn			- the SPI channel to use
 * 					isMaster	-	1: the device is to act as a bus master
 * 									0: the device is an SPI slave
 * 					frmEn		-	1: frame mode is enabled
 * 								-	0: frame mode is disabled
 * 					frmMaster	-	0: if frame mode is enabled, the device is a frame slave (FRMSYNC is input)
 * 									1: if frame mode is enabled, the device is a frame master (FRMSYNC is output)
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Inits the SPI channel 1 to use 16 bit words
 * 					Performs the device initialization in both master/slave modes.
 *
 * Note:            None
 ********************************************************************/
void SpiInitDevice(SpiChannel chn, int isMaster, int frmEn, int frmMaster)
{
	SpiOpenFlags	oFlags=SPI_OPEN_MODE8|SPI_OPEN_SMP_END;	// SPI open mode
	if(isMaster)
	{
		oFlags|=SPI_OPEN_MSTEN;
	}
	if(frmEn)
	{
		oFlags|=SPI_OPEN_FRMEN;
		if(!frmMaster)
		{
			oFlags|=SPI_OPEN_FSP_IN;
		}
	}

	SpiChnOpen(chn, oFlags, 4);	// divide fpb by 4, configure the I/O ports. Not using SS in this example

}

#endif