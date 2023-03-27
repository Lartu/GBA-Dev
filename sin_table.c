//
// Sine Lookup Table
//

#define PI 65536

const uint32 sin_lut[256]=
{
	0x0000, 0x0032, 0x0064, 0x0096, 0x00C8, 0x00FB, 0x012D, 0x015F, 
	0x0191, 0x01C3, 0x01F5, 0x0227, 0x0259, 0x028A, 0x02BC, 0x02ED, 
	0x031F, 0x0350, 0x0381, 0x03B2, 0x03E3, 0x0413, 0x0444, 0x0474, 
	0x04A5, 0x04D5, 0x0504, 0x0534, 0x0563, 0x0593, 0x05C2, 0x05F0, 
	0x061F, 0x064D, 0x067B, 0x06A9, 0x06D7, 0x0704, 0x0731, 0x075E, 
	0x078A, 0x07B7, 0x07E2, 0x080E, 0x0839, 0x0864, 0x088F, 0x08B9, 
	0x08E3, 0x090D, 0x0936, 0x095F, 0x0987, 0x09B0, 0x09D7, 0x09FF, 
	0x0A26, 0x0A4D, 0x0A73, 0x0A99, 0x0ABE, 0x0AE3, 0x0B08, 0x0B2C, 
	0x0B50, 0x0B73, 0x0B96, 0x0BB8, 0x0BDA, 0x0BFC, 0x0C1D, 0x0C3E, 
	0x0C5E, 0x0C7D, 0x0C9D, 0x0CBB, 0x0CD9, 0x0CF7, 0x0D14, 0x0D31, 
	0x0D4D, 0x0D69, 0x0D84, 0x0D9F, 0x0DB9, 0x0DD2, 0x0DEB, 0x0E04, 
	0x0E1C, 0x0E33, 0x0E4A, 0x0E60, 0x0E76, 0x0E8B, 0x0EA0, 0x0EB4, 
	0x0EC8, 0x0EDB, 0x0EED, 0x0EFF, 0x0F10, 0x0F21, 0x0F31, 0x0F40, 
	0x0F4F, 0x0F5D, 0x0F6B, 0x0F78, 0x0F85, 0x0F91, 0x0F9C, 0x0FA7, 
	0x0FB1, 0x0FBA, 0x0FC3, 0x0FCB, 0x0FD3, 0x0FDA, 0x0FE1, 0x0FE7, 
	0x0FEC, 0x0FF0, 0x0FF4, 0x0FF8, 0x0FFB, 0x0FFD, 0x0FFE, 0x0FFF, 
	0x0FFF, 0x0FFF, 0x0FFE, 0x0FFD, 0x0FFB, 0x0FF8, 0x0FF4, 0x0FF0, 
	0x0FEC, 0x0FE7, 0x0FE1, 0x0FDA, 0x0FD3, 0x0FCB, 0x0FC3, 0x0FBA, 
	0x0FB1, 0x0FA7, 0x0F9C, 0x0F91, 0x0F85, 0x0F78, 0x0F6B, 0x0F5D, 
	0x0F4F, 0x0F40, 0x0F31, 0x0F21, 0x0F10, 0x0EFF, 0x0EED, 0x0EDB, 
	0x0EC8, 0x0EB4, 0x0EA0, 0x0E8B, 0x0E76, 0x0E60, 0x0E4A, 0x0E33, 
	0x0E1C, 0x0E04, 0x0DEB, 0x0DD2, 0x0DB9, 0x0D9F, 0x0D84, 0x0D69, 
	0x0D4D, 0x0D31, 0x0D14, 0x0CF7, 0x0CD9, 0x0CBB, 0x0C9D, 0x0C7D, 
	0x0C5E, 0x0C3E, 0x0C1D, 0x0BFC, 0x0BDA, 0x0BB8, 0x0B96, 0x0B73, 
	0x0B50, 0x0B2C, 0x0B08, 0x0AE3, 0x0ABE, 0x0A99, 0x0A73, 0x0A4D, 
	0x0A26, 0x09FF, 0x09D7, 0x09B0, 0x0987, 0x095F, 0x0936, 0x090D, 
	0x08E3, 0x08B9, 0x088F, 0x0864, 0x0839, 0x080E, 0x07E2, 0x07B7, 
	0x078A, 0x075E, 0x0731, 0x0704, 0x06D7, 0x06A9, 0x067B, 0x064D, 
	0x061F, 0x05F0, 0x05C2, 0x0593, 0x0563, 0x0534, 0x0504, 0x04D5, 
	0x04A5, 0x0474, 0x0444, 0x0413, 0x03E3, 0x03B2, 0x0381, 0x0350, 
	0x031F, 0x02ED, 0x02BC, 0x028A, 0x0259, 0x0227, 0x01F5, 0x01C3, 
	0x0191, 0x015F, 0x012D, 0x00FB, 0x00C8, 0x0096, 0x0064, 0x0032, 
};
