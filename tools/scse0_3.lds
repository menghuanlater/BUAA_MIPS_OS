OUTPUT_ARCH(mips)
/*
指定输出可执行文件的平台为mips
*/
ENTRY(_start)
/*
指定输出可执行文件的起始代码段为_start
*/
SECTIONS
{

/*To do:
  fill in the correct address of the key section
  such as text data bss ...
*/
	. = 0x80010000;
	.text : {*(.text)}
	.data : {*(.data)}
	.bss : {*(.bss)}
	. = 0x80400000;
end = . ;
}
