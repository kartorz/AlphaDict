#ifndef _ENDIAN_NUMBER_H_
#define _ENDIAN_NUNBER_H_

#include "type.h"

#include <endian.h>

inline void endian_write_u32_le(u8 *buf, u32 v32)
{
	for (int i=0; i<4; i++) {
		buf[i] = v32 & 0x000000ff;
		v32 >> 8;
	}
}

inline void endian_write_u16_le(u8 *buf, u16 v16)
{
	buf[0] = v16 & 0x00ff;
	buf[1] = v16 >> 8;
}

inline u32 endian_read_u32_le(const u8 *buf)
{
# if 	__BYTE_ORDER == __LITTLE_ENDIAN
	return *((u32 *)buf);
# else
	return buf[3]<<24 | buf[2]<<16 | buf[1]<<8 | buf[0];
# endif
}

inline u16 endian_read_u16_le(const u8 *buf)
{
# if 	__BYTE_ORDER == __LITTLE_ENDIAN
	return *((u16 *)buf);
# else
	return buf[1]<<8 | buf[0];
# endif
}

inline void endian_write_u32_bg(u8 *buf, u32 v32)
{
	for (int i=0; i<4; i++) {
		buf[3-i] = v32 & 0x000000ff;
		v32 >> 8;
	}
}

inline void endian_write_u16_bg(u8 *buf, u16 v16)
{
	buf[1] = v16 & 0x00ff;
	buf[0] = v16 >> 8;
}

inline u32 endia_read_u32_bg(const u8 *buf)
{
# if	 __BYTE_ORDER == __LITTLE_ENDIAN
	return *((u32 *)buf);
# else
	return buf[0]<<24 | buf[1]<<16 | buf[2]<<8 | buf[3];
# endif
}

inline u16 endian_read_u16_bg(const u8 *buf)
{
# if	__BYTE_ORDER == __LITTLE_ENDIAN
	return *((u16 *)buf);
# else
	return buf[0]<<8 | buf[1];
# endif
}

#endif
