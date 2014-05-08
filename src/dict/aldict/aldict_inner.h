#ifndef _ALDICT_INNER_H_
#define _ALDICT_INNER_H_

#include "endian_number.h"

#define ALD_BLOCK	256
#define ALD_INVALID_ADDR 0xFFFFFFFF
#define ALD_ADDR_MAX  0x7FFFFFFF

#define F_LOCSTRINX  0x80000000

#define BP(a, b)	(b-a+1)

#define ALD_MAGIC_L  0x77
#define ALD_MAGIC_H  0x88

#define ALD_LANLEN_MAX 15 /* for src_lan and det_lan */

struct aldict_header {
	u8 magic        [ BP(1, 2)     ];
	u8 h_version  	[ BP(3, 3)     ];
	u8 p_date       [ BP(4, 7)     ];
	u8 p_identi     [ BP(8, 67)    ];
	u8 d_version    [ BP(68, 69)   ];
	u8 d_identi	    [ BP(70, 129)  ];
	u8 d_entries	[ BP(130, 133) ];
	u8 loc_chrindex	[ BP(134, 134) ];
	u8 loc_strindex [ BP(135, 138) ];
	u8 loc_data     [ BP(139, 142) ];
	u8 src_lan      [ BP(143, 157) ];
	u8 det_lan      [ BP(158, 172) ];
};

struct aldict_charindex {
	u8 wchr         [ BP(1, 4) ];
  	u8 location  	[ BP(5, 8) ];
	u8 len_content 	[ BP(9, 10) ];
};

struct aldict_stringindex {
	u8 location     [ BP(1, 4) ];
	u8 len_str      [ BP(5, 5) ];
  	u8 str          [1];
};

struct aldict_dataitem {
    u8  len_word;
	u8  *ptr_word;
    u8  len_phon;
	u8  *ptr_phon;
    u16 len_expl;
    u8  *ptr_expl;
};

#define ALD_BLOCK_NR(pos)  (pos/ALD_BLOCK+1)

#define ald_write_u32	endian_write_u32_le

#define ald_write_u16   endian_write_u16_le

#define ald_read_u32	endian_read_u32_le

#define ald_read_u16	endian_read_u16_le

#endif

