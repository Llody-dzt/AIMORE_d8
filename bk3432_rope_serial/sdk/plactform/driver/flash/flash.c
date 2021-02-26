
#include "flash.h"
#include "string.h"
#include "rf.h"
#include "BK3432_reg.h"


/// Specific to EMBED BEKEN FLASH  (type 1)
#define FLASH_SECT_SIZE1         0x00000200  //  512 Byte
#define FLASH_SECT_SIZE2         0x00001000  //  4 KByte
#define FLASH_BOUNDARY256B       0x00000100  // 256 Byte



/// Flash environment structure
struct flash_env_tag
{
	uint32_t    length[2];

	uint8_t     space_type[2];
};


static struct flash_env_tag flash_env;



static void flash_unlock(void)
{
	// Unlock all sectors
	REG_AHB3_MFC_WP0 = 0xA5;
	REG_AHB3_MFC_WP1 = 0xC3;
}

static void flash_writeKey(void)
{
	// write control key
	REG_AHB3_MFC_KEYWORD = 0x58A9;
	REG_AHB3_MFC_KEYWORD = 0xA958;
}

static void flash_clearKey(void)
{
	// write control key
	REG_AHB3_MFC_KEYWORD = 0x0;
	REG_AHB3_MFC_KEYWORD = 0x0;
}

static void flash_start(void)
{
	REG_AHB3_MFC_CONTROL |= 0x01;
}

static uint8_t flash_status(void)
{
	return REG_AHB3_MFC_CONTROL & 0x01;
}

void flash_init(void)
{
	// Init flash environment
	flash_env.length[0]    =  FLASH_MAIN_SIZE;
	flash_env.space_type[0]  = FLASH_SPACE_TYPE_MAIN;

	flash_env.length[1]    =  FLASH_NVR_SIZE;
	flash_env.space_type[1]  = FLASH_SPACE_TYPE_NVR;

}


uint32_t flash_readword(uint8_t type,uint32_t address)
{
	uint32_t reg;
	uint32_t data;


	REG_AHB3_MFC_ADDR = address;

	flash_writeKey(); // write key
	flash_unlock(); //wipe off WP

	reg = REG_AHB3_MFC_CONTROL;
	reg &= ~((0x07 << 2) | (0x03 << 5)); //clear control mode and space
	reg  |= ( MFC_CONTROL_READ | (type << 5)); // set write mode and control space
	REG_AHB3_MFC_CONTROL = reg;

	flash_start();

	while(flash_status() == 1);

	data = REG_AHB3_MFC_DATA;
	flash_clearKey();
	return data;
}

void flash_writeword(uint8_t type,uint32_t address, uint32_t data)
{
	uint32_t reg;

	REG_AHB3_MFC_ADDR = address;
	/* Write the data */
	REG_AHB3_MFC_DATA = data;
	flash_writeKey(); // write key
	flash_unlock(); //wipe off WP
	reg = REG_AHB3_MFC_CONTROL;
	reg &= ~((0x07 << 2) | (0x03 << 5)); //clear control mode and space
	reg  |= ( MFC_CONTROL_WRITE | (type << 5)); // set write mode and control space
	REG_AHB3_MFC_CONTROL = reg;
	flash_start();
	while(flash_status()) ;
	flash_clearKey();

}



void flash_erase_sector(uint8_t type,uint32_t address)
{

	uint32_t reg;

	REG_AHB3_MFC_ADDR = address;

	flash_writeKey(); // write key
	flash_unlock(); //wipe off WP

	reg = REG_AHB3_MFC_CONTROL;
	reg &= ~((0x07 << 2) | (0x03 << 5)); //clear control mode and space
	reg  |= ( MFC_CONTROL_SECT_ERASE | (type << 5)); // set write mode and control space
	REG_AHB3_MFC_CONTROL = reg;

	flash_start();

	while(flash_status());
	flash_clearKey();
}




void flash_erase(uint8_t flash_type, uint32_t address, uint32_t size)
{
	uint32_t end_address;

	end_address = address + size;

	// Check parameters
	if ( (flash_type > FLASH_SPACE_TYPE_NVR)  )
	{
		return ;
	}

	address = address & ~(FLASH_SECT_SIZE1 - 1);

	while(address < end_address)
	{

		flash_erase_sector(flash_type,address);

		/* Calculate next sector address (below 0x000100) */
		address += FLASH_SECT_SIZE1;

	}

	return ;
}

