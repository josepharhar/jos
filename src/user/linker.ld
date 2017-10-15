ENTRY(main)

SECTIONS {
  . = 0x80000000000;

  .text : {
    *(*)
  }

}
