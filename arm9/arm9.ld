OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)

PHDRS
{
	code   PT_LOAD FLAGS(5) /* Read | Execute */;
	rodata PT_LOAD FLAGS(4) /* Read */;
	data   PT_LOAD FLAGS(6) /* Read | Write */;
}

SECTIONS
{
	/* =========== CODE section =========== */

	PROVIDE(__start__ = 0x08000040);
	. = __start__;

	.text ALIGN(4) :
	{
		/* .init */
		KEEP( *(.crt0) )
		KEEP( *(.init) )
		. = ALIGN(4);

		/* .text */
		*(.text)
		*(.text.*)
		*(.glue_7)
		*(.glue_7t)
		*(.stub)
		*(.gnu.warning)
		*(.gnu.linkonce.t*)
		. = ALIGN(4);

		/* .fini */
		KEEP( *(.fini) )
		. = ALIGN(4);
	} : code = 0xFF

	/* =========== RODATA section =========== */

	. = ALIGN(4);

	.rodata :
	{
		*(.rodata)
		*(.roda)
		*(.rodata.*)
		*all.rodata*(*)
		*(.gnu.linkonce.r*)
		SORT(CONSTRUCTORS)
		. = ALIGN(4);
	} : rodata = 0xFF

	.ARM.extab : { *(.ARM.extab* .gnu.linkonce.armextab.*) } : rodata
	__exidx_start = .;
	ARM.exidx : { *(.ARM.exidx* .gnu.linkonce.armexidx.*) } : rodata
	__exidx_end = .;

	/* =========== DATA section =========== */

	. = ALIGN(4);

	.data :
	{
		*(.data)
		*(.data.*)
		*(.gnu.linkonce.d*)
		CONSTRUCTORS
		. = ALIGN(4);
	} : data = 0xFF

	.tdata ALIGN(4) :
	{
		__tdata_lma = .;
		*(.tdata)
		*(.tdata.*)
		*(.gnu.linkonce.td.*)
		. = ALIGN(4);
		__tdata_lma_end = .;
	} : data = 0xFF

	.tbss ALIGN(4) :
	{
		*(.tbss)
		*(.tbss.*)
		*(.gnu.linkonce.tb.*)
		*(.tcommon)
		. = ALIGN(4);
	} : data = 0xFF

	.preinit_array ALIGN(4) :
	{
		PROVIDE (__preinit_array_start = .);
		KEEP (*(.preinit_array))
		PROVIDE (__preinit_array_end = .);
	} : data = 0xFF

	.init_array ALIGN(4) :
	{
		PROVIDE (__init_array_start = .);
		KEEP (*(SORT(.init_array.*)))
		KEEP (*(.init_array))
		PROVIDE (__init_array_end = .);
	} : data = 0xFF

	.fini_array ALIGN(4) :
	{
		PROVIDE (__fini_array_start = .);
		KEEP (*(.fini_array))
		KEEP (*(SORT(.fini_array.*)))
		PROVIDE (__fini_array_end = .);
	} : data = 0xFF

	.ctors ALIGN(4) :
	{
		KEEP (*crtbegin.o(.ctors)) /* MUST be first -- GCC requires it */
		KEEP (*(EXCLUDE_FILE (*crtend.o) .ctors))
		KEEP (*(SORT(.ctors.*)))
		KEEP (*(.ctors))
	} : data = 0xFF

	.dtors ALIGN(4) :
	{
		KEEP (*crtbegin.o(.dtors))
		KEEP (*(EXCLUDE_FILE (*crtend.o) .dtors))
		KEEP (*(SORT(.dtors.*)))
		KEEP (*(.dtors))
	} : data = 0xFF

	__bss_start__ = .;
	.bss ALIGN(4) (NOLOAD):
	{
		*(.dynbss)
		*(.bss)
		*(.bss.*)
		*(.gnu.linkonce.b*)
		*(COMMON)
		. = ALIGN(4);

		/* Reserve space for the TLS segment of the main thread */
		__tls_start = .;
		. += + SIZEOF(.tdata) + SIZEOF(.tbss);
		__tls_end = .;
	} : data
	__bss_end__ = .;

	__end__ = ABSOLUTE(.) ;

	/* ==================
	   ==== Metadata ====
	   ================== */

	/* Discard sections that difficult post-processing */
	/DISCARD/ : { *(.group .comment .note) }

	/* Stabs debugging sections. */
	.stab          0 : { *(.stab) }
	.stabstr       0 : { *(.stabstr) }
	.stab.excl     0 : { *(.stab.excl) }
	.stab.exclstr  0 : { *(.stab.exclstr) }
	.stab.index    0 : { *(.stab.index) }
	.stab.indexstr 0 : { *(.stab.indexstr) }

	/* DWARF debug sections.
	   Symbols in the DWARF debugging sections are relative to the beginning
	   of the section so we begin them at 0. */

	/* DWARF 1 */
	.debug          0 : { *(.debug) }
	.line           0 : { *(.line) }

	/* GNU DWARF 1 extensions */
	.debug_srcinfo  0 : { *(.debug_srcinfo) }
	.debug_sfnames  0 : { *(.debug_sfnames) }

	/* DWARF 1.1 and DWARF 2 */
	.debug_aranges  0 : { *(.debug_aranges) }
	.debug_pubnames 0 : { *(.debug_pubnames) }

	/* DWARF 2 */
	.debug_info     0 : { *(.debug_info) }
	.debug_abbrev   0 : { *(.debug_abbrev) }
	.debug_line     0 : { *(.debug_line) }
	.debug_frame    0 : { *(.debug_frame) }
	.debug_str      0 : { *(.debug_str) }
	.debug_loc      0 : { *(.debug_loc) }
	.debug_macinfo  0 : { *(.debug_macinfo) }
}