void flash_write(uint8_t flash_space, uint32_t address, uint32_t length, uint8_t *buffer)
{
	uint32_t data = 0xffffffff;

	uint8_t  cnt;

	//uint8_t 	subcnt;

	//uint8_t *p_data = (uint8_t *)&data;

	//uint8_t offset;

	//uint8_t lenoffset;

	// Check parameters
	if ( (flash_space > FLASH_SPACE_TYPE_NVR) || (( length) > flash_env.length[flash_space]) )
	{
		return ;
	}

	while(length >= 4)
	{
		data = (buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24));

		flash_writeword(flash_space,address , data);

		address += 1;
		buffer += 4;
		length -= 4;
		//bprintf("Data = %x length = %x\r\n",data,length);
	}

	if(length != 0)
	{
		/* write last mutil byte */
		data = 0xffffffff;

		for(cnt = 0; cnt < length; cnt++)
		{
			data &= ~(0xff << (8 * cnt));
			data |= (buffer[cnt] << (8 * cnt )) ;
		}

		flash_writeword(flash_space,address, data);
		//	uart_printf("Data2 = 0x%08x length = %d\r\n",Data,length);
	}

	return ;
}

void flash_read(uint8_t flash_space, uint32_t address, uint32_t length, uint8_t *buffer)
{
	uint32_t data = 0xffffffff;

	//uint8_t offset;

	//uint8_t lenoffset;

	uint8_t cnt;

	//uint8_t subcnt;

	// Check parameters
	if (  length > flash_env.length[flash_space] )
	{
		return ;
	}

	if ( flash_space > FLASH_SPACE_TYPE_NVR )
	{
		return ;
	}


	while(length >= 4)
	{

		data = flash_readword(flash_space,address);
		//bprintf("read data0 = %x\r\n",data);
		memcpy(buffer, &data, 4);
		address += 1;
		buffer += 4;
		length -= 4;
	}

	if(length != 0)
	{

		/* read last byte */
		data = flash_readword(flash_space,address);

		for(cnt = 0; cnt < length; cnt++)
		{
			*buffer++ = ( data >> (cnt  * 8) & 0xFF );
		}

	}

	return ;
}

uint16_t crc16 (uint8_t *pbuff, uint32_t len)
{
	uint32_t i,j;
	uint8_t ds;
	uint16_t crc = 0xffff;
	static uint16_t poly[2]= {0, 0xa001};
	for(j=len; j>0; j--)
	{
		ds = *pbuff++;
		for(i=0; i<8; i++)
		{
			crc = (crc >> 1) ^ poly[(crc ^ ds ) & 1];
			ds = ds >> 1;
		}
	}
	return crc;
}

void write_lmecc_pointq(uint8_t *buff1, uint8_t *buff2)
{
	/*   uint8 temp_buff[60],i;
	   uint16 temp_crc;

	   memcpy(&temp_buff[0],buff1,28);
	   memcpy(&temp_buff[28],buff2,28);

	   flash_erase_sector(0x42000);
	   temp_buff[56]=FLASH_WRITE_ECC_DATA0;
	   temp_buff[57]=FLASH_WRITE_ECC_DATA1;
	   temp_crc=crc16(temp_buff,58);
	   temp_buff[58]=temp_crc&0xff;
	   temp_buff[59]=temp_crc>>8;

	   flash_write_data(temp_buff,0x42000,60);
	*/

}


uint8_t read_lmecc_pointq_status(void)
{

	/*    uint8 temp_buff[60],i;
	    uint16 crc_data=0;

	    flash_read_data(temp_buff,0x42000,60);


	    crc_data=crc16(temp_buff,58);

	    if((temp_buff[56]==FLASH_WRITE_ECC_DATA0) && (temp_buff[57]==FLASH_WRITE_ECC_DATA1)
	        && ((crc_data&0xff)==temp_buff[58])&&((crc_data>>8)==temp_buff[59]) )
	    {
	        return 1;
	    }
	    else
	        return 0;
	 */
	return 0;
}

void read_lmecc_pointq( uint8_t *buff1,uint8_t *buff2 )
{
	/*   uint8 temp_buff[58];

	   flash_read_data(temp_buff,0x42000,58);

	   memcpy(buff1,&temp_buff[0],28);
	   memcpy(buff2,&temp_buff[28],28);
	   */
}




