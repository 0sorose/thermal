void 7seg4(uint8_t fig[])
{
  uint8_t out[2] = {0,0};
  /*  top pins:     seg1  a    f    seg2  seg3  b     -    -
      bottom pins:  e     d   col   c     g     seg4  -    -  */
out = {128, 0}; // anode for each segment.
out = {16, 0};
out = {8, 0};
out = {0, 4};

}
