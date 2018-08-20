/*
 * Author: Daniel Liscinsky
 */



char invalid_byte_strings[][48] = {
	"999999999999999999999999999:hi",
	" 2:hello",
	"1:lots_of_data",
	"5hello",
	"",
	"1",
	"12",
	"2:",
	"0xh:1234567890123456",
};
