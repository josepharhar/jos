ENTRY(main)
SECTIONS {
  . = 0x6400000;
  .text : {
    *(*)
  }
}
