#ifndef _ALDICT_INNER_H_
#define _ALDICT_INNER_H_

#include "endian_number.h"

#define ALD_BLOCK	256

#define ALD_INVALID_ADDR 0xFFFFFFFF

#define BP(a, b)	(b-a+1)

struct aldict_header {
	u8 magic	[ BP(1, 2)     ];
	u8 h_version  	[ BP(3, 3)     ];
	u8 p_date	[ BP(4, 7)     ];
	u8 p_identi	[ BP(8, 67)    ];
	u8 d_version	[ BP(68, 69)   ];
	u8 d_identi	[ BP(70, 129)  ];
	u8 d_entries	[ BP(130, 133) ];
	u8 loc_chrindex	[ BP(134, 137) ];
	u8 loc_strindex [ BP(138, 141) ];
	u8 loc_data	[ BP(142, 145) ];
};

struct aldict_charindex {
	u8 wchr		[ BP(1, 4) ];
  	u8 location  	[ BP(5, 8) ];
	u8 len_content 	[ BP(9, 10) ];
};

struct aldict_stringindex {
	u8 location 	[ BP(1, 4) ];
	u8 len_str	[ BP(5, 5) ];
  	u8 str  	[1];
};

#define ALD_BLOCK_NR(pos)  (pos/ALD_BLOCK+1)

#define ald_write_u32	endian_write_u32_le

#define ald_write_u16   endian_write_u16_le

#define ald_read_u32	endian_read_u32_le

#define ald_read_u16	endian_read_u16_le

#endif

