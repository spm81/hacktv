FUNCARD EMULATOR FOR SYSTER TERMINAL G1 + G2

EE-PROM-MAPPING

Address			Function

0x01 - 0x08		Answer to reset (02 00)
0x09 - 0x10		Saved Channels for G1 Terminal (4x 2 Byte)
0x11 - 0x18		Answer for 57 00 request
0x19 - 0x20		Answer for 57 01 request
0x21 - 0x28		Answer for 57 02 request

0x29 - 0x30		1st KEY mapped to Trigger @address 0x69 + 0x6A
0x31 - 0x38		2nd KEY mapped to Trigger @address 0x6B + 0x6C
0x39 - 0x40		3rd KEY mapped to Trigger @address 0x6D + 0x6E
0x41 - 0x48		4th KEY mapped to Trigger @address 0x6F + 0x70
0x49 - 0x50		5th KEY mapped to Trigger @address 0x71 + 0x72
0x51 - 0x58		6th KEY mapped to Trigger @address 0x73 + 0x74
0x59 - 0x60		7th KEY mapped to Trigger @address 0x75 + 0x76
0x61 - 0x68		8th KEY mapped to Trigger @address 0x77 + 0x78

0x69 - 0x78		8 x 2 Byte Trigger for Keys 1 - 8
0x79 - 0x81		9 Byte long answer for Info-Key (abo info) 5F 00 00 00
0x82 - 0x8A		9 Byte long answer for Info-Key (abo info) 5F 00 01 00

NEXT FEW BYTES 		copyright info (please do not edit)

you can edit the eeprom with cpluche:

cmd C1 XX yy 00     will write one byte to address XX
cmd C4 XX yy yy yy yy yy yy yy yy will write 8 Bytes (yy) to eeprom address XX where XX is the start address

e.g.: 
C4 09 19 00 1A 00 1B 00 1C 00  will write the saved channels S17, S18, S19 and S20 for tuning the G1 Terminal
C1 01 FF 00 			will write the first ATR-Byte to FF

(to use later)
you have 8 keys on the card... each key have a so called "trigger". The trigger are the first two bytes after 06 00 XX YY.
so if you write the trigger @ address 0x69 and 0x9A to 0x11 and 0x22, the key @ address 0x29 will be sent as answer.
