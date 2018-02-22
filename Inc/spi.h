#ifndef __SPI_H__
#define __SPI_H__

#define SPI0_MODE_MASTER 	(0)
#define SPI0_MODE_SLAVE		!(SPI0_MODE_MASTER)

extern uint32_t spi_init(uint32_t mode);
extern void spi_tx(uint8_t* pData, uint32_t len);
#endif /*END_OF_SPI_H_*/
