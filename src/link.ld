/* $Id: sections 5834 2013-10-08 17:04:08Z os $ */

ENTRY(boot)
OUTPUT_FORMAT(elf32-i386)

SECTIONS
{
  . = 0x100000;     /* Startadresse des Systems */

  .text :
   {
     *(".text")
     *(".text.*")
     *(".text$")
     *(".init")
     *(".fini")
     *(".gnu.linkonce.*")
   }

   .init_array     :
   {
      PROVIDE_HIDDEN (__init_array_start = .);
      KEEP (*(SORT(.init_array.*)))
      KEEP (*(.init_array))
      KEEP (*(".ctors"))
      KEEP (*(".ctor"))
      PROVIDE_HIDDEN (__init_array_end = .);
   }

   .fini_array     :
   {
      PROVIDE_HIDDEN (__fini_array_start = .);
      KEEP (*(SORT(.fini_array.*)))
      KEEP (*(.fini_array))
      KEEP (*(".dtors"))
      KEEP (*(".dtor"))
      PROVIDE_HIDDEN (__fini_array_end = .);
   }

  .data :
   {
     *(".data")
     *(".data$")
     *(".rodata")
     *(".rodata.*")
     *(".got")
     *(".got.plt")
     *(".eh_frame")
     *(".eh_fram")
     *(".jcr")
     *(".note.*")
   }

  .bss :
   {
     ___BSS_START__ = .;
     *(".bss")
     *(".bss.*")
     ___BSS_END__ = .;
   }

/*
  /DISCARD/ :
   {
     *(".note")
     *(".comment")
     *(".debug_line")
     *(".debug_info")
     *(".debug_abbrev")
     *(".debug_aranges")
   }
*/
}
